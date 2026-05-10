// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WUOverheadNameWidget.generated.h"

/**
 * Compact in-world name label used by player and NPC screen-space widget components.
 */
UCLASS(Blueprintable)
class WU_API UWUOverheadNameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWUOverheadNameWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Nameplate")
	void SetNameText(const FText& NewNameText);

	UFUNCTION(BlueprintCallable, Category = "Nameplate")
	void SetSubtitleText(const FText& NewSubtitleText);

	UFUNCTION(BlueprintCallable, Category = "Nameplate")
	void SetNameColor(FSlateColor NewNameColor);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	FText GetNameText() const;
	FText GetSubtitleText() const;
	FSlateColor GetNameColor() const;
	EVisibility GetSubtitleVisibility() const;

	FText NameText;
	FText SubtitleText;
	FSlateColor NameColor = FSlateColor(FLinearColor(0.0f, 0.84f, 0.38f, 1.0f));
	FSlateColor SubtitleColor = FSlateColor(FLinearColor(0.62f, 0.82f, 1.0f, 1.0f));
};
