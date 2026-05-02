// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "WUZoneVolume.generated.h"

class AWUCharacter;
class AWUZoneVolume;
class UBoxComponent;
class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWUZoneCharacterEvent, AWUZoneVolume*, Zone, AWUCharacter*, Character);

/**
 * Gameplay zone volume for named outdoor subregions inside the persistent world.
 * Handles exploration XP and stores the graveyard/map IDs that later systems can query.
 */
UCLASS(BlueprintType, Blueprintable)
class WU_API AWUZoneVolume : public AVolume
{
	GENERATED_BODY()

public:
	AWUZoneVolume();

	UFUNCTION(BlueprintPure, Category = "WU|Zone")
	FName GetZoneId() const;

	UFUNCTION(BlueprintPure, Category = "WU|Zone")
	FText GetDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "WU|Zone")
	FName GetMapRegionId() const;

	UFUNCTION(BlueprintPure, Category = "WU|Zone")
	FName GetGraveyardTag() const;

	UFUNCTION(BlueprintPure, Category = "WU|Zone")
	int32 GetExplorationExperience() const;

	UFUNCTION(BlueprintPure, Category = "WU|Zone")
	bool ShouldAwardExplorationExperience() const;

	UPROPERTY(BlueprintAssignable, Category = "WU|Zone")
	FWUZoneCharacterEvent OnCharacterEnteredZone;

	UPROPERTY(BlueprintAssignable, Category = "WU|Zone")
	FWUZoneCharacterEvent OnCharacterExitedZone;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WU|Zone", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> ZoneBounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WU|Zone")
	FName ZoneId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WU|Zone")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WU|Zone")
	FName MapRegionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WU|Zone")
	FName GraveyardTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WU|Zone|Progression", meta = (ClampMin = 0))
	int32 ExplorationExperience = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WU|Zone|Progression")
	bool bAwardExplorationExperience = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WU|Zone|Progression")
	bool bAwardOnlyOncePerCharacter = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WU|Zone|Debug")
	bool bLogZoneChanges = false;

private:
	UFUNCTION()
	void OnZoneBoundsBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnZoneBoundsEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void HandleCharacterEntered(AWUCharacter* Character);
	void HandleCharacterExited(AWUCharacter* Character);
	void AwardExplorationExperience(AWUCharacter* Character);

	TSet<TWeakObjectPtr<AWUCharacter>> OccupyingCharacters;
	TSet<TWeakObjectPtr<AWUCharacter>> CharactersAwardedExploration;
};
