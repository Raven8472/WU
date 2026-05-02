// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "WUZoneNameWidget.generated.h"

class AWUCharacter;
class UTexture2D;

/**
 * Compact zone name readout placed beneath the minimap for world-zone testing.
 */
UCLASS(Blueprintable)
class WU_API UWUZoneNameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWUZoneNameWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D WidgetSize = FVector2D(280.0f, 30.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> PanelTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor ZoneTextColor = FSlateColor(FLinearColor(0.96f, 0.85f, 0.62f, 1.0f));

	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	AWUCharacter* GetCharacter() const;
	bool HasZoneName() const;
	FText GetZoneNameText() const;
	void ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin = FMargin(0.0f));

	FSlateBrush PanelBrush;
};
