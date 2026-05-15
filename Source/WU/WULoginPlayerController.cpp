// Copyright Epic Games, Inc. All Rights Reserved.

#include "WULoginPlayerController.h"
#include "CharacterCreation/WUCharacterCreatorPreviewActor.h"
#include "CharacterCreation/WUCharacterCreatorStage.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/Scene.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineUtils.h"
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
		ApplyCharacterCreatorStage(Request.Race);
		PreviewActor->ApplyCreateRequest(Request);
		PreviewActor->PrestreamTextures(CharacterCreatorPreviewTextureStreamSeconds, true);
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
		UGameplayStatics::OpenLevel(
			this,
			FName(TEXT("/Game/ThirdPerson/Lvl_MagicalBritain_Persistent")),
			true,
			TEXT("game=/Game/Blueprints/Core/BP_WUBaseGameMode.BP_WUBaseGameMode_C"));
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
		const int32 RenderTargetWidth = FMath::Max(CharacterCreatorPreviewRenderTargetSize.X, 128);
		const int32 RenderTargetHeight = FMath::Max(CharacterCreatorPreviewRenderTargetSize.Y, 128);

		CharacterCreatorPreviewRenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("CharacterCreatorPreviewRenderTarget"));
		CharacterCreatorPreviewRenderTarget->ClearColor = FLinearColor(0.015f, 0.012f, 0.01f, 1.0f);
		CharacterCreatorPreviewRenderTarget->InitAutoFormat(RenderTargetWidth, RenderTargetHeight);
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
		CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
		CaptureComponent->bCaptureEveryFrame = true;
		CaptureComponent->bCaptureOnMovement = true;
		CaptureComponent->bAlwaysPersistRenderingState = true;
		CaptureComponent->LODDistanceFactor = 0.5f;
		CaptureComponent->FOVAngle = 42.0f;
		CaptureComponent->PostProcessBlendWeight = 1.0f;
		CaptureComponent->PostProcessSettings.bOverride_Sharpen = true;
		CaptureComponent->PostProcessSettings.Sharpen = CharacterCreatorPreviewSharpen;
		CaptureComponent->ShowFlags.SetAntiAliasing(true);
		CaptureComponent->ShowFlags.SetTemporalAA(true);
		CaptureComponent->ShowFlags.SetScreenPercentage(true);
		CaptureComponent->ShowFlags.SetLighting(true);
		CaptureComponent->ShowFlags.SetFog(true);
		CaptureComponent->ShowFlags.SetAtmosphere(true);
		CaptureComponent->ShowFlags.SetPostProcessing(true);
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
		ResolveCharacterCreatorSpawnTransform(ActiveCharacterCreatorPreviewRace),
		SpawnParameters);

	if (CharacterCreatorPreviewActor && CharacterCreatorSceneCaptureActor)
	{
		USceneCaptureComponent2D* CaptureComponent = CharacterCreatorSceneCaptureActor->GetCaptureComponent2D();
		CaptureComponent->ShowOnlyActors.Reset();
	}

	return CharacterCreatorPreviewActor;
}

const AWUCharacterCreatorStage* AWULoginPlayerController::FindCharacterCreatorStage(EWUCharacterRace Race) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AWUCharacterCreatorStage> It(World); It; ++It)
	{
		const AWUCharacterCreatorStage* Stage = *It;
		if (Stage && Stage->BloodStatus == Race)
		{
			return Stage;
		}
	}

	return nullptr;
}

FTransform AWULoginPlayerController::ResolveCharacterCreatorSpawnTransform(EWUCharacterRace Race) const
{
	if (const AWUCharacterCreatorStage* Stage = FindCharacterCreatorStage(Race))
	{
		return Stage->GetCharacterSpawnTransform();
	}

	return FTransform(CharacterCreatorPreviewStageRotation, CharacterCreatorPreviewStageLocation);
}

void AWULoginPlayerController::ApplyCharacterCreatorStage(EWUCharacterRace Race, bool bForce)
{
	const bool bStageChanged = !bHasActiveCharacterCreatorPreviewRace || ActiveCharacterCreatorPreviewRace != Race;
	ActiveCharacterCreatorPreviewRace = Race;
	bHasActiveCharacterCreatorPreviewRace = true;

	if (!CharacterCreatorPreviewActor || (!bForce && !bStageChanged))
	{
		return;
	}

	CharacterCreatorPreviewActor->SetActorTransform(ResolveCharacterCreatorSpawnTransform(Race));
}

void AWULoginPlayerController::PositionCharacterCreatorPreviewCapture()
{
	if (!CharacterCreatorSceneCaptureActor)
	{
		return;
	}

	FVector CameraLocation;
	FVector LookAtLocation;

	if (const AWUCharacterCreatorStage* Stage = FindCharacterCreatorStage(ActiveCharacterCreatorPreviewRace))
	{
		CameraLocation = Stage->GetCameraLocation();
		LookAtLocation = Stage->GetLookAtLocation();
	}
	else
	{
		const FVector PreviewLocation = CharacterCreatorPreviewActor
			? CharacterCreatorPreviewActor->GetActorLocation()
			: CharacterCreatorPreviewStageLocation;
		CameraLocation = PreviewLocation + CharacterCreatorPreviewCameraOffset;
		LookAtLocation = PreviewLocation + CharacterCreatorPreviewLookAtOffset;
	}

	CharacterCreatorSceneCaptureActor->SetActorLocation(CameraLocation);
	CharacterCreatorSceneCaptureActor->SetActorRotation((LookAtLocation - CameraLocation).Rotation());
}
