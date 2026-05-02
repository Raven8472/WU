// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/WUZoneVolume.h"

#include "Components/BoxComponent.h"
#include "Components/BrushComponent.h"
#include "Engine/Engine.h"
#include "WUCharacter.h"

AWUZoneVolume::AWUZoneVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	SetCanBeDamaged(false);

	BrushColor = FColor(90, 170, 255, 96);

	if (UBrushComponent* BrushComp = GetBrushComponent())
	{
		BrushComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BrushComp->SetGenerateOverlapEvents(false);
		BrushComp->SetHiddenInGame(true);
	}

	ZoneBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBounds"));
	ZoneBounds->SetupAttachment(RootComponent);
	ZoneBounds->SetBoxExtent(FVector(1000.0f, 1000.0f, 300.0f));
	ZoneBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ZoneBounds->SetCollisionObjectType(ECC_WorldDynamic);
	ZoneBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
	ZoneBounds->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ZoneBounds->SetGenerateOverlapEvents(true);
	ZoneBounds->SetCanEverAffectNavigation(false);
	ZoneBounds->SetHiddenInGame(true);
	ZoneBounds->ShapeColor = FColor(90, 170, 255, 180);

	ZoneBounds->OnComponentBeginOverlap.AddDynamic(this, &AWUZoneVolume::OnZoneBoundsBeginOverlap);
	ZoneBounds->OnComponentEndOverlap.AddDynamic(this, &AWUZoneVolume::OnZoneBoundsEndOverlap);
}

void AWUZoneVolume::OnZoneBoundsBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (AWUCharacter* Character = Cast<AWUCharacter>(OtherActor))
	{
		HandleCharacterEntered(Character);
	}
}

void AWUZoneVolume::OnZoneBoundsEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (AWUCharacter* Character = Cast<AWUCharacter>(OtherActor))
	{
		HandleCharacterExited(Character);
	}
}

FName AWUZoneVolume::GetZoneId() const
{
	return ZoneId;
}

FText AWUZoneVolume::GetDisplayName() const
{
	return DisplayName;
}

FName AWUZoneVolume::GetMapRegionId() const
{
	return MapRegionId;
}

FName AWUZoneVolume::GetGraveyardTag() const
{
	return GraveyardTag;
}

int32 AWUZoneVolume::GetExplorationExperience() const
{
	return ExplorationExperience;
}

bool AWUZoneVolume::ShouldAwardExplorationExperience() const
{
	return bAwardExplorationExperience && ExplorationExperience > 0;
}

void AWUZoneVolume::HandleCharacterEntered(AWUCharacter* Character)
{
	if (!Character || OccupyingCharacters.Contains(Character))
	{
		return;
	}

	OccupyingCharacters.Add(Character);

	if (bLogZoneChanges)
	{
		UE_LOG(LogTemp, Log, TEXT("WU Zone Entered | Zone=%s | Character=%s"), *ZoneId.ToString(), *GetNameSafe(Character));
	}

	OnCharacterEnteredZone.Broadcast(this, Character);

	if (Character->HasAuthority())
	{
		Character->SetCurrentZone(ZoneId, DisplayName, MapRegionId, GraveyardTag);
	}

	AwardExplorationExperience(Character);
}

void AWUZoneVolume::HandleCharacterExited(AWUCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	OccupyingCharacters.Remove(Character);

	if (bLogZoneChanges)
	{
		UE_LOG(LogTemp, Log, TEXT("WU Zone Exited | Zone=%s | Character=%s"), *ZoneId.ToString(), *GetNameSafe(Character));
	}

	OnCharacterExitedZone.Broadcast(this, Character);
}

void AWUZoneVolume::AwardExplorationExperience(AWUCharacter* Character)
{
	if (!Character || !ShouldAwardExplorationExperience() || !Character->HasAuthority())
	{
		return;
	}

	if (bAwardOnlyOncePerCharacter && CharactersAwardedExploration.Contains(Character))
	{
		return;
	}

	CharactersAwardedExploration.Add(Character);
	Character->AwardExperience(ExplorationExperience, EWUExperienceSource::Exploration);

	if (bLogZoneChanges)
	{
		UE_LOG(LogTemp, Log, TEXT("WU Zone Exploration XP | Zone=%s | Character=%s | XP=%d"),
			*ZoneId.ToString(),
			*GetNameSafe(Character),
			ExplorationExperience);
	}
}
