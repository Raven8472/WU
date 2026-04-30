// Copyright Epic Games, Inc. All Rights Reserved.

#include "WULoginPlayerController.h"
#include "CharacterCreation/WUCharacterCreatorPreviewActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "UI/WUCharacterSelectWidget.h"
#include "UI/WULoginScreenWidget.h"
#include "WULoginGameMode.h"

AWULoginPlayerController::AWULoginPlayerController()
{
	LoginWidgetClass = UWULoginScreenWidget::StaticClass();
	CharacterSelectWidgetClass = UWUCharacterSelectWidget::StaticClass();
	CharacterCreatorPreviewActorClass = AWUCharacterCreatorPreviewActor::StaticClass();
}

void AWULoginPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ApplyMenuInputMode();
	SetupCharacterCreatorPreviewRig();

	if (IsLocalPlayerController() && LoginWidgetClass)
	{
		LoginWidget = CreateWidget<UWULoginScreenWidget>(this, LoginWidgetClass);
		if (LoginWidget)
		{
			LoginWidget->OnLoginFlowReady.AddDynamic(this, &AWULoginPlayerController::ShowCharacterSelect);
			LoginWidget->AddToPlayerScreen(10);
		}
	}
}

void AWULoginPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CharacterCreatorSceneCaptureActor)
	{
		CharacterCreatorSceneCaptureActor->Destroy();
		CharacterCreatorSceneCaptureActor = nullptr;
	}

	if (CharacterCreatorPreviewActor)
	{
		CharacterCreatorPreviewActor->Destroy();
		CharacterCreatorPreviewActor = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AWULoginPlayerController::PreviewCharacterCreateRequest(const FWUCharacterCreateRequest& Request)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (AWUCharacterCreatorPreviewActor* PreviewActor = EnsureCharacterCreatorPreviewActor())
	{
		PreviewActor->ApplyCreateRequest(Request);
		PreviewActor->SetActorHiddenInGame(false);
		PositionCharacterCreatorPreviewCapture();

		if (CharacterCreatorSceneCaptureActor)
		{
			CharacterCreatorSceneCaptureActor->GetCaptureComponent2D()->CaptureScene();
		}
	}
}

void AWULoginPlayerController::RotateCharacterCreatorPreview(float YawDelta)
{
	if (CharacterCreatorPreviewActor)
	{
		CharacterCreatorPreviewActor->RotatePreview(YawDelta);

		if (CharacterCreatorSceneCaptureActor)
		{
			CharacterCreatorSceneCaptureActor->GetCaptureComponent2D()->CaptureScene();
		}
	}
}

UTextureRenderTarget2D* AWULoginPlayerController::GetCharacterCreatorPreviewRenderTarget() const
{
	return CharacterCreatorPreviewRenderTarget;
}

void AWULoginPlayerController::ShowCharacterSelect()
{
	if (!IsLocalPlayerController() || !CharacterSelectWidgetClass)
	{
		return;
	}

	if (LoginWidget)
	{
		LoginWidget->RemoveFromParent();
		LoginWidget = nullptr;
	}

	if (!CharacterSelectWidget)
	{
		CharacterSelectWidget = CreateWidget<UWUCharacterSelectWidget>(this, CharacterSelectWidgetClass);
		if (CharacterSelectWidget)
		{
			CharacterSelectWidget->OnEnterGameRequested.AddDynamic(this, &AWULoginPlayerController::EnterGame);
		}
	}

	if (CharacterSelectWidget && !CharacterSelectWidget->IsInViewport())
	{
		CharacterSelectWidget->AddToPlayerScreen(10);
	}

	ApplyMenuInputMode();
}

void AWULoginPlayerController::EnterGame()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (GetNetMode() == NM_Standalone)
	{
		RemoveMenuWidgets();
		UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/ThirdPerson/Lvl_WU_Prototype")));
		return;
	}

	if (CharacterSelectWidget)
	{
		CharacterSelectWidget->SetStatusText(FText::FromString(TEXT("Waiting for other players...")));
	}

	if (HasAuthority())
	{
		RequestEnterGameOnServer();
	}
	else
	{
		ServerRequestEnterGame();
	}
}

void AWULoginPlayerController::ServerRequestEnterGame_Implementation()
{
	RequestEnterGameOnServer();
}

void AWULoginPlayerController::RequestEnterGameOnServer()
{
	if (UWorld* World = GetWorld())
	{
		if (AWULoginGameMode* LoginGameMode = World->GetAuthGameMode<AWULoginGameMode>())
		{
			LoginGameMode->MarkPlayerReadyToEnterGame(this);
		}
	}
}

void AWULoginPlayerController::RemoveMenuWidgets()
{
	if (LoginWidget)
	{
		LoginWidget->RemoveFromParent();
		LoginWidget = nullptr;
	}

	if (CharacterSelectWidget)
	{
		CharacterSelectWidget->RemoveFromParent();
		CharacterSelectWidget = nullptr;
	}
}

void AWULoginPlayerController::ApplyMenuInputMode()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AWULoginPlayerController::SetupCharacterCreatorPreviewRig()
{
	if (!IsLocalPlayerController() || !GetWorld())
	{
		return;
	}

	if (!CharacterCreatorPreviewRenderTarget)
	{
		CharacterCreatorPreviewRenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("CharacterCreatorPreviewRenderTarget"));
		CharacterCreatorPreviewRenderTarget->ClearColor = FLinearColor(0.015f, 0.012f, 0.01f, 1.0f);
		CharacterCreatorPreviewRenderTarget->InitAutoFormat(512, 768);
		CharacterCreatorPreviewRenderTarget->UpdateResourceImmediate(true);
	}

	if (!CharacterCreatorSceneCaptureActor)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CharacterCreatorSceneCaptureActor = GetWorld()->SpawnActor<ASceneCapture2D>(
			ASceneCapture2D::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParameters);
	}

	if (CharacterCreatorSceneCaptureActor)
	{
		USceneCaptureComponent2D* CaptureComponent = CharacterCreatorSceneCaptureActor->GetCaptureComponent2D();
		CaptureComponent->TextureTarget = CharacterCreatorPreviewRenderTarget;
		CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		CaptureComponent->bCaptureEveryFrame = true;
		CaptureComponent->bCaptureOnMovement = true;
		CaptureComponent->FOVAngle = 32.0f;
		CaptureComponent->ShowFlags.SetLighting(false);
		CaptureComponent->ShowFlags.SetFog(false);
		CaptureComponent->ShowFlags.SetAtmosphere(false);
		CaptureComponent->ShowFlags.SetPostProcessing(false);
		PositionCharacterCreatorPreviewCapture();
	}
}

AWUCharacterCreatorPreviewActor* AWULoginPlayerController::EnsureCharacterCreatorPreviewActor()
{
	if (CharacterCreatorPreviewActor || !GetWorld() || !CharacterCreatorPreviewActorClass)
	{
		return CharacterCreatorPreviewActor;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CharacterCreatorPreviewActor = GetWorld()->SpawnActor<AWUCharacterCreatorPreviewActor>(
		CharacterCreatorPreviewActorClass,
		FVector(100000.0f, 100000.0f, 100.0f),
		FRotator(0.0f, 180.0f, 0.0f),
		SpawnParameters);

	if (CharacterCreatorPreviewActor && CharacterCreatorSceneCaptureActor)
	{
		USceneCaptureComponent2D* CaptureComponent = CharacterCreatorSceneCaptureActor->GetCaptureComponent2D();
		CaptureComponent->ShowOnlyActors.Reset();
		CaptureComponent->ShowOnlyActorComponents(CharacterCreatorPreviewActor);
	}

	return CharacterCreatorPreviewActor;
}

void AWULoginPlayerController::PositionCharacterCreatorPreviewCapture()
{
	if (!CharacterCreatorSceneCaptureActor)
	{
		return;
	}

	const FVector PreviewLocation = FVector(100000.0f, 100000.0f, 100.0f);
	const FVector CameraLocation = PreviewLocation + FVector(260.0f, -530.0f, 150.0f);
	const FVector LookAtLocation = PreviewLocation + FVector(0.0f, 0.0f, 92.0f);

	CharacterCreatorSceneCaptureActor->SetActorLocation(CameraLocation);
	CharacterCreatorSceneCaptureActor->SetActorRotation((LookAtLocation - CameraLocation).Rotation());
}
