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

UENUM(BlueprintType)
enum class EWUItemUseType : uint8
{
	None,
	ClubCharter
};

USTRUCT(BlueprintType)
struct FWUVendorItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Vendor")
	FName VendorTableId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Vendor")
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Vendor")
	int64 PriceKnuts = 0;
};

USTRUCT(BlueprintType)
struct FWUVendorTable
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Vendor")
	FName VendorTableId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Vendor")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Vendor")
	TArray<FWUVendorItem> Items;
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
	EWUItemUseType UseType = EWUItemUseType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory")
	FLinearColor ItemTint = FLinearColor(0.80f, 0.72f, 0.52f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory|Visuals")
	FName IconId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Inventory|Visuals")
	FString IconTexturePath;

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
	constexpr int64 KnutsPerSickle = 29;
	constexpr int64 SicklesPerGalleon = 17;
	constexpr int64 KnutsPerGalleon = KnutsPerSickle * SicklesPerGalleon;
	constexpr int64 ClubCharterPriceKnuts = KnutsPerGalleon + (2 * KnutsPerSickle) + 12;

	const TArray<EWUEquipmentSlot>& GetAllEquipmentSlots();
	const TArray<FName>& GetStarterEquippedItemIds();
	const TArray<FName>& GetStarterInventoryItemIds();
	const TArray<FWUVendorTable>& GetVendorTables();
	const FWUVendorTable* FindVendorTable(FName VendorTableId);
	const FWUVendorItem* FindVendorItem(FName VendorTableId, FName ItemId);
	const FWUInventoryItem* FindItemDefinition(FName ItemId);
	FWUInventoryItem MakeItem(FName ItemId);
	bool IsUsableItem(const FWUInventoryItem& Item);
	FName GetClubCharterItemId();
	FName GetClubVendorTableId();
	FText EquipmentSlotToText(EWUEquipmentSlot Slot);
	FText ItemUseTypeToText(EWUItemUseType UseType);
	FText FormatCurrencyAmountKnuts(int64 AmountKnuts);
	FString GetShortItemLabel(const FWUInventoryItem& Item);
}
