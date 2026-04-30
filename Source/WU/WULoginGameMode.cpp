// Copyright Epic Games, Inc. All Rights Reserved.

#include "WULoginGameMode.h"
#include "WULoginPlayerController.h"
#include "Engine/World.h"

AWULoginGameMode::AWULoginGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = AWULoginPlayerController::StaticClass();
}

void AWULoginGameMode::MarkPlayerReadyToEnterGame(APlayerController* PlayerController)
{
	if (!PlayerController || bTravelRequested)
	{
		return;
	}

	ReadyPlayers.Add(PlayerController);

	if (AreAllConnectedPlayersReady())
	{
		TravelToGameMap();
	}
}

bool AWULoginGameMode::AreAllConnectedPlayersReady() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	int32 ConnectedPlayers = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		if (PlayerController)
		{
			++ConnectedPlayers;
			if (!ReadyPlayers.Contains(PlayerController))
			{
				return false;
			}
		}
	}

	return ConnectedPlayers > 0;
}

void AWULoginGameMode::TravelToGameMap()
{
	if (bTravelRequested || GameMapPath.IsEmpty())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		bTravelRequested = true;
		World->ServerTravel(GameMapPath);
	}
}
