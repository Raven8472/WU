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
	TObjectPtr<USkeletalMeshComponent> HairMeshComponent;

private:

	USkeletalMesh* LoadSkeletalMeshForPath(const TCHAR* AssetPath) const;
	UMaterialInterface* LoadMaterialForPath(const TCHAR* AssetPath) const;

	const TCHAR* GetBodyMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetHairMeshPath(EWUCharacterSex Sex, int32 HairStyleIndex) const;
	const TCHAR* GetBodyMaterialPath(EWUCharacterSex Sex, int32 SkinPresetIndex) const;
	const TCHAR* GetHairMaterialPath(int32 HairColorIndex) const;

	int32 NormalizeIndex(int32 Index, int32 Count) const;
};
