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

	HairMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));
	HairMeshComponent->SetupAttachment(BodyMeshComponent);
	HairMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HairMeshComponent->SetGenerateOverlapEvents(false);
	HairMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
}

void AWUCharacterCreatorPreviewActor::ApplyCreateRequest(const FWUCharacterCreateRequest& Request)
{
	if (USkeletalMesh* BodyMesh = LoadSkeletalMeshForPath(GetBodyMeshPath(Request.Sex)))
	{
		BodyMeshComponent->SetSkeletalMesh(BodyMesh);
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

	if (UMaterialInterface* BodyMaterial = LoadMaterialForPath(GetBodyMaterialPath(Request.Sex, Request.SkinPresetIndex)))
	{
		const int32 MaterialCount = BodyMeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			BodyMeshComponent->SetMaterial(MaterialIndex, BodyMaterial);
		}
	}

	if (UMaterialInterface* HairMaterial = LoadMaterialForPath(GetHairMaterialPath(Request.HairColorIndex)))
	{
		const int32 MaterialCount = HairMeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			HairMeshComponent->SetMaterial(MaterialIndex, HairMaterial);
		}
	}
}

void AWUCharacterCreatorPreviewActor::RotatePreview(float YawDelta)
{
	AddActorLocalRotation(FRotator(0.0f, YawDelta, 0.0f));
}

USkeletalMesh* AWUCharacterCreatorPreviewActor::LoadSkeletalMeshForPath(const TCHAR* AssetPath) const
{
	return LoadObject<USkeletalMesh>(nullptr, AssetPath);
}

UMaterialInterface* AWUCharacterCreatorPreviewActor::LoadMaterialForPath(const TCHAR* AssetPath) const
{
	return LoadObject<UMaterialInterface>(nullptr, AssetPath);
}

const TCHAR* AWUCharacterCreatorPreviewActor::GetBodyMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/SK_Hu_F_FullBody.SK_Hu_F_FullBody")
		: TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/SK_Hu_M_FullBody.SK_Hu_M_FullBody");
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
