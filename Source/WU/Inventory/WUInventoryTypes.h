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
	FText EquipmentSlotToText(EWUEquipmentSlot Slot);
	FString GetShortItemLabel(const FWUInventoryItem& Item);
}
