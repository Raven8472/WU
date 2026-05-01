// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/WUInventoryTypes.h"
#include "Styling/SlateBrush.h"
#include "WUInventoryWidget.generated.h"

class UTexture2D;

/**
 * Native inventory shell for the WU prototype.
 * Shows fixed starter bag capacity without introducing temporary item data.
 */
UCLASS(Blueprintable)
class WU_API UWUInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UWUInventoryWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ShowInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void HideInventory();

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsInventoryOpen() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetUnlockedInventorySlotCount() const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 TotalBagSlots = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 StartingBagCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 SlotsPerBag = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D InventorySize = FVector2D(520.0f, 340.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D InventorySlotSize = FVector2D(34.0f, 34.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D BagSlotSize = FVector2D(40.0f, 40.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> PanelTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> SlotTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> DisabledSlotTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor LabelColor = FSlateColor(FLinearColor(0.96f, 0.85f, 0.65f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor MutedLabelColor = FSlateColor(FLinearColor(0.48f, 0.45f, 0.38f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor ValueColor = FSlateColor(FLinearColor(0.90f, 0.92f, 0.96f, 1.0f));

	virtual TSharedRef<SWidget> RebuildWidget() override;

private:

	EVisibility GetInventoryVisibility() const;
	bool IsBagSlotUnlocked(int32 BagSlotIndex) const;
	TSharedRef<SWidget> CreateBagSlot(int32 BagSlotIndex) const;
	TSharedRef<SWidget> CreateBagSection(int32 BagIndex);
	TSharedRef<SWidget> CreateInventorySlot(int32 AbsoluteSlotIndex);
	FText GetInventorySlotText(int32 AbsoluteSlotIndex) const;
	FText GetInventorySlotTooltipText(int32 AbsoluteSlotIndex) const;
	FSlateColor GetInventorySlotTextColor(int32 AbsoluteSlotIndex) const;
	FLinearColor GetInventorySlotTint(int32 AbsoluteSlotIndex) const;
	FReply HandleInventorySlotClicked(int32 AbsoluteSlotIndex);

	void ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin = FMargin(0.0f));

private:

	bool bInventoryOpen = false;

	FSlateBrush PanelBrush;
	FSlateBrush SlotBrush;
	FSlateBrush DisabledSlotBrush;
};
