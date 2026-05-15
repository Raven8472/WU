// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterCreation/WUCharacterCreatorPreviewActor.h"
#include "CharacterCreation/WUCharacterAppearanceApplier.h"
#include "CharacterCreation/WUCharacterAssetPaths.h"
#include "Components/SkeletalMeshComponent.h"

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
	BodyMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

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
	FWUCharacterAppearanceMeshSet Meshes;
	Meshes.BodyMesh = BodyMeshComponent;
	Meshes.HeadMesh = HeadMeshComponent;
	Meshes.HairMesh = HairMeshComponent;
	Meshes.BrowsMesh = BrowsMeshComponent;
	Meshes.BeardMesh = BeardMeshComponent;
	Meshes.PantsMesh = PantsMeshComponent;
	Meshes.HandsMesh = HandsMeshComponent;
	Meshes.BracersMesh = BracersMeshComponent;
	Meshes.ChestOutfitMesh = ChestOutfitMeshComponent;
	Meshes.ChestAddOutfitMesh = ChestAddOutfitMeshComponent;
	Meshes.BeltOutfitMesh = BeltOutfitMeshComponent;
	Meshes.BootsOutfitMesh = BootsOutfitMeshComponent;

	FWUCharacterAppearanceApplyOptions Options;
	Options.MaterialOuter = this;
	Options.bApplyAnimationBlueprint = false;
	WUCharacterAppearance::ApplyAppearance(WUCharacterAppearance::FromCreateRequest(Request), Meshes, Options);
	PlayIdleAnimation(Request.Sex);
}

void AWUCharacterCreatorPreviewActor::RotatePreview(float YawDelta)
{
	AddActorLocalRotation(FRotator(0.0f, YawDelta, 0.0f));
}

void AWUCharacterCreatorPreviewActor::ConfigureModularMeshComponent(USkeletalMeshComponent* MeshComponent) const
{
	WUCharacterAppearance::ConfigureModularMeshComponent(MeshComponent, BodyMeshComponent);
}

void AWUCharacterCreatorPreviewActor::PlayIdleAnimation(EWUCharacterSex Sex) const
{
	if (!BodyMeshComponent)
	{
		return;
	}

	if (UAnimationAsset* IdleAnimation = WUCharacterAppearance::LoadAnimationAssetForPath(FWUCharacterAssetPaths::IdleAnimation(Sex)))
	{
		BodyMeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		BodyMeshComponent->PlayAnimation(IdleAnimation, true);
	}
}
