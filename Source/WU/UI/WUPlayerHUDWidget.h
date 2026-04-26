// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WUPlayerHUDWidget.generated.h"

class AWUCharacter;
class AWUPlayerController;
class UTexture2D;

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

	/** Returns the owning WU player controller */
	UFUNCTION(BlueprintPure, Category = "HUD")
	AWUPlayerController* GetWUPlayerController() const;

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

	/** Returns the currently selected target character, if any */
	UFUNCTION(BlueprintPure, Category = "HUD|Target")
	AWUCharacter* GetTargetCharacter() const;

	/** True when the player currently has a target selected */
	UFUNCTION(BlueprintPure, Category = "HUD|Target")
	bool HasTarget() const;

	/** Current target health percentage normalized to 0..1 */
	UFUNCTION(BlueprintPure, Category = "HUD|Target")
	float GetTargetHealthPercent() const;

	/** Current target health as a rounded number */
	UFUNCTION(BlueprintPure, Category = "HUD|Target")
	int32 GetTargetHealthRounded() const;

	/** Maximum target health as a rounded number */
	UFUNCTION(BlueprintPure, Category = "HUD|Target")
	int32 GetTargetMaxHealthRounded() const;

	/** Display name for the current target */
	UFUNCTION(BlueprintPure, Category = "HUD|Target")
	FText GetTargetDisplayName() const;

	/** Portrait texture for the current target */
	UFUNCTION(BlueprintPure, Category = "HUD|Target")
	UTexture2D* GetTargetPortraitTexture() const;
};
