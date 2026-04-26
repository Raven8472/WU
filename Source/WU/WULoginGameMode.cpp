// Copyright Epic Games, Inc. All Rights Reserved.

#include "WULoginGameMode.h"
#include "WULoginPlayerController.h"

AWULoginGameMode::AWULoginGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = AWULoginPlayerController::StaticClass();
}
