// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"
#include "GameFramework/Character.h"
#include "NPC/WUNpcTypes.h"
#include "WUNpcCharacter.generated.h"

class UAnimationAsset;
class UMaterialInterface;
class USkeletalMesh;
class USkeletalMeshComponent;
class UTexture2D;
class UWidgetComponent;
class UWUNpcDefinition;
class UWUOverheadNameWidget;

UCLASS(Blueprintable)
class WU_API AWUNpcCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	AWUNpcCharacter();

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void RefreshNpcFromDefinition();

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void ApplyNpcAppearance(const FWUCharacterAppearance& NewAppearance);

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void SetNpcProfile(const FWUNpcProfile& NewProfile);

	UFUNCTION(BlueprintPure, Category = "NPC")
	FWUNpcProfile GetNpcProfile() const;

	UFUNCTION(BlueprintPure, Category = "NPC")
	FWUCharacterAppearance GetNpcAppearance() const;

	UFUNCTION(BlueprintPure, Category = "NPC")
	FText GetNpcDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "NPC")
	FText GetInteractionPrompt() const;

	UFUNCTION(BlueprintPure, Category = "NPC")
	FText GetVendorTypeLabel() const;

	UFUNCTION(BlueprintPure, Category = "NPC")
	bool CanOfferQuest() const;

	UFUNCTION(BlueprintPure, Category = "NPC")
	bool CanOpenVendor() const;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	TObjectPtr<UWUNpcDefinition> NpcDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	FWUNpcProfile Profile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Appearance")
	FWUCharacterAppearance NpcAppearance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Appearance")
	TObjectPtr<USkeletalMeshComponent> HeadMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Appearance")
	TObjectPtr<USkeletalMeshComponent> HairMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Appearance")
	TObjectPtr<USkeletalMeshComponent> BrowsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Appearance")
	TObjectPtr<USkeletalMeshComponent> BeardMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Appearance")
	TObjectPtr<USkeletalMeshComponent> PantsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Appearance")
	TObjectPtr<USkeletalMeshComponent> HandsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Appearance")
	TObjectPtr<USkeletalMeshComponent> BracersMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Appearance")
	TObjectPtr<USkeletalMeshComponent> ChestOutfitMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Appearance")
	TObjectPtr<USkeletalMeshComponent> ChestAddOutfitMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Appearance")
	TObjectPtr<USkeletalMeshComponent> BeltOutfitMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Appearance")
	TObjectPtr<USkeletalMeshComponent> BootsOutfitMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|UI")
	TObjectPtr<UWidgetComponent> OverheadNameComponent;

private:

	void UpdateNpcAppearance();
	void RefreshOverheadName();
	UWUOverheadNameWidget* GetOverheadNameWidget() const;
	void ConfigureBodyMeshComponent() const;
	void ConfigureModularMeshComponent(USkeletalMeshComponent* MeshComponent) const;
	void SetModularMesh(USkeletalMeshComponent* MeshComponent, const TCHAR* AssetPath) const;
	void ShowModularMesh(USkeletalMeshComponent* MeshComponent) const;
	void HideModularMesh(USkeletalMeshComponent* MeshComponent) const;
	USkeletalMesh* LoadSkeletalMeshForPath(const TCHAR* AssetPath) const;
	UMaterialInterface* LoadMaterialForPath(const TCHAR* AssetPath) const;
	UTexture2D* LoadTextureForPath(const TCHAR* AssetPath) const;
	UClass* LoadAnimClassForPath(const TCHAR* AssetPath) const;
};
