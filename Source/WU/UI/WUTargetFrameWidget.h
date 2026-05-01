// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "WUTargetFrameWidget.generated.h"

class AWUCharacter;
class UTexture2D;

/**
 * Native enemy target frame for the HUD.
 * Shows the current target portrait, name, health, and Magic without requiring binary UMG asset edits.
 */
UCLASS(Blueprintable)
class WU_API UWUTargetFrameWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UWUTargetFrameWidget(const FObjectInitializer& ObjectInitializer);

protected:

	/** Overall target frame size in viewport pixels */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D FrameSize = FVector2D(260.0f, 82.0f);

	/** Texture used for the outer portrait frame */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> PortraitFrameTexture;

	/** Texture used for the health bar frame */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> BarFrameTexture;

	/** Texture used for the health bar fill */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> HealthFillTexture;

	/** Texture used for the Magic bar fill */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> MagicFillTexture;

	/** Texture used for the backing plate */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> PanelTexture;

	/** Tint applied to the target health fill */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FLinearColor HealthFillTint = FLinearColor(1.0f, 0.18f, 0.12f, 1.0f);

	/** Tint applied to the target Magic fill */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FLinearColor MagicFillTint = FLinearColor(0.14f, 0.46f, 1.0f, 1.0f);

	/** Tint applied to the fallback portrait background */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FLinearColor PortraitBackgroundTint = FLinearColor(0.09f, 0.045f, 0.04f, 0.96f);

	/** Accent tint for target labels */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor LabelColor = FSlateColor(FLinearColor(0.96f, 0.85f, 0.65f, 1.0f));

	/** Text color for readable values */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor ValueColor = FSlateColor(FLinearColor(1.0f, 0.96f, 0.9f, 1.0f));

	virtual TSharedRef<SWidget> RebuildWidget() override;

	bool HasTarget() const;
	FText GetTargetNameText() const;
	FText GetHealthText() const;
	FText GetMagicText() const;
	FText GetFallbackPortraitText() const;
	TOptional<float> GetHealthPercent() const;
	TOptional<float> GetMagicPercent() const;
	const FSlateBrush* GetPortraitImageBrush() const;

	virtual AWUCharacter* GetTargetCharacter() const;

	void ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin = FMargin(0.0f));
	void ConfigureColorBrush(FSlateBrush& Brush, const FLinearColor& Color, const FVector2D& ImageSize);

	FSlateBrush PortraitFrameBrush;
	FSlateBrush BarFrameBrush;
	FSlateBrush HealthFillBrush;
	FSlateBrush MagicFillBrush;
	FSlateBrush PanelBrush;
	mutable FSlateBrush PortraitImageBrush;
	FSlateBrush PortraitFallbackBrush;
	FSlateBrush HealthBackgroundBrush;
	FSlateBrush MagicBackgroundBrush;
	FProgressBarStyle HealthBarStyle;
	FProgressBarStyle MagicBarStyle;
};
