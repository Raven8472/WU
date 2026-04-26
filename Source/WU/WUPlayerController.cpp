// Copyright Epic Games, Inc. All Rights Reserved.


#include "WUPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/Widget.h"
#include "InputCoreTypes.h"
#include "WUCharacter.h"
#include "UI/WUPlayerFrameWidget.h"
#include "UI/WUTargetFrameWidget.h"
#include "WU.h"
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
		ApplyGameplayInputMode();
		ShowTargetingDebugMessage(TEXT("WU targeting ready"));
	}
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
	if (!IsLocalPlayerController() || !GetWorld())
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
	if (!GetWorld())
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
