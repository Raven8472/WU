// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterCreation/WUCharacterCreatorPreviewActor.h"
#include "CharacterCreation/WUCharacterAssetPaths.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
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
	if (USkeletalMesh* BodyMesh = LoadSkeletalMeshForPath(FWUCharacterAssetPaths::BodyMesh(Request.Sex)))
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

	if (USkeletalMesh* HeadMesh = LoadSkeletalMeshForPath(FWUCharacterAssetPaths::HeadMesh(Request.Sex)))
	{
		HeadMeshComponent->SetSkeletalMesh(HeadMesh);
		HeadMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(HeadMeshComponent);
	}

	if (USkeletalMesh* HairMesh = LoadSkeletalMeshForPath(FWUCharacterAssetPaths::HairMesh(Request.Sex, Request.HairStyleIndex)))
	{
		HairMeshComponent->SetSkeletalMesh(HairMesh);
		HairMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(HairMeshComponent);
	}
	else
	{
		HideModularMesh(HairMeshComponent);
	}

	if (USkeletalMesh* BrowsMesh = LoadSkeletalMeshForPath(FWUCharacterAssetPaths::BrowsMesh(Request.Sex, Request.BrowStyleIndex)))
	{
		BrowsMeshComponent->SetSkeletalMesh(BrowsMesh);
		BrowsMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(BrowsMeshComponent);
	}
	else
	{
		HideModularMesh(BrowsMeshComponent);
	}

	if (USkeletalMesh* BeardMesh = LoadSkeletalMeshForPath(FWUCharacterAssetPaths::BeardMesh(Request.Sex, Request.BeardStyleIndex)))
	{
		BeardMeshComponent->SetSkeletalMesh(BeardMesh);
		BeardMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(BeardMeshComponent);
	}
	else
	{
		HideModularMesh(BeardMeshComponent);
	}

	if (USkeletalMesh* PantsMesh = LoadSkeletalMeshForPath(FWUCharacterAssetPaths::PantsMesh(Request.Sex)))
	{
		PantsMeshComponent->SetSkeletalMesh(PantsMesh);
		PantsMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(PantsMeshComponent);
	}

	if (USkeletalMesh* HandsMesh = LoadSkeletalMeshForPath(FWUCharacterAssetPaths::HandsMesh(Request.Sex)))
	{
		HandsMeshComponent->SetSkeletalMesh(HandsMesh);
		HandsMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(HandsMeshComponent);
	}

	if (USkeletalMesh* BracersMesh = LoadSkeletalMeshForPath(FWUCharacterAssetPaths::BracersMesh(Request.Sex)))
	{
		BracersMeshComponent->SetSkeletalMesh(BracersMesh);
		BracersMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(BracersMeshComponent);
	}

	if (USkeletalMesh* ChestOutfitMesh = LoadSkeletalMeshForPath(FWUCharacterAssetPaths::StarterChestOutfitMesh(Request.Sex)))
	{
		ChestOutfitMeshComponent->SetSkeletalMesh(ChestOutfitMesh);
		ChestOutfitMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(ChestOutfitMeshComponent);
	}

	if (USkeletalMesh* ChestAddOutfitMesh = LoadSkeletalMeshForPath(FWUCharacterAssetPaths::StarterChestAddOutfitMesh(Request.Sex)))
	{
		ChestAddOutfitMeshComponent->SetSkeletalMesh(ChestAddOutfitMesh);
		ChestAddOutfitMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(ChestAddOutfitMeshComponent);
	}

	if (USkeletalMesh* BeltOutfitMesh = LoadSkeletalMeshForPath(FWUCharacterAssetPaths::StarterBeltOutfitMesh(Request.Sex)))
	{
		BeltOutfitMeshComponent->SetSkeletalMesh(BeltOutfitMesh);
		BeltOutfitMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(BeltOutfitMeshComponent);
	}

	if (USkeletalMesh* BootsOutfitMesh = LoadSkeletalMeshForPath(FWUCharacterAssetPaths::StarterBootsOutfitMesh(Request.Sex)))
	{
		BootsOutfitMeshComponent->SetSkeletalMesh(BootsOutfitMesh);
		BootsOutfitMeshComponent->SetLeaderPoseComponent(BodyMeshComponent);
		ShowModularMesh(BootsOutfitMeshComponent);
	}

	if (UMaterialInterface* BodyMaterial = LoadMaterialForPath(FWUCharacterAssetPaths::BodyMaterial(Request.Sex, Request.SkinPresetIndex)))
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

	if (UMaterialInterface* HeadMaterial = LoadMaterialForPath(FWUCharacterAssetPaths::HeadMaterial(Request.Sex, Request.HeadPresetIndex)))
	{
		UMaterialInterface* ResolvedHeadMaterial = HeadMaterial;
		if (UTexture2D* UnderhairTexture = LoadTextureForPath(FWUCharacterAssetPaths::UnderhairTexture(Request.Sex, Request.HairColorIndex)))
		{
			UMaterialInstanceDynamic* DynamicHeadMaterial = UMaterialInstanceDynamic::Create(HeadMaterial, this);
			DynamicHeadMaterial->SetTextureParameterValue(TEXT("Underhair_D"), UnderhairTexture);
			ResolvedHeadMaterial = DynamicHeadMaterial;
		}

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
				HeadMeshComponent->SetMaterial(MaterialIndex, ResolvedHeadMaterial);
				bAppliedHeadMaterial = true;
			}
		}

		if (!bAppliedHeadMaterial && HeadMeshComponent->GetNumMaterials() > 0)
		{
			HeadMeshComponent->SetMaterial(0, ResolvedHeadMaterial);
		}
	}

	if (UMaterialInterface* EyeMaterial = LoadMaterialForPath(FWUCharacterAssetPaths::EyeMaterial(Request.EyeColorIndex)))
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

	if (UMaterialInterface* HairMaterial = LoadMaterialForPath(FWUCharacterAssetPaths::HairMaterial(Request.HairColorIndex)))
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

UTexture2D* AWUCharacterCreatorPreviewActor::LoadTextureForPath(const TCHAR* AssetPath) const
{
	if (!AssetPath)
	{
		return nullptr;
	}

	return LoadObject<UTexture2D>(nullptr, AssetPath);
}
