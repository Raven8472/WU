// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WUInventoryTypes.generated.h"

UENUM(BlueprintType)
enum class EWUEquipmentSlot : uint8
{
	Hat,
	ChestRobes,
	Shirt,
	Undershirt,
	Belt,
	Gloves,
	PantsSkirt,
	Shoes,
	Ring1,
	Ring2,
	Nicnak1,
	Nicnak2,
	Earring1,
	Earring2,
	Wand,
	Bracelet1,
	Bracelet2
};

UENUM(BlueprintType)
enum class EWUItemVisualLayer : uint8
{
	None,
	ChestOutfit,
	ChestAddOutfit,
	BeltOutfit,
	Bracers,
	Pants,
	Boots
};

USTRUCT(BlueprintType)
struct FWUInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory")
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory")
	EWUEquipmentSlot EquipmentSlot = EWUEquipmentSlot::Wand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory")
	bool bEquippable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory")
	FLinearColor ItemTint = FLinearColor(0.80f, 0.72f, 0.52f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory|Visuals")
	FName IconId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory|Visuals")
	EWUItemVisualLayer VisualLayer = EWUItemVisualLayer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory|Visuals")
	bool bCoversTorso = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory|Visuals")
	bool bCoversLegs = false;
};

USTRUCT(BlueprintType)
struct FWUInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory")
	bool bHasItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory")
	FWUInventoryItem Item;
};

USTRUCT(BlueprintType)
struct FWUEquipmentSlotEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory")
	EWUEquipmentSlot Slot = EWUEquipmentSlot::Hat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory")
	bool bHasItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory")
	FWUInventoryItem Item;
};

namespace WUInventory
{
	const TArray<EWUEquipmentSlot>& GetAllEquipmentSlots();
	const TArray<FName>& GetStarterEquippedItemIds();
	const TArray<FName>& GetStarterInventoryItemIds();
	const FWUInventoryItem* FindItemDefinition(FName ItemId);
	FWUInventoryItem MakeItem(FName ItemId);
	FText EquipmentSlotToText(EWUEquipmentSlot Slot);
	FString GetShortItemLabel(const FWUInventoryItem& Item);
}
