// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUOverheadNameVisibilityComponent.h"

#include "CollisionQueryParams.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

UWUOverheadNameVisibilityComponent::UWUOverheadNameVisibilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 0.2f;
}

void UWUOverheadNameVisibilityComponent::SetOverheadNameComponent(UWidgetComponent* NewOverheadNameComponent)
{
	OverheadNameComponent = NewOverheadNameComponent;
}

void UWUOverheadNameVisibilityComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const UWorld* World = GetWorld(); World && World->GetNetMode() == NM_DedicatedServer)
	{
		SetComponentTickEnabled(false);
		ApplyNameplateVisibility(false);
		return;
	}

	ApplyNameplateVisibility(ShouldShowNameplate());
}

void UWUOverheadNameVisibilityComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ApplyNameplateVisibility(ShouldShowNameplate());
}

bool UWUOverheadNameVisibilityComponent::ShouldShowNameplate() const
{
	const UWorld* World = GetWorld();
	const AActor* Owner = GetOwner();
	if (!World || !Owner || !OverheadNameComponent)
	{
		return false;
	}

	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if (!PlayerController || !PlayerController->PlayerCameraManager)
	{
		return false;
	}

	if (bHideForLocalPawn)
	{
		if (const APawn* OwnerPawn = Cast<APawn>(Owner))
		{
			if (OwnerPawn->IsLocallyControlled())
			{
				return false;
			}
		}
	}

	const FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FVector NameplateLocation = OverheadNameComponent->GetComponentLocation();

	if (MaxVisibleDistance > 0.0f
		&& FVector::DistSquared(CameraLocation, NameplateLocation) > FMath::Square(MaxVisibleDistance))
	{
		return false;
	}

	if (!bRequireLineOfSight)
	{
		return true;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WUOverheadNameLineOfSight), false);
	QueryParams.AddIgnoredActor(Owner);
	if (const APawn* ViewPawn = PlayerController->GetPawn())
	{
		QueryParams.AddIgnoredActor(ViewPawn);
	}
	if (const AActor* ViewTarget = PlayerController->GetViewTarget())
	{
		QueryParams.AddIgnoredActor(ViewTarget);
	}

	FHitResult Hit;
	return !World->LineTraceSingleByChannel(
		Hit,
		CameraLocation,
		NameplateLocation,
		LineOfSightTraceChannel,
		QueryParams);
}

void UWUOverheadNameVisibilityComponent::ApplyNameplateVisibility(bool bShouldShow)
{
	if (!OverheadNameComponent)
	{
		return;
	}

	OverheadNameComponent->SetVisibility(bShouldShow, true);
	OverheadNameComponent->SetHiddenInGame(!bShouldShow, true);
}
