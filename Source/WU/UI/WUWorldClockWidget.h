// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "WUWorldClockWidget.generated.h"

class UTexture2D;

/**
 * Compact server-backed world clock readout.
 */
UCLASS(Blueprintable)
class WU_API UWUWorldClockWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWUWorldClockWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D WidgetSize = FVector2D(220.0f, 46.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> PanelTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor LabelTextColor = FSlateColor(FLinearColor(0.80f, 0.74f, 0.62f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor ClockTextColor = FSlateColor(FLinearColor(0.96f, 0.85f, 0.62f, 1.0f));

	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	FText GetClockText() const;
	void ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin = FMargin(0.0f));

	FSlateBrush PanelBrush;
};
