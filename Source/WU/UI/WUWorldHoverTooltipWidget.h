// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WUWorldHoverTooltipWidget.generated.h"

/**
 * Bottom-right world hover readout for player and NPC identity.
 */
UCLASS(Blueprintable)
class WU_API UWUWorldHoverTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWUWorldHoverTooltipWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "World Hover")
	void ShowTooltip(const FText& NewTitleText, const FText& NewDetailText, FSlateColor NewTitleColor);

	UFUNCTION(BlueprintCallable, Category = "World Hover")
	void HideTooltip();

	UFUNCTION(BlueprintPure, Category = "World Hover")
	bool IsTooltipVisible() const;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	FText GetTitleText() const;
	FText GetDetailText() const;
	FSlateColor GetTitleColor() const;
	EVisibility GetTooltipVisibility() const;
	EVisibility GetDetailVisibility() const;

	FText TitleText;
	FText DetailText;
	FSlateColor TitleColor = FSlateColor(FLinearColor(0.0f, 0.84f, 0.38f, 1.0f));
	bool bTooltipVisible = false;
};
