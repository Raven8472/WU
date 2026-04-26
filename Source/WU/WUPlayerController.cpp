// Copyright Epic Games, Inc. All Rights Reserved.


#include "WUPlayerController.h"
#include "Backend/WUClientSessionSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/Widget.h"
#include "InputCoreTypes.h"
#include "CharacterCreation/WUCharacterCreatorPreviewActor.h"
#include "WUCharacter.h"
#include "UI/WUCharacterCreatorWidget.h"
#include "UI/WUChatWidget.h"
#include "UI/WUInventoryWidget.h"
#include "UI/WUPlayerFrameWidget.h"
#include "UI/WUTargetFrameWidget.h"
#include "WU.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Input/SVirtualJoystick.h"

namespace
{
	void ApplyViewportUnitFrameSlot(UUserWidget* Widget, const FVector2D& Size, const FVector2D& Position, const FAnchors& Anchors, const FVector2D& Alignment)
	{
		if (!Widget)
		{
			return;
		}

		Widget->SetDesiredSizeInViewport(Size);
		Widget->SetPositionInViewport(Position, false);
		Widget->SetAnchorsInViewport(Anchors);
		Widget->SetAlignmentInViewport(Alignment);
	}
}

AWUPlayerController::AWUPlayerController()
{
	CharacterCreatorWidgetClass = UWUCharacterCreatorWidget::StaticClass();
	CharacterCreatorPreviewActorClass = AWUCharacterCreatorPreviewActor::StaticClass();
	ChatWidgetClass = UWUChatWidget::StaticClass();
	InventoryWidgetClass = UWUInventoryWidget::StaticClass();
	PlayerFrameWidgetClass = UWUPlayerFrameWidget::StaticClass();
	TargetFrameWidgetClass = UWUTargetFrameWidget::StaticClass();
}

void AWUPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWUPlayerController, CurrentTarget);
}

void AWUPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController() && PlayerHUDWidgetClass)
	{
		PlayerHUDWidget = CreateWidget<UUserWidget>(this, PlayerHUDWidgetClass);

		if (PlayerHUDWidget)
		{
			PlayerHUDWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			PlayerHUDWidget->AddToPlayerScreen(5);
		}
	}

	if (!PlayerFrameWidgetClass)
	{
		PlayerFrameWidgetClass = UWUPlayerFrameWidget::StaticClass();
	}

	if (IsLocalPlayerController() && PlayerFrameWidgetClass)
	{
		PlayerFrameWidget = CreateWidget<UWUPlayerFrameWidget>(this, PlayerFrameWidgetClass);

		if (PlayerFrameWidget)
		{
			PlayerFrameWidget->AddToPlayerScreen(6);
			ApplyViewportUnitFrameSlot(PlayerFrameWidget, PlayerFrameViewportSize, PlayerFrameViewportPosition, FAnchors(0.5f, 1.0f), FVector2D(1.0f, 1.0f));

			if (bHideLegacyPlayerFrameWidget && PlayerHUDWidget)
			{
				if (UWidget* LegacyPlayerFrame = PlayerHUDWidget->GetWidgetFromName(TEXT("WBP_HUD_PlayerFrame")))
				{
					LegacyPlayerFrame->SetVisibility(ESlateVisibility::Collapsed);
				}
			}
		}
	}

	if (!ChatWidgetClass)
	{
		ChatWidgetClass = UWUChatWidget::StaticClass();
	}

	if (IsLocalPlayerController() && ChatWidgetClass)
	{
		ChatWidget = CreateWidget<UWUChatWidget>(this, ChatWidgetClass);

		if (ChatWidget)
		{
			ChatWidget->AddToPlayerScreen(7);
			ApplyViewportUnitFrameSlot(ChatWidget, ChatViewportSize, ChatViewportPosition, FAnchors(0.0f, 1.0f), FVector2D(0.0f, 1.0f));
		}
	}

	if (!InventoryWidgetClass)
	{
		InventoryWidgetClass = UWUInventoryWidget::StaticClass();
	}

	if (IsLocalPlayerController() && InventoryWidgetClass)
	{
		InventoryWidget = CreateWidget<UWUInventoryWidget>(this, InventoryWidgetClass);

		if (InventoryWidget)
		{
			InventoryWidget->AddToPlayerScreen(8);
			ApplyViewportUnitFrameSlot(InventoryWidget, InventoryViewportSize, InventoryViewportPosition, FAnchors(1.0f, 1.0f), FVector2D(1.0f, 1.0f));
		}
	}

	if (!CharacterCreatorWidgetClass)
	{
		CharacterCreatorWidgetClass = UWUCharacterCreatorWidget::StaticClass();
	}

	if (!CharacterCreatorPreviewActorClass)
	{
		CharacterCreatorPreviewActorClass = AWUCharacterCreatorPreviewActor::StaticClass();
	}

	if (IsLocalPlayerController() && CharacterCreatorWidgetClass)
	{
		CharacterCreatorWidget = CreateWidget<UWUCharacterCreatorWidget>(this, CharacterCreatorWidgetClass);

		if (CharacterCreatorWidget)
		{
			CharacterCreatorWidget->AddToPlayerScreen(9);
			ApplyViewportUnitFrameSlot(CharacterCreatorWidget, CharacterCreatorViewportSize, CharacterCreatorViewportPosition, FAnchors(0.0f, 0.0f), FVector2D(0.0f, 0.0f));
		}
	}

	if (!TargetFrameWidgetClass)
	{
		TargetFrameWidgetClass = UWUTargetFrameWidget::StaticClass();
	}

	if (IsLocalPlayerController() && TargetFrameWidgetClass)
	{
		TargetFrameWidget = CreateWidget<UWUTargetFrameWidget>(this, TargetFrameWidgetClass);

		if (TargetFrameWidget)
		{
			TargetFrameWidget->AddToPlayerScreen(6);
			ApplyViewportUnitFrameSlot(TargetFrameWidget, TargetFrameViewportSize, TargetFrameViewportPosition, FAnchors(0.5f, 1.0f), FVector2D(0.0f, 1.0f));
			ShowTargetingDebugMessage(TEXT("Target frame spawned"), FColor::Green);
		}
	}

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogWU, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}

	if (IsLocalPlayerController())
	{
		ApplySelectedCharacterSessionContext();
		ApplyGameplayInputMode();
		ShowTargetingDebugMessage(TEXT("WU targeting ready"));

		if (GetWorld())
		{
			GetWorldTimerManager().SetTimer(
				CharacterLocationSaveTimerHandle,
				this,
				&AWUPlayerController::SaveSelectedCharacterLocation,
				10.0f,
				true);
		}
	}
}

void AWUPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLocalPlayerController())
	{
		SaveSelectedCharacterLocation();

		if (GetWorld())
		{
			GetWorldTimerManager().ClearTimer(CharacterLocationSaveTimerHandle);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AWUPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ApplySelectedCharacterSessionContext();
}

void AWUPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
	ApplySelectedCharacterSessionContext();
}

void AWUPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ClickTargetAction)
		{
			EnhancedInputComponent->BindAction(ClickTargetAction, ETriggerEvent::Started, this, &AWUPlayerController::TargetUnderCursor);
		}

		if (TabTargetAction)
		{
			EnhancedInputComponent->BindAction(TabTargetAction, ETriggerEvent::Started, this, &AWUPlayerController::TargetNextCharacter);
		}
	}

	if (!ClickTargetAction)
	{
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AWUPlayerController::TargetUnderCursor);
	}

	if (!TabTargetAction)
	{
		InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AWUPlayerController::TargetNextCharacter);
	}

	InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &AWUPlayerController::OpenChatInput);
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AWUPlayerController::CloseChatInput);
	InputComponent->BindKey(EKeys::I, IE_Pressed, this, &AWUPlayerController::ToggleInventory);
	InputComponent->BindKey(EKeys::B, IE_Pressed, this, &AWUPlayerController::ToggleInventory);
	InputComponent->BindKey(EKeys::C, IE_Pressed, this, &AWUPlayerController::ToggleCharacterCreator);
}

bool AWUPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

AWUCharacter* AWUPlayerController::GetCurrentTarget() const
{
	return IsSelectableTarget(CurrentTarget) ? CurrentTarget : nullptr;
}

bool AWUPlayerController::HasCurrentTarget() const
{
	return GetCurrentTarget() != nullptr;
}

void AWUPlayerController::SetCurrentTarget(AWUCharacter* NewTarget)
{
	if (!IsSelectableTarget(NewTarget))
	{
		NewTarget = nullptr;
	}

	if (CurrentTarget == NewTarget)
	{
		return;
	}

	if (CurrentTarget)
	{
		CurrentTarget->OnDestroyed.RemoveDynamic(this, &AWUPlayerController::OnCurrentTargetDestroyed);
	}

	CurrentTarget = NewTarget;

	if (CurrentTarget)
	{
		CurrentTarget->OnDestroyed.AddUniqueDynamic(this, &AWUPlayerController::OnCurrentTargetDestroyed);
	}

	OnTargetChanged.Broadcast(CurrentTarget);
	ForceNetUpdate();

	ShowTargetingDebugMessage(
		CurrentTarget
			? FString::Printf(TEXT("Target selected: %s"), *CurrentTarget->GetDisplayName().ToString())
			: TEXT("Target cleared"),
		CurrentTarget ? FColor::Green : FColor::Yellow
	);
}

void AWUPlayerController::AutoTargetDamagedCharacter(AWUCharacter* DamagedCharacter)
{
	if (!DamagedCharacter)
	{
		return;
	}

	SetCurrentTarget(DamagedCharacter);

	if (IsLocalPlayerController())
	{
		ShowTargetingDebugMessage(TEXT("Auto target from damage"), FColor::Green);
		return;
	}

	Client_AutoTargetDamagedCharacter(DamagedCharacter);
}

void AWUPlayerController::ClearCurrentTarget()
{
	SetCurrentTarget(nullptr);
}

void AWUPlayerController::TargetUnderCursor()
{
	if (!IsLocalPlayerController() || !GetWorld() || IsChatInputOpen())
	{
		return;
	}

	ShowTargetingDebugMessage(TEXT("Click target input"));

	FVector WorldLocation;
	FVector WorldDirection;
	if (!DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		ShowTargetingDebugMessage(TEXT("Click target: no mouse world ray"), FColor::Yellow);
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WUTargetUnderCursor), true);
	QueryParams.AddIgnoredActor(GetPawn());

	const FVector TraceEnd = WorldLocation + (WorldDirection * TargetTraceDistance);

	TArray<FHitResult> VisibilityHits;
	GetWorld()->LineTraceMultiByChannel(
		VisibilityHits,
		WorldLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	for (const FHitResult& Hit : VisibilityHits)
	{
		if (AWUCharacter* HitCharacter = Cast<AWUCharacter>(Hit.GetActor()))
		{
			if (IsSelectableTarget(HitCharacter))
			{
				SetCurrentTarget(HitCharacter);
				ShowTargetingDebugMessage(TEXT("Click target: visibility hit"));
				return;
			}
		}
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FHitResult> PawnHits;
	GetWorld()->LineTraceMultiByObjectType(
		PawnHits,
		WorldLocation,
		TraceEnd,
		ObjectParams,
		QueryParams
	);

	for (const FHitResult& Hit : PawnHits)
	{
		if (AWUCharacter* HitCharacter = Cast<AWUCharacter>(Hit.GetActor()))
		{
			if (IsSelectableTarget(HitCharacter))
			{
				SetCurrentTarget(HitCharacter);
				ShowTargetingDebugMessage(TEXT("Click target: pawn hit"));
				return;
			}
		}
	}

	AWUCharacter* NearbyRayTarget = FindSelectableTargetNearRay(WorldLocation, WorldDirection, TargetTraceDistance, ClickTargetRayTolerance);
	SetCurrentTarget(NearbyRayTarget);
	if (!NearbyRayTarget)
	{
		ShowTargetingDebugMessage(TEXT("Click target: no selectable target"), FColor::Yellow);
	}
}

void AWUPlayerController::TargetNextCharacter()
{
	if (!GetWorld() || IsChatInputOpen())
	{
		return;
	}

	ShowTargetingDebugMessage(TEXT("Tab target input"));

	FVector ViewLocation;
	FRotator ViewRotation;
	GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector ViewForward = ViewRotation.Vector();
	const float PreferredMinDot = FMath::Cos(FMath::DegreesToRadians(TabTargetPreferredAngleDegrees));
	const APawn* ControlledPawn = GetPawn();
	const FVector Origin = ControlledPawn ? ControlledPawn->GetActorLocation() : ViewLocation;

	TArray<AWUCharacter*> Candidates;
	for (TActorIterator<AWUCharacter> It(GetWorld()); It; ++It)
	{
		AWUCharacter* Candidate = *It;
		if (!IsSelectableTarget(Candidate))
		{
			continue;
		}

		const FVector ToCandidate = Candidate->GetActorLocation() - Origin;
		const float Distance = ToCandidate.Size();
		if (Distance > TabTargetMaxDistance)
		{
			continue;
		}

		Candidates.Add(Candidate);
	}

	if (Candidates.IsEmpty())
	{
		ClearCurrentTarget();
		ShowTargetingDebugMessage(TEXT("Tab target: no candidates"), FColor::Yellow);
		return;
	}

	Candidates.Sort([Origin, ViewLocation, ViewForward, PreferredMinDot](const AWUCharacter& Left, const AWUCharacter& Right)
	{
		const FVector LeftFromView = Left.GetActorLocation() - ViewLocation;
		const FVector RightFromView = Right.GetActorLocation() - ViewLocation;
		const float LeftDot = FVector::DotProduct(ViewForward, LeftFromView.GetSafeNormal());
		const float RightDot = FVector::DotProduct(ViewForward, RightFromView.GetSafeNormal());
		const bool bLeftPreferred = LeftDot >= PreferredMinDot;
		const bool bRightPreferred = RightDot >= PreferredMinDot;

		if (bLeftPreferred != bRightPreferred)
		{
			return bLeftPreferred;
		}

		if (!FMath::IsNearlyEqual(LeftDot, RightDot))
		{
			return LeftDot > RightDot;
		}

		return FVector::DistSquared(Left.GetActorLocation(), Origin) < FVector::DistSquared(Right.GetActorLocation(), Origin);
	});

	int32 CurrentIndex = INDEX_NONE;
	if (CurrentTarget)
	{
		Candidates.Find(CurrentTarget, CurrentIndex);
	}

	const int32 NextIndex = Candidates.IsValidIndex(CurrentIndex + 1) ? CurrentIndex + 1 : 0;
	SetCurrentTarget(Candidates[NextIndex]);
	ShowTargetingDebugMessage(FString::Printf(TEXT("Tab target: %d candidates"), Candidates.Num()));
}

void AWUPlayerController::OpenChatInput()
{
	if (!IsLocalPlayerController() || !ChatWidget)
	{
		return;
	}

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	ChatWidget->OpenInput();
}

void AWUPlayerController::CloseChatInput()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (ChatWidget && ChatWidget->IsInputOpen())
	{
		ChatWidget->CloseInput();
		SetIgnoreMoveInput(false);
		SetIgnoreLookInput(false);
		ApplyGameplayInputMode();
		return;
	}

	if (IsInventoryOpen())
	{
		HideInventory();
		return;
	}

	if (IsCharacterCreatorOpen())
	{
		HideCharacterCreator();
		return;
	}

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
	ApplyGameplayInputMode();
}

void AWUPlayerController::SubmitChatMessage(const FString& RawMessage)
{
	const FString SanitizedMessage = SanitizeChatMessage(RawMessage);
	if (SanitizedMessage.IsEmpty())
	{
		return;
	}

	Server_SendChatMessage(SanitizedMessage);
}

void AWUPlayerController::ToggleInventory()
{
	if (!IsLocalPlayerController() || !InventoryWidget || IsChatInputOpen() || IsCharacterCreatorOpen())
	{
		return;
	}

	InventoryWidget->ToggleInventory();

	if (InventoryWidget->IsInventoryOpen())
	{
		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;

		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
	else
	{
		ApplyGameplayInputMode();
	}
}

void AWUPlayerController::ShowInventory()
{
	if (!IsLocalPlayerController() || !InventoryWidget || IsChatInputOpen() || IsCharacterCreatorOpen())
	{
		return;
	}

	if (!InventoryWidget->IsInventoryOpen())
	{
		InventoryWidget->ShowInventory();
	}

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AWUPlayerController::HideInventory()
{
	if (!IsLocalPlayerController() || !InventoryWidget)
	{
		return;
	}

	InventoryWidget->HideInventory();
	ApplyGameplayInputMode();
}

void AWUPlayerController::ToggleCharacterCreator()
{
	if (!IsLocalPlayerController() || !CharacterCreatorWidget || IsChatInputOpen())
	{
		return;
	}

	if (CharacterCreatorWidget->IsCreatorOpen())
	{
		HideCharacterCreator();
	}
	else
	{
		ShowCharacterCreator();
	}
}

void AWUPlayerController::ShowCharacterCreator()
{
	if (!IsLocalPlayerController() || !CharacterCreatorWidget || IsChatInputOpen())
	{
		return;
	}

	if (InventoryWidget && InventoryWidget->IsInventoryOpen())
	{
		InventoryWidget->HideInventory();
	}

	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	CharacterCreatorWidget->ShowCreator();
	if (CharacterCreatorPreviewActor)
	{
		CharacterCreatorPreviewActor->SetActorHiddenInGame(false);
	}
}

void AWUPlayerController::HideCharacterCreator()
{
	if (!IsLocalPlayerController() || !CharacterCreatorWidget)
	{
		return;
	}

	CharacterCreatorWidget->HideCreator();

	if (CharacterCreatorPreviewActor)
	{
		CharacterCreatorPreviewActor->SetActorHiddenInGame(true);
	}

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
	ApplyGameplayInputMode();
}

void AWUPlayerController::PreviewCharacterCreateRequest(const FWUCharacterCreateRequest& Request)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (AWUCharacterCreatorPreviewActor* PreviewActor = EnsureCharacterCreatorPreviewActor())
	{
		PositionCharacterCreatorPreviewActor();
		PreviewActor->ApplyCreateRequest(Request);
		PreviewActor->SetActorHiddenInGame(false);
	}
}

void AWUPlayerController::RotateCharacterCreatorPreview(float YawDelta)
{
	if (CharacterCreatorPreviewActor)
	{
		CharacterCreatorPreviewActor->RotatePreview(YawDelta);
	}
}

void AWUPlayerController::SubmitCharacterCreateRequest(const FWUCharacterCreateRequest& Request)
{
	FWUCharacterCreateRequest SanitizedRequest = Request;
	SanitizedRequest.CharacterName = SanitizeCharacterName(Request.CharacterName);

	if (SanitizedRequest.CharacterName.Len() < 3)
	{
		ShowTargetingDebugMessage(TEXT("Character name must be at least 3 characters"), FColor::Yellow);
		return;
	}

	const FString Summary = FString::Printf(
		TEXT("Draft character: %s %s %s"),
		*SanitizedRequest.CharacterName,
		*WUCharacterCreation::RaceToString(SanitizedRequest.Race),
		*WUCharacterCreation::SexToString(SanitizedRequest.Sex)
	);

	UE_LOG(LogWU, Display, TEXT("%s"), *Summary);
	ShowTargetingDebugMessage(Summary, FColor::Green);
}

bool AWUPlayerController::IsChatInputOpen() const
{
	return ChatWidget && ChatWidget->IsInputOpen();
}

bool AWUPlayerController::IsInventoryOpen() const
{
	return InventoryWidget && InventoryWidget->IsInventoryOpen();
}

bool AWUPlayerController::IsCharacterCreatorOpen() const
{
	return CharacterCreatorWidget && CharacterCreatorWidget->IsCreatorOpen();
}

AWUCharacterCreatorPreviewActor* AWUPlayerController::EnsureCharacterCreatorPreviewActor()
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
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters
	);

	return CharacterCreatorPreviewActor;
}

void AWUPlayerController::PositionCharacterCreatorPreviewActor()
{
	if (!CharacterCreatorPreviewActor)
	{
		return;
	}

	const APawn* ControlledPawn = GetPawn();
	const FVector Origin = ControlledPawn ? ControlledPawn->GetActorLocation() : FVector::ZeroVector;
	const FVector Forward = ControlledPawn ? ControlledPawn->GetActorForwardVector() : FVector::ForwardVector;
	const FVector Right = ControlledPawn ? ControlledPawn->GetActorRightVector() : FVector::RightVector;
	const FVector PreviewLocation = Origin + (Forward * 180.0f) + (Right * 190.0f) - FVector(0.0f, 0.0f, 92.0f);
	const FRotator PreviewRotation(0.0f, (Origin - PreviewLocation).Rotation().Yaw, 0.0f);

	CharacterCreatorPreviewActor->SetActorLocationAndRotation(PreviewLocation, PreviewRotation);
}

FString AWUPlayerController::SanitizeCharacterName(const FString& RawName) const
{
	FString Result;
	const FString TrimmedName = RawName.TrimStartAndEnd();

	for (const TCHAR CharacterValue : TrimmedName)
	{
		if (FChar::IsAlpha(CharacterValue))
		{
			Result.AppendChar(CharacterValue);
		}
	}

	if (Result.Len() > 16)
	{
		Result.LeftInline(16);
	}

	if (!Result.IsEmpty())
	{
		Result[0] = FChar::ToUpper(Result[0]);
		for (int32 Index = 1; Index < Result.Len(); ++Index)
		{
			Result[Index] = FChar::ToLower(Result[Index]);
		}
	}

	return Result;
}

FString AWUPlayerController::SanitizeChatMessage(const FString& RawMessage) const
{
	FString Message = RawMessage.TrimStartAndEnd();
	Message.ReplaceInline(TEXT("\r"), TEXT(" "));
	Message.ReplaceInline(TEXT("\n"), TEXT(" "));
	Message.ReplaceInline(TEXT("\t"), TEXT(" "));

	if (MaxChatMessageLength > 0 && Message.Len() > MaxChatMessageLength)
	{
		Message = Message.Left(MaxChatMessageLength).TrimStartAndEnd();
	}

	return Message;
}

FString AWUPlayerController::GetChatDisplayName() const
{
	if (const AWUCharacter* WUCharacter = Cast<AWUCharacter>(GetPawn()))
	{
		const FString CharacterName = WUCharacter->GetDisplayName().ToString();
		if (!CharacterName.IsEmpty())
		{
			return CharacterName;
		}
	}

	if (PlayerState)
	{
		const FString PlayerName = PlayerState->GetPlayerName();
		if (!PlayerName.IsEmpty())
		{
			return PlayerName;
		}
	}

	return TEXT("Player");
}

void AWUPlayerController::ApplySelectedCharacterSessionContext()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UWUClientSessionSubsystem* Session = GameInstance ? GameInstance->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
	if (!Session || Session->GetSelectedCharacterId().IsEmpty())
	{
		return;
	}

	const FString& SelectedCharacterId = Session->GetSelectedCharacterId();
	const FWUBackendCharacterSummary* SelectedCharacter = nullptr;
	for (const FWUBackendCharacterSummary& SessionCharacter : Session->GetCharacters())
	{
		if (SessionCharacter.CharacterId == SelectedCharacterId)
		{
			SelectedCharacter = &SessionCharacter;
			break;
		}
	}

	if (!SelectedCharacter)
	{
		return;
	}

	if (PlayerState)
	{
		PlayerState->SetPlayerName(SelectedCharacter->Name);
	}

	if (AWUCharacter* WUCharacter = Cast<AWUCharacter>(GetPawn()))
	{
		WUCharacter->SetDisplayName(FText::FromString(SelectedCharacter->Name));

		if (AppliedSessionSpawnCharacterId != SelectedCharacterId && !SelectedCharacter->Location.IsNearlyZero())
		{
			WUCharacter->SetActorLocation(
				SelectedCharacter->Location.ToVector(),
				false,
				nullptr,
				ETeleportType::TeleportPhysics);
			AppliedSessionSpawnCharacterId = SelectedCharacterId;
		}
	}

	if (AppliedSessionCharacterId != SelectedCharacterId)
	{
		AppliedSessionCharacterId = SelectedCharacterId;
		ShowTargetingDebugMessage(FString::Printf(TEXT("Entered as %s"), *SelectedCharacter->Name), FColor::Green);
	}
}

void AWUPlayerController::SaveSelectedCharacterLocation()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UWUClientSessionSubsystem* Session = GameInstance ? GameInstance->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
	const APawn* ControlledPawn = GetPawn();
	if (!Session || !ControlledPawn || Session->GetSelectedCharacterId().IsEmpty())
	{
		return;
	}

	Session->SaveSelectedCharacterLocation(ControlledPawn->GetActorLocation());
}

void AWUPlayerController::Server_SendChatMessage_Implementation(const FString& Message)
{
	if (!GetWorld())
	{
		return;
	}

	const FString SanitizedMessage = SanitizeChatMessage(Message);
	if (SanitizedMessage.IsEmpty())
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastChatMessageServerTime < ChatMessageCooldownSeconds)
	{
		return;
	}

	LastChatMessageServerTime = Now;
	const FString SenderName = GetChatDisplayName();

	for (TActorIterator<AWUPlayerController> It(GetWorld()); It; ++It)
	{
		if (AWUPlayerController* Recipient = *It)
		{
			Recipient->Client_ReceiveChatMessage(SenderName, SanitizedMessage);
		}
	}
}

void AWUPlayerController::Client_ReceiveChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
	if (ChatWidget)
	{
		ChatWidget->AddChatMessage(FText::FromString(SenderName), FText::FromString(Message));
	}
}

void AWUPlayerController::ApplyGameplayInputMode()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	const bool bUseCursor = bShowGameplayCursor && !ShouldUseTouchControls();
	bShowMouseCursor = bUseCursor;
	bEnableClickEvents = bUseCursor;
	bEnableMouseOverEvents = bUseCursor;

	if (bUseCursor)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
	else
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
	}
}

void AWUPlayerController::ApplyUIInputMode()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

bool AWUPlayerController::IsSelectableTarget(const AWUCharacter* Candidate) const
{
	return Candidate
		&& Candidate != GetPawn()
		&& !Candidate->IsDead()
		&& !Candidate->IsPendingKillPending();
}

void AWUPlayerController::ShowTargetingDebugMessage(const FString& Message, const FColor& Color) const
{
	if (bShowTargetingDebugMessages && GEngine && IsLocalPlayerController())
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.8f, Color, Message);
	}
}

AWUCharacter* AWUPlayerController::FindSelectableTargetNearRay(const FVector& RayOrigin, const FVector& RayDirection, float MaxDistance, float RayTolerance) const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	const FVector SafeRayDirection = RayDirection.GetSafeNormal();
	AWUCharacter* BestTarget = nullptr;
	float BestScore = TNumericLimits<float>::Max();

	for (TActorIterator<AWUCharacter> It(GetWorld()); It; ++It)
	{
		AWUCharacter* Candidate = *It;
		if (!IsSelectableTarget(Candidate))
		{
			continue;
		}

		const FVector TargetLocation = Candidate->GetActorLocation();
		const FVector ToTarget = TargetLocation - RayOrigin;
		const float DistanceAlongRay = FVector::DotProduct(ToTarget, SafeRayDirection);
		if (DistanceAlongRay < 0.0f || DistanceAlongRay > MaxDistance)
		{
			continue;
		}

		const FVector ClosestPoint = RayOrigin + (SafeRayDirection * DistanceAlongRay);
		const float DistanceFromRay = FVector::Dist(TargetLocation, ClosestPoint);
		const UCapsuleComponent* Capsule = Candidate->GetCapsuleComponent();
		const float CandidateRadius = Capsule ? Capsule->GetScaledCapsuleRadius() : 50.0f;
		const float AllowedDistance = RayTolerance + CandidateRadius;
		if (DistanceFromRay > AllowedDistance)
		{
			continue;
		}

		const float Score = DistanceFromRay + (DistanceAlongRay * 0.001f);
		if (Score < BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

void AWUPlayerController::Client_AutoTargetDamagedCharacter_Implementation(AWUCharacter* DamagedCharacter)
{
	SetCurrentTarget(DamagedCharacter);
	ShowTargetingDebugMessage(TEXT("Auto target from damage"), FColor::Green);
}

void AWUPlayerController::OnRep_CurrentTarget()
{
	OnTargetChanged.Broadcast(CurrentTarget);
	ShowTargetingDebugMessage(
		CurrentTarget
			? FString::Printf(TEXT("Replicated target: %s"), *CurrentTarget->GetDisplayName().ToString())
			: TEXT("Replicated target cleared"),
		CurrentTarget ? FColor::Green : FColor::Yellow
	);
}

void AWUPlayerController::OnCurrentTargetDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor == CurrentTarget)
	{
		ClearCurrentTarget();
	}
}
