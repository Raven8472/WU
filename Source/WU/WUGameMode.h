// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WUGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS()
class WU_API AWUGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	/** Constructor */
	AWUGameMode();

	/** Character Blueprint class to spawn for players */
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<APawn> PlayerPawnClass;

	/** Distance used to separate players when a map only has one PlayerStart */
	UPROPERTY(EditAnywhere, Category = "Spawning", meta = (ClampMin = 0.0f, Units = "cm"))
	float FallbackSpawnSpacing = 180.0f;

	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	int32 SpawnedPlayerCount = 0;
};

