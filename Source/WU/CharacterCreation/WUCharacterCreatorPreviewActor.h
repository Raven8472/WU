// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"
#include "WUCharacterCreatorPreviewActor.generated.h"

class USkeletalMeshComponent;
class USceneComponent;
class USkeletalMesh;
class UMaterialInterface;

/**
 * Lightweight world preview actor used by the local character creator shell.
 */
UCLASS()
class WU_API AWUCharacterCreatorPreviewActor : public AActor
{
	GENERATED_BODY()

public:

	AWUCharacterCreatorPreviewActor();

	UFUNCTION(BlueprintCallable, Category = "Character Creation")
	void ApplyCreateRequest(const FWUCharacterCreateRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "Character Creation")
	void RotatePreview(float YawDelta);

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Creation")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Creation")
	TObjectPtr<USkeletalMeshComponent> BodyMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Creation")
	TObjectPtr<USkeletalMeshComponent> HeadMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Creation")
	TObjectPtr<USkeletalMeshComponent> HairMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Creation")
	TObjectPtr<USkeletalMeshComponent> BrowsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Creation")
	TObjectPtr<USkeletalMeshComponent> BeardMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Creation")
	TObjectPtr<USkeletalMeshComponent> PantsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Creation")
	TObjectPtr<USkeletalMeshComponent> HandsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Creation")
	TObjectPtr<USkeletalMeshComponent> BracersMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Creation")
	TObjectPtr<USkeletalMeshComponent> ChestOutfitMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Creation")
	TObjectPtr<USkeletalMeshComponent> ChestAddOutfitMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Creation")
	TObjectPtr<USkeletalMeshComponent> BeltOutfitMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Creation")
	TObjectPtr<USkeletalMeshComponent> BootsOutfitMeshComponent;

private:

	void ConfigureModularMeshComponent(USkeletalMeshComponent* MeshComponent) const;
	USkeletalMesh* LoadSkeletalMeshForPath(const TCHAR* AssetPath) const;
	UMaterialInterface* LoadMaterialForPath(const TCHAR* AssetPath) const;

	const TCHAR* GetBodyMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetHeadMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetHairMeshPath(EWUCharacterSex Sex, int32 HairStyleIndex) const;
	const TCHAR* GetBrowsMeshPath(EWUCharacterSex Sex, int32 BrowStyleIndex) const;
	const TCHAR* GetBeardMeshPath(EWUCharacterSex Sex, int32 BeardStyleIndex) const;
	const TCHAR* GetPantsMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetHandsMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetBracersMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetStarterChestOutfitMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetStarterChestAddOutfitMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetStarterBeltOutfitMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetStarterBootsOutfitMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetBodyMaterialPath(EWUCharacterSex Sex, int32 SkinPresetIndex) const;
	const TCHAR* GetHeadMaterialPath(EWUCharacterSex Sex, int32 HeadPresetIndex) const;
	const TCHAR* GetEyeMaterialPath(int32 EyeColorIndex) const;
	const TCHAR* GetHairMaterialPath(int32 HairColorIndex) const;

	int32 NormalizeIndex(int32 Index, int32 Count) const;
};
