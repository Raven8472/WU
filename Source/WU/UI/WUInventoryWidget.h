// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Backend/WUClientSessionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/WUInventoryTypes.h"
#include "Styling/SlateBrush.h"
#include "WUInventoryWidget.generated.h"

class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWUInventoryItemUseRequestedSignature, int32, SlotIndex, FWUInventoryItem, Item);

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

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FVector2D GetDesiredInventoryPanelSize() const;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FWUInventoryItemUseRequestedSignature OnItemUseRequested;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 TotalBagSlots = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 StartingBagCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 SlotsPerBag = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D InventorySize = FVector2D(620.0f, 420.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D InventorySlotSize = FVector2D(42.0f, 42.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D BagSlotSize = FVector2D(42.0f, 42.0f);

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

	int32 GetVisibleBagSectionCount() const;
	EVisibility GetInventoryVisibility() const;
	bool IsBagSlotUnlocked(int32 BagSlotIndex) const;
	TSharedRef<SWidget> CreateBagSlot(int32 BagSlotIndex) const;
	TSharedRef<SWidget> CreateCurrencySection() const;
	TSharedRef<SWidget> CreateBagSection(int32 BagIndex);
	TSharedRef<SWidget> CreateInventorySlot(int32 AbsoluteSlotIndex);
	FText GetCarriedCurrencyText() const;
	FText GetCurrencyTooltipText() const;
	FText GetInventorySlotText(int32 AbsoluteSlotIndex) const;
	FText GetInventorySlotTooltipText(int32 AbsoluteSlotIndex) const;
	FSlateColor GetInventorySlotTextColor(int32 AbsoluteSlotIndex) const;
	FLinearColor GetInventorySlotTint(int32 AbsoluteSlotIndex) const;
	const FSlateBrush* GetInventorySlotIconBrush(int32 AbsoluteSlotIndex);
	EVisibility GetInventorySlotIconVisibility(int32 AbsoluteSlotIndex);
	EVisibility GetInventorySlotTextVisibility(int32 AbsoluteSlotIndex);
	FReply HandleInventorySlotClicked(int32 AbsoluteSlotIndex);

	void ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin = FMargin(0.0f));
	void RequestCurrencySnapshot() const;
	UWUClientSessionSubsystem* GetSessionSubsystem() const;

	UFUNCTION()
	void HandleCurrencySnapshotLoaded(const FWUBackendCurrencySnapshot& Snapshot);

private:

	bool bInventoryOpen = false;

	FSlateBrush PanelBrush;
	FSlateBrush SlotBrush;
	FSlateBrush DisabledSlotBrush;
	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<UTexture2D>> IconTextureCache;
	TMap<FString, FSlateBrush> IconBrushCache;
};
