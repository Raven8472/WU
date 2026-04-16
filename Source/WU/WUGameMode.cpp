// Copyright Epic Games, Inc. All Rights Reserved.

#include "WUGameMode.h"
#include "WUCharacter.h"
//Temporary until we have a proper GameModeBase class to include
#include "Engine/Engine.h"

AWUGameMode::AWUGameMode()
{
	DefDefaultPawnClass = nullptr;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Green,
			TEXT("WUGameMode Active")
		);
	}
}