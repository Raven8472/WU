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
	HeadMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadMeshComponent->SetGenerateOverlapEvents(false);
	HeadMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);

	HairMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));
	HairMeshComponent->SetupAttachment(BodyMeshComponent);
	HairMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HairMeshComponent->SetGenerateOverlapEvents(false);
	HairMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);

	BrowsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BrowsMesh"));
	BrowsMeshComponent->SetupAttachment(BodyMeshComponent);
	BrowsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BrowsMeshComponent->SetGenerateOverlapEvents(false);
	BrowsMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);

	BeardMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BeardMesh"));
	BeardMeshComponent->SetupAttachment(BodyMeshComponent);
	BeardMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeardMeshComponent->SetGenerateOverlapEvents(false);
	BeardMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
}

void AWUCharacterCreatorPreviewActor::ApplyCreateRequest(const FWUCharacterCreateRequest& Request)
{
	if (USkeletalMesh* BodyMesh = LoadSkeletalMeshForPath(GetBodyMeshPath(Request.Sex)))
	{
		BodyMeshComponent->SetSkeletalMesh(BodyMesh);
	}

	if (USkeletalMesh* HeadMesh = LoadSkeletalMeshForPath(GetHeadMeshPath(Request.Sex)))
	{
		HeadMeshComponent->SetSkeletalMesh(HeadMesh);
		HeadMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		HeadMeshComponent->SetVisibility(true);
	}

	if (USkeletalMesh* HairMesh = LoadSkeletalMeshForPath(GetHairMeshPath(Request.Sex, Request.HairStyleIndex)))
	{
		HairMeshComponent->SetSkeletalMesh(HairMesh);
		HairMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		HairMeshComponent->SetVisibility(true);
	}
	else
	{
		HairMeshComponent->SetVisibility(false);
	}

	if (USkeletalMesh* BrowsMesh = LoadSkeletalMeshForPath(GetBrowsMeshPath(Request.Sex, Request.BrowStyleIndex)))
	{
		BrowsMeshComponent->SetSkeletalMesh(BrowsMesh);
		BrowsMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		BrowsMeshComponent->SetVisibility(true);
	}
	else
	{
		BrowsMeshComponent->SetVisibility(false);
	}

	if (USkeletalMesh* BeardMesh = LoadSkeletalMeshForPath(GetBeardMeshPath(Request.Sex, Request.BeardStyleIndex)))
	{
		BeardMeshComponent->SetSkeletalMesh(BeardMesh);
		BeardMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		BeardMeshComponent->SetVisibility(true);
	}
	else
	{
		BeardMeshComponent->SetVisibility(false);
	}

	if (UMaterialInterface* BodyMaterial = LoadMaterialForPath(GetBodyMaterialPath(Request.Sex, Request.SkinPresetIndex)))
	{
		const int32 MaterialCount = BodyMeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			BodyMeshComponent->SetMaterial(MaterialIndex, BodyMaterial);
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

const TCHAR* AWUCharacterCreatorPreviewActor::GetBodyMaterialPath(EWUCharacterSex Sex, int32 SkinPresetIndex) const
{
	static const TCHAR* FemaleBodyMaterialPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Body/Presets/MI_Hu_F_Body_Peasant_Bl.MI_Hu_F_Body_Peasant_Bl"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Body/Presets/MI_Hu_F_Body_Peasant_Br.MI_Hu_F_Body_Peasant_Br"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Body/Presets/MI_Hu_F_Body_Peasant_Rd.MI_Hu_F_Body_Peasant_Rd")
	};

	static const TCHAR* MaleBodyMaterialPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Body/Presets/MI_Hu_M_Body_Peasant_Bl.MI_Hu_M_Body_Peasant_Bl"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Body/Presets/MI_Hu_M_Body_Peasant_Br.MI_Hu_M_Body_Peasant_Br"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Body/Presets/MI_Hu_M_Body_Peasant_Rd.MI_Hu_M_Body_Peasant_Rd")
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
