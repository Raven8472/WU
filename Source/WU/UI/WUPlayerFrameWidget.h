// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/WUTargetFrameWidget.h"
#include "WUPlayerFrameWidget.generated.h"

/**
 * Native player unit frame for the HUD.
 * Reuses the target frame layout but binds to the owning pawn instead of the selected target.
 */
UCLASS(Blueprintable)
class WU_API UWUPlayerFrameWidget : public UWUTargetFrameWidget
{
	GENERATED_BODY()

public:

	UWUPlayerFrameWidget(const FObjectInitializer& ObjectInitializer);

protected:

	virtual AWUCharacter* GetTargetCharacter() const override;
};
