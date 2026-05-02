// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterCreation/WUCharacterCreatorPreviewActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"

AWUCharacterCreatorPreviewActor::AWUCharacterCreatorPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BodyMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	BodyMeshComponent->SetupAttachment(SceneRoot);
	BodyMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMeshComponent->SetGenerateOverlapEvents(false);

	HeadMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
	HeadMeshComponent->SetupAttachment(BodyMeshComponent);
	ConfigureModularMeshComponent(HeadMeshComponent);

	HairMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));
	HairMeshComponent->SetupAttachment(BodyMeshComponent);
	ConfigureModularMeshComponent(HairMeshComponent);

	BrowsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BrowsMesh"));
	BrowsMeshComponent->SetupAttachment(BodyMeshComponent);
	ConfigureModularMeshComponent(BrowsMeshComponent);

	BeardMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BeardMesh"));
	BeardMeshComponent->SetupAttachment(BodyMeshComponent);
	ConfigureModularMeshComponent(BeardMeshComponent);

	PantsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PantsMesh"));
	PantsMeshComponent->SetupAttachment(BodyMeshComponent);
	ConfigureModularMeshComponent(PantsMeshComponent);

	HandsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandsMesh"));
	HandsMeshComponent->SetupAttachment(BodyMeshComponent);
	ConfigureModularMeshComponent(HandsMeshComponent);

	BracersMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BracersMesh"));
	BracersMeshComponent->SetupAttachment(BodyMeshComponent);
	ConfigureModularMeshComponent(BracersMeshComponent);

	ChestOutfitMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestOutfitMesh"));
	ChestOutfitMeshComponent->SetupAttachment(BodyMeshComponent);
	ConfigureModularMeshComponent(ChestOutfitMeshComponent);

	ChestAddOutfitMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestAddOutfitMesh"));
	ChestAddOutfitMeshComponent->SetupAttachment(BodyMeshComponent);
	ConfigureModularMeshComponent(ChestAddOutfitMeshComponent);

	BeltOutfitMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BeltOutfitMesh"));
	BeltOutfitMeshComponent->SetupAttachment(BodyMeshComponent);
	ConfigureModularMeshComponent(BeltOutfitMeshComponent);

	BootsOutfitMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BootsOutfitMesh"));
	BootsOutfitMeshComponent->SetupAttachment(BodyMeshComponent);
	ConfigureModularMeshComponent(BootsOutfitMeshComponent);
}

void AWUCharacterCreatorPreviewActor::ApplyCreateRequest(const FWUCharacterCreateRequest& Request)
{
	if (USkeletalMesh* BodyMesh = LoadSkeletalMeshForPath(GetBodyMeshPath(Request.Sex)))
	{
		BodyMeshComponent->SetSkeletalMesh(BodyMesh);
	}

	BodyMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	BodyMeshComponent->SetVisibility(false, false);
	BodyMeshComponent->SetHiddenInGame(true, false);

	const auto ShowModularMesh = [](USkeletalMeshComponent* MeshComponent)
	{
		if (!MeshComponent)
		{
			return;
		}

		MeshComponent->SetVisibility(true, false);
		MeshComponent->SetHiddenInGame(false, false);
	};

	const auto HideModularMesh = [](USkeletalMeshComponent* MeshComponent)
	{
		if (!MeshComponent)
		{
			return;
		}

		MeshComponent->SetVisibility(false, false);
		MeshComponent->SetHiddenInGame(true, false);
	};

	if (USkeletalMesh* HeadMesh = LoadSkeletalMeshForPath(GetHeadMeshPath(Request.Sex)))
	{
		HeadMeshComponent->SetSkeletalMesh(HeadMesh);
		HeadMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(HeadMeshComponent);
	}

	if (USkeletalMesh* HairMesh = LoadSkeletalMeshForPath(GetHairMeshPath(Request.Sex, Request.HairStyleIndex)))
	{
		HairMeshComponent->SetSkeletalMesh(HairMesh);
		HairMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(HairMeshComponent);
	}
	else
	{
		HideModularMesh(HairMeshComponent);
	}

	if (USkeletalMesh* BrowsMesh = LoadSkeletalMeshForPath(GetBrowsMeshPath(Request.Sex, Request.BrowStyleIndex)))
	{
		BrowsMeshComponent->SetSkeletalMesh(BrowsMesh);
		BrowsMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(BrowsMeshComponent);
	}
	else
	{
		HideModularMesh(BrowsMeshComponent);
	}

	if (USkeletalMesh* BeardMesh = LoadSkeletalMeshForPath(GetBeardMeshPath(Request.Sex, Request.BeardStyleIndex)))
	{
		BeardMeshComponent->SetSkeletalMesh(BeardMesh);
		BeardMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(BeardMeshComponent);
	}
	else
	{
		HideModularMesh(BeardMeshComponent);
	}

	if (USkeletalMesh* PantsMesh = LoadSkeletalMeshForPath(GetPantsMeshPath(Request.Sex)))
	{
		PantsMeshComponent->SetSkeletalMesh(PantsMesh);
		PantsMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(PantsMeshComponent);
	}

	if (USkeletalMesh* HandsMesh = LoadSkeletalMeshForPath(GetHandsMeshPath(Request.Sex)))
	{
		HandsMeshComponent->SetSkeletalMesh(HandsMesh);
		HandsMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(HandsMeshComponent);
	}

	if (USkeletalMesh* BracersMesh = LoadSkeletalMeshForPath(GetBracersMeshPath(Request.Sex)))
	{
		BracersMeshComponent->SetSkeletalMesh(BracersMesh);
		BracersMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(BracersMeshComponent);
	}

	if (USkeletalMesh* ChestOutfitMesh = LoadSkeletalMeshForPath(GetStarterChestOutfitMeshPath(Request.Sex)))
	{
		ChestOutfitMeshComponent->SetSkeletalMesh(ChestOutfitMesh);
		ChestOutfitMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(ChestOutfitMeshComponent);
	}

	if (USkeletalMesh* ChestAddOutfitMesh = LoadSkeletalMeshForPath(GetStarterChestAddOutfitMeshPath(Request.Sex)))
	{
		ChestAddOutfitMeshComponent->SetSkeletalMesh(ChestAddOutfitMesh);
		ChestAddOutfitMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(ChestAddOutfitMeshComponent);
	}

	if (USkeletalMesh* BeltOutfitMesh = LoadSkeletalMeshForPath(GetStarterBeltOutfitMeshPath(Request.Sex)))
	{
		BeltOutfitMeshComponent->SetSkeletalMesh(BeltOutfitMesh);
		BeltOutfitMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(BeltOutfitMeshComponent);
	}

	if (USkeletalMesh* BootsOutfitMesh = LoadSkeletalMeshForPath(GetStarterBootsOutfitMeshPath(Request.Sex)))
	{
		BootsOutfitMeshComponent->SetSkeletalMesh(BootsOutfitMesh);
		BootsOutfitMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(BootsOutfitMeshComponent);
	}

	if (UMaterialInterface* BodyMaterial = LoadMaterialForPath(GetBodyMaterialPath(Request.Sex, Request.SkinPresetIndex)))
	{
		const int32 MaterialCount = BodyMeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			BodyMeshComponent->SetMaterial(MaterialIndex, BodyMaterial);
		}

		const int32 HandsMaterialCount = HandsMeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < HandsMaterialCount; ++MaterialIndex)
		{
			HandsMeshComponent->SetMaterial(MaterialIndex, BodyMaterial);
		}

		const int32 BracersMaterialCount = BracersMeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < BracersMaterialCount; ++MaterialIndex)
		{
			BracersMeshComponent->SetMaterial(MaterialIndex, BodyMaterial);
		}

		const int32 ChestBodyMaterialIndex = ChestOutfitMeshComponent->GetMaterialIndex(TEXT("M_Body"));
		if (ChestBodyMaterialIndex != INDEX_NONE)
		{
			ChestOutfitMeshComponent->SetMaterial(ChestBodyMaterialIndex, BodyMaterial);
		}
	}

	if (UMaterialInterface* HeadMaterial = LoadMaterialForPath(GetHeadMaterialPath(Request.Sex, Request.HeadPresetIndex)))
	{
		bool bAppliedHeadMaterial = false;
		const TArray<FName> MaterialSlotNames = HeadMeshComponent->GetMaterialSlotNames();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotNames.Num(); ++MaterialIndex)
		{
			const FString SlotName = MaterialSlotNames[MaterialIndex].ToString();
			if (!SlotName.Contains(TEXT("Eye"), ESearchCase::IgnoreCase)
				&& (SlotName.Contains(TEXT("Head"), ESearchCase::IgnoreCase)
					|| SlotName.Contains(TEXT("Face"), ESearchCase::IgnoreCase)
					|| SlotName.Contains(TEXT("Skin"), ESearchCase::IgnoreCase)))
			{
				HeadMeshComponent->SetMaterial(MaterialIndex, HeadMaterial);
				bAppliedHeadMaterial = true;
			}
		}

		if (!bAppliedHeadMaterial && HeadMeshComponent->GetNumMaterials() > 0)
		{
			HeadMeshComponent->SetMaterial(0, HeadMaterial);
		}
	}

	if (UMaterialInterface* EyeMaterial = LoadMaterialForPath(GetEyeMaterialPath(Request.EyeColorIndex)))
	{
		bool bAppliedEyeMaterial = false;
		const TArray<FName> MaterialSlotNames = HeadMeshComponent->GetMaterialSlotNames();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotNames.Num(); ++MaterialIndex)
		{
			const FString SlotName = MaterialSlotNames[MaterialIndex].ToString();
			if (SlotName.Contains(TEXT("Eye"), ESearchCase::IgnoreCase))
			{
				HeadMeshComponent->SetMaterial(MaterialIndex, EyeMaterial);
				bAppliedEyeMaterial = true;
			}
		}

		if (!bAppliedEyeMaterial && HeadMeshComponent->GetNumMaterials() > 1)
		{
			HeadMeshComponent->SetMaterial(1, EyeMaterial);
		}
	}

	if (UMaterialInterface* HairMaterial = LoadMaterialForPath(GetHairMaterialPath(Request.HairColorIndex)))
	{
		const int32 MaterialCount = HairMeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			HairMeshComponent->SetMaterial(MaterialIndex, HairMaterial);
		}

		const int32 BrowsMaterialCount = BrowsMeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < BrowsMaterialCount; ++MaterialIndex)
		{
			BrowsMeshComponent->SetMaterial(MaterialIndex, HairMaterial);
		}

		const int32 BeardMaterialCount = BeardMeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < BeardMaterialCount; ++MaterialIndex)
		{
			BeardMeshComponent->SetMaterial(MaterialIndex, HairMaterial);
		}
	}
}

void AWUCharacterCreatorPreviewActor::RotatePreview(float YawDelta)
{
	AddActorLocalRotation(FRotator(0.0f, YawDelta, 0.0f));
}

void AWUCharacterCreatorPreviewActor::ConfigureModularMeshComponent(USkeletalMeshComponent* MeshComponent) const
{
	if (!MeshComponent)
	{
		return;
	}

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
}

USkeletalMesh* AWUCharacterCreatorPreviewActor::LoadSkeletalMeshForPath(const TCHAR* AssetPath) const
{
	if (!AssetPath)
	{
		return nullptr;
	}

	return LoadObject<USkeletalMesh>(nullptr, AssetPath);
}

UMaterialInterface* AWUCharacterCreatorPreviewActor::LoadMaterialForPath(const TCHAR* AssetPath) const
{
	if (!AssetPath)
	{
		return nullptr;
	}

	return LoadObject<UMaterialInterface>(nullptr, AssetPath);
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetBodyMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/SK_Hu_F_FullBody.SK_Hu_F_FullBody")
		: TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/SK_Hu_M_FullBody.SK_Hu_M_FullBody");
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetHeadMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Base/SK_Hu_F_Head.SK_Hu_F_Head")
		: TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Base/SK_Hu_M_Head.SK_Hu_M_Head");
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetHairMeshPath(EWUCharacterSex Sex, int32 HairStyleIndex) const
{
	static const TCHAR* FemaleHairPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Customization/Hair/SK_Hu_F_Hair_01.SK_Hu_F_Hair_01"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Customization/Hair/SK_Hu_F_Hair_02.SK_Hu_F_Hair_02"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Customization/Hair/SK_Hu_F_Hair_03.SK_Hu_F_Hair_03"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Customization/Hair/SK_Hu_F_Hair_04.SK_Hu_F_Hair_04"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Customization/Hair/SK_Hu_F_Hair_07.SK_Hu_F_Hair_07"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Customization/Hair/SK_Hu_F_Hair_08.SK_Hu_F_Hair_08"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Customization/Hair/SK_Hu_F_Hair_10.SK_Hu_F_Hair_10")
	};

	static const TCHAR* MaleHairPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Hair/SK_Hu_M_Hair_01.SK_Hu_M_Hair_01"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Hair/SK_Hu_M_Hair_02.SK_Hu_M_Hair_02"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Hair/SK_Hu_M_Hair_03.SK_Hu_M_Hair_03"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Hair/SK_Hu_M_Hair_04.SK_Hu_M_Hair_04"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Hair/SK_Hu_M_Hair_09.SK_Hu_M_Hair_09")
	};

	if (Sex == EWUCharacterSex::Female)
	{
		return FemaleHairPaths[NormalizeIndex(HairStyleIndex, UE_ARRAY_COUNT(FemaleHairPaths))];
	}

	return MaleHairPaths[NormalizeIndex(HairStyleIndex, UE_ARRAY_COUNT(MaleHairPaths))];
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetBrowsMeshPath(EWUCharacterSex Sex, int32 BrowStyleIndex) const
{
	if (Sex == EWUCharacterSex::Female)
	{
		return nullptr;
	}

	static const TCHAR* MaleBrowsPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Brows_01.SK_Hu_M_Brows_01"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Brows_02.SK_Hu_M_Brows_02"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Brows_03.SK_Hu_M_Brows_03"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Brows_04.SK_Hu_M_Brows_04")
	};

	return MaleBrowsPaths[NormalizeIndex(BrowStyleIndex, UE_ARRAY_COUNT(MaleBrowsPaths))];
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetBeardMeshPath(EWUCharacterSex Sex, int32 BeardStyleIndex) const
{
	if (Sex == EWUCharacterSex::Female || BeardStyleIndex <= 0)
	{
		return nullptr;
	}

	static const TCHAR* MaleBeardPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_01.SK_Hu_M_Beard_01"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_02.SK_Hu_M_Beard_02"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_03.SK_Hu_M_Beard_03"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_05.SK_Hu_M_Beard_05"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_06.SK_Hu_M_Beard_06"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_07.SK_Hu_M_Beard_07")
	};

	return MaleBeardPaths[NormalizeIndex(BeardStyleIndex - 1, UE_ARRAY_COUNT(MaleBeardPaths))];
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetPantsMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Base/SK_Hu_F_Pants.SK_Hu_F_Pants")
		: TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Base/SK_Hu_M_Pants.SK_Hu_M_Pants");
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetHandsMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Base/SK_Hu_F_Hands.SK_Hu_F_Hands")
		: TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Base/SK_Hu_M_Hands.SK_Hu_M_Hands");
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetBracersMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Base/SK_Hu_F_Bracers.SK_Hu_F_Bracers")
		: TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Base/SK_Hu_M_Bracers.SK_Hu_M_Bracers");
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetStarterChestOutfitMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/Chest/SK_Hu_F_Chest_Peasant_01.SK_Hu_F_Chest_Peasant_01")
		: TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/Chest/SK_Hu_M_Chest_Peasant_01.SK_Hu_M_Chest_Peasant_01");
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetStarterChestAddOutfitMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/ChestAdd/SK_Hu_F_ChestAdd_Peasant.SK_Hu_F_ChestAdd_Peasant")
		: TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/ChestAdd/SK_Hu_M_ChestAdd_Peasant.SK_Hu_M_ChestAdd_Peasant");
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetStarterBeltOutfitMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/Belt/SK_Hu_F_Belt_Peasant.SK_Hu_F_Belt_Peasant")
		: TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/Belt/SK_Hu_M_Belt_Peasant.SK_Hu_M_Belt_Peasant");
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetStarterBootsOutfitMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/Boots/SK_Hu_F_Boots_Peasant.SK_Hu_F_Boots_Peasant")
		: TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/Boots/SK_Hu_M_Boots_Peasant.SK_Hu_M_Boots_Peasant");
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetBodyMaterialPath(EWUCharacterSex Sex, int32 SkinPresetIndex) const
{
	static const TCHAR* FemaleBodyMaterialPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_01.MI_Hu_F_Body_01"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_02.MI_Hu_F_Body_02"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_03.MI_Hu_F_Body_03"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_04.MI_Hu_F_Body_04"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_05.MI_Hu_F_Body_05")
	};

	static const TCHAR* MaleBodyMaterialPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_01.MI_Hu_M_Body_01"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_02.MI_Hu_M_Body_02"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_03.MI_Hu_M_Body_03"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_04.MI_Hu_M_Body_04"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_05.MI_Hu_M_Body_05")
	};

	if (Sex == EWUCharacterSex::Female)
	{
		return FemaleBodyMaterialPaths[NormalizeIndex(SkinPresetIndex, UE_ARRAY_COUNT(FemaleBodyMaterialPaths))];
	}

	return MaleBodyMaterialPaths[NormalizeIndex(SkinPresetIndex, UE_ARRAY_COUNT(MaleBodyMaterialPaths))];
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetHeadMaterialPath(EWUCharacterSex Sex, int32 HeadPresetIndex) const
{
	static const TCHAR* FemaleHeadMaterialPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_01_A.MI_Hu_F_Head_01_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_02_A.MI_Hu_F_Head_02_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_03_A.MI_Hu_F_Head_03_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_04_A.MI_Hu_F_Head_04_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_05_A.MI_Hu_F_Head_05_A")
	};

	static const TCHAR* MaleHeadMaterialPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_01_A.MI_Hu_M_Head_01_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_02_A.MI_Hu_M_Head_02_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_03_A.MI_Hu_M_Head_03_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_04_A.MI_Hu_M_Head_04_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_05_A.MI_Hu_M_Head_05_A")
	};

	if (Sex == EWUCharacterSex::Female)
	{
		return FemaleHeadMaterialPaths[NormalizeIndex(HeadPresetIndex, UE_ARRAY_COUNT(FemaleHeadMaterialPaths))];
	}

	return MaleHeadMaterialPaths[NormalizeIndex(HeadPresetIndex, UE_ARRAY_COUNT(MaleHeadMaterialPaths))];
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetEyeMaterialPath(int32 EyeColorIndex) const
{
	static const TCHAR* EyeMaterialPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Eye/MI_HU_Eye_Bl.MI_HU_Eye_Bl"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Eye/MI_HU_Eye_Br.MI_HU_Eye_Br"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Eye/MI_HU_Eye_Gn.MI_HU_Eye_Gn"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Eye/MI_HU_Eye_Pe.MI_HU_Eye_Pe")
	};

	return EyeMaterialPaths[NormalizeIndex(EyeColorIndex, UE_ARRAY_COUNT(EyeMaterialPaths))];
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetHairMaterialPath(int32 HairColorIndex) const
{
	static const TCHAR* HairMaterialPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Hair/MI_HU_Hair_01_Bk.MI_HU_Hair_01_Bk"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Hair/MI_HU_Hair_01_Bd.MI_HU_Hair_01_Bd"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Hair/MI_HU_Hair_01_Br.MI_HU_Hair_01_Br"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Hair/MI_HU_Hair_01_Gr.MI_HU_Hair_01_Gr")
	};

	return HairMaterialPaths[NormalizeIndex(HairColorIndex, UE_ARRAY_COUNT(HairMaterialPaths))];
}

int32 AWUCharacterCreatorPreviewActor::NormalizeIndex(int32 Index, int32 Count) const
{
	if (Count <= 0)
	{
		return 0;
	}

	const int32 Mod = Index % Count;
	return Mod < 0 ? Mod + Count : Mod;
}
