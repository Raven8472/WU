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

	virtual void PostLogin(APlayerController* NewPlayer) override;
};

