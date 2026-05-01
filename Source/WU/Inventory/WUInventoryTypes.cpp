// Copyright Epic Games, Inc. All Rights Reserved.

#include "Inventory/WUInventoryTypes.h"

#define LOCTEXT_NAMESPACE "WUInventoryTypes"

namespace WUInventory
{
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
