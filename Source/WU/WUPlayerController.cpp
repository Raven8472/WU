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
#include "NPC/WUNpcCharacter.h"
#include "WUCharacter.h"
#include "UI/WUCharacterCreatorWidget.h"
#include "UI/WUCharacterPanelWidget.h"
#include "UI/WUChatWidget.h"
#include "UI/WUClubCharterPromptWidget.h"
#include "UI/WUExperienceBarWidget.h"
#include "UI/WUInventoryWidget.h"
#include "UI/WUPlayerFrameWidget.h"
#include "UI/WUSocialWidget.h"
#include "UI/WUTargetFrameWidget.h"
#include "UI/WUVendorWidget.h"
#include "UI/WUWorldClockWidget.h"
#include "UI/WUWorldHoverTooltipWidget.h"
#include "UI/WUZoneNameWidget.h"
#include "World/WUDayNightCycleActor.h"
#include "WU.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Input/SVirtualJoystick.h"

namespace
{
	const FLinearColor FriendlyEmeraldNameColor(0.0f, 0.84f, 0.38f, 1.0f);
	const FLinearColor NeutralEnemyNameColor(1.0f, 0.86f, 0.20f, 1.0f);
	const FLinearColor HostileEnemyNameColor(1.0f, 0.18f, 0.12f, 1.0f);

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

	FSlateColor GetNpcDispositionSlateColor(EWUNpcDisposition Disposition)
	{
		switch (Disposition)
		{
		case EWUNpcDisposition::NeutralEnemy:
			return FSlateColor(NeutralEnemyNameColor);
		case EWUNpcDisposition::HostileEnemy:
			return FSlateColor(HostileEnemyNameColor);
		case EWUNpcDisposition::Friendly:
		default:
			return FSlateColor(FriendlyEmeraldNameColor);
		}
	}
}

AWUPlayerController::AWUPlayerController()
{
	CharacterCreatorWidgetClass = UWUCharacterCreatorWidget::StaticClass();
	CharacterCreatorPreviewActorClass = AWUCharacterCreatorPreviewActor::StaticClass();
	CharacterPanelWidgetClass = UWUCharacterPanelWidget::StaticClass();
	ChatWidgetClass = UWUChatWidget::StaticClass();
	ClubCharterPromptWidgetClass = UWUClubCharterPromptWidget::StaticClass();
	ExperienceBarWidgetClass = UWUExperienceBarWidget::StaticClass();
	InventoryWidgetClass = UWUInventoryWidget::StaticClass();
	PlayerFrameWidgetClass = UWUPlayerFrameWidget::StaticClass();
	SocialWidgetClass = UWUSocialWidget::StaticClass();
	TargetFrameWidgetClass = UWUTargetFrameWidget::StaticClass();
	VendorWidgetClass = UWUVendorWidget::StaticClass();
	WorldHoverTooltipWidgetClass = UWUWorldHoverTooltipWidget::StaticClass();
	WorldClockWidgetClass = UWUWorldClockWidget::StaticClass();
	ZoneNameWidgetClass = UWUZoneNameWidget::StaticClass();
	DayNightCycleActorClass = AWUDayNightCycleActor::StaticClass();
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

	if (!ExperienceBarWidgetClass)
	{
		ExperienceBarWidgetClass = UWUExperienceBarWidget::StaticClass();
	}

	if (IsLocalPlayerController() && ExperienceBarWidgetClass)
	{
		ExperienceBarWidget = CreateWidget<UWUExperienceBarWidget>(this, ExperienceBarWidgetClass);

		if (ExperienceBarWidget)
		{
			ExperienceBarWidget->AddToPlayerScreen(7);
			ApplyViewportUnitFrameSlot(ExperienceBarWidget, ExperienceBarViewportSize, ExperienceBarViewportPosition, FAnchors(0.5f, 1.0f), FVector2D(0.5f, 1.0f));
		}
	}

	if (!ZoneNameWidgetClass)
	{
		ZoneNameWidgetClass = UWUZoneNameWidget::StaticClass();
	}

	if (IsLocalPlayerController() && ZoneNameWidgetClass)
	{
		ZoneNameWidget = CreateWidget<UWUZoneNameWidget>(this, ZoneNameWidgetClass);

		if (ZoneNameWidget)
		{
			ZoneNameWidget->AddToPlayerScreen(7);
			ApplyViewportUnitFrameSlot(ZoneNameWidget, ZoneNameViewportSize, ZoneNameViewportPosition, FAnchors(1.0f, 0.0f), FVector2D(1.0f, 0.0f));
		}
	}

	if (!WorldClockWidgetClass)
	{
		WorldClockWidgetClass = UWUWorldClockWidget::StaticClass();
	}

	if (IsLocalPlayerController() && WorldClockWidgetClass)
	{
		WorldClockWidget = CreateWidget<UWUWorldClockWidget>(this, WorldClockWidgetClass);

		if (WorldClockWidget)
		{
			WorldClockWidget->AddToPlayerScreen(7);
			ApplyViewportUnitFrameSlot(WorldClockWidget, WorldClockViewportSize, WorldClockViewportPosition, FAnchors(1.0f, 0.0f), FVector2D(1.0f, 0.0f));
		}
	}

	if (IsLocalPlayerController() && bAutoSpawnDayNightCycleActor)
	{
		if (!DayNightCycleActorClass)
		{
			DayNightCycleActorClass = AWUDayNightCycleActor::StaticClass();
		}

		if (UWorld* World = GetWorld())
		{
			for (TActorIterator<AWUDayNightCycleActor> It(World); It; ++It)
			{
				DayNightCycleActor = *It;
				break;
			}

			if (!DayNightCycleActor && DayNightCycleActorClass)
			{
				FActorSpawnParameters SpawnParameters;
				SpawnParameters.Owner = this;
				SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				DayNightCycleActor = World->SpawnActor<AWUDayNightCycleActor>(
					DayNightCycleActorClass,
					FVector::ZeroVector,
					FRotator::ZeroRotator,
					SpawnParameters);
			}
		}
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

	if (!SocialWidgetClass)
	{
		SocialWidgetClass = UWUSocialWidget::StaticClass();
	}

	if (IsLocalPlayerController() && SocialWidgetClass)
	{
		SocialWidget = CreateWidget<UWUSocialWidget>(this, SocialWidgetClass);

		if (SocialWidget)
		{
			SocialWidget->AddToPlayerScreen(9);
			ApplyViewportUnitFrameSlot(SocialWidget, SocialViewportSize, SocialViewportPosition, FAnchors(0.0f, 0.0f), FVector2D(0.0f, 0.0f));
			SocialWidget->OnIncludeOfflineChanged.AddDynamic(this, &AWUPlayerController::HandleSocialIncludeOfflineChanged);
			SocialWidget->OnRefreshRequested.AddDynamic(this, &AWUPlayerController::HandleSocialRefreshRequested);
			SocialWidget->OnInviteRequested.AddDynamic(this, &AWUPlayerController::HandleSocialInviteRequested);
			SocialWidget->OnKickRequested.AddDynamic(this, &AWUPlayerController::HandleSocialKickRequested);
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
			ApplyViewportUnitFrameSlot(InventoryWidget, InventoryWidget->GetDesiredInventoryPanelSize(), InventoryViewportPosition, FAnchors(1.0f, 1.0f), FVector2D(1.0f, 1.0f));
			InventoryWidget->OnItemUseRequested.AddDynamic(this, &AWUPlayerController::HandleInventoryItemUseRequested);
		}
	}

	if (!VendorWidgetClass)
	{
		VendorWidgetClass = UWUVendorWidget::StaticClass();
	}

	if (IsLocalPlayerController() && VendorWidgetClass)
	{
		VendorWidget = CreateWidget<UWUVendorWidget>(this, VendorWidgetClass);

		if (VendorWidget)
		{
			VendorWidget->AddToPlayerScreen(9);
			ApplyViewportUnitFrameSlot(VendorWidget, VendorViewportSize, VendorViewportPosition, FAnchors(0.0f, 0.5f), FVector2D(0.0f, 0.5f));
		}
	}

	if (!CharacterPanelWidgetClass)
	{
		CharacterPanelWidgetClass = UWUCharacterPanelWidget::StaticClass();
	}

	if (IsLocalPlayerController() && CharacterPanelWidgetClass)
	{
		CharacterPanelWidget = CreateWidget<UWUCharacterPanelWidget>(this, CharacterPanelWidgetClass);

		if (CharacterPanelWidget)
		{
			CharacterPanelWidget->AddToPlayerScreen(9);
			ApplyViewportUnitFrameSlot(CharacterPanelWidget, CharacterPanelViewportSize, CharacterPanelViewportPosition, FAnchors(0.5f, 0.5f), FVector2D(0.5f, 0.5f));
		}
	}

	if (!ClubCharterPromptWidgetClass)
	{
		ClubCharterPromptWidgetClass = UWUClubCharterPromptWidget::StaticClass();
	}

	if (IsLocalPlayerController() && ClubCharterPromptWidgetClass)
	{
		ClubCharterPromptWidget = CreateWidget<UWUClubCharterPromptWidget>(this, ClubCharterPromptWidgetClass);

		if (ClubCharterPromptWidget)
		{
			ClubCharterPromptWidget->AddToPlayerScreen(12);
			ApplyViewportUnitFrameSlot(ClubCharterPromptWidget, ClubCharterPromptViewportSize, ClubCharterPromptViewportPosition, FAnchors(0.5f, 0.5f), FVector2D(0.5f, 0.5f));
			ClubCharterPromptWidget->OnClubNameSubmitted.AddDynamic(this, &AWUPlayerController::HandleClubCharterSubmitted);
			ClubCharterPromptWidget->OnPromptCancelled.AddDynamic(this, &AWUPlayerController::HandleClubCharterPromptCancelled);
			ClubCharterPromptWidget->HidePrompt();
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
			CharacterCreatorWidget->AddToPlayerScreen(10);
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

	if (!WorldHoverTooltipWidgetClass)
	{
		WorldHoverTooltipWidgetClass = UWUWorldHoverTooltipWidget::StaticClass();
	}

	if (IsLocalPlayerController() && WorldHoverTooltipWidgetClass)
	{
		WorldHoverTooltipWidget = CreateWidget<UWUWorldHoverTooltipWidget>(this, WorldHoverTooltipWidgetClass);

		if (WorldHoverTooltipWidget)
		{
			WorldHoverTooltipWidget->AddToPlayerScreen(7);
			ApplyViewportUnitFrameSlot(WorldHoverTooltipWidget, WorldHoverTooltipViewportSize, WorldHoverTooltipViewportPosition, FAnchors(1.0f, 1.0f), FVector2D(1.0f, 1.0f));
			WorldHoverTooltipWidget->HideTooltip();
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
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
			{
				Session->OnInventorySnapshotLoaded.AddDynamic(this, &AWUPlayerController::HandleInventorySnapshotLoaded);
				Session->OnCharacterUpdated.AddDynamic(this, &AWUPlayerController::HandleSessionCharacterUpdated);
				Session->OnClubCreated.AddDynamic(this, &AWUPlayerController::HandleClubCreated);
				Session->OnClubInviteCreated.AddDynamic(this, &AWUPlayerController::HandleClubInviteCreated);
				Session->OnClubMemberRemoved.AddDynamic(this, &AWUPlayerController::HandleClubMemberRemoved);
				Session->OnClubRosterLoaded.AddDynamic(this, &AWUPlayerController::HandleClubRosterLoaded);
				Session->OnRequestFailed.AddDynamic(this, &AWUPlayerController::HandleSessionRequestFailed);
			}
		}

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

void AWUPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (IsLocalPlayerController())
	{
		UpdateWorldHoverTooltip();
	}
}

void AWUPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLocalPlayerController())
	{
		SaveSelectedCharacterLocation();

		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
			{
				Session->OnInventorySnapshotLoaded.RemoveDynamic(this, &AWUPlayerController::HandleInventorySnapshotLoaded);
				Session->OnCharacterUpdated.RemoveDynamic(this, &AWUPlayerController::HandleSessionCharacterUpdated);
				Session->OnClubCreated.RemoveDynamic(this, &AWUPlayerController::HandleClubCreated);
				Session->OnClubInviteCreated.RemoveDynamic(this, &AWUPlayerController::HandleClubInviteCreated);
				Session->OnClubMemberRemoved.RemoveDynamic(this, &AWUPlayerController::HandleClubMemberRemoved);
				Session->OnClubRosterLoaded.RemoveDynamic(this, &AWUPlayerController::HandleClubRosterLoaded);
				Session->OnRequestFailed.RemoveDynamic(this, &AWUPlayerController::HandleSessionRequestFailed);
			}
		}

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
		if (TabTargetAction)
		{
			EnhancedInputComponent->BindAction(TabTargetAction, ETriggerEvent::Started, this, &AWUPlayerController::TargetNextCharacter);
		}
	}

	if (!TabTargetAction)
	{
		InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AWUPlayerController::TargetNextCharacter);
	}

	InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &AWUPlayerController::OpenChatInput);
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AWUPlayerController::CloseChatInput);
	InputComponent->BindKey(EKeys::I, IE_Pressed, this, &AWUPlayerController::ToggleInventory);
	InputComponent->BindKey(EKeys::B, IE_Pressed, this, &AWUPlayerController::ToggleInventory);
	InputComponent->BindKey(EKeys::O, IE_Pressed, this, &AWUPlayerController::ToggleSocialPanel);
	InputComponent->BindKey(EKeys::C, IE_Pressed, this, &AWUPlayerController::ToggleCharacterPanel);
	InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AWUPlayerController::InteractWithNpc);
	InputComponent->BindKey(EKeys::K, IE_Pressed, this, &AWUPlayerController::ToggleCharacterCreator);
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
	if (NearbyRayTarget)
	{
		SetCurrentTarget(NearbyRayTarget);
	}
	else
	{
		ShowTargetingDebugMessage(TEXT("Click target: keeping current target"), FColor::Yellow);
	}
}

void AWUPlayerController::HandlePrimaryWorldClick()
{
	if (!IsLocalPlayerController() || HasInteractiveOverlayOpen())
	{
		return;
	}

	if (AWUNpcCharacter* NpcCharacter = FindNpcUnderCursor())
	{
		ShowVendorForNpc(NpcCharacter);
		return;
	}

	TargetUnderCursor();
}

void AWUPlayerController::InteractWithNpc()
{
	if (!IsLocalPlayerController() || HasInteractiveOverlayOpen())
	{
		return;
	}

	AWUNpcCharacter* NpcCharacter = FindNpcUnderCursor();
	if (!NpcCharacter)
	{
		NpcCharacter = FindNearestInteractableNpc();
	}

	if (!NpcCharacter)
	{
		ShowTargetingDebugMessage(TEXT("No NPC in range"), FColor::Yellow);
		return;
	}

	ShowVendorForNpc(NpcCharacter);
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
		ShowTargetingDebugMessage(TEXT("Tab target: no candidates - keeping current target"), FColor::Yellow);
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

	if (IsClubCharterPromptOpen())
	{
		HideClubCharterPrompt();
		return;
	}

	if (IsInventoryOpen())
	{
		HideInventory();
		return;
	}

	if (IsSocialPanelOpen())
	{
		HideSocialPanel();
		return;
	}

	if (IsCharacterPanelOpen())
	{
		HideCharacterPanel();
		return;
	}

	if (IsCharacterCreatorOpen())
	{
		HideCharacterCreator();
		return;
	}

	if (IsVendorOpen())
	{
		HideVendor();
		return;
	}

	if (HasCurrentTarget())
	{
		ClearCurrentTarget();
		return;
	}

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
	ApplyGameplayInputMode();
}

void AWUPlayerController::SubmitChatMessage(const FString& RawMessage)
{
	const FString SanitizedCommand = SanitizeChatMessage(RawMessage);
	if (SanitizedCommand.IsEmpty())
	{
		return;
	}

	auto TryConsumePayloadCommand = [&SanitizedCommand](const TCHAR* CommandText, FString& OutPayload) -> bool
	{
		const FString Command(CommandText);
		if (SanitizedCommand.Equals(Command, ESearchCase::IgnoreCase))
		{
			OutPayload.Empty();
			return true;
		}

		const FString CommandWithSpace = Command + TEXT(" ");
		if (SanitizedCommand.StartsWith(CommandWithSpace, ESearchCase::IgnoreCase))
		{
			OutPayload = SanitizedCommand.Mid(CommandWithSpace.Len()).TrimStartAndEnd();
			return true;
		}

		return false;
	};

	FString InviteTarget;
	if (TryConsumePayloadCommand(TEXT("/cinvite"), InviteTarget)
		|| TryConsumePayloadCommand(TEXT("/clubinvite"), InviteTarget))
	{
		if (InviteTarget.IsEmpty())
		{
			if (ChatWidget)
			{
				ChatWidget->AddChatMessage(NSLOCTEXT("WUPlayerController", "SystemChatSender", "System"), NSLOCTEXT("WUPlayerController", "ClubInviteUsage", "Usage: /cinvite character-name"));
			}
			return;
		}

		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
			{
				Session->InviteCharacterToSelectedClub(InviteTarget);
				if (ChatWidget)
				{
					ChatWidget->AddChatMessage(
						NSLOCTEXT("WUPlayerController", "SystemChatSender", "System"),
						FText::Format(NSLOCTEXT("WUPlayerController", "ClubInviteSending", "Sending club invite to {0}..."), FText::FromString(InviteTarget)));
				}
			}
		}
		return;
	}

	FString PreparedMessage;
	EWUChatChannel Channel = EWUChatChannel::Say;
	if (!TryPrepareOutgoingChatMessage(SanitizedCommand, PreparedMessage, Channel))
	{
		return;
	}

	FWUClubSummary Club;
	FString ClubId;
	if (Channel == EWUChatChannel::Club || Channel == EWUChatChannel::Officer)
	{
		if (!TryGetSelectedCharacterClub(Club) || !Club.HasClub())
		{
			if (ChatWidget)
			{
				ChatWidget->AddChatMessage(NSLOCTEXT("WUPlayerController", "SystemChatSender", "System"), NSLOCTEXT("WUPlayerController", "NotInClubChat", "You are not in a club."));
			}
			return;
		}

		if (Channel == EWUChatChannel::Officer && Club.Rank != EWUClubRank::Officer && Club.Rank != EWUClubRank::President)
		{
			if (ChatWidget)
			{
				ChatWidget->AddChatMessage(NSLOCTEXT("WUPlayerController", "SystemChatSender", "System"), NSLOCTEXT("WUPlayerController", "OfficerChatUnavailable", "Officer chat requires officer or president rank."));
			}
			return;
		}

		ClubId = Club.ClubId;
	}

	Server_SendChatMessage(PreparedMessage, Channel, ClubId);
}

void AWUPlayerController::ToggleSocialPanel()
{
	if (!IsLocalPlayerController() || !SocialWidget || IsChatInputOpen() || IsCharacterCreatorOpen() || IsClubCharterPromptOpen())
	{
		return;
	}

	if (SocialWidget->IsSocialOpen())
	{
		HideSocialPanel();
	}
	else
	{
		ShowSocialPanel();
	}
}

void AWUPlayerController::ShowSocialPanel()
{
	if (!IsLocalPlayerController() || !SocialWidget || IsChatInputOpen() || IsCharacterCreatorOpen() || IsClubCharterPromptOpen())
	{
		return;
	}

	if (InventoryWidget && InventoryWidget->IsInventoryOpen())
	{
		InventoryWidget->HideInventory();
	}

	if (CharacterPanelWidget && CharacterPanelWidget->IsPanelOpen())
	{
		CharacterPanelWidget->HidePanel();
	}

	if (VendorWidget && VendorWidget->IsVendorOpen())
	{
		VendorWidget->HideVendor();
	}

	FWUClubSummary Club;
	TryGetSelectedCharacterClub(Club);
	SocialWidget->ShowSocial(Club, bSocialIncludeOffline);

	if (Club.HasClub())
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
			{
				Session->LoadSelectedClubRoster(bSocialIncludeOffline);
			}
		}
	}

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AWUPlayerController::HideSocialPanel()
{
	if (!IsLocalPlayerController() || !SocialWidget)
	{
		return;
	}

	SocialWidget->HideSocial();
	ApplyGameplayInputMode();
}

void AWUPlayerController::ToggleInventory()
{
	if (!IsLocalPlayerController() || !InventoryWidget || IsChatInputOpen() || IsCharacterCreatorOpen() || IsClubCharterPromptOpen())
	{
		return;
	}

	if (IsCharacterPanelOpen())
	{
		CharacterPanelWidget->HidePanel();
	}

	if (IsSocialPanelOpen())
	{
		SocialWidget->HideSocial();
	}

	if (IsVendorOpen())
	{
		VendorWidget->HideVendor();
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

	if (IsCharacterPanelOpen())
	{
		CharacterPanelWidget->HidePanel();
	}

	if (IsSocialPanelOpen())
	{
		SocialWidget->HideSocial();
	}

	if (IsVendorOpen())
	{
		VendorWidget->HideVendor();
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

void AWUPlayerController::ShowVendorForNpc(AWUNpcCharacter* NpcCharacter)
{
	if (!IsLocalPlayerController() || !VendorWidget || !NpcCharacter || !IsNpcInInteractionRange(NpcCharacter))
	{
		return;
	}

	if (!NpcCharacter->CanOpenVendor() && !NpcCharacter->CanOfferQuest())
	{
		ShowTargetingDebugMessage(NpcCharacter->GetInteractionPrompt().ToString(), FColor::Yellow);
		return;
	}

	if (InventoryWidget && InventoryWidget->IsInventoryOpen())
	{
		InventoryWidget->HideInventory();
	}

	if (CharacterPanelWidget && CharacterPanelWidget->IsPanelOpen())
	{
		CharacterPanelWidget->HidePanel();
	}

	if (SocialWidget && SocialWidget->IsSocialOpen())
	{
		SocialWidget->HideSocial();
	}

	if (CharacterCreatorWidget && CharacterCreatorWidget->IsCreatorOpen())
	{
		HideCharacterCreator();
	}

	const FWUNpcProfile Profile = NpcCharacter->GetNpcProfile();
	VendorWidget->ShowVendor(Profile, NpcCharacter->GetNpcDisplayName(), NpcCharacter->GetInteractionPrompt());

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	ShowTargetingDebugMessage(FString::Printf(TEXT("Vendor: %s"), *NpcCharacter->GetNpcDisplayName().ToString()), FColor::Green);
}

void AWUPlayerController::HideVendor()
{
	if (!IsLocalPlayerController() || !VendorWidget)
	{
		return;
	}

	VendorWidget->HideVendor();
	ApplyGameplayInputMode();
}

void AWUPlayerController::ToggleCharacterPanel()
{
	if (!IsLocalPlayerController() || !CharacterPanelWidget || IsChatInputOpen() || IsCharacterCreatorOpen())
	{
		return;
	}

	if (InventoryWidget && InventoryWidget->IsInventoryOpen())
	{
		InventoryWidget->HideInventory();
	}

	if (VendorWidget && VendorWidget->IsVendorOpen())
	{
		VendorWidget->HideVendor();
	}

	if (SocialWidget && SocialWidget->IsSocialOpen())
	{
		SocialWidget->HideSocial();
	}

	CharacterPanelWidget->TogglePanel();

	if (CharacterPanelWidget->IsPanelOpen())
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

void AWUPlayerController::ShowCharacterPanel()
{
	if (!IsLocalPlayerController() || !CharacterPanelWidget || IsChatInputOpen() || IsCharacterCreatorOpen())
	{
		return;
	}

	if (InventoryWidget && InventoryWidget->IsInventoryOpen())
	{
		InventoryWidget->HideInventory();
	}

	if (VendorWidget && VendorWidget->IsVendorOpen())
	{
		VendorWidget->HideVendor();
	}

	if (SocialWidget && SocialWidget->IsSocialOpen())
	{
		SocialWidget->HideSocial();
	}

	if (!CharacterPanelWidget->IsPanelOpen())
	{
		CharacterPanelWidget->ShowPanel();
	}

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AWUPlayerController::HideCharacterPanel()
{
	if (!IsLocalPlayerController() || !CharacterPanelWidget)
	{
		return;
	}

	CharacterPanelWidget->HidePanel();
	ApplyGameplayInputMode();
}

void AWUPlayerController::ToggleCharacterCreator()
{
	if (!IsLocalPlayerController() || !CharacterCreatorWidget || IsChatInputOpen())
	{
		return;
	}

	if (IsCharacterPanelOpen())
	{
		HideCharacterPanel();
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

	if (CharacterPanelWidget && CharacterPanelWidget->IsPanelOpen())
	{
		CharacterPanelWidget->HidePanel();
	}

	if (VendorWidget && VendorWidget->IsVendorOpen())
	{
		VendorWidget->HideVendor();
	}

	if (SocialWidget && SocialWidget->IsSocialOpen())
	{
		SocialWidget->HideSocial();
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

void AWUPlayerController::RequestSelectedCharacterExperience(int32 Amount, EWUExperienceSource Source)
{
	if (Amount <= 0)
	{
		return;
	}

	Server_RequestSelectedCharacterExperience(Amount, Source);
}

void AWUPlayerController::GrantExplorationExperience(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	const AWUCharacter* WUCharacter = Cast<AWUCharacter>(GetPawn());
	const FName ZoneId = WUCharacter ? WUCharacter->GetCurrentZoneId() : NAME_None;
	if (ZoneId.IsNone())
	{
		return;
	}

	Server_RequestZoneExplorationExperience(Amount, ZoneId);
}

void AWUPlayerController::GrantQuestTurnInExperience(int32 Amount)
{
	RequestSelectedCharacterExperience(Amount, EWUExperienceSource::QuestTurnIn);
}

void AWUPlayerController::Client_HandleExperienceAward_Implementation(int32 Amount, EWUExperienceSource Source, const FString& SourceKey)
{
	UGameInstance* GameInstance = GetGameInstance();
	UWUClientSessionSubsystem* Session = GameInstance ? GameInstance->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
	if (Session)
	{
		Session->AwardSelectedCharacterExperience(Amount, Source, SourceKey);
	}
}

bool AWUPlayerController::IsChatInputOpen() const
{
	return ChatWidget && ChatWidget->IsInputOpen();
}

bool AWUPlayerController::IsInventoryOpen() const
{
	return InventoryWidget && InventoryWidget->IsInventoryOpen();
}

bool AWUPlayerController::IsVendorOpen() const
{
	return VendorWidget && VendorWidget->IsVendorOpen();
}

bool AWUPlayerController::IsCharacterPanelOpen() const
{
	return CharacterPanelWidget && CharacterPanelWidget->IsPanelOpen();
}

bool AWUPlayerController::IsCharacterCreatorOpen() const
{
	return CharacterCreatorWidget && CharacterCreatorWidget->IsCreatorOpen();
}

bool AWUPlayerController::IsClubCharterPromptOpen() const
{
	return ClubCharterPromptWidget && ClubCharterPromptWidget->IsPromptOpen();
}

bool AWUPlayerController::IsSocialPanelOpen() const
{
	return SocialWidget && SocialWidget->IsSocialOpen();
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
	if (!ServerChatDisplayName.IsEmpty())
	{
		return ServerChatDisplayName;
	}

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

bool AWUPlayerController::TryPrepareOutgoingChatMessage(const FString& RawMessage, FString& OutMessage, EWUChatChannel& OutChannel) const
{
	OutMessage = SanitizeChatMessage(RawMessage);
	OutChannel = EWUChatChannel::Say;
	if (OutMessage.IsEmpty())
	{
		return false;
	}

	auto TryConsumeCommand = [&OutMessage, &OutChannel](const TCHAR* CommandText, EWUChatChannel Channel) -> bool
	{
		const FString Command(CommandText);
		if (OutMessage.Equals(Command, ESearchCase::IgnoreCase))
		{
			OutMessage.Empty();
			OutChannel = Channel;
			return true;
		}

		const FString CommandWithSpace = Command + TEXT(" ");
		if (OutMessage.StartsWith(CommandWithSpace, ESearchCase::IgnoreCase))
		{
			OutMessage = OutMessage.Mid(CommandWithSpace.Len()).TrimStartAndEnd();
			OutChannel = Channel;
			return true;
		}

		return false;
	};

	TryConsumeCommand(TEXT("/c"), EWUChatChannel::Club)
		|| TryConsumeCommand(TEXT("/club"), EWUChatChannel::Club)
		|| TryConsumeCommand(TEXT("/o"), EWUChatChannel::Officer)
		|| TryConsumeCommand(TEXT("/officer"), EWUChatChannel::Officer)
		|| TryConsumeCommand(TEXT("/g"), EWUChatChannel::General)
		|| TryConsumeCommand(TEXT("/general"), EWUChatChannel::General)
		|| TryConsumeCommand(TEXT("/1"), EWUChatChannel::General)
		|| TryConsumeCommand(TEXT("/s"), EWUChatChannel::Say)
		|| TryConsumeCommand(TEXT("/say"), EWUChatChannel::Say);

	return !OutMessage.IsEmpty();
}

bool AWUPlayerController::TryGetSelectedCharacterClub(FWUClubSummary& OutClub) const
{
	OutClub = FWUClubSummary();

	const UGameInstance* GameInstance = GetGameInstance();
	const UWUClientSessionSubsystem* Session = GameInstance ? GameInstance->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
	if (!Session || Session->GetSelectedCharacterId().IsEmpty())
	{
		return false;
	}

	const FString& SelectedCharacterId = Session->GetSelectedCharacterId();
	for (const FWUBackendCharacterSummary& SessionCharacter : Session->GetCharacters())
	{
		if (SessionCharacter.CharacterId == SelectedCharacterId)
		{
			OutClub = SessionCharacter.Club;
			return OutClub.HasClub();
		}
	}

	return false;
}

void AWUPlayerController::UpdateServerChatIdentityFromSession()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UWUClientSessionSubsystem* Session = GameInstance ? GameInstance->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
	if (!Session || Session->GetSelectedCharacterId().IsEmpty())
	{
		Server_UpdateChatIdentity(TEXT(""), GetChatDisplayName(), TEXT(""), EWUClubRank::None);
		return;
	}

	const FString& SelectedCharacterId = Session->GetSelectedCharacterId();
	for (const FWUBackendCharacterSummary& SessionCharacter : Session->GetCharacters())
	{
		if (SessionCharacter.CharacterId == SelectedCharacterId)
		{
			Server_UpdateChatIdentity(
				SessionCharacter.CharacterId,
				SessionCharacter.Name,
				SessionCharacter.Club.ClubId,
				SessionCharacter.Club.Rank);
			return;
		}
	}
}

void AWUPlayerController::ApplySelectedCharacterSessionContext()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UWUClientSessionSubsystem* Session = GameInstance ? GameInstance->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
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
		WUCharacter->ApplyCharacterProgressionState(SelectedCharacter->Race, SelectedCharacter->Level, SelectedCharacter->Experience);
		WUCharacter->ApplyCharacterAppearance(SelectedCharacter->Appearance);
		WUCharacter->SetDisplayName(FText::FromString(SelectedCharacter->Name));

		if (AppliedSessionSpawnCharacterId != SelectedCharacterId && !SelectedCharacter->Location.IsNearlyZero())
		{
			const FVector SavedLocation = SelectedCharacter->Location.ToVector();
			if (HasAuthority())
			{
				WUCharacter->SetActorLocation(SavedLocation, false, nullptr, ETeleportType::TeleportPhysics);
				WUCharacter->ForceNetUpdate();
			}
			else
			{
				Server_ApplySelectedCharacterSpawnLocation(SavedLocation);
			}
			AppliedSessionSpawnCharacterId = SelectedCharacterId;
		}
	}

	if (AppliedSessionCharacterId != SelectedCharacterId)
	{
		AppliedSessionCharacterId = SelectedCharacterId;
		ShowTargetingDebugMessage(FString::Printf(TEXT("Entered as %s"), *SelectedCharacter->Name), FColor::Green);
	}

	if (AppliedInventoryCharacterId != SelectedCharacterId)
	{
		if (Session->HasInventorySnapshot() && Session->GetInventorySnapshot().CharacterId == SelectedCharacterId)
		{
			ApplyInventorySnapshotToCurrentPawn(Session->GetInventorySnapshot());
		}
		else
		{
			Session->RefreshSelectedInventorySnapshot();
		}
	}

	UpdateServerChatIdentityFromSession();
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

void AWUPlayerController::HandleInventorySnapshotLoaded(const FWUBackendInventorySnapshot& Snapshot)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UWUClientSessionSubsystem* Session = GameInstance ? GameInstance->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
	if (!Session || Snapshot.CharacterId != Session->GetSelectedCharacterId())
	{
		return;
	}

	ApplyInventorySnapshotToCurrentPawn(Snapshot);
}

void AWUPlayerController::HandleSessionCharacterUpdated(const FWUBackendCharacterSummary& UpdatedCharacter)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UWUClientSessionSubsystem* Session = GameInstance ? GameInstance->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
	if (!Session || UpdatedCharacter.CharacterId != Session->GetSelectedCharacterId())
	{
		return;
	}

	if (AWUCharacter* WUCharacter = Cast<AWUCharacter>(GetPawn()))
	{
		WUCharacter->ApplyCharacterProgressionState(UpdatedCharacter.Race, UpdatedCharacter.Level, UpdatedCharacter.Experience);
		WUCharacter->SetDisplayName(FText::FromString(UpdatedCharacter.Name));
	}

	UpdateServerChatIdentityFromSession();
}

void AWUPlayerController::HandleInventoryItemUseRequested(int32 SlotIndex, FWUInventoryItem Item)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (Item.UseType == EWUItemUseType::ClubCharter)
	{
		ShowClubCharterPrompt(SlotIndex, Item);
		return;
	}

	ShowTargetingDebugMessage(TEXT("That item cannot be used yet."), FColor::Yellow);
}

void AWUPlayerController::HandleClubCharterSubmitted(const FString& ClubName, int32 SlotIndex, FWUInventoryItem Item)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UWUClientSessionSubsystem* Session = GameInstance ? GameInstance->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
	if (!Session)
	{
		if (ClubCharterPromptWidget)
		{
			ClubCharterPromptWidget->SetStatusText(NSLOCTEXT("WUPlayerController", "ClubCharterSessionUnavailable", "Backend session is not available."));
		}
		return;
	}

	PendingClubCharterSlotIndex = SlotIndex;
	Session->CreateClubFromSelectedCharter(ClubName, SlotIndex, Item.ItemId.ToString());
}

void AWUPlayerController::HandleClubCharterPromptCancelled()
{
	PendingClubCharterSlotIndex = INDEX_NONE;
	RestoreInputAfterModalPrompt();
}

void AWUPlayerController::HandleClubCreated(const FWUClubSummary& Club)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	const int32 ConsumedSlotIndex = PendingClubCharterSlotIndex;
	HideClubCharterPrompt();

	if (ConsumedSlotIndex != INDEX_NONE)
	{
		if (AWUCharacter* WUCharacter = Cast<AWUCharacter>(GetPawn()))
		{
			WUCharacter->RemoveInventoryItemAtSlot(ConsumedSlotIndex);
		}
	}

	PendingClubCharterSlotIndex = INDEX_NONE;
	UpdateServerChatIdentityFromSession();

	if (SocialWidget && SocialWidget->IsSocialOpen())
	{
		SocialWidget->ShowSocial(Club, bSocialIncludeOffline);
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
			{
				Session->LoadSelectedClubRoster(bSocialIncludeOffline);
			}
		}
	}

	ShowTargetingDebugMessage(FString::Printf(TEXT("Club created: %s"), *Club.Name), FColor::Green);
}

void AWUPlayerController::HandleClubInviteCreated(const FWUBackendClubInviteSummary& Invite)
{
	(void)Invite;

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (SocialWidget && SocialWidget->IsSocialOpen())
	{
		SocialWidget->SetStatusText(NSLOCTEXT("WUPlayerController", "ClubInviteSent", "Club invite sent."));
	}

	ShowTargetingDebugMessage(TEXT("Club invite sent."), FColor::Green);
}

void AWUPlayerController::HandleClubMemberRemoved(const FString& CharacterId)
{
	(void)CharacterId;

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (SocialWidget && SocialWidget->IsSocialOpen())
	{
		SocialWidget->SetStatusText(NSLOCTEXT("WUPlayerController", "ClubMemberRemoved", "Club member removed."));
		HandleSocialRefreshRequested();
	}

	ShowTargetingDebugMessage(TEXT("Club member removed."), FColor::Green);
}

void AWUPlayerController::HandleClubRosterLoaded(const TArray<FWUClubMemberSummary>& Members)
{
	if (!IsLocalPlayerController() || !SocialWidget || !SocialWidget->IsSocialOpen())
	{
		return;
	}

	SocialWidget->SetClubRoster(Members, bSocialIncludeOffline);
}

void AWUPlayerController::HandleSocialIncludeOfflineChanged(bool bIncludeOffline)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	bSocialIncludeOffline = bIncludeOffline;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
		{
			Session->LoadSelectedClubRoster(bSocialIncludeOffline);
		}
	}
}

void AWUPlayerController::HandleSocialRefreshRequested()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
		{
			Session->LoadSelectedClubRoster(bSocialIncludeOffline);
		}
	}
}

void AWUPlayerController::HandleSocialInviteRequested(const FString& CharacterNameOrId)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
		{
			Session->InviteCharacterToSelectedClub(CharacterNameOrId);
		}
	}
}

void AWUPlayerController::HandleSocialKickRequested(const FString& CharacterId)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
		{
			Session->KickCharacterFromSelectedClub(CharacterId);
		}
	}
}

void AWUPlayerController::HandleSessionRequestFailed(const FString& ErrorMessage)
{
	if (IsClubCharterPromptOpen() && ClubCharterPromptWidget)
	{
		ClubCharterPromptWidget->SetStatusText(FText::FromString(ErrorMessage));
	}

	if (SocialWidget && SocialWidget->IsSocialOpen())
	{
		SocialWidget->SetStatusText(FText::FromString(ErrorMessage));
	}
}

void AWUPlayerController::ShowClubCharterPrompt(int32 SlotIndex, const FWUInventoryItem& Item)
{
	if (!IsLocalPlayerController() || !ClubCharterPromptWidget)
	{
		return;
	}

	PendingClubCharterSlotIndex = INDEX_NONE;
	ClubCharterPromptWidget->ShowPrompt(SlotIndex, Item);

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AWUPlayerController::HideClubCharterPrompt()
{
	if (!ClubCharterPromptWidget || !ClubCharterPromptWidget->IsPromptOpen())
	{
		PendingClubCharterSlotIndex = INDEX_NONE;
		return;
	}

	ClubCharterPromptWidget->HidePrompt();
	PendingClubCharterSlotIndex = INDEX_NONE;
	RestoreInputAfterModalPrompt();
}

void AWUPlayerController::RestoreInputAfterModalPrompt()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	if (HasInteractiveOverlayOpen())
	{
		ApplyUIInputMode();
	}
	else
	{
		ApplyGameplayInputMode();
	}
}

bool AWUPlayerController::ApplyInventorySnapshotToCurrentPawn(const FWUBackendInventorySnapshot& Snapshot)
{
	AWUCharacter* WUCharacter = Cast<AWUCharacter>(GetPawn());
	if (!WUCharacter)
	{
		return false;
	}

	TArray<FName> ItemIds;
	for (const FWUBackendInventoryItem& Item : Snapshot.Items)
	{
		const int32 Quantity = FMath::Max(1, Item.Quantity);
		for (int32 Count = 0; Count < Quantity; ++Count)
		{
			ItemIds.Add(FName(*Item.ItemId));
		}
	}

	WUCharacter->ApplyPersistentInventoryItemIds(ItemIds);
	AppliedInventoryCharacterId = Snapshot.CharacterId;
	return true;
}

void AWUPlayerController::Server_RequestSelectedCharacterExperience_Implementation(int32 Amount, EWUExperienceSource Source)
{
	if (Amount <= 0)
	{
		return;
	}

	if (AWUCharacter* WUCharacter = Cast<AWUCharacter>(GetPawn()))
	{
		WUCharacter->AwardExperience(Amount, Source);
	}
}

void AWUPlayerController::Server_RequestZoneExplorationExperience_Implementation(int32 Amount, FName ZoneId)
{
	if (Amount <= 0 || ZoneId.IsNone())
	{
		return;
	}

	if (AWUCharacter* WUCharacter = Cast<AWUCharacter>(GetPawn()))
	{
		WUCharacter->AwardExplorationExperience(Amount, ZoneId);
	}
}

void AWUPlayerController::Server_ApplySelectedCharacterSpawnLocation_Implementation(FVector SavedLocation)
{
	if (SavedLocation.IsNearlyZero())
	{
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	ControlledPawn->SetActorLocation(SavedLocation, false, nullptr, ETeleportType::TeleportPhysics);
	ControlledPawn->ForceNetUpdate();
}

void AWUPlayerController::Server_SendChatMessage_Implementation(const FString& Message, EWUChatChannel Channel, const FString& ClubId)
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
	const FString SenderClubId = !ServerChatClubId.IsEmpty() ? ServerChatClubId : ClubId;
	const FName SenderZoneId = Cast<AWUCharacter>(GetPawn()) ? Cast<AWUCharacter>(GetPawn())->GetCurrentZoneId() : NAME_None;

	if ((Channel == EWUChatChannel::Club || Channel == EWUChatChannel::Officer) && SenderClubId.IsEmpty())
	{
		Client_ReceiveChatMessage(TEXT("System"), TEXT("You are not in a club."));
		return;
	}

	if (Channel == EWUChatChannel::Officer
		&& ServerChatClubRank != EWUClubRank::Officer
		&& ServerChatClubRank != EWUClubRank::President)
	{
		Client_ReceiveChatMessage(TEXT("System"), TEXT("Officer chat requires officer or president rank."));
		return;
	}

	const FString ChannelLabel = [&Channel]()
	{
		switch (Channel)
		{
		case EWUChatChannel::General:
			return FString(TEXT("General"));
		case EWUChatChannel::Club:
			return FString(TEXT("Club"));
		case EWUChatChannel::Officer:
			return FString(TEXT("Officer"));
		case EWUChatChannel::Say:
		default:
			return FString(TEXT("Say"));
		}
	}();
	const FString RoutedSenderName = FString::Printf(TEXT("[%s] %s"), *ChannelLabel, *SenderName);

	for (TActorIterator<AWUPlayerController> It(GetWorld()); It; ++It)
	{
		if (AWUPlayerController* Recipient = *It)
		{
			bool bShouldReceive = false;
			if (Channel == EWUChatChannel::Club || Channel == EWUChatChannel::Officer)
			{
				bShouldReceive = !SenderClubId.IsEmpty() && Recipient->ServerChatClubId == SenderClubId;
				if (Channel == EWUChatChannel::Officer)
				{
					bShouldReceive = bShouldReceive
						&& (Recipient->ServerChatClubRank == EWUClubRank::Officer || Recipient->ServerChatClubRank == EWUClubRank::President);
				}
			}
			else
			{
				const AWUCharacter* RecipientCharacter = Cast<AWUCharacter>(Recipient->GetPawn());
				const FName RecipientZoneId = RecipientCharacter ? RecipientCharacter->GetCurrentZoneId() : NAME_None;
				bShouldReceive = SenderZoneId.IsNone() || RecipientZoneId.IsNone() || RecipientZoneId == SenderZoneId;
			}

			if (bShouldReceive)
			{
				Recipient->Client_ReceiveChatMessage(RoutedSenderName, SanitizedMessage);
			}
		}
	}
}

void AWUPlayerController::Server_UpdateChatIdentity_Implementation(const FString& CharacterId, const FString& DisplayName, const FString& ClubId, EWUClubRank ClubRank)
{
	ServerChatCharacterId = CharacterId.Left(64);
	ServerChatDisplayName = SanitizeCharacterName(DisplayName);
	if (ServerChatDisplayName.IsEmpty())
	{
		ServerChatDisplayName = DisplayName.Left(32).TrimStartAndEnd();
	}
	ServerChatClubId = ClubId.Left(64);
	ServerChatClubRank = ClubRank;
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

bool AWUPlayerController::HasInteractiveOverlayOpen() const
{
	return IsChatInputOpen()
		|| IsInventoryOpen()
		|| IsVendorOpen()
		|| IsSocialPanelOpen()
		|| IsCharacterPanelOpen()
		|| IsCharacterCreatorOpen()
		|| IsClubCharterPromptOpen();
}

AWUNpcCharacter* AWUPlayerController::FindNpcUnderCursor() const
{
	if (!IsLocalPlayerController() || !GetWorld())
	{
		return nullptr;
	}

	FVector WorldLocation;
	FVector WorldDirection;
	if (!DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		return nullptr;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WUNpcUnderCursor), true);
	QueryParams.AddIgnoredActor(GetPawn());

	const FVector TraceEnd = WorldLocation + (WorldDirection * TargetTraceDistance);
	auto FindNpcInHits = [this](const TArray<FHitResult>& Hits) -> AWUNpcCharacter*
	{
		for (const FHitResult& Hit : Hits)
		{
			if (AWUNpcCharacter* NpcCharacter = Cast<AWUNpcCharacter>(Hit.GetActor()))
			{
				if (IsNpcInInteractionRange(NpcCharacter))
				{
					return NpcCharacter;
				}
			}
		}

		return nullptr;
	};

	TArray<FHitResult> VisibilityHits;
	GetWorld()->LineTraceMultiByChannel(
		VisibilityHits,
		WorldLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParams);

	if (AWUNpcCharacter* NpcCharacter = FindNpcInHits(VisibilityHits))
	{
		return NpcCharacter;
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FHitResult> PawnHits;
	GetWorld()->LineTraceMultiByObjectType(
		PawnHits,
		WorldLocation,
		TraceEnd,
		ObjectParams,
		QueryParams);

	return FindNpcInHits(PawnHits);
}

AWUNpcCharacter* AWUPlayerController::FindNearestInteractableNpc() const
{
	if (!GetWorld() || !GetPawn())
	{
		return nullptr;
	}

	const APawn* ControlledPawn = GetPawn();
	const FVector PawnLocation = ControlledPawn->GetActorLocation();
	const FVector PawnForward = ControlledPawn->GetActorForwardVector();
	const float MaxDistanceSquared = FMath::Square(NpcInteractionDistance);

	AWUNpcCharacter* BestNpc = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (TActorIterator<AWUNpcCharacter> It(GetWorld()); It; ++It)
	{
		AWUNpcCharacter* NpcCharacter = *It;
		if (!NpcCharacter || NpcCharacter->IsPendingKillPending())
		{
			continue;
		}

		const FVector ToNpc = NpcCharacter->GetActorLocation() - PawnLocation;
		const float DistanceSquared = ToNpc.SizeSquared();
		if (DistanceSquared > MaxDistanceSquared)
		{
			continue;
		}

		if (FVector::DotProduct(PawnForward, ToNpc.GetSafeNormal()) < 0.15f)
		{
			continue;
		}

		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestNpc = NpcCharacter;
		}
	}

	return BestNpc;
}

bool AWUPlayerController::IsNpcInInteractionRange(const AWUNpcCharacter* NpcCharacter) const
{
	const APawn* ControlledPawn = GetPawn();
	if (!NpcCharacter || !ControlledPawn)
	{
		return false;
	}

	return FVector::DistSquared(ControlledPawn->GetActorLocation(), NpcCharacter->GetActorLocation()) <= FMath::Square(NpcInteractionDistance);
}

void AWUPlayerController::UpdateWorldHoverTooltip()
{
	if (!WorldHoverTooltipWidget)
	{
		return;
	}

	if (HasInteractiveOverlayOpen())
	{
		WorldHoverTooltipWidget->HideTooltip();
		return;
	}

	AActor* HoverActor = FindWorldHoverActorUnderCursor();
	if (!HoverActor)
	{
		WorldHoverTooltipWidget->HideTooltip();
		return;
	}

	if (const AWUNpcCharacter* NpcCharacter = Cast<AWUNpcCharacter>(HoverActor))
	{
		const FWUNpcProfile Profile = NpcCharacter->GetNpcProfile();
		WorldHoverTooltipWidget->ShowTooltip(
			NpcCharacter->GetNpcDisplayName(),
			NpcCharacter->GetVendorTypeLabel(),
			GetNpcDispositionSlateColor(Profile.Disposition));
		return;
	}

	if (const AWUCharacter* WUCharacter = Cast<AWUCharacter>(HoverActor))
	{
		WorldHoverTooltipWidget->ShowTooltip(
			WUCharacter->GetDisplayName(),
			FText::FromString(WUCharacterCreation::RaceToString(WUCharacter->GetBloodStatus())),
			FSlateColor(FriendlyEmeraldNameColor));
		return;
	}

	WorldHoverTooltipWidget->HideTooltip();
}

AActor* AWUPlayerController::FindWorldHoverActorUnderCursor() const
{
	if (!IsLocalPlayerController() || !GetWorld())
	{
		return nullptr;
	}

	FVector WorldLocation;
	FVector WorldDirection;
	if (!DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		return nullptr;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WUWorldHoverUnderCursor), true);
	QueryParams.AddIgnoredActor(GetPawn());

	const FVector TraceEnd = WorldLocation + (WorldDirection * TargetTraceDistance);
	auto ResolveHoverActor = [](AActor* Candidate) -> AActor*
	{
		if (Cast<AWUNpcCharacter>(Candidate) || Cast<AWUCharacter>(Candidate))
		{
			return Candidate;
		}

		return nullptr;
	};

	TArray<FHitResult> VisibilityHits;
	GetWorld()->LineTraceMultiByChannel(
		VisibilityHits,
		WorldLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParams);

	for (const FHitResult& Hit : VisibilityHits)
	{
		if (AActor* HoverActor = ResolveHoverActor(Hit.GetActor()))
		{
			return HoverActor;
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
		QueryParams);

	for (const FHitResult& Hit : PawnHits)
	{
		if (AActor* HoverActor = ResolveHoverActor(Hit.GetActor()))
		{
			return HoverActor;
		}
	}

	return nullptr;
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
