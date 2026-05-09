// Copyright Epic Games, Inc. All Rights Reserved.

#include "Inventory/WUInventoryTypes.h"

#define LOCTEXT_NAMESPACE "WUInventoryTypes"

namespace WUInventory
{
	namespace
	{
		FWUInventoryItem MakeDefinition(
			FName ItemId,
			const TCHAR* DisplayName,
			EWUEquipmentSlot EquipmentSlot,
			const FLinearColor& ItemTint,
			EWUItemVisualLayer VisualLayer = EWUItemVisualLayer::None,
			bool bCoversTorso = false,
			bool bCoversLegs = false,
			bool bEquippable = true,
			EWUItemUseType UseType = EWUItemUseType::None,
			const TCHAR* IconTexturePath = TEXT(""))
		{
			FWUInventoryItem Item;
			Item.ItemId = ItemId;
			Item.DisplayName = DisplayName;
			Item.EquipmentSlot = EquipmentSlot;
			Item.bEquippable = bEquippable;
			Item.UseType = UseType;
			Item.ItemTint = ItemTint;
			Item.IconId = ItemId;
			Item.IconTexturePath = IconTexturePath;
			Item.VisualLayer = VisualLayer;
			Item.bCoversTorso = bCoversTorso;
			Item.bCoversLegs = bCoversLegs;
			return Item;
		}

		const TArray<FWUInventoryItem>& GetItemDefinitions()
		{
			static const TArray<FWUInventoryItem> Definitions =
			{
				MakeDefinition(
					TEXT("starter_robes"),
					TEXT("First-Year Robes"),
					EWUEquipmentSlot::ChestRobes,
					FLinearColor(0.18f, 0.25f, 0.36f, 1.0f),
					EWUItemVisualLayer::ChestAddOutfit,
					true,
					false),
				MakeDefinition(
					TEXT("starter_shirt"),
					TEXT("Linen Shirt"),
					EWUEquipmentSlot::Shirt,
					FLinearColor(0.82f, 0.78f, 0.68f, 1.0f),
					EWUItemVisualLayer::ChestOutfit,
					true,
					false),
				MakeDefinition(
					TEXT("starter_belt"),
					TEXT("Leather Belt"),
					EWUEquipmentSlot::Belt,
					FLinearColor(0.35f, 0.24f, 0.16f, 1.0f),
					EWUItemVisualLayer::BeltOutfit),
				MakeDefinition(
					TEXT("starter_gloves"),
					TEXT("School Gloves"),
					EWUEquipmentSlot::Gloves,
					FLinearColor(0.30f, 0.28f, 0.25f, 1.0f),
					EWUItemVisualLayer::Bracers),
				MakeDefinition(
					TEXT("starter_pants"),
					TEXT("School Trousers"),
					EWUEquipmentSlot::PantsSkirt,
					FLinearColor(0.34f, 0.36f, 0.32f, 1.0f),
					EWUItemVisualLayer::Pants,
					false,
					true),
				MakeDefinition(
					TEXT("starter_shoes"),
					TEXT("School Shoes"),
					EWUEquipmentSlot::Shoes,
					FLinearColor(0.22f, 0.18f, 0.14f, 1.0f),
					EWUItemVisualLayer::Boots),
				MakeDefinition(
					TEXT("starter_wand"),
					TEXT("Holly Wand"),
					EWUEquipmentSlot::Wand,
					FLinearColor(0.75f, 0.55f, 0.28f, 1.0f)),
				MakeDefinition(
					TEXT("starter_hat"),
					TEXT("Wool Hat"),
					EWUEquipmentSlot::Hat,
					FLinearColor(0.32f, 0.27f, 0.22f, 1.0f)),
				MakeDefinition(
					TEXT("starter_ring"),
					TEXT("Copper Ring"),
					EWUEquipmentSlot::Ring1,
					FLinearColor(0.86f, 0.46f, 0.22f, 1.0f)),
				MakeDefinition(
					TEXT("starter_bracelet"),
					TEXT("Woven Bracelet"),
					EWUEquipmentSlot::Bracelet1,
					FLinearColor(0.40f, 0.58f, 0.34f, 1.0f)),
				MakeDefinition(
					TEXT("starter_nicnak"),
					TEXT("Tiny Nicnak"),
					EWUEquipmentSlot::Nicnak1,
					FLinearColor(0.45f, 0.64f, 0.88f, 1.0f)),
				MakeDefinition(
					GetClubCharterItemId(),
					TEXT("Club Charter"),
					EWUEquipmentSlot::Nicnak1,
					FLinearColor(0.90f, 0.66f, 0.22f, 1.0f),
					EWUItemVisualLayer::None,
					false,
					false,
					false,
					EWUItemUseType::ClubCharter,
					TEXT("/Game/UI/Icons/Items/T_Item_ClubCharter.T_Item_ClubCharter"))
			};

			return Definitions;
		}
	}

	const TArray<EWUEquipmentSlot>& GetAllEquipmentSlots()
	{
		static const TArray<EWUEquipmentSlot> Slots =
		{
			EWUEquipmentSlot::Hat,
			EWUEquipmentSlot::ChestRobes,
			EWUEquipmentSlot::Shirt,
			EWUEquipmentSlot::Undershirt,
			EWUEquipmentSlot::Belt,
			EWUEquipmentSlot::Gloves,
			EWUEquipmentSlot::PantsSkirt,
			EWUEquipmentSlot::Shoes,
			EWUEquipmentSlot::Ring1,
			EWUEquipmentSlot::Ring2,
			EWUEquipmentSlot::Nicnak1,
			EWUEquipmentSlot::Nicnak2,
			EWUEquipmentSlot::Earring1,
			EWUEquipmentSlot::Earring2,
			EWUEquipmentSlot::Wand,
			EWUEquipmentSlot::Bracelet1,
			EWUEquipmentSlot::Bracelet2
		};

		return Slots;
	}

	const TArray<FName>& GetStarterEquippedItemIds()
	{
		static const TArray<FName> ItemIds =
		{
			TEXT("starter_robes"),
			TEXT("starter_shirt"),
			TEXT("starter_belt"),
			TEXT("starter_gloves"),
			TEXT("starter_pants"),
			TEXT("starter_shoes")
		};

		return ItemIds;
	}

	const TArray<FName>& GetStarterInventoryItemIds()
	{
		static const TArray<FName> ItemIds =
		{
			TEXT("starter_wand"),
			TEXT("starter_hat"),
			TEXT("starter_ring"),
			TEXT("starter_bracelet"),
			TEXT("starter_nicnak")
		};

		return ItemIds;
	}

	const TArray<FWUVendorTable>& GetVendorTables()
	{
		static const TArray<FWUVendorTable> VendorTables =
		{
			FWUVendorTable
			{
				GetClubVendorTableId(),
				LOCTEXT("ClubVendorTableName", "Club Vendor"),
				TArray<FWUVendorItem>
				{
					FWUVendorItem
					{
						GetClubVendorTableId(),
						GetClubCharterItemId(),
						ClubCharterPriceKnuts
					}
				}
			}
		};

		return VendorTables;
	}

	const FWUVendorTable* FindVendorTable(FName VendorTableId)
	{
		for (const FWUVendorTable& VendorTable : GetVendorTables())
		{
			if (VendorTable.VendorTableId == VendorTableId)
			{
				return &VendorTable;
			}
		}

		return nullptr;
	}

	const FWUVendorItem* FindVendorItem(FName VendorTableId, FName ItemId)
	{
		if (const FWUVendorTable* VendorTable = FindVendorTable(VendorTableId))
		{
			for (const FWUVendorItem& VendorItem : VendorTable->Items)
			{
				if (VendorItem.ItemId == ItemId)
				{
					return &VendorItem;
				}
			}
		}

		return nullptr;
	}

	const FWUInventoryItem* FindItemDefinition(FName ItemId)
	{
		for (const FWUInventoryItem& Definition : GetItemDefinitions())
		{
			if (Definition.ItemId == ItemId)
			{
				return &Definition;
			}
		}

		return nullptr;
	}

	FWUInventoryItem MakeItem(FName ItemId)
	{
		if (const FWUInventoryItem* Definition = FindItemDefinition(ItemId))
		{
			return *Definition;
		}

		FWUInventoryItem MissingItem;
		MissingItem.ItemId = ItemId;
		MissingItem.DisplayName = ItemId.ToString();
		MissingItem.IconId = ItemId;
		MissingItem.bEquippable = false;
		MissingItem.ItemTint = FLinearColor(0.45f, 0.18f, 0.18f, 1.0f);
		return MissingItem;
	}

	bool IsUsableItem(const FWUInventoryItem& Item)
	{
		return Item.UseType != EWUItemUseType::None;
	}

	FName GetClubCharterItemId()
	{
		return TEXT("club_charter");
	}

	FName GetClubVendorTableId()
	{
		return TEXT("vendor_table_01");
	}

	FText EquipmentSlotToText(EWUEquipmentSlot Slot)
	{
		switch (Slot)
		{
		case EWUEquipmentSlot::Hat:
			return LOCTEXT("HatSlot", "Hat");
		case EWUEquipmentSlot::ChestRobes:
			return LOCTEXT("ChestRobesSlot", "Chest / Robes");
		case EWUEquipmentSlot::Shirt:
			return LOCTEXT("ShirtSlot", "Shirt");
		case EWUEquipmentSlot::Undershirt:
			return LOCTEXT("UndershirtSlot", "Undershirt");
		case EWUEquipmentSlot::Belt:
			return LOCTEXT("BeltSlot", "Belt");
		case EWUEquipmentSlot::Gloves:
			return LOCTEXT("GlovesSlot", "Gloves");
		case EWUEquipmentSlot::PantsSkirt:
			return LOCTEXT("PantsSkirtSlot", "Pants / Skirt");
		case EWUEquipmentSlot::Shoes:
			return LOCTEXT("ShoesSlot", "Shoes");
		case EWUEquipmentSlot::Ring1:
			return LOCTEXT("RingOneSlot", "Ring 1");
		case EWUEquipmentSlot::Ring2:
			return LOCTEXT("RingTwoSlot", "Ring 2");
		case EWUEquipmentSlot::Nicnak1:
			return LOCTEXT("NicnakOneSlot", "Nicnak 1");
		case EWUEquipmentSlot::Nicnak2:
			return LOCTEXT("NicnakTwoSlot", "Nicnak 2");
		case EWUEquipmentSlot::Earring1:
			return LOCTEXT("EarringOneSlot", "Earring 1");
		case EWUEquipmentSlot::Earring2:
			return LOCTEXT("EarringTwoSlot", "Earring 2");
		case EWUEquipmentSlot::Wand:
			return LOCTEXT("WandSlot", "Wand");
		case EWUEquipmentSlot::Bracelet1:
			return LOCTEXT("BraceletOneSlot", "Bracelet 1");
		case EWUEquipmentSlot::Bracelet2:
			return LOCTEXT("BraceletTwoSlot", "Bracelet 2");
		default:
			return LOCTEXT("UnknownSlot", "Unknown");
		}
	}

	FText ItemUseTypeToText(EWUItemUseType UseType)
	{
		switch (UseType)
		{
		case EWUItemUseType::ClubCharter:
			return LOCTEXT("ClubCharterUseType", "Creates a new club");
		case EWUItemUseType::None:
		default:
			return LOCTEXT("NoUseType", "No use");
		}
	}

	FText FormatCurrencyAmountKnuts(int64 AmountKnuts)
	{
		const int64 ClampedAmount = FMath::Max<int64>(0, AmountKnuts);
		const int64 Galleons = ClampedAmount / KnutsPerGalleon;
		const int64 RemainderAfterGalleons = ClampedAmount % KnutsPerGalleon;
		const int64 Sickles = RemainderAfterGalleons / KnutsPerSickle;
		const int64 Knuts = RemainderAfterGalleons % KnutsPerSickle;

		return FText::Format(
			LOCTEXT("CurrencyAmount", "{0} G  {1} S  {2} K"),
			FText::AsNumber(Galleons),
			FText::AsNumber(Sickles),
			FText::AsNumber(Knuts));
	}

	FString GetShortItemLabel(const FWUInventoryItem& Item)
	{
		if (Item.DisplayName.IsEmpty())
		{
			return TEXT("");
		}

		TArray<FString> Words;
		Item.DisplayName.ParseIntoArrayWS(Words);

		FString Result;
		for (const FString& Word : Words)
		{
			if (!Word.IsEmpty())
			{
				Result.AppendChar(FChar::ToUpper(Word[0]));
			}
		}

		if (Result.IsEmpty())
		{
			Result = Item.DisplayName.Left(2).ToUpper();
		}

		if (Result.Len() > 3)
		{
			Result.LeftInline(3);
		}

		return Result;
	}
}

#undef LOCTEXT_NAMESPACE
