// Copyright Epic Games, Inc. All Rights Reserved.

#include "NPC/WUNpcCharacter.h"
#include "CharacterCreation/WUCharacterAssetPaths.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NPC/WUNpcDefinition.h"

AWUNpcCharacter::AWUNpcCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	ConfigureBodyMeshComponent();

	HeadMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
	HeadMeshComponent->SetupAttachment(GetMesh());
	ConfigureModularMeshComponent(HeadMeshComponent);

	HairMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));
	HairMeshComponent->SetupAttachment(GetMesh());
	ConfigureModularMeshComponent(HairMeshComponent);

	BrowsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BrowsMesh"));
	BrowsMeshComponent->SetupAttachment(GetMesh());
	ConfigureModularMeshComponent(BrowsMeshComponent);

	BeardMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BeardMesh"));
	BeardMeshComponent->SetupAttachment(GetMesh());
	ConfigureModularMeshComponent(BeardMeshComponent);

	PantsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PantsMesh"));
	PantsMeshComponent->SetupAttachment(GetMesh());
	ConfigureModularMeshComponent(PantsMeshComponent);

	HandsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandsMesh"));
	HandsMeshComponent->SetupAttachment(GetMesh());
	ConfigureModularMeshComponent(HandsMeshComponent);

	BracersMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BracersMesh"));
	BracersMeshComponent->SetupAttachment(GetMesh());
	ConfigureModularMeshComponent(BracersMeshComponent);

	ChestOutfitMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestOutfitMesh"));
	ChestOutfitMeshComponent->SetupAttachment(GetMesh());
	ConfigureModularMeshComponent(ChestOutfitMeshComponent);

	ChestAddOutfitMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestAddOutfitMesh"));
	ChestAddOutfitMeshComponent->SetupAttachment(GetMesh());
	ConfigureModularMeshComponent(ChestAddOutfitMeshComponent);

	BeltOutfitMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BeltOutfitMesh"));
	BeltOutfitMeshComponent->SetupAttachment(GetMesh());
	ConfigureModularMeshComponent(BeltOutfitMeshComponent);

	BootsOutfitMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BootsOutfitMesh"));
	BootsOutfitMeshComponent->SetupAttachment(GetMesh());
	ConfigureModularMeshComponent(BootsOutfitMeshComponent);
}

void AWUNpcCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshNpcFromDefinition();
}

void AWUNpcCharacter::BeginPlay()
{
	Super::BeginPlay();

	RefreshNpcFromDefinition();
}

void AWUNpcCharacter::RefreshNpcFromDefinition()
{
	if (NpcDefinition)
	{
		Profile = NpcDefinition->Profile;
		NpcAppearance = NpcDefinition->Appearance;
	}

	UpdateNpcAppearance();
}

void AWUNpcCharacter::ApplyNpcAppearance(const FWUCharacterAppearance& NewAppearance)
{
	NpcAppearance = NewAppearance;
	UpdateNpcAppearance();
}

void AWUNpcCharacter::SetNpcProfile(const FWUNpcProfile& NewProfile)
{
	Profile = NewProfile;
}

FWUNpcProfile AWUNpcCharacter::GetNpcProfile() const
{
	return Profile;
}

FWUCharacterAppearance AWUNpcCharacter::GetNpcAppearance() const
{
	return NpcAppearance;
}

FText AWUNpcCharacter::GetNpcDisplayName() const
{
	if (!Profile.DisplayName.IsEmpty())
	{
		return Profile.DisplayName;
	}

	return Profile.NpcId.IsNone() ? NSLOCTEXT("WUNpc", "FallbackNpcDisplayName", "NPC") : FText::FromName(Profile.NpcId);
}

FText AWUNpcCharacter::GetInteractionPrompt() const
{
	if (!Profile.InteractionPrompt.IsEmpty())
	{
		return Profile.InteractionPrompt;
	}

	switch (Profile.Role)
	{
	case EWUNpcRole::QuestGiver:
		return NSLOCTEXT("WUNpc", "QuestGiverPrompt", "Talk");
	case EWUNpcRole::Vendor:
		return NSLOCTEXT("WUNpc", "VendorPrompt", "Shop");
	case EWUNpcRole::Banker:
		return NSLOCTEXT("WUNpc", "BankerPrompt", "Bank");
	default:
		return NSLOCTEXT("WUNpc", "AmbientPrompt", "Interact");
	}
}

bool AWUNpcCharacter::CanOfferQuest() const
{
	return Profile.Role == EWUNpcRole::QuestGiver && !Profile.QuestId.IsNone();
}

bool AWUNpcCharacter::CanOpenVendor() const
{
	return Profile.Role == EWUNpcRole::Vendor && !Profile.VendorTableId.IsNone();
}

void AWUNpcCharacter::UpdateNpcAppearance()
{
	if (USkeletalMesh* BodyMesh = LoadSkeletalMeshForPath(FWUCharacterAssetPaths::BodyMesh(NpcAppearance.Sex)))
	{
		GetMesh()->SetSkeletalMesh(BodyMesh);
	}

	if (UClass* AnimClass = LoadAnimClassForPath(FWUCharacterAssetPaths::AnimationBlueprint(NpcAppearance.Sex)))
	{
		GetMesh()->SetAnimInstanceClass(AnimClass);
	}

	ConfigureBodyMeshComponent();

	SetModularMesh(HeadMeshComponent, FWUCharacterAssetPaths::HeadMesh(NpcAppearance.Sex));
	SetModularMesh(HairMeshComponent, FWUCharacterAssetPaths::HairMesh(NpcAppearance.Sex, NpcAppearance.HairStyleIndex));
	SetModularMesh(BrowsMeshComponent, FWUCharacterAssetPaths::BrowsMesh(NpcAppearance.Sex, NpcAppearance.BrowStyleIndex));
	SetModularMesh(BeardMeshComponent, FWUCharacterAssetPaths::BeardMesh(NpcAppearance.Sex, NpcAppearance.BeardStyleIndex));
	SetModularMesh(PantsMeshComponent, FWUCharacterAssetPaths::PantsMesh(NpcAppearance.Sex));
	SetModularMesh(HandsMeshComponent, FWUCharacterAssetPaths::HandsMesh(NpcAppearance.Sex));
	SetModularMesh(BracersMeshComponent, FWUCharacterAssetPaths::BracersMesh(NpcAppearance.Sex));
	SetModularMesh(ChestOutfitMeshComponent, FWUCharacterAssetPaths::StarterChestOutfitMesh(NpcAppearance.Sex));
	SetModularMesh(ChestAddOutfitMeshComponent, FWUCharacterAssetPaths::StarterChestAddOutfitMesh(NpcAppearance.Sex));
	SetModularMesh(BeltOutfitMeshComponent, FWUCharacterAssetPaths::StarterBeltOutfitMesh(NpcAppearance.Sex));
	SetModularMesh(BootsOutfitMeshComponent, FWUCharacterAssetPaths::StarterBootsOutfitMesh(NpcAppearance.Sex));

	if (UMaterialInterface* BodyMaterial = LoadMaterialForPath(FWUCharacterAssetPaths::BodyMaterial(NpcAppearance.Sex, NpcAppearance.SkinPresetIndex)))
	{
		const int32 BodyMaterialCount = GetMesh()->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < BodyMaterialCount; ++MaterialIndex)
		{
			GetMesh()->SetMaterial(MaterialIndex, BodyMaterial);
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

	if (UMaterialInterface* HeadMaterial = LoadMaterialForPath(FWUCharacterAssetPaths::HeadMaterial(NpcAppearance.Sex, NpcAppearance.HeadPresetIndex)))
	{
		UMaterialInterface* ResolvedHeadMaterial = HeadMaterial;
		if (UTexture2D* UnderhairTexture = LoadTextureForPath(FWUCharacterAssetPaths::UnderhairTexture(NpcAppearance.Sex, NpcAppearance.HairColorIndex)))
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
				&& !SlotName.Contains(TEXT("Facial"), ESearchCase::IgnoreCase)
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

	if (UMaterialInterface* FacialsMaterial = LoadMaterialForPath(FWUCharacterAssetPaths::FacialsMaterial(NpcAppearance.Sex, NpcAppearance.HairColorIndex)))
	{
		const TArray<FName> MaterialSlotNames = HeadMeshComponent->GetMaterialSlotNames();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotNames.Num(); ++MaterialIndex)
		{
			const FString SlotName = MaterialSlotNames[MaterialIndex].ToString();
			if (SlotName.Contains(TEXT("Facial"), ESearchCase::IgnoreCase))
			{
				HeadMeshComponent->SetMaterial(MaterialIndex, FacialsMaterial);
			}
		}
	}

	if (UMaterialInterface* EyeMaterial = LoadMaterialForPath(FWUCharacterAssetPaths::EyeMaterial(NpcAppearance.EyeColorIndex)))
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

	if (UMaterialInterface* HairMaterial = LoadMaterialForPath(FWUCharacterAssetPaths::HairMaterial(NpcAppearance.HairColorIndex)))
	{
		const int32 HairMaterialCount = HairMeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < HairMaterialCount; ++MaterialIndex)
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

void AWUNpcCharacter::ConfigureBodyMeshComponent() const
{
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	GetMesh()->SetVisibility(false, false);
	GetMesh()->SetHiddenInGame(true, false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetGenerateOverlapEvents(false);
}

void AWUNpcCharacter::ConfigureModularMeshComponent(USkeletalMeshComponent* MeshComponent) const
{
	if (!MeshComponent)
	{
		return;
	}

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetLeaderPoseComponent(GetMesh());
}

void AWUNpcCharacter::SetModularMesh(USkeletalMeshComponent* MeshComponent, const TCHAR* AssetPath) const
{
	if (!MeshComponent)
	{
		return;
	}

	if (USkeletalMesh* LoadedMesh = LoadSkeletalMeshForPath(AssetPath))
	{
		MeshComponent->SetSkeletalMesh(LoadedMesh);
		MeshComponent->SetLeaderPoseComponent(GetMesh());
		ShowModularMesh(MeshComponent);
		return;
	}

	MeshComponent->SetSkeletalMesh(nullptr);
	HideModularMesh(MeshComponent);
}

void AWUNpcCharacter::ShowModularMesh(USkeletalMeshComponent* MeshComponent) const
{
	if (!MeshComponent)
	{
		return;
	}

	MeshComponent->SetVisibility(true, false);
	MeshComponent->SetHiddenInGame(false, false);
}

void AWUNpcCharacter::HideModularMesh(USkeletalMeshComponent* MeshComponent) const
{
	if (!MeshComponent)
	{
		return;
	}

	MeshComponent->SetVisibility(false, false);
	MeshComponent->SetHiddenInGame(true, false);
}

USkeletalMesh* AWUNpcCharacter::LoadSkeletalMeshForPath(const TCHAR* AssetPath) const
{
	return AssetPath ? LoadObject<USkeletalMesh>(nullptr, AssetPath) : nullptr;
}

UMaterialInterface* AWUNpcCharacter::LoadMaterialForPath(const TCHAR* AssetPath) const
{
	return AssetPath ? LoadObject<UMaterialInterface>(nullptr, AssetPath) : nullptr;
}

UTexture2D* AWUNpcCharacter::LoadTextureForPath(const TCHAR* AssetPath) const
{
	return AssetPath ? LoadObject<UTexture2D>(nullptr, AssetPath) : nullptr;
}

UClass* AWUNpcCharacter::LoadAnimClassForPath(const TCHAR* AssetPath) const
{
	return AssetPath ? LoadClass<UAnimInstance>(nullptr, AssetPath) : nullptr;
}
