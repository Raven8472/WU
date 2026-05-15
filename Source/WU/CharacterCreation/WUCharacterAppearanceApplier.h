// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"

class UAnimationAsset;
class UMaterialInterface;
class USkeletalMesh;
class USkeletalMeshComponent;
class UTexture2D;

struct WU_API FWUCharacterAppearanceMeshSet
{
	USkeletalMeshComponent* BodyMesh = nullptr;
	USkeletalMeshComponent* HeadMesh = nullptr;
	USkeletalMeshComponent* HairMesh = nullptr;
	USkeletalMeshComponent* BrowsMesh = nullptr;
	USkeletalMeshComponent* BeardMesh = nullptr;
	USkeletalMeshComponent* PantsMesh = nullptr;
	USkeletalMeshComponent* HandsMesh = nullptr;
	USkeletalMeshComponent* BracersMesh = nullptr;
	USkeletalMeshComponent* ChestOutfitMesh = nullptr;
	USkeletalMeshComponent* ChestAddOutfitMesh = nullptr;
	USkeletalMeshComponent* BeltOutfitMesh = nullptr;
	USkeletalMeshComponent* BootsOutfitMesh = nullptr;
};

struct WU_API FWUCharacterAppearanceApplyOptions
{
	UObject* MaterialOuter = nullptr;
	bool bApplyAnimationBlueprint = true;
	bool bApplyBodyMaterialToNonHeadSlots = false;
	bool bApplyFaceMaterialsToBody = false;
};

namespace WUCharacterAppearance
{
	WU_API FWUCharacterAppearance FromCreateRequest(const FWUCharacterCreateRequest& Request);

	WU_API void ConfigureModularMeshComponent(
		USkeletalMeshComponent* MeshComponent,
		USkeletalMeshComponent* LeaderPoseComponent,
		bool bIgnoreCameraCollision = false,
		bool bReplicateComponent = false);

	WU_API void HideBodyMeshForModularAppearance(USkeletalMeshComponent* BodyMeshComponent);
	WU_API void SetMeshVisible(USkeletalMeshComponent* MeshComponent, bool bVisible);

	WU_API void ApplyAppearance(
		const FWUCharacterAppearance& Appearance,
		const FWUCharacterAppearanceMeshSet& Meshes,
		const FWUCharacterAppearanceApplyOptions& Options = FWUCharacterAppearanceApplyOptions());

	WU_API USkeletalMesh* LoadSkeletalMeshForPath(const TCHAR* AssetPath);
	WU_API UMaterialInterface* LoadMaterialForPath(const TCHAR* AssetPath);
	WU_API UTexture2D* LoadTextureForPath(const TCHAR* AssetPath);
	WU_API UAnimationAsset* LoadAnimationAssetForPath(const TCHAR* AssetPath);
	WU_API UClass* LoadAnimClassForPath(const TCHAR* AssetPath);
}
