// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/WUInventoryTypes.h"
#include "Styling/SlateBrush.h"
#include "WUCharacterPanelWidget.generated.h"

class UTexture2D;

/**
 * Native character sheet shell for the WU prototype.
 * Provides the first-pass equipment paper doll and stat readout.
 */
UCLASS(Blueprintable)
class WU_API UWUCharacterPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UWUCharacterPanelWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Character")
	void TogglePanel();

	UFUNCTION(BlueprintCallable, Category = "Character")
	void ShowPanel();

	UFUNCTION(BlueprintCallable, Category = "Character")
	void HidePanel();

	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsPanelOpen() const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D PanelSize = FVector2D(560.0f, 520.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D EquipmentSlotSize = FVector2D(36.0f, 36.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> PanelTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> SlotTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor LabelColor = FSlateColor(FLinearColor(0.96f, 0.85f, 0.65f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor MutedLabelColor = FSlateColor(FLinearColor(0.58f, 0.54f, 0.46f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor ValueColor = FSlateColor(FLinearColor(0.90f, 0.92f, 0.96f, 1.0f));

	virtual TSharedRef<SWidget> RebuildWidget() override;

private:

	EVisibility GetPanelVisibility() const;
	FText GetCharacterNameText() const;
	FText GetCharacterSubtitleText() const;
	FText GetPrimaryStatsText() const;
	FText GetDerivedStatsText() const;
	TSharedRef<SWidget> CreateEquipmentColumn(const TArray<EWUEquipmentSlot>& Slots);
	TSharedRef<SWidget> CreateEquipmentSlot(EWUEquipmentSlot EquipmentSlot);
	FText GetEquipmentSlotItemText(EWUEquipmentSlot EquipmentSlot) const;
	FText GetEquipmentSlotTooltipText(EWUEquipmentSlot EquipmentSlot) const;
	FSlateColor GetEquipmentSlotItemColor(EWUEquipmentSlot EquipmentSlot) const;
	FLinearColor GetEquipmentSlotTint(EWUEquipmentSlot EquipmentSlot) const;
	FReply HandleEquipmentSlotClicked(EWUEquipmentSlot EquipmentSlot);
	void ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin = FMargin(0.0f));

private:

	bool bPanelOpen = false;

	FSlateBrush PanelBrush;
	FSlateBrush SlotBrush;
};
