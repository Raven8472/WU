// Copyright Epic Games, Inc. All Rights Reserved.

#include "NPC/WUNpcCharacter.h"
#include "CharacterCreation/WUCharacterAppearanceApplier.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "NPC/WUNpcDefinition.h"
#include "UI/WUOverheadNameWidget.h"
#include "UI/WUOverheadNameVisibilityComponent.h"

AWUNpcCharacter::AWUNpcCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	ConfigureBodyMeshComponent();

	OverheadNameComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadName"));
	OverheadNameComponent->SetupAttachment(RootComponent);
	OverheadNameComponent->SetWidgetClass(UWUOverheadNameWidget::StaticClass());
	OverheadNameComponent->SetWidgetSpace(EWidgetSpace::Screen);
	OverheadNameComponent->SetDrawAtDesiredSize(true);
	OverheadNameComponent->SetPivot(FVector2D(0.5f, 1.0f));
	OverheadNameComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 138.0f));
	OverheadNameComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OverheadNameComponent->SetGenerateOverlapEvents(false);

	OverheadNameVisibilityComponent = CreateDefaultSubobject<UWUOverheadNameVisibilityComponent>(TEXT("OverheadNameVisibility"));
	OverheadNameVisibilityComponent->SetOverheadNameComponent(OverheadNameComponent);

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
	RefreshOverheadName();
}

void AWUNpcCharacter::BeginPlay()
{
	Super::BeginPlay();

	RefreshNpcFromDefinition();
	RefreshOverheadName();
}

void AWUNpcCharacter::RefreshNpcFromDefinition()
{
	if (NpcDefinition)
	{
		Profile = NpcDefinition->Profile;
		NpcAppearance = NpcDefinition->Appearance;
	}

	UpdateNpcAppearance();
	RefreshOverheadName();
}

void AWUNpcCharacter::ApplyNpcAppearance(const FWUCharacterAppearance& NewAppearance)
{
	NpcAppearance = NewAppearance;
	UpdateNpcAppearance();
}

void AWUNpcCharacter::SetNpcProfile(const FWUNpcProfile& NewProfile)
{
	Profile = NewProfile;
	RefreshOverheadName();
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

void AWUNpcCharacter::RefreshOverheadName()
{
	if (UWUOverheadNameWidget* NameWidget = GetOverheadNameWidget())
	{
		FLinearColor NameColor(0.0f, 0.84f, 0.38f, 1.0f);
		switch (Profile.Disposition)
		{
		case EWUNpcDisposition::NeutralEnemy:
			NameColor = FLinearColor(1.0f, 0.86f, 0.20f, 1.0f);
			break;
		case EWUNpcDisposition::HostileEnemy:
			NameColor = FLinearColor(1.0f, 0.18f, 0.12f, 1.0f);
			break;
		case EWUNpcDisposition::Friendly:
		default:
			break;
		}

		NameWidget->SetNameText(GetNpcDisplayName());
		NameWidget->SetSubtitleText(FText::GetEmpty());
		NameWidget->SetNameColor(FSlateColor(NameColor));
	}
}

UWUOverheadNameWidget* AWUNpcCharacter::GetOverheadNameWidget() const
{
	return OverheadNameComponent
		? Cast<UWUOverheadNameWidget>(OverheadNameComponent->GetUserWidgetObject())
		: nullptr;
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

FText AWUNpcCharacter::GetVendorTypeLabel() const
{
	if (!Profile.VendorTypeLabel.IsEmpty())
	{
		return Profile.VendorTypeLabel;
	}

	if (CanOpenVendor())
	{
		return NSLOCTEXT("WUNpc", "GenericVendorType", "Vendor");
	}

	if (Profile.Role == EWUNpcRole::Banker)
	{
		return NSLOCTEXT("WUNpc", "BankerVendorType", "Banker");
	}

	return FText::GetEmpty();
}

bool AWUNpcCharacter::CanOfferQuest() const
{
	return !Profile.QuestId.IsNone();
}

bool AWUNpcCharacter::CanOpenVendor() const
{
	return !Profile.VendorTableId.IsNone();
}

void AWUNpcCharacter::UpdateNpcAppearance()
{
	ConfigureBodyMeshComponent();

	FWUCharacterAppearanceMeshSet Meshes;
	Meshes.BodyMesh = GetMesh();
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
	Options.bApplyAnimationBlueprint = true;
	WUCharacterAppearance::ApplyAppearance(NpcAppearance, Meshes, Options);
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
	WUCharacterAppearance::ConfigureModularMeshComponent(MeshComponent, GetMesh());
}
