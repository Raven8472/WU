// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterCreation/WUCharacterAppearanceApplier.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimInstance.h"
#include "CharacterCreation/WUCharacterAssetPaths.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	bool SlotContains(const FName SlotName, const TCHAR* Fragment)
	{
		return SlotName.ToString().Contains(Fragment, ESearchCase::IgnoreCase);
	}

	void ApplyMaterialToAllSlots(USkeletalMeshComponent* MeshComponent, UMaterialInterface* Material)
	{
		if (!MeshComponent || !Material)
		{
			return;
		}

		const int32 MaterialCount = MeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			MeshComponent->SetMaterial(MaterialIndex, Material);
		}
	}

	void ApplyBodyMaterial(USkeletalMeshComponent* BodyMesh, UMaterialInterface* BodyMaterial, bool bOnlyNonHeadSlots)
	{
		if (!BodyMesh || !BodyMaterial)
		{
			return;
		}

		if (!bOnlyNonHeadSlots)
		{
			ApplyMaterialToAllSlots(BodyMesh, BodyMaterial);
			return;
		}

		const TArray<FName> BodyMaterialSlotNames = BodyMesh->GetMaterialSlotNames();
		bool bAppliedBodyMaterial = false;
		for (int32 MaterialIndex = 0; MaterialIndex < BodyMaterialSlotNames.Num(); ++MaterialIndex)
		{
			const FName SlotName = BodyMaterialSlotNames[MaterialIndex];
			if (!SlotContains(SlotName, TEXT("Eye"))
				&& !SlotContains(SlotName, TEXT("Head"))
				&& !SlotContains(SlotName, TEXT("Face")))
			{
				BodyMesh->SetMaterial(MaterialIndex, BodyMaterial);
				bAppliedBodyMaterial = true;
			}
		}

		if (!bAppliedBodyMaterial)
		{
			ApplyMaterialToAllSlots(BodyMesh, BodyMaterial);
		}
	}

	void ApplyHeadMaterialToMesh(USkeletalMeshComponent* MeshComponent, UMaterialInterface* HeadMaterial)
	{
		if (!MeshComponent || !HeadMaterial)
		{
			return;
		}

		bool bAppliedHeadMaterial = false;
		const TArray<FName> MaterialSlotNames = MeshComponent->GetMaterialSlotNames();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotNames.Num(); ++MaterialIndex)
		{
			const FName SlotName = MaterialSlotNames[MaterialIndex];
			if (!SlotContains(SlotName, TEXT("Eye"))
				&& !SlotContains(SlotName, TEXT("Facial"))
				&& (SlotContains(SlotName, TEXT("Head"))
					|| SlotContains(SlotName, TEXT("Face"))
					|| SlotContains(SlotName, TEXT("Skin"))))
			{
				MeshComponent->SetMaterial(MaterialIndex, HeadMaterial);
				bAppliedHeadMaterial = true;
			}
		}

		if (!bAppliedHeadMaterial && MeshComponent->GetNumMaterials() > 0)
		{
			MeshComponent->SetMaterial(0, HeadMaterial);
		}
	}

	void ApplyHeadMaterialToBodySlots(USkeletalMeshComponent* BodyMesh, UMaterialInterface* HeadMaterial)
	{
		if (!BodyMesh || !HeadMaterial)
		{
			return;
		}

		const TArray<FName> BodyMaterialSlotNames = BodyMesh->GetMaterialSlotNames();
		for (int32 MaterialIndex = 0; MaterialIndex < BodyMaterialSlotNames.Num(); ++MaterialIndex)
		{
			const FName SlotName = BodyMaterialSlotNames[MaterialIndex];
			if (!SlotContains(SlotName, TEXT("Eye"))
				&& !SlotContains(SlotName, TEXT("Facial"))
				&& (SlotContains(SlotName, TEXT("Head"))
					|| SlotContains(SlotName, TEXT("Face"))))
			{
				BodyMesh->SetMaterial(MaterialIndex, HeadMaterial);
			}
		}
	}

	void ApplyFacialsMaterialToMesh(USkeletalMeshComponent* MeshComponent, UMaterialInterface* FacialsMaterial)
	{
		if (!MeshComponent || !FacialsMaterial)
		{
			return;
		}

		const TArray<FName> MaterialSlotNames = MeshComponent->GetMaterialSlotNames();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotNames.Num(); ++MaterialIndex)
		{
			if (SlotContains(MaterialSlotNames[MaterialIndex], TEXT("Facial")))
			{
				MeshComponent->SetMaterial(MaterialIndex, FacialsMaterial);
			}
		}
	}

	void ApplyEyeMaterialToMesh(USkeletalMeshComponent* MeshComponent, UMaterialInterface* EyeMaterial)
	{
		if (!MeshComponent || !EyeMaterial)
		{
			return;
		}

		bool bAppliedEyeMaterial = false;
		const TArray<FName> MaterialSlotNames = MeshComponent->GetMaterialSlotNames();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotNames.Num(); ++MaterialIndex)
		{
			if (SlotContains(MaterialSlotNames[MaterialIndex], TEXT("Eye")))
			{
				MeshComponent->SetMaterial(MaterialIndex, EyeMaterial);
				bAppliedEyeMaterial = true;
			}
		}

		if (!bAppliedEyeMaterial && MeshComponent->GetNumMaterials() > 1)
		{
			MeshComponent->SetMaterial(1, EyeMaterial);
		}
	}

	void ApplyEyeMaterialToBodySlots(USkeletalMeshComponent* BodyMesh, UMaterialInterface* EyeMaterial)
	{
		if (!BodyMesh || !EyeMaterial)
		{
			return;
		}

		const TArray<FName> BodyMaterialSlotNames = BodyMesh->GetMaterialSlotNames();
		for (int32 MaterialIndex = 0; MaterialIndex < BodyMaterialSlotNames.Num(); ++MaterialIndex)
		{
			if (SlotContains(BodyMaterialSlotNames[MaterialIndex], TEXT("Eye")))
			{
				BodyMesh->SetMaterial(MaterialIndex, EyeMaterial);
			}
		}
	}

	void SetModularMesh(USkeletalMeshComponent* MeshComponent, USkeletalMeshComponent* LeaderPoseComponent, const TCHAR* AssetPath)
	{
		if (!MeshComponent)
		{
			return;
		}

		if (USkeletalMesh* LoadedMesh = WUCharacterAppearance::LoadSkeletalMeshForPath(AssetPath))
		{
			MeshComponent->SetSkeletalMesh(LoadedMesh);
			MeshComponent->SetLeaderPoseComponent(LeaderPoseComponent);
			WUCharacterAppearance::SetMeshVisible(MeshComponent, true);
			return;
		}

		MeshComponent->SetSkeletalMesh(nullptr);
		WUCharacterAppearance::SetMeshVisible(MeshComponent, false);
	}
}

FWUCharacterAppearance WUCharacterAppearance::FromCreateRequest(const FWUCharacterCreateRequest& Request)
{
	FWUCharacterAppearance Appearance;
	Appearance.Sex = Request.Sex;
	Appearance.SkinPresetIndex = Request.SkinPresetIndex;
	Appearance.HeadPresetIndex = Request.HeadPresetIndex;
	Appearance.HairStyleIndex = Request.HairStyleIndex;
	Appearance.HairColorIndex = Request.HairColorIndex;
	Appearance.EyeColorIndex = Request.EyeColorIndex;
	Appearance.BrowStyleIndex = Request.BrowStyleIndex;
	Appearance.BeardStyleIndex = Request.BeardStyleIndex;
	return Appearance;
}

void WUCharacterAppearance::ConfigureModularMeshComponent(
	USkeletalMeshComponent* MeshComponent,
	USkeletalMeshComponent* LeaderPoseComponent,
	bool bIgnoreCameraCollision,
	bool bReplicateComponent)
{
	if (!MeshComponent)
	{
		return;
	}

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (bIgnoreCameraCollision)
	{
		MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}

	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetLeaderPoseComponent(LeaderPoseComponent);
	MeshComponent->SetIsReplicated(bReplicateComponent);
}

void WUCharacterAppearance::HideBodyMeshForModularAppearance(USkeletalMeshComponent* BodyMeshComponent)
{
	if (!BodyMeshComponent)
	{
		return;
	}

	BodyMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	BodyMeshComponent->SetVisibility(false, false);
	BodyMeshComponent->SetHiddenInGame(true, false);
}

void WUCharacterAppearance::SetMeshVisible(USkeletalMeshComponent* MeshComponent, bool bVisible)
{
	if (!MeshComponent)
	{
		return;
	}

	MeshComponent->SetVisibility(bVisible, false);
	MeshComponent->SetHiddenInGame(!bVisible, false);
}

void WUCharacterAppearance::ApplyAppearance(
	const FWUCharacterAppearance& Appearance,
	const FWUCharacterAppearanceMeshSet& Meshes,
	const FWUCharacterAppearanceApplyOptions& Options)
{
	if (!Meshes.BodyMesh)
	{
		return;
	}

	if (USkeletalMesh* BodyMesh = LoadSkeletalMeshForPath(FWUCharacterAssetPaths::BodyMesh(Appearance.Sex)))
	{
		Meshes.BodyMesh->SetSkeletalMesh(BodyMesh);
	}

	if (Options.bApplyAnimationBlueprint)
	{
		if (UClass* AnimClass = LoadAnimClassForPath(FWUCharacterAssetPaths::AnimationBlueprint(Appearance.Sex)))
		{
			Meshes.BodyMesh->SetAnimInstanceClass(AnimClass);
		}
	}

	HideBodyMeshForModularAppearance(Meshes.BodyMesh);

	SetModularMesh(Meshes.HeadMesh, Meshes.BodyMesh, FWUCharacterAssetPaths::HeadMesh(Appearance.Sex));
	SetModularMesh(Meshes.HairMesh, Meshes.BodyMesh, FWUCharacterAssetPaths::HairMesh(Appearance.Sex, Appearance.HairStyleIndex));
	SetModularMesh(Meshes.BrowsMesh, Meshes.BodyMesh, FWUCharacterAssetPaths::BrowsMesh(Appearance.Sex, Appearance.BrowStyleIndex));
	SetModularMesh(Meshes.BeardMesh, Meshes.BodyMesh, FWUCharacterAssetPaths::BeardMesh(Appearance.Sex, Appearance.BeardStyleIndex));
	SetModularMesh(Meshes.PantsMesh, Meshes.BodyMesh, FWUCharacterAssetPaths::PantsMesh(Appearance.Sex));
	SetModularMesh(Meshes.HandsMesh, Meshes.BodyMesh, FWUCharacterAssetPaths::HandsMesh(Appearance.Sex));
	SetModularMesh(Meshes.BracersMesh, Meshes.BodyMesh, FWUCharacterAssetPaths::BracersMesh(Appearance.Sex));
	SetModularMesh(Meshes.ChestOutfitMesh, Meshes.BodyMesh, FWUCharacterAssetPaths::StarterChestOutfitMesh(Appearance.Sex));
	SetModularMesh(Meshes.ChestAddOutfitMesh, Meshes.BodyMesh, FWUCharacterAssetPaths::StarterChestAddOutfitMesh(Appearance.Sex));
	SetModularMesh(Meshes.BeltOutfitMesh, Meshes.BodyMesh, FWUCharacterAssetPaths::StarterBeltOutfitMesh(Appearance.Sex));
	SetModularMesh(Meshes.BootsOutfitMesh, Meshes.BodyMesh, FWUCharacterAssetPaths::StarterBootsOutfitMesh(Appearance.Sex));

	if (UMaterialInterface* BodyMaterial = LoadMaterialForPath(FWUCharacterAssetPaths::BodyMaterial(Appearance.Sex, Appearance.SkinPresetIndex)))
	{
		ApplyBodyMaterial(Meshes.BodyMesh, BodyMaterial, Options.bApplyBodyMaterialToNonHeadSlots);
		ApplyMaterialToAllSlots(Meshes.HandsMesh, BodyMaterial);
		ApplyMaterialToAllSlots(Meshes.BracersMesh, BodyMaterial);

		if (Meshes.ChestOutfitMesh)
		{
			const int32 ChestBodyMaterialIndex = Meshes.ChestOutfitMesh->GetMaterialIndex(TEXT("M_Body"));
			if (ChestBodyMaterialIndex != INDEX_NONE)
			{
				Meshes.ChestOutfitMesh->SetMaterial(ChestBodyMaterialIndex, BodyMaterial);
			}
		}
	}

	if (UMaterialInterface* HeadMaterial = LoadMaterialForPath(FWUCharacterAssetPaths::HeadMaterial(Appearance.Sex, Appearance.HeadPresetIndex)))
	{
		UMaterialInterface* ResolvedHeadMaterial = HeadMaterial;
		if (UTexture2D* UnderhairTexture = LoadTextureForPath(FWUCharacterAssetPaths::UnderhairTexture(Appearance.Sex, Appearance.HairColorIndex)))
		{
			UObject* DynamicMaterialOuter = Options.MaterialOuter ? Options.MaterialOuter : Meshes.HeadMesh;
			if (!DynamicMaterialOuter)
			{
				DynamicMaterialOuter = Meshes.BodyMesh;
			}

			if (UMaterialInstanceDynamic* DynamicHeadMaterial = UMaterialInstanceDynamic::Create(HeadMaterial, DynamicMaterialOuter))
			{
				DynamicHeadMaterial->SetTextureParameterValue(TEXT("Underhair_D"), UnderhairTexture);
				ResolvedHeadMaterial = DynamicHeadMaterial;
			}
		}

		ApplyHeadMaterialToMesh(Meshes.HeadMesh, ResolvedHeadMaterial);
		if (Options.bApplyFaceMaterialsToBody)
		{
			ApplyHeadMaterialToBodySlots(Meshes.BodyMesh, ResolvedHeadMaterial);
		}
	}

	if (UMaterialInterface* FacialsMaterial = LoadMaterialForPath(FWUCharacterAssetPaths::FacialsMaterial(Appearance.Sex, Appearance.HairColorIndex)))
	{
		ApplyFacialsMaterialToMesh(Meshes.HeadMesh, FacialsMaterial);
		if (Options.bApplyFaceMaterialsToBody)
		{
			ApplyFacialsMaterialToMesh(Meshes.BodyMesh, FacialsMaterial);
		}
	}

	if (UMaterialInterface* EyeMaterial = LoadMaterialForPath(FWUCharacterAssetPaths::EyeMaterial(Appearance.EyeColorIndex)))
	{
		ApplyEyeMaterialToMesh(Meshes.HeadMesh, EyeMaterial);
		if (Options.bApplyFaceMaterialsToBody)
		{
			ApplyEyeMaterialToBodySlots(Meshes.BodyMesh, EyeMaterial);
		}
	}

	if (UMaterialInterface* HairMaterial = LoadMaterialForPath(FWUCharacterAssetPaths::HairMaterial(Appearance.HairColorIndex)))
	{
		ApplyMaterialToAllSlots(Meshes.HairMesh, HairMaterial);
		ApplyMaterialToAllSlots(Meshes.BrowsMesh, HairMaterial);
		ApplyMaterialToAllSlots(Meshes.BeardMesh, HairMaterial);
	}
}

USkeletalMesh* WUCharacterAppearance::LoadSkeletalMeshForPath(const TCHAR* AssetPath)
{
	return AssetPath ? LoadObject<USkeletalMesh>(nullptr, AssetPath) : nullptr;
}

UMaterialInterface* WUCharacterAppearance::LoadMaterialForPath(const TCHAR* AssetPath)
{
	return AssetPath ? LoadObject<UMaterialInterface>(nullptr, AssetPath) : nullptr;
}

UTexture2D* WUCharacterAppearance::LoadTextureForPath(const TCHAR* AssetPath)
{
	return AssetPath ? LoadObject<UTexture2D>(nullptr, AssetPath) : nullptr;
}

UAnimationAsset* WUCharacterAppearance::LoadAnimationAssetForPath(const TCHAR* AssetPath)
{
	return AssetPath ? LoadObject<UAnimationAsset>(nullptr, AssetPath) : nullptr;
}

UClass* WUCharacterAppearance::LoadAnimClassForPath(const TCHAR* AssetPath)
{
	return AssetPath ? LoadClass<UAnimInstance>(nullptr, AssetPath) : nullptr;
}
