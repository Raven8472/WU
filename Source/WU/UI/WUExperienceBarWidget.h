// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "WUExperienceBarWidget.generated.h"

class AWUCharacter;
class UTexture2D;

/**
 * Compact player experience bar that sits above the action bar.
 * Keeps level progression visible during play without crowding the unit frames.
 */
UCLASS(Blueprintable)
class WU_API UWUExperienceBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UWUExperienceBarWidget(const FObjectInitializer& ObjectInitializer);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D WidgetSize = FVector2D(560.0f, 26.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> PanelTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> BarFrameTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> ExperienceFillTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FLinearColor ExperienceFillTint = FLinearColor(0.84f, 0.66f, 0.18f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor LabelColor = FSlateColor(FLinearColor(0.96f, 0.85f, 0.65f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor ValueColor = FSlateColor(FLinearColor(1.0f, 0.96f, 0.9f, 1.0f));

	virtual TSharedRef<SWidget> RebuildWidget() override;

private:

	AWUCharacter* GetCharacter() const;
	bool HasCharacter() const;
	FText GetExperienceText() const;
	TOptional<float> GetExperiencePercent() const;
	void ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin = FMargin(0.0f));
	void ConfigureColorBrush(FSlateBrush& Brush, const FLinearColor& Color, const FVector2D& ImageSize);

	FSlateBrush PanelBrush;
	FSlateBrush BarFrameBrush;
	FSlateBrush ExperienceFillBrush;
	FSlateBrush ExperienceBackgroundBrush;
	FProgressBarStyle ExperienceBarStyle;
};
