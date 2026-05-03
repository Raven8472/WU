// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WULoginGameMode.generated.h"

UCLASS()
class WU_API AWULoginGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWULoginGameMode();

	void MarkPlayerReadyToEnterGame(APlayerController* PlayerController);

protected:
	UPROPERTY(EditAnywhere, Category = "WU|Login")
	FString GameMapPath = TEXT("/Game/ThirdPerson/Lvl_MagicalBritain_Persistent?game=/Game/Blueprints/Core/BP_WUBaseGameMode.BP_WUBaseGameMode_C");

private:
	bool AreAllConnectedPlayersReady() const;
	void TravelToGameMap();

	UPROPERTY()
	TSet<TObjectPtr<APlayerController>> ReadyPlayers;

	bool bTravelRequested = false;
};
