// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterCreation/WUCharacterAssetPaths.h"

#define WU_STYLIZED_CHARACTER_ROOT TEXT("/Game/StylizedCharacter")
#define WU_STYLIZED_CHARACTER_PATH(RelativePath) WU_STYLIZED_CHARACTER_ROOT TEXT(RelativePath)

const TCHAR* FWUCharacterAssetPaths::BodyMesh(EWUCharacterSex Sex)
{
	return Sex == EWUCharacterSex::Female
		? WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Female/SK_Hu_F_FullBody.SK_Hu_F_FullBody")
		: WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/SK_Hu_M_FullBody.SK_Hu_M_FullBody");
}

const TCHAR* FWUCharacterAssetPaths::HeadMesh(EWUCharacterSex Sex)
{
	return Sex == EWUCharacterSex::Female
		? WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Female/Base/SK_Hu_F_Head.SK_Hu_F_Head")
		: WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Base/SK_Hu_M_Head.SK_Hu_M_Head");
}

const TCHAR* FWUCharacterAssetPaths::HairMesh(EWUCharacterSex Sex, int32 HairStyleIndex)
{
	static const TCHAR* FemaleHairPaths[] =
	{
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Female/Customization/Hair/SK_Hu_F_Hair_01.SK_Hu_F_Hair_01"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Female/Customization/Hair/SK_Hu_F_Hair_02.SK_Hu_F_Hair_02"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Female/Customization/Hair/SK_Hu_F_Hair_03.SK_Hu_F_Hair_03"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Female/Customization/Hair/SK_Hu_F_Hair_04.SK_Hu_F_Hair_04"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Female/Customization/Hair/SK_Hu_F_Hair_07.SK_Hu_F_Hair_07"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Female/Customization/Hair/SK_Hu_F_Hair_08.SK_Hu_F_Hair_08"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Female/Customization/Hair/SK_Hu_F_Hair_10.SK_Hu_F_Hair_10")
	};

	static const TCHAR* MaleHairPaths[] =
	{
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Hair/SK_Hu_M_Hair_01.SK_Hu_M_Hair_01"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Hair/SK_Hu_M_Hair_02.SK_Hu_M_Hair_02"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Hair/SK_Hu_M_Hair_03.SK_Hu_M_Hair_03"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Hair/SK_Hu_M_Hair_04.SK_Hu_M_Hair_04"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Hair/SK_Hu_M_Hair_09.SK_Hu_M_Hair_09")
	};

	if (Sex == EWUCharacterSex::Female)
	{
		return FemaleHairPaths[NormalizeIndex(HairStyleIndex, UE_ARRAY_COUNT(FemaleHairPaths))];
	}

	return MaleHairPaths[NormalizeIndex(HairStyleIndex, UE_ARRAY_COUNT(MaleHairPaths))];
}

const TCHAR* FWUCharacterAssetPaths::BrowsMesh(EWUCharacterSex Sex, int32 BrowStyleIndex)
{
	if (Sex == EWUCharacterSex::Female)
	{
		return nullptr;
	}

	static const TCHAR* MaleBrowsPaths[] =
	{
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Brows_01.SK_Hu_M_Brows_01"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Brows_02.SK_Hu_M_Brows_02"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Brows_03.SK_Hu_M_Brows_03"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Brows_04.SK_Hu_M_Brows_04")
	};

	return MaleBrowsPaths[NormalizeIndex(BrowStyleIndex, UE_ARRAY_COUNT(MaleBrowsPaths))];
}

const TCHAR* FWUCharacterAssetPaths::BeardMesh(EWUCharacterSex Sex, int32 BeardStyleIndex)
{
	if (Sex == EWUCharacterSex::Female || BeardStyleIndex <= 0)
	{
		return nullptr;
	}

	static const TCHAR* MaleBeardPaths[] =
	{
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_01.SK_Hu_M_Beard_01"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_02.SK_Hu_M_Beard_02"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_03.SK_Hu_M_Beard_03"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_05.SK_Hu_M_Beard_05"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_06.SK_Hu_M_Beard_06"),
		WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_07.SK_Hu_M_Beard_07")
	};

	return MaleBeardPaths[NormalizeIndex(BeardStyleIndex - 1, UE_ARRAY_COUNT(MaleBeardPaths))];
}

const TCHAR* FWUCharacterAssetPaths::PantsMesh(EWUCharacterSex Sex)
{
	return Sex == EWUCharacterSex::Female
		? WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Female/Base/SK_Hu_F_Pants.SK_Hu_F_Pants")
		: WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Base/SK_Hu_M_Pants.SK_Hu_M_Pants");
}

const TCHAR* FWUCharacterAssetPaths::HandsMesh(EWUCharacterSex Sex)
{
	return Sex == EWUCharacterSex::Female
		? WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Female/Base/SK_Hu_F_Hands.SK_Hu_F_Hands")
		: WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Base/SK_Hu_M_Hands.SK_Hu_M_Hands");
}

const TCHAR* FWUCharacterAssetPaths::BracersMesh(EWUCharacterSex Sex)
{
	return Sex == EWUCharacterSex::Female
		? WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Female/Base/SK_Hu_F_Bracers.SK_Hu_F_Bracers")
		: WU_STYLIZED_CHARACTER_PATH("/Meshes/Character/Human/Male/Base/SK_Hu_M_Bracers.SK_Hu_M_Bracers");
}

const TCHAR* FWUCharacterAssetPaths::StarterChestOutfitMesh(EWUCharacterSex Sex)
{
	return Sex == EWUCharacterSex::Female
		? WU_STYLIZED_CHARACTER_PATH("/Meshes/Item/Equipment/Chest/SK_Hu_F_Chest_Peasant_01.SK_Hu_F_Chest_Peasant_01")
		: WU_STYLIZED_CHARACTER_PATH("/Meshes/Item/Equipment/Chest/SK_Hu_M_Chest_Peasant_01.SK_Hu_M_Chest_Peasant_01");
}

const TCHAR* FWUCharacterAssetPaths::StarterChestAddOutfitMesh(EWUCharacterSex Sex)
{
	return Sex == EWUCharacterSex::Female
		? WU_STYLIZED_CHARACTER_PATH("/Meshes/Item/Equipment/ChestAdd/SK_Hu_F_ChestAdd_Peasant.SK_Hu_F_ChestAdd_Peasant")
		: WU_STYLIZED_CHARACTER_PATH("/Meshes/Item/Equipment/ChestAdd/SK_Hu_M_ChestAdd_Peasant.SK_Hu_M_ChestAdd_Peasant");
}

const TCHAR* FWUCharacterAssetPaths::StarterBeltOutfitMesh(EWUCharacterSex Sex)
{
	return Sex == EWUCharacterSex::Female
		? WU_STYLIZED_CHARACTER_PATH("/Meshes/Item/Equipment/Belt/SK_Hu_F_Belt_Peasant.SK_Hu_F_Belt_Peasant")
		: WU_STYLIZED_CHARACTER_PATH("/Meshes/Item/Equipment/Belt/SK_Hu_M_Belt_Peasant.SK_Hu_M_Belt_Peasant");
}

const TCHAR* FWUCharacterAssetPaths::StarterBootsOutfitMesh(EWUCharacterSex Sex)
{
	return Sex == EWUCharacterSex::Female
		? WU_STYLIZED_CHARACTER_PATH("/Meshes/Item/Equipment/Boots/SK_Hu_F_Boots_Peasant.SK_Hu_F_Boots_Peasant")
		: WU_STYLIZED_CHARACTER_PATH("/Meshes/Item/Equipment/Boots/SK_Hu_M_Boots_Peasant.SK_Hu_M_Boots_Peasant");
}

const TCHAR* FWUCharacterAssetPaths::BodyMaterial(EWUCharacterSex Sex, int32 SkinPresetIndex)
{
	static const TCHAR* FemaleBodyMaterialPaths[] =
	{
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_01.MI_Hu_F_Body_01"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_02.MI_Hu_F_Body_02"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_03.MI_Hu_F_Body_03"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_04.MI_Hu_F_Body_04"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_05.MI_Hu_F_Body_05")
	};

	static const TCHAR* MaleBodyMaterialPaths[] =
	{
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_01.MI_Hu_M_Body_01"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_02.MI_Hu_M_Body_02"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_03.MI_Hu_M_Body_03"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_04.MI_Hu_M_Body_04"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_05.MI_Hu_M_Body_05")
	};

	if (Sex == EWUCharacterSex::Female)
	{
		return FemaleBodyMaterialPaths[NormalizeIndex(SkinPresetIndex, UE_ARRAY_COUNT(FemaleBodyMaterialPaths))];
	}

	return MaleBodyMaterialPaths[NormalizeIndex(SkinPresetIndex, UE_ARRAY_COUNT(MaleBodyMaterialPaths))];
}

const TCHAR* FWUCharacterAssetPaths::HeadMaterial(EWUCharacterSex Sex, int32 HeadPresetIndex)
{
	static const TCHAR* FemaleHeadMaterialPaths[] =
	{
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_01_A.MI_Hu_F_Head_01_A"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_02_A.MI_Hu_F_Head_02_A"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_03_A.MI_Hu_F_Head_03_A"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_04_A.MI_Hu_F_Head_04_A"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_05_A.MI_Hu_F_Head_05_A")
	};

	static const TCHAR* MaleHeadMaterialPaths[] =
	{
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_01_A.MI_Hu_M_Head_01_A"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_02_A.MI_Hu_M_Head_02_A"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_03_A.MI_Hu_M_Head_03_A"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_04_A.MI_Hu_M_Head_04_A"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_05_A.MI_Hu_M_Head_05_A")
	};

	if (Sex == EWUCharacterSex::Female)
	{
		return FemaleHeadMaterialPaths[NormalizeIndex(HeadPresetIndex, UE_ARRAY_COUNT(FemaleHeadMaterialPaths))];
	}

	return MaleHeadMaterialPaths[NormalizeIndex(HeadPresetIndex, UE_ARRAY_COUNT(MaleHeadMaterialPaths))];
}

const TCHAR* FWUCharacterAssetPaths::EyeMaterial(int32 EyeColorIndex)
{
	static const TCHAR* EyeMaterialPaths[] =
	{
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Eye/MI_HU_Eye_Bl.MI_HU_Eye_Bl"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Eye/MI_HU_Eye_Br.MI_HU_Eye_Br"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Eye/MI_HU_Eye_Gn.MI_HU_Eye_Gn"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Eye/MI_HU_Eye_Pe.MI_HU_Eye_Pe")
	};

	return EyeMaterialPaths[NormalizeIndex(EyeColorIndex, UE_ARRAY_COUNT(EyeMaterialPaths))];
}

const TCHAR* FWUCharacterAssetPaths::HairMaterial(int32 HairColorIndex)
{
	static const TCHAR* HairMaterialPaths[] =
	{
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Hair/MI_HU_Hair_01_Bk.MI_HU_Hair_01_Bk"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Hair/MI_HU_Hair_01_Bd.MI_HU_Hair_01_Bd"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Hair/MI_HU_Hair_01_Br.MI_HU_Hair_01_Br"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Hair/MI_HU_Hair_01_Gr.MI_HU_Hair_01_Gr")
	};

	return HairMaterialPaths[NormalizeIndex(HairColorIndex, UE_ARRAY_COUNT(HairMaterialPaths))];
}

const TCHAR* FWUCharacterAssetPaths::FacialsMaterial(EWUCharacterSex Sex, int32 HairColorIndex)
{
	static const TCHAR* FemaleFacialsMaterialPaths[] =
	{
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Female/Facials/MI_Hu_F_Facials_Bk.MI_Hu_F_Facials_Bk"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Female/Facials/MI_Hu_F_Facials_Bd.MI_Hu_F_Facials_Bd"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Female/Facials/MI_Hu_F_Facials_Br.MI_Hu_F_Facials_Br"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Female/Facials/MI_Hu_F_Facials_Gr.MI_Hu_F_Facials_Gr")
	};

	static const TCHAR* MaleFacialsMaterialPaths[] =
	{
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Male/Facials/MI_Hu_M_Facials_Bk.MI_Hu_M_Facials_Bk"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Male/Facials/MI_Hu_M_Facials_Bd.MI_Hu_M_Facials_Bd"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Male/Facials/MI_Hu_M_Facials_Br.MI_Hu_M_Facials_Br"),
		WU_STYLIZED_CHARACTER_PATH("/Materials/Instances/Character/Human/Male/Facials/MI_Hu_M_Facials_Gr.MI_Hu_M_Facials_Gr")
	};

	if (Sex == EWUCharacterSex::Female)
	{
		return FemaleFacialsMaterialPaths[NormalizeIndex(HairColorIndex, UE_ARRAY_COUNT(FemaleFacialsMaterialPaths))];
	}

	return MaleFacialsMaterialPaths[NormalizeIndex(HairColorIndex, UE_ARRAY_COUNT(MaleFacialsMaterialPaths))];
}

const TCHAR* FWUCharacterAssetPaths::UnderhairTexture(EWUCharacterSex Sex, int32 HairColorIndex)
{
	static const TCHAR* FemaleUnderhairTexturePaths[] =
	{
		WU_STYLIZED_CHARACTER_PATH("/Textures/Character/Human/Female/Head/T_HU_F_Head_UH_Bk_D.T_HU_F_Head_UH_Bk_D"),
		WU_STYLIZED_CHARACTER_PATH("/Textures/Character/Human/Female/Head/T_HU_F_Head_UH_Bd_D.T_HU_F_Head_UH_Bd_D"),
		WU_STYLIZED_CHARACTER_PATH("/Textures/Character/Human/Female/Head/T_HU_F_Head_UH_Br_D.T_HU_F_Head_UH_Br_D"),
		WU_STYLIZED_CHARACTER_PATH("/Textures/Character/Human/Female/Head/T_HU_F_Head_UH_Gr_D.T_HU_F_Head_UH_Gr_D")
	};

	static const TCHAR* MaleUnderhairTexturePaths[] =
	{
		WU_STYLIZED_CHARACTER_PATH("/Textures/Character/Human/Male/Head/T_HU_M_Head_UH_Bk_D.T_HU_M_Head_UH_Bk_D"),
		WU_STYLIZED_CHARACTER_PATH("/Textures/Character/Human/Male/Head/T_HU_M_Head_UH_Bd_D.T_HU_M_Head_UH_Bd_D"),
		WU_STYLIZED_CHARACTER_PATH("/Textures/Character/Human/Male/Head/T_HU_M_Head_UH_Br_D.T_HU_M_Head_UH_Br_D"),
		WU_STYLIZED_CHARACTER_PATH("/Textures/Character/Human/Male/Head/T_HU_M_Head_UH_Gr_D.T_HU_M_Head_UH_Gr_D")
	};

	if (Sex == EWUCharacterSex::Female)
	{
		return FemaleUnderhairTexturePaths[NormalizeIndex(HairColorIndex, UE_ARRAY_COUNT(FemaleUnderhairTexturePaths))];
	}

	return MaleUnderhairTexturePaths[NormalizeIndex(HairColorIndex, UE_ARRAY_COUNT(MaleUnderhairTexturePaths))];
}

const TCHAR* FWUCharacterAssetPaths::AnimationBlueprint(EWUCharacterSex Sex)
{
	return Sex == EWUCharacterSex::Female
		? WU_STYLIZED_CHARACTER_PATH("/Animations/Character/Human/Female/ABP_Hu_F.ABP_Hu_F_C")
		: WU_STYLIZED_CHARACTER_PATH("/Animations/Character/Human/Male/ABP_Hu_M.ABP_Hu_M_C");
}

const TCHAR* FWUCharacterAssetPaths::BackpedalAnimation(EWUCharacterSex Sex, float Right)
{
	if (Sex == EWUCharacterSex::Female)
	{
		if (Right < -0.2f)
		{
			return WU_STYLIZED_CHARACTER_PATH("/Animations/Character/Human/Female/A_Hu_F_Walk_BackwardLeft.A_Hu_F_Walk_BackwardLeft");
		}

		if (Right > 0.2f)
		{
			return WU_STYLIZED_CHARACTER_PATH("/Animations/Character/Human/Female/A_Hu_F_Walk_BackwardRight.A_Hu_F_Walk_BackwardRight");
		}

		return WU_STYLIZED_CHARACTER_PATH("/Animations/Character/Human/Female/A_Hu_F_Walk_Backwards.A_Hu_F_Walk_Backwards");
	}

	if (Right < -0.2f)
	{
		return WU_STYLIZED_CHARACTER_PATH("/Animations/Character/Human/Male/A_Hu_M_Walk_BackwardLeft.A_Hu_M_Walk_BackwardLeft");
	}

	if (Right > 0.2f)
	{
		return WU_STYLIZED_CHARACTER_PATH("/Animations/Character/Human/Male/A_Hu_M_Walk_BackwardRight.A_Hu_M_Walk_BackwardRight");
	}

	return WU_STYLIZED_CHARACTER_PATH("/Animations/Character/Human/Male/A_Hu_M_Walk_Backwards.A_Hu_M_Walk_Backwards");
}

const TCHAR* FWUCharacterAssetPaths::TurnInPlaceAnimation(EWUCharacterSex Sex, float YawDeltaDegrees)
{
	const bool bTurnRight = YawDeltaDegrees > 0.0f;

	if (Sex == EWUCharacterSex::Female)
	{
		return bTurnRight
			? WU_STYLIZED_CHARACTER_PATH("/Animations/Character/Human/Female/A_Hu_F_Idle_StepRight.A_Hu_F_Idle_StepRight")
			: WU_STYLIZED_CHARACTER_PATH("/Animations/Character/Human/Female/A_Hu_F_Idle_StepLeft.A_Hu_F_Idle_StepLeft");
	}

	return bTurnRight
		? WU_STYLIZED_CHARACTER_PATH("/Animations/Character/Human/Male/A_Hu_M_Idle_StepRight.A_Hu_M_Idle_StepRight")
		: WU_STYLIZED_CHARACTER_PATH("/Animations/Character/Human/Male/A_Hu_M_Idle_StepLeft.A_Hu_M_Idle_StepLeft");
}

int32 FWUCharacterAssetPaths::NormalizeIndex(int32 Index, int32 Count)
{
	if (Count <= 0)
	{
		return 0;
	}

	const int32 Mod = Index % Count;
	return Mod < 0 ? Mod + Count : Mod;
}

#undef WU_STYLIZED_CHARACTER_PATH
#undef WU_STYLIZED_CHARACTER_ROOT
