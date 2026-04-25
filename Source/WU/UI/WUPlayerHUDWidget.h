// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WUPlayerHUDWidget.generated.h"

class AWUCharacter;

/**
 * Base player HUD widget for the WU prototype.
 * Intended to back a Widget Blueprint with health globe, action bar, and death-state UI.
 */
UCLASS(Abstract, Blueprintable)
class WU_API UWUPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Returns the locally owned WU character if one is currently possessed */
	UFUNCTION(BlueprintPure, Category = "HUD")
	AWUCharacter* GetWUCharacter() const;

	/** Current health percentage normalized to 0..1 */
	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetHealthPercent() const;

	/** Current health as a rounded number */
	UFUNCTION(BlueprintPure, Category = "HUD")
	int32 GetHealthRounded() const;

	/** Maximum health as a rounded number */
	UFUNCTION(BlueprintPure, Category = "HUD")
	int32 GetMaxHealthRounded() const;

	/** True when the player is in the dead state */
	UFUNCTION(BlueprintPure, Category = "HUD")
	bool IsPlayerDead() const;

	/** True when the player is a released spirit running back to the corpse */
	UFUNCTION(BlueprintPure, Category = "HUD")
	bool IsPlayerReleasedSpirit() const;

	/** Prompt text for the current release/revive state */
	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetDeathPromptText() const;
};
