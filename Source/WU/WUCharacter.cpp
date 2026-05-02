// Copyright Epic Games, Inc. All Rights Reserved.

#include "WUCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "WU.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Blueprint/UserWidget.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "WUPlayerController.h"

namespace
{
	TAutoConsoleVariable<int32> CVarWULogCharacterMaterialSlots(
		TEXT("wu.Character.LogMaterialSlots"),
		0,
		TEXT("Logs WU modular character material slots. 0=off, 1=once per mesh/component, 2=every appearance refresh."),
		ECVF_Default);

	void LogMaterialSlotsForComponent(const AActor* Owner, const TCHAR* Label, const USkeletalMeshComponent* MeshComponent)
	{
		if (!MeshComponent || CVarWULogCharacterMaterialSlots.GetValueOnGameThread() <= 0)
		{
			return;
		}

		const FString MeshPath = GetPathNameSafe(MeshComponent->GetSkeletalMeshAsset());
		if (MeshPath.IsEmpty())
		{
			return;
		}

		const FString LogKey = FString::Printf(TEXT("%s|%s"), Label, *MeshPath);
		static TSet<FString> LoggedMaterialSlotKeys;
		if (CVarWULogCharacterMaterialSlots.GetValueOnGameThread() == 1)
		{
			if (LoggedMaterialSlotKeys.Contains(LogKey))
			{
				return;
			}

			LoggedMaterialSlotKeys.Add(LogKey);
		}

		const TArray<FName> MaterialSlotNames = MeshComponent->GetMaterialSlotNames();
		UE_LOG(LogTemp, Warning, TEXT("WU Material Slots | Owner=%s | Component=%s | Mesh=%s | Visible=%s | HiddenInGame=%s | SlotCount=%d | MaterialCount=%d"),
			*GetNameSafe(Owner),
			Label,
			*MeshPath,
			MeshComponent->IsVisible() ? TEXT("true") : TEXT("false"),
			MeshComponent->bHiddenInGame ? TEXT("true") : TEXT("false"),
			MaterialSlotNames.Num(),
			MeshComponent->GetNumMaterials());

		for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotNames.Num(); ++MaterialIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("WU Material Slots |   [%d] Slot=%s | Material=%s"),
				MaterialIndex,
				*MaterialSlotNames[MaterialIndex].ToString(),
				*GetPathNameSafe(MeshComponent->GetMaterial(MaterialIndex)));
		}
	}
}

AWUCharacter::AWUCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	DefaultRotationRateDegreesPerSecond = GetCharacterMovement()->RotationRate.Yaw;

	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

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

	ApplyCameraCollisionRules();

	bReplicates = true;

	PrimaryStats = WUCharacterStats::CalculatePrimaryStats(BloodStatus, CharacterLevel);
	DerivedStats = WUCharacterStats::CalculateDerivedStats(PrimaryStats);
	MaxHealth = DerivedStats.MaxHealth;
	MaxMagic = DerivedStats.MaxMagic;
	CharacterExperience = 0;
	Health = MaxHealth;
	Magic = MaxMagic;
	bIsDead = false;
	bHasReleased = false;
	DeathWidget = nullptr;
	CorpseMarker = nullptr;
}

void AWUCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWUCharacter, Health);
	DOREPLIFETIME(AWUCharacter, MaxHealth);
	DOREPLIFETIME(AWUCharacter, Magic);
	DOREPLIFETIME(AWUCharacter, MaxMagic);
	DOREPLIFETIME(AWUCharacter, bIsDead);
	DOREPLIFETIME(AWUCharacter, DeathLocation);
	DOREPLIFETIME(AWUCharacter, bHasReleased);
	DOREPLIFETIME(AWUCharacter, bInCombat);
	DOREPLIFETIME(AWUCharacter, CurrentZoneId);
	DOREPLIFETIME(AWUCharacter, CurrentZoneDisplayName);
	DOREPLIFETIME(AWUCharacter, CurrentMapRegionId);
	DOREPLIFETIME(AWUCharacter, CurrentGraveyardTag);
	DOREPLIFETIME(AWUCharacter, BloodStatus);
	DOREPLIFETIME(AWUCharacter, CharacterLevel);
	DOREPLIFETIME(AWUCharacter, CharacterExperience);
	DOREPLIFETIME(AWUCharacter, PrimaryStats);
	DOREPLIFETIME(AWUCharacter, DerivedStats);
	DOREPLIFETIME(AWUCharacter, InventorySlots);
	DOREPLIFETIME(AWUCharacter, EquipmentSlots);
	DOREPLIFETIME(AWUCharacter, CharacterAppearance);
}

void AWUCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultRotationRateDegreesPerSecond = GetCharacterMovement()->RotationRate.Yaw;
	ApplyCameraCollisionRules();

	if (HasAuthority())
	{
		ApplyCharacterProgressionInternal(BloodStatus, CharacterLevel, CharacterExperience, true);
		bIsDead = false;
		bHasReleased = false;
		InitializeInventoryStorage();
		SeedStarterInventory();
		ForceNetUpdate();
	}

	DeathWidget = nullptr;

	ApplyCharacterAppearanceMeshes();

	// Apply the initial alive/dead collision and movement state on spawn so
	// freshly spawned players match the same rules used after later state changes.
	UpdateDeathStateEffects();
}

void AWUCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateMouseSteering(DeltaSeconds);

	if (HasAuthority())
	{
		UpdateCombatState();
		RegenerateResources(DeltaSeconds);
	}
}

void AWUCharacter::OnRep_DeathState()
{
	UpdateDeathStateEffects();
}

void AWUCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
					LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
				{
					if (DefaultMappingContext)
					{
						Subsystem->AddMappingContext(DefaultMappingContext, 0);
					}
				}
			}
		}

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AWUCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AWUCharacter::StopMove);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &AWUCharacter::StopMove);

		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AWUCharacter::MouseLook);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AWUCharacter::Look);
		//attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AWUCharacter::StartAttack);
		//release
		EnhancedInputComponent->BindAction(ReleaseAction, ETriggerEvent::Started, this, &AWUCharacter::RequestRelease);

		PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AWUCharacter::OnLeftMousePressed);
		PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AWUCharacter::OnLeftMouseReleased);
		PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AWUCharacter::OnRightMousePressed);
		PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &AWUCharacter::OnRightMouseReleased);
		PlayerInputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AWUCharacter::TargetNextInput);
		PlayerInputComponent->BindAxisKey(EKeys::MouseWheelAxis, this, &AWUCharacter::ZoomCamera);
	}
	else
	{
		UE_LOG(LogWU, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AWUCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AWUCharacter::StopMove(const FInputActionValue& Value)
{
	bKeyboardTurnInPlaceActive = false;
	EndBackpedal();
	EndTurnInPlace(true);
}

void AWUCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AWUCharacter::MouseLook(const FInputActionValue& Value)
{
	if (!IsMouseCameraOrbitActive())
	{
		return;
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	PendingLeftClickDragDistance += LookAxisVector.Size();

	if (PendingLeftClickDragDistance >= ClickTargetDragThreshold)
	{
		bSuppressLeftClickTargetOnRelease = true;
	}

	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AWUCharacter::ZoomCamera(float AxisValue)
{
	if (!CameraBoom || FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	const float MinLength = FMath::Min(MinCameraBoomLength, MaxCameraBoomLength);
	const float MaxLength = FMath::Max(MinCameraBoomLength, MaxCameraBoomLength);
	CameraBoom->TargetArmLength = FMath::Clamp(
		CameraBoom->TargetArmLength - (AxisValue * MouseWheelZoomStep),
		MinLength,
		MaxLength);
}

void AWUCharacter::OnLeftMousePressed()
{
	if (!IsGameplayMouseInputAllowed())
	{
		return;
	}

	bLeftMouseLookHeld = true;
	bSuppressLeftClickTargetOnRelease = false;
	PendingLeftClickDragDistance = 0.0f;
}

void AWUCharacter::OnLeftMouseReleased()
{
	const bool bShouldTargetUnderCursor =
		bLeftMouseLookHeld
		&& !bSuppressLeftClickTargetOnRelease
		&& !bRightMouseLookHeld
		&& IsGameplayMouseInputAllowed();

	bLeftMouseLookHeld = false;
	PendingLeftClickDragDistance = 0.0f;
	bSuppressLeftClickTargetOnRelease = false;

	if (bShouldTargetUnderCursor)
	{
		TargetUnderCursorInput();
	}
}

void AWUCharacter::OnRightMousePressed()
{
	if (!IsGameplayMouseInputAllowed())
	{
		return;
	}

	bKeyboardTurnInPlaceActive = false;
	bRightMouseLookHeld = true;

	if (bLeftMouseLookHeld)
	{
		bSuppressLeftClickTargetOnRelease = true;
	}
}

void AWUCharacter::OnRightMouseReleased()
{
	bRightMouseLookHeld = false;
	EndTurnInPlace(true);
}

void AWUCharacter::TargetUnderCursorInput()
{
	if (AWUPlayerController* WUPC = Cast<AWUPlayerController>(GetController()))
	{
		WUPC->TargetUnderCursor();
	}
}

void AWUCharacter::TargetNextInput()
{
	if (AWUPlayerController* WUPC = Cast<AWUPlayerController>(GetController()))
	{
		WUPC->TargetNextCharacter();
	}
}

bool AWUCharacter::IsGameplayMouseInputAllowed() const
{
	if (const AWUPlayerController* WUPC = Cast<AWUPlayerController>(GetController()))
	{
		return !WUPC->HasInteractiveOverlayOpen();
	}

	return true;
}

bool AWUCharacter::IsMouseCameraOrbitActive() const
{
	return IsGameplayMouseInputAllowed() && (bLeftMouseLookHeld || bRightMouseLookHeld);
}

bool AWUCharacter::IsMouseFacingControlActive() const
{
	return IsGameplayMouseInputAllowed() && bRightMouseLookHeld;
}

bool AWUCharacter::IsDualMouseDriveActive() const
{
	return IsGameplayMouseInputAllowed() && bLeftMouseLookHeld && bRightMouseLookHeld;
}

void AWUCharacter::UpdateMouseSteering(float DeltaSeconds)
{
	if (!IsLocallyControlled() || !GetCharacterMovement())
	{
		return;
	}

	const bool bMouseFacingActive =
		IsMouseFacingControlActive()
		&& GetController() != nullptr
		&& !bIsDead;

	GetCharacterMovement()->bUseControllerDesiredRotation = bMouseFacingActive;
	GetCharacterMovement()->RotationRate = FRotator(
		0.0f,
		bMouseFacingActive ? MouseFacingTurnRateDegreesPerSecond : DefaultRotationRateDegreesPerSecond,
		0.0f);

	if (!bMouseFacingActive)
	{
		if (!bKeyboardTurnInPlaceActive)
		{
			EndTurnInPlace(true);
		}
		return;
	}

	bKeyboardTurnInPlaceActive = false;

	if (IsDualMouseDriveActive())
	{
		EndTurnInPlace(true);
		AddMovementInput(GetActorForwardVector(), 1.0f);
		return;
	}

	if (GetVelocity().SizeSquared2D() > KINDA_SMALL_NUMBER)
	{
		EndTurnInPlace(true);
		return;
	}

	const float YawDeltaDegrees = FMath::FindDeltaAngleDegrees(
		GetActorRotation().Yaw,
		GetControlRotation().Yaw);

	if (FMath::Abs(YawDeltaDegrees) >= TurnInPlaceActivationAngleDegrees)
	{
		BeginTurnInPlace(YawDeltaDegrees);
		return;
	}

	EndTurnInPlace();
}

void AWUCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		const bool bHasForwardInput = !FMath::IsNearlyZero(Forward);
		const bool bHasRightInput = !FMath::IsNearlyZero(Right);

		if (bHasForwardInput)
		{
			bKeyboardTurnInPlaceActive = false;
			EndTurnInPlace(true);
		}

		const bool bWantsBackpedal = Forward < -KINDA_SMALL_NUMBER;

		if (bWantsBackpedal)
		{
			bKeyboardTurnInPlaceActive = false;
			BeginBackpedal(Right);
			AddMovementInput(GetActorForwardVector(), Forward);
			AddMovementInput(GetActorRightVector(), Right);
			return;
		}

		EndBackpedal();

		if (bHasForwardInput)
		{
			AddMovementInput(GetActorForwardVector(), Forward);
		}

		if (bHasRightInput && !IsMouseFacingControlActive())
		{
			if (!bHasForwardInput)
			{
				bKeyboardTurnInPlaceActive = true;
				BeginTurnInPlace(Right);
			}

			const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0f;
			const float YawDelta = Right * KeyboardTurnRateDegreesPerSecond * DeltaSeconds;
			AddActorLocalRotation(FRotator(0.0f, YawDelta, 0.0f));
		}
	}
}

void AWUCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AWUCharacter::DoJumpStart()
{
	Jump();
}

void AWUCharacter::DoJumpEnd()
{
	StopJumping();
}

float AWUCharacter::CalculateDamage() const
{
	return BaseAttackDamage * (1.0f + (DerivedStats.SpellPowerPercent * 0.01f));
}

void AWUCharacter::ApplyCharacterProgression(EWUCharacterRace NewBloodStatus, int32 NewLevel)
{
	ApplyCharacterProgressionInternal(NewBloodStatus, NewLevel, 0, true);

	if (!HasAuthority())
	{
		ServerApplyCharacterProgression(NewBloodStatus, NewLevel);
	}
}

void AWUCharacter::ServerApplyCharacterProgression_Implementation(EWUCharacterRace NewBloodStatus, int32 NewLevel)
{
	ApplyCharacterProgressionInternal(NewBloodStatus, NewLevel, 0, true);
	ForceNetUpdate();
}

void AWUCharacter::ApplyCharacterProgressionState(EWUCharacterRace NewBloodStatus, int32 NewLevel, int32 NewExperience)
{
	ApplyCharacterProgressionInternal(NewBloodStatus, NewLevel, NewExperience, true);

	if (!HasAuthority())
	{
		ServerApplyCharacterProgressionState(NewBloodStatus, NewLevel, NewExperience);
	}
}

void AWUCharacter::ServerApplyCharacterProgressionState_Implementation(EWUCharacterRace NewBloodStatus, int32 NewLevel, int32 NewExperience)
{
	ApplyCharacterProgressionInternal(NewBloodStatus, NewLevel, NewExperience, true);
	ForceNetUpdate();
}

void AWUCharacter::ApplyCharacterProgressionInternal(EWUCharacterRace NewBloodStatus, int32 NewLevel, int32 NewExperience, bool bResetResources)
{
	const FWUExperienceProgression NormalizedProgression = WUCharacterStats::ResolveExperienceAward(NewLevel, NewExperience, 0);
	BloodStatus = NewBloodStatus;
	CharacterLevel = NormalizedProgression.Level;
	CharacterExperience = NormalizedProgression.Experience;
	PrimaryStats = WUCharacterStats::CalculatePrimaryStats(BloodStatus, CharacterLevel);
	DerivedStats = WUCharacterStats::CalculateDerivedStats(PrimaryStats);

	MaxHealth = DerivedStats.MaxHealth;
	MaxMagic = DerivedStats.MaxMagic;

	if (bResetResources)
	{
		Health = MaxHealth;
		Magic = MaxMagic;
		bInCombat = false;
	}
	else
	{
		Health = FMath::Clamp(Health, 0.0f, MaxHealth);
		Magic = FMath::Clamp(Magic, 0.0f, MaxMagic);
	}
}

void AWUCharacter::OnRep_CharacterStats()
{
	const FWUExperienceProgression NormalizedProgression = WUCharacterStats::ResolveExperienceAward(CharacterLevel, CharacterExperience, 0);
	CharacterLevel = NormalizedProgression.Level;
	CharacterExperience = NormalizedProgression.Experience;
	Health = FMath::Clamp(Health, 0.0f, MaxHealth);
	Magic = FMath::Clamp(Magic, 0.0f, MaxMagic);
}

void AWUCharacter::OnRep_InventoryChanged()
{
	ApplyEquippedItemMeshes();
}

void AWUCharacter::OnRep_CharacterAppearance()
{
	ApplyCharacterAppearanceMeshes();
}

void AWUCharacter::OnRep_CurrentZone()
{
}

float AWUCharacter::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f) : 0.0f;
}

float AWUCharacter::GetCurrentHealth() const
{
	return Health;
}

float AWUCharacter::GetMaxHealth() const
{
	return MaxHealth;
}

float AWUCharacter::GetMagicPercent() const
{
	return MaxMagic > 0.0f ? FMath::Clamp(Magic / MaxMagic, 0.0f, 1.0f) : 0.0f;
}

float AWUCharacter::GetCurrentMagic() const
{
	return Magic;
}

float AWUCharacter::GetMaxMagic() const
{
	return MaxMagic;
}

bool AWUCharacter::IsInCombat() const
{
	return bInCombat;
}

TArray<FWUInventorySlot> AWUCharacter::GetInventorySlots() const
{
	return InventorySlots;
}

TArray<FWUEquipmentSlotEntry> AWUCharacter::GetEquipmentSlots() const
{
	return EquipmentSlots;
}

bool AWUCharacter::GetInventorySlot(int32 SlotIndex, FWUInventorySlot& OutSlot) const
{
	if (!InventorySlots.IsValidIndex(SlotIndex))
	{
		OutSlot = FWUInventorySlot();
		return false;
	}

	OutSlot = InventorySlots[SlotIndex];
	return true;
}

bool AWUCharacter::GetEquippedItem(EWUEquipmentSlot EquipmentSlot, FWUInventoryItem& OutItem) const
{
	const int32 EquipmentIndex = FindEquipmentEntryIndex(EquipmentSlot);
	if (!EquipmentSlots.IsValidIndex(EquipmentIndex) || !EquipmentSlots[EquipmentIndex].bHasItem)
	{
		OutItem = FWUInventoryItem();
		return false;
	}

	OutItem = EquipmentSlots[EquipmentIndex].Item;
	return true;
}

FWUCharacterAppearance AWUCharacter::GetCharacterAppearance() const
{
	return CharacterAppearance;
}

void AWUCharacter::ApplyCharacterAppearance(const FWUCharacterAppearance& NewAppearance)
{
	CharacterAppearance = NewAppearance;
	ApplyCharacterAppearanceMeshes();

	if (!HasAuthority())
	{
		ServerApplyCharacterAppearance(NewAppearance);
		return;
	}

	ForceNetUpdate();
}

bool AWUCharacter::EquipInventorySlot(int32 SlotIndex)
{
	if (!HasAuthority())
	{
		ServerEquipInventorySlot(SlotIndex);
		return true;
	}

	InitializeInventoryStorage();

	if (!InventorySlots.IsValidIndex(SlotIndex) || !InventorySlots[SlotIndex].bHasItem)
	{
		return false;
	}

	const FWUInventoryItem ItemToEquip = InventorySlots[SlotIndex].Item;
	if (!ItemToEquip.bEquippable)
	{
		return false;
	}

	const int32 EquipmentIndex = FindEquipmentEntryIndex(ItemToEquip.EquipmentSlot);
	if (!EquipmentSlots.IsValidIndex(EquipmentIndex))
	{
		return false;
	}

	if (EquipmentSlots[EquipmentIndex].bHasItem)
	{
		InventorySlots[SlotIndex].Item = EquipmentSlots[EquipmentIndex].Item;
		InventorySlots[SlotIndex].bHasItem = true;
	}
	else
	{
		InventorySlots[SlotIndex] = FWUInventorySlot();
	}

	EquipmentSlots[EquipmentIndex].Item = ItemToEquip;
	EquipmentSlots[EquipmentIndex].bHasItem = true;
	ApplyEquippedItemMeshes();
	ForceNetUpdate();
	return true;
}

bool AWUCharacter::UnequipEquipmentSlot(EWUEquipmentSlot EquipmentSlot)
{
	if (!HasAuthority())
	{
		ServerUnequipEquipmentSlot(EquipmentSlot);
		return true;
	}

	InitializeInventoryStorage();

	const int32 EquipmentIndex = FindEquipmentEntryIndex(EquipmentSlot);
	if (!EquipmentSlots.IsValidIndex(EquipmentIndex) || !EquipmentSlots[EquipmentIndex].bHasItem)
	{
		return false;
	}

	const int32 FreeInventorySlot = FindFirstFreeInventorySlot();
	if (!InventorySlots.IsValidIndex(FreeInventorySlot))
	{
		return false;
	}

	InventorySlots[FreeInventorySlot].Item = EquipmentSlots[EquipmentIndex].Item;
	InventorySlots[FreeInventorySlot].bHasItem = true;
	EquipmentSlots[EquipmentIndex].Item = FWUInventoryItem();
	EquipmentSlots[EquipmentIndex].bHasItem = false;
	ApplyEquippedItemMeshes();
	ForceNetUpdate();
	return true;
}

EWUCharacterRace AWUCharacter::GetBloodStatus() const
{
	return BloodStatus;
}

int32 AWUCharacter::GetCharacterLevel() const
{
	return CharacterLevel;
}

int32 AWUCharacter::GetCharacterExperience() const
{
	return CharacterExperience;
}

int32 AWUCharacter::GetExperienceToNextLevel() const
{
	return WUCharacterStats::GetExperienceToNextLevel(CharacterLevel);
}

float AWUCharacter::GetExperiencePercent() const
{
	const int32 ExperienceToNext = GetExperienceToNextLevel();
	return ExperienceToNext > 0
		? FMath::Clamp(static_cast<float>(CharacterExperience) / static_cast<float>(ExperienceToNext), 0.0f, 1.0f)
		: 1.0f;
}

FWUPrimaryStats AWUCharacter::GetPrimaryStats() const
{
	return PrimaryStats;
}

FWUDerivedStats AWUCharacter::GetDerivedStats() const
{
	return DerivedStats;
}

void AWUCharacter::AwardExperience(int32 Amount, EWUExperienceSource Source)
{
	if (!HasAuthority() || Amount <= 0)
	{
		return;
	}

	const FWUExperienceProgression UpdatedProgression = WUCharacterStats::ResolveExperienceAward(
		CharacterLevel,
		CharacterExperience,
		Amount);
	const bool bLeveledUp = UpdatedProgression.Level != CharacterLevel;
	ApplyCharacterProgressionInternal(BloodStatus, UpdatedProgression.Level, UpdatedProgression.Experience, false);
	ForceNetUpdate();

	if (AWUPlayerController* OwningController = Cast<AWUPlayerController>(GetController()))
	{
		OwningController->Client_HandleExperienceAward(Amount, Source);
	}

	if (GEngine)
	{
		const FString SourceLabel = StaticEnum<EWUExperienceSource>()
			? StaticEnum<EWUExperienceSource>()->GetNameStringByValue(static_cast<int64>(Source))
			: TEXT("Experience");
		const FString ProgressionMessage = bLeveledUp
			? FString::Printf(TEXT("%s gained %d %s XP and reached level %d"), *GetName(), Amount, *SourceLabel, CharacterLevel)
			: FString::Printf(TEXT("%s gained %d %s XP"), *GetName(), Amount, *SourceLabel);
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, ProgressionMessage);
	}
}

FText AWUCharacter::GetDisplayName() const
{
	if (const APlayerState* CurrentPlayerState = GetPlayerState())
	{
		const FString PlayerName = CurrentPlayerState->GetPlayerName();
		if (!PlayerName.IsEmpty())
		{
			return FText::FromString(PlayerName);
		}
	}

	if (!DisplayName.IsEmpty())
	{
		return DisplayName;
	}

	return FText::FromString(GetName());
}

void AWUCharacter::SetDisplayName(const FText& NewDisplayName)
{
	DisplayName = NewDisplayName;
}

UTexture2D* AWUCharacter::GetPortraitTexture() const
{
	return PortraitTexture;
}

FName AWUCharacter::GetCurrentZoneId() const
{
	return CurrentZoneId;
}

FText AWUCharacter::GetCurrentZoneDisplayName() const
{
	return FText::FromString(CurrentZoneDisplayName);
}

FName AWUCharacter::GetCurrentMapRegionId() const
{
	return CurrentMapRegionId;
}

FName AWUCharacter::GetCurrentGraveyardTag() const
{
	return CurrentGraveyardTag;
}

void AWUCharacter::SetCurrentZone(FName NewZoneId, const FText& NewDisplayName, FName NewMapRegionId, FName NewGraveyardTag)
{
	if (!HasAuthority() || NewZoneId.IsNone())
	{
		return;
	}

	if (CurrentZoneId == NewZoneId
		&& CurrentMapRegionId == NewMapRegionId
		&& CurrentGraveyardTag == NewGraveyardTag
		&& CurrentZoneDisplayName == NewDisplayName.ToString())
	{
		return;
	}

	CurrentZoneId = NewZoneId;
	CurrentZoneDisplayName = NewDisplayName.ToString();
	CurrentMapRegionId = NewMapRegionId;
	CurrentGraveyardTag = NewGraveyardTag;
	ForceNetUpdate();
}

bool AWUCharacter::IsDead() const
{
	return bIsDead;
}

bool AWUCharacter::HasReleased() const
{
	return bHasReleased;
}

bool AWUCharacter::IsReleasedSpirit() const
{
	return bIsDead && bHasReleased;
}

bool AWUCharacter::CanReviveAtCorpse() const
{
	if (!IsReleasedSpirit())
	{
		return false;
	}

	return FVector::Dist(GetActorLocation(), DeathLocation) <= 200.0f;
}

void AWUCharacter::UpdateDeathStateEffects()
{
	// Dead and waiting to release
	if (bIsDead && !bHasReleased)
	{
		GetCharacterMovement()->DisableMovement();

		if (GetMesh())
		{
			GetMesh()->SetRenderCustomDepth(false);
		}

		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		return;
	}

	// Dead but released: spirit form
	if (bIsDead && bHasReleased)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		GetCharacterMovement()->MaxWalkSpeed = 700.0f;

		// Let spirit players pass through other players
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

		// Temporary visual hook for spirit form
		if (GetMesh())
		{
			GetMesh()->SetRenderCustomDepth(true);
		}

		return;
	}

	// Alive
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	if (GetMesh())
	{
		GetMesh()->SetRenderCustomDepth(false);
	}
}

void AWUCharacter::EnterCombatState()
{
	if (!HasAuthority())
	{
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		LastCombatEventTimeSeconds = World->GetTimeSeconds();
	}

	bInCombat = true;
}

void AWUCharacter::UpdateCombatState()
{
	if (!bInCombat)
	{
		return;
	}

	if (bIsDead || Health <= 0.0f)
	{
		bInCombat = false;
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (World->GetTimeSeconds() - LastCombatEventTimeSeconds >= CombatTimeoutSeconds)
	{
		bInCombat = false;
	}
}

void AWUCharacter::RegenerateResources(float DeltaSeconds)
{
	if (DeltaSeconds <= 0.0f || bIsDead || Health <= 0.0f)
	{
		return;
	}

	const float HealthRegenRate = bInCombat
		? DerivedStats.HealthRegenInCombatPerSecond
		: DerivedStats.HealthRegenOutOfCombatPerSecond;
	const float MagicRegenRate = bInCombat
		? DerivedStats.MagicRegenInCombatPerSecond
		: DerivedStats.MagicRegenOutOfCombatPerSecond;

	if (HealthRegenRate > 0.0f && Health < MaxHealth)
	{
		Health = FMath::Min(MaxHealth, Health + (HealthRegenRate * DeltaSeconds));
	}

	if (MagicRegenRate > 0.0f && Magic < MaxMagic)
	{
		Magic = FMath::Min(MaxMagic, Magic + (MagicRegenRate * DeltaSeconds));
	}
}

void AWUCharacter::InitializeInventoryStorage()
{
	if (MaxInventorySlots < 1)
	{
		MaxInventorySlots = 1;
	}

	if (InventorySlots.Num() != MaxInventorySlots)
	{
		InventorySlots.SetNum(MaxInventorySlots);
	}

	const TArray<EWUEquipmentSlot>& AllEquipmentSlots = WUInventory::GetAllEquipmentSlots();
	for (EWUEquipmentSlot EquipmentSlot : AllEquipmentSlots)
	{
		if (FindEquipmentEntryIndex(EquipmentSlot) == INDEX_NONE)
		{
			FWUEquipmentSlotEntry Entry;
			Entry.Slot = EquipmentSlot;
			EquipmentSlots.Add(Entry);
		}
	}
}

void AWUCharacter::SeedStarterInventory()
{
	bool bHasAnyItem = false;
	for (const FWUInventorySlot& InventorySlot : InventorySlots)
	{
		if (InventorySlot.bHasItem)
		{
			bHasAnyItem = true;
			break;
		}
	}

	if (!bHasAnyItem)
	{
		for (const FWUEquipmentSlotEntry& EquipmentEntry : EquipmentSlots)
		{
			if (EquipmentEntry.bHasItem)
			{
				bHasAnyItem = true;
				break;
			}
		}
	}

	if (bHasAnyItem)
	{
		if (!HasAnyEquippedItem())
		{
			for (const FName StarterItemId : WUInventory::GetStarterEquippedItemIds())
			{
				EquipStarterVisualItem(WUInventory::MakeItem(StarterItemId));
			}
		}
		return;
	}

	for (const FName StarterItemId : WUInventory::GetStarterEquippedItemIds())
	{
		EquipStarterVisualItem(WUInventory::MakeItem(StarterItemId));
	}

	for (const FName StarterItemId : WUInventory::GetStarterInventoryItemIds())
	{
		AddItemToInventory(WUInventory::MakeItem(StarterItemId));
	}
}

void AWUCharacter::EquipStarterVisualItem(const FWUInventoryItem& Item)
{
	const int32 EquipmentIndex = FindEquipmentEntryIndex(Item.EquipmentSlot);
	if (!EquipmentSlots.IsValidIndex(EquipmentIndex) || EquipmentSlots[EquipmentIndex].bHasItem)
	{
		return;
	}

	FWUInventoryItem ItemToEquip = Item;
	for (FWUInventorySlot& InventorySlot : InventorySlots)
	{
		if (InventorySlot.bHasItem && InventorySlot.Item.ItemId == Item.ItemId)
		{
			ItemToEquip = InventorySlot.Item;
			InventorySlot = FWUInventorySlot();
			break;
		}
	}

	EquipmentSlots[EquipmentIndex].Item = ItemToEquip;
	EquipmentSlots[EquipmentIndex].bHasItem = true;
}

void AWUCharacter::BeginBackpedal(float Right)
{
	bKeyboardTurnInPlaceActive = false;
	EndTurnInPlace(true);

	if (!bBackpedaling)
	{
		PreBackpedalMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	}

	bBackpedaling = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = FMath::Min(PreBackpedalMaxWalkSpeed, BackpedalMaxWalkSpeed);

	const TCHAR* BackpedalAnimationPath = GetBackpedalAnimationPath(CharacterAppearance.Sex, Right);
	if (!BackpedalAnimationPath || CurrentBackpedalAnimationPath == BackpedalAnimationPath)
	{
		return;
	}

	if (UAnimationAsset* BackpedalAnimation = LoadAnimationAssetForPath(BackpedalAnimationPath))
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		GetMesh()->SetPlayRate(1.0f);
		GetMesh()->PlayAnimation(BackpedalAnimation, true);
		bBackpedalAnimationActive = true;
		CurrentBackpedalAnimationPath = BackpedalAnimationPath;
	}
}

void AWUCharacter::EndBackpedal()
{
	if (bBackpedaling)
	{
		GetCharacterMovement()->MaxWalkSpeed = PreBackpedalMaxWalkSpeed;
	}

	bBackpedaling = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	if (!bBackpedalAnimationActive)
	{
		return;
	}

	bBackpedalAnimationActive = false;
	CurrentBackpedalAnimationPath.Reset();
	RestoreDefaultLocomotionAnimation();
}

void AWUCharacter::BeginTurnInPlace(float YawDeltaDegrees)
{
	if (bBackpedaling)
	{
		return;
	}

	const TCHAR* TurnAnimationPath = GetTurnInPlaceAnimationPath(CharacterAppearance.Sex, YawDeltaDegrees);
	if (!TurnAnimationPath)
	{
		return;
	}

	if (bTurnInPlaceAnimationActive && CurrentTurnInPlaceAnimationPath == TurnAnimationPath)
	{
		return;
	}

	if (UAnimationAsset* TurnAnimation = LoadAnimationAssetForPath(TurnAnimationPath))
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		GetMesh()->PlayAnimation(TurnAnimation, true);
		GetMesh()->SetPlayRate(TurnInPlaceAnimationPlayRate);
		bTurnInPlaceAnimationActive = true;
		CurrentTurnInPlaceAnimationPath = TurnAnimationPath;

		if (const UWorld* World = GetWorld())
		{
			TurnInPlaceAnimationHoldUntilSeconds = World->GetTimeSeconds() + TurnInPlaceAnimationMinSeconds;
		}
	}
}

void AWUCharacter::EndTurnInPlace(bool bForce)
{
	if (!bTurnInPlaceAnimationActive)
	{
		return;
	}

	if (!bForce)
	{
		if (const UWorld* World = GetWorld())
		{
			if (World->GetTimeSeconds() < TurnInPlaceAnimationHoldUntilSeconds)
			{
				return;
			}
		}
	}

	bTurnInPlaceAnimationActive = false;
	CurrentTurnInPlaceAnimationPath.Reset();
	TurnInPlaceAnimationHoldUntilSeconds = 0.0f;

	if (!bBackpedalAnimationActive)
	{
		RestoreDefaultLocomotionAnimation();
	}
}

void AWUCharacter::RestoreDefaultLocomotionAnimation()
{
	if (UClass* AnimClass = LoadAnimClassForPath(GetAnimationBlueprintPath(CharacterAppearance.Sex)))
	{
		GetMesh()->SetPlayRate(1.0f);
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(AnimClass);
	}
}

int32 AWUCharacter::FindEquipmentEntryIndex(EWUEquipmentSlot EquipmentSlot) const
{
	for (int32 Index = 0; Index < EquipmentSlots.Num(); ++Index)
	{
		if (EquipmentSlots[Index].Slot == EquipmentSlot)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

int32 AWUCharacter::FindFirstFreeInventorySlot() const
{
	for (int32 Index = 0; Index < InventorySlots.Num(); ++Index)
	{
		if (!InventorySlots[Index].bHasItem)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

bool AWUCharacter::HasAnyEquippedItem() const
{
	for (const FWUEquipmentSlotEntry& EquipmentEntry : EquipmentSlots)
	{
		if (EquipmentEntry.bHasItem)
		{
			return true;
		}
	}

	return false;
}

bool AWUCharacter::AddItemToInventory(const FWUInventoryItem& Item)
{
	const int32 FreeInventorySlot = FindFirstFreeInventorySlot();
	if (!InventorySlots.IsValidIndex(FreeInventorySlot))
	{
		return false;
	}

	InventorySlots[FreeInventorySlot].Item = Item;
	InventorySlots[FreeInventorySlot].bHasItem = true;
	return true;
}

void AWUCharacter::ApplyCameraCollisionRules() const
{
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}

	if (USkeletalMeshComponent* BodyMesh = GetMesh())
	{
		BodyMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
}

void AWUCharacter::ConfigureModularMeshComponent(USkeletalMeshComponent* MeshComponent) const
{
	if (!MeshComponent)
	{
		return;
	}

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetLeaderPoseComponent(GetMesh());
	MeshComponent->SetIsReplicated(false);
}

void AWUCharacter::ApplyCharacterAppearanceMeshes()
{
	if (USkeletalMesh* BodyMesh = LoadSkeletalMeshForPath(GetBodyMeshPath(CharacterAppearance.Sex)))
	{
		GetMesh()->SetSkeletalMesh(BodyMesh);
	}

	if (UClass* AnimClass = LoadAnimClassForPath(GetAnimationBlueprintPath(CharacterAppearance.Sex)))
	{
		GetMesh()->SetAnimInstanceClass(AnimClass);
	}

	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	GetMesh()->SetVisibility(false, false);
	GetMesh()->SetHiddenInGame(true, false);

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

	if (USkeletalMesh* HeadMesh = LoadSkeletalMeshForPath(GetHeadMeshPath(CharacterAppearance.Sex)))
	{
		HeadMeshComponent->SetSkeletalMesh(HeadMesh);
		HeadMeshComponent->SetLeaderPoseComponent(GetMesh());
		ShowModularMesh(HeadMeshComponent);
	}

	if (USkeletalMesh* HairMesh = LoadSkeletalMeshForPath(GetHairMeshPath(CharacterAppearance.Sex, CharacterAppearance.HairStyleIndex)))
	{
		HairMeshComponent->SetSkeletalMesh(HairMesh);
		HairMeshComponent->SetLeaderPoseComponent(GetMesh());
		ShowModularMesh(HairMeshComponent);
	}
	else
	{
		HideModularMesh(HairMeshComponent);
	}

	if (USkeletalMesh* BrowsMesh = LoadSkeletalMeshForPath(GetBrowsMeshPath(CharacterAppearance.Sex, CharacterAppearance.BrowStyleIndex)))
	{
		BrowsMeshComponent->SetSkeletalMesh(BrowsMesh);
		BrowsMeshComponent->SetLeaderPoseComponent(GetMesh());
		ShowModularMesh(BrowsMeshComponent);
	}
	else
	{
		HideModularMesh(BrowsMeshComponent);
	}

	if (USkeletalMesh* BeardMesh = LoadSkeletalMeshForPath(GetBeardMeshPath(CharacterAppearance.Sex, CharacterAppearance.BeardStyleIndex)))
	{
		BeardMeshComponent->SetSkeletalMesh(BeardMesh);
		BeardMeshComponent->SetLeaderPoseComponent(GetMesh());
		ShowModularMesh(BeardMeshComponent);
	}
	else
	{
		HideModularMesh(BeardMeshComponent);
	}

	if (USkeletalMesh* PantsMesh = LoadSkeletalMeshForPath(GetPantsMeshPath(CharacterAppearance.Sex)))
	{
		PantsMeshComponent->SetSkeletalMesh(PantsMesh);
		PantsMeshComponent->SetLeaderPoseComponent(GetMesh());
		ShowModularMesh(PantsMeshComponent);
	}

	if (USkeletalMesh* HandsMesh = LoadSkeletalMeshForPath(GetHandsMeshPath(CharacterAppearance.Sex)))
	{
		HandsMeshComponent->SetSkeletalMesh(HandsMesh);
		HandsMeshComponent->SetLeaderPoseComponent(GetMesh());
		ShowModularMesh(HandsMeshComponent);
	}

	if (USkeletalMesh* BracersMesh = LoadSkeletalMeshForPath(GetBracersMeshPath(CharacterAppearance.Sex)))
	{
		BracersMeshComponent->SetSkeletalMesh(BracersMesh);
		BracersMeshComponent->SetLeaderPoseComponent(GetMesh());
		ShowModularMesh(BracersMeshComponent);
	}

	if (USkeletalMesh* ChestOutfitMesh = LoadSkeletalMeshForPath(GetStarterChestOutfitMeshPath(CharacterAppearance.Sex)))
	{
		ChestOutfitMeshComponent->SetSkeletalMesh(ChestOutfitMesh);
		ChestOutfitMeshComponent->SetLeaderPoseComponent(GetMesh());
		ShowModularMesh(ChestOutfitMeshComponent);
	}

	if (USkeletalMesh* ChestAddOutfitMesh = LoadSkeletalMeshForPath(GetStarterChestAddOutfitMeshPath(CharacterAppearance.Sex)))
	{
		ChestAddOutfitMeshComponent->SetSkeletalMesh(ChestAddOutfitMesh);
		ChestAddOutfitMeshComponent->SetLeaderPoseComponent(GetMesh());
		ShowModularMesh(ChestAddOutfitMeshComponent);
	}

	if (USkeletalMesh* BeltOutfitMesh = LoadSkeletalMeshForPath(GetStarterBeltOutfitMeshPath(CharacterAppearance.Sex)))
	{
		BeltOutfitMeshComponent->SetSkeletalMesh(BeltOutfitMesh);
		BeltOutfitMeshComponent->SetLeaderPoseComponent(GetMesh());
		ShowModularMesh(BeltOutfitMeshComponent);
	}

	if (USkeletalMesh* BootsOutfitMesh = LoadSkeletalMeshForPath(GetStarterBootsOutfitMeshPath(CharacterAppearance.Sex)))
	{
		BootsOutfitMeshComponent->SetSkeletalMesh(BootsOutfitMesh);
		BootsOutfitMeshComponent->SetLeaderPoseComponent(GetMesh());
		ShowModularMesh(BootsOutfitMeshComponent);
	}

	if (UMaterialInterface* BodyMaterial = LoadMaterialForPath(GetBodyMaterialPath(CharacterAppearance.Sex, CharacterAppearance.SkinPresetIndex)))
	{
		const TArray<FName> BodyMaterialSlotNames = GetMesh()->GetMaterialSlotNames();
		bool bAppliedBodyMaterial = false;
		for (int32 MaterialIndex = 0; MaterialIndex < BodyMaterialSlotNames.Num(); ++MaterialIndex)
		{
			const FString SlotName = BodyMaterialSlotNames[MaterialIndex].ToString();
			if (!SlotName.Contains(TEXT("Eye"), ESearchCase::IgnoreCase)
				&& !SlotName.Contains(TEXT("Head"), ESearchCase::IgnoreCase)
				&& !SlotName.Contains(TEXT("Face"), ESearchCase::IgnoreCase))
			{
				GetMesh()->SetMaterial(MaterialIndex, BodyMaterial);
				bAppliedBodyMaterial = true;
			}
		}

		if (!bAppliedBodyMaterial)
		{
			const int32 MaterialCount = GetMesh()->GetNumMaterials();
			for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
			{
				GetMesh()->SetMaterial(MaterialIndex, BodyMaterial);
			}
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

	if (UMaterialInterface* HeadMaterial = LoadMaterialForPath(GetHeadMaterialPath(CharacterAppearance.Sex, CharacterAppearance.HeadPresetIndex)))
	{
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
				HeadMeshComponent->SetMaterial(MaterialIndex, HeadMaterial);
				bAppliedHeadMaterial = true;
			}
		}

		if (!bAppliedHeadMaterial && HeadMeshComponent->GetNumMaterials() > 0)
		{
			HeadMeshComponent->SetMaterial(0, HeadMaterial);
		}

		const TArray<FName> BodyMaterialSlotNames = GetMesh()->GetMaterialSlotNames();
		for (int32 MaterialIndex = 0; MaterialIndex < BodyMaterialSlotNames.Num(); ++MaterialIndex)
		{
			const FString SlotName = BodyMaterialSlotNames[MaterialIndex].ToString();
			if (!SlotName.Contains(TEXT("Eye"), ESearchCase::IgnoreCase)
				&& (SlotName.Contains(TEXT("Head"), ESearchCase::IgnoreCase)
					|| SlotName.Contains(TEXT("Face"), ESearchCase::IgnoreCase)))
			{
				GetMesh()->SetMaterial(MaterialIndex, HeadMaterial);
			}
		}
	}

	if (UMaterialInterface* EyeMaterial = LoadMaterialForPath(GetEyeMaterialPath(CharacterAppearance.EyeColorIndex)))
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

		const TArray<FName> BodyMaterialSlotNames = GetMesh()->GetMaterialSlotNames();
		for (int32 MaterialIndex = 0; MaterialIndex < BodyMaterialSlotNames.Num(); ++MaterialIndex)
		{
			const FString SlotName = BodyMaterialSlotNames[MaterialIndex].ToString();
			if (SlotName.Contains(TEXT("Eye"), ESearchCase::IgnoreCase))
			{
				GetMesh()->SetMaterial(MaterialIndex, EyeMaterial);
			}
		}
	}

	if (UMaterialInterface* HairMaterial = LoadMaterialForPath(GetHairMaterialPath(CharacterAppearance.HairColorIndex)))
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

	ApplyEquippedItemMeshes();

	LogMaterialSlotsForComponent(this, TEXT("FullBody"), GetMesh());
	LogMaterialSlotsForComponent(this, TEXT("Head"), HeadMeshComponent);
	LogMaterialSlotsForComponent(this, TEXT("Hair"), HairMeshComponent);
	LogMaterialSlotsForComponent(this, TEXT("Brows"), BrowsMeshComponent);
	LogMaterialSlotsForComponent(this, TEXT("Beard"), BeardMeshComponent);
	LogMaterialSlotsForComponent(this, TEXT("Pants"), PantsMeshComponent);
	LogMaterialSlotsForComponent(this, TEXT("Hands"), HandsMeshComponent);
	LogMaterialSlotsForComponent(this, TEXT("Bracers"), BracersMeshComponent);
	LogMaterialSlotsForComponent(this, TEXT("ChestOutfit"), ChestOutfitMeshComponent);
	LogMaterialSlotsForComponent(this, TEXT("ChestAddOutfit"), ChestAddOutfitMeshComponent);
	LogMaterialSlotsForComponent(this, TEXT("BeltOutfit"), BeltOutfitMeshComponent);
	LogMaterialSlotsForComponent(this, TEXT("BootsOutfit"), BootsOutfitMeshComponent);
}

void AWUCharacter::ApplyEquippedItemMeshes()
{
	if (EquipmentSlots.IsEmpty())
	{
		return;
	}

	const auto SetVisualEnabled = [](USkeletalMeshComponent* MeshComponent, bool bEnabled)
	{
		if (!MeshComponent)
		{
			return;
		}

		MeshComponent->SetVisibility(bEnabled, false);
		MeshComponent->SetHiddenInGame(!bEnabled, false);
	};

	SetVisualEnabled(PantsMeshComponent, IsItemVisualLayerRenderable(EWUItemVisualLayer::Pants));
	SetVisualEnabled(BracersMeshComponent, IsItemVisualLayerRenderable(EWUItemVisualLayer::Bracers));
	SetVisualEnabled(ChestOutfitMeshComponent, IsItemVisualLayerRenderable(EWUItemVisualLayer::ChestOutfit));
	SetVisualEnabled(ChestAddOutfitMeshComponent, IsItemVisualLayerRenderable(EWUItemVisualLayer::ChestAddOutfit));
	SetVisualEnabled(BeltOutfitMeshComponent, IsItemVisualLayerRenderable(EWUItemVisualLayer::BeltOutfit));
	SetVisualEnabled(BootsOutfitMeshComponent, IsItemVisualLayerRenderable(EWUItemVisualLayer::Boots));
	UpdateMasterBodyVisibilityForEquipment();
}

void AWUCharacter::UpdateMasterBodyVisibilityForEquipment()
{
	USkeletalMeshComponent* BodyMesh = GetMesh();
	if (!BodyMesh)
	{
		return;
	}

	const bool bNeedsBaseBody = !HasRenderedFullBodyReplacementVisuals();
	BodyMesh->SetVisibility(bNeedsBaseBody, false);
	BodyMesh->SetHiddenInGame(!bNeedsBaseBody, false);

	if (HeadMeshComponent)
	{
		// The fallback full-body meshes include a head. Showing the separate
		// modular head at the same time causes face z-fighting in naked/partial
		// gear states.
		HeadMeshComponent->SetVisibility(!bNeedsBaseBody, false);
		HeadMeshComponent->SetHiddenInGame(bNeedsBaseBody, false);
	}
}

bool AWUCharacter::HasRenderedFullBodyReplacementVisuals() const
{
	return IsItemVisualLayerRenderable(EWUItemVisualLayer::ChestOutfit)
		&& IsItemVisualLayerRenderable(EWUItemVisualLayer::Pants);
}

bool AWUCharacter::IsItemVisualLayerEquipped(EWUItemVisualLayer VisualLayer) const
{
	if (VisualLayer == EWUItemVisualLayer::None)
	{
		return false;
	}

	for (const FWUEquipmentSlotEntry& EquipmentEntry : EquipmentSlots)
	{
		if (EquipmentEntry.bHasItem && EquipmentEntry.Item.VisualLayer == VisualLayer)
		{
			return true;
		}
	}

	return false;
}

bool AWUCharacter::IsItemVisualLayerRenderable(EWUItemVisualLayer VisualLayer) const
{
	if (!IsItemVisualLayerEquipped(VisualLayer))
	{
		return false;
	}

	switch (VisualLayer)
	{
	case EWUItemVisualLayer::None:
		return false;
	case EWUItemVisualLayer::ChestOutfit:
		return true;
	case EWUItemVisualLayer::ChestAddOutfit:
	case EWUItemVisualLayer::Bracers:
	case EWUItemVisualLayer::Pants:
		return IsItemVisualLayerEquipped(EWUItemVisualLayer::ChestOutfit);
	case EWUItemVisualLayer::BeltOutfit:
	case EWUItemVisualLayer::Boots:
		return IsItemVisualLayerEquipped(EWUItemVisualLayer::ChestOutfit)
			&& IsItemVisualLayerEquipped(EWUItemVisualLayer::Pants);
	default:
		return false;
	}
}

USkeletalMesh* AWUCharacter::LoadSkeletalMeshForPath(const TCHAR* AssetPath) const
{
	return AssetPath ? LoadObject<USkeletalMesh>(nullptr, AssetPath) : nullptr;
}

UMaterialInterface* AWUCharacter::LoadMaterialForPath(const TCHAR* AssetPath) const
{
	return AssetPath ? LoadObject<UMaterialInterface>(nullptr, AssetPath) : nullptr;
}

UAnimationAsset* AWUCharacter::LoadAnimationAssetForPath(const TCHAR* AssetPath) const
{
	return AssetPath ? LoadObject<UAnimationAsset>(nullptr, AssetPath) : nullptr;
}

UClass* AWUCharacter::LoadAnimClassForPath(const TCHAR* AssetPath) const
{
	return AssetPath ? LoadClass<UAnimInstance>(nullptr, AssetPath) : nullptr;
}

const TCHAR* AWUCharacter::GetBodyMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/SK_Hu_F_FullBody.SK_Hu_F_FullBody")
		: TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/SK_Hu_M_FullBody.SK_Hu_M_FullBody");
}

const TCHAR* AWUCharacter::GetHeadMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Base/SK_Hu_F_Head.SK_Hu_F_Head")
		: TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Base/SK_Hu_M_Head.SK_Hu_M_Head");
}

const TCHAR* AWUCharacter::GetHairMeshPath(EWUCharacterSex Sex, int32 HairStyleIndex) const
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
		return FemaleHairPaths[NormalizeAppearanceIndex(HairStyleIndex, UE_ARRAY_COUNT(FemaleHairPaths))];
	}

	return MaleHairPaths[NormalizeAppearanceIndex(HairStyleIndex, UE_ARRAY_COUNT(MaleHairPaths))];
}

const TCHAR* AWUCharacter::GetBrowsMeshPath(EWUCharacterSex Sex, int32 BrowStyleIndex) const
{
	if (Sex == EWUCharacterSex::Female)
	{
		return nullptr;
	}

	static const TCHAR* MaleBrowsPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Brows_01.SK_Hu_M_Brows_01"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Brows_02.SK_Hu_M_Brows_02"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Brows_03.SK_Hu_M_Brows_03"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Brows_04.SK_Hu_M_Brows_04")
	};

	return MaleBrowsPaths[NormalizeAppearanceIndex(BrowStyleIndex, UE_ARRAY_COUNT(MaleBrowsPaths))];
}

const TCHAR* AWUCharacter::GetBeardMeshPath(EWUCharacterSex Sex, int32 BeardStyleIndex) const
{
	if (Sex == EWUCharacterSex::Female || BeardStyleIndex <= 0)
	{
		return nullptr;
	}

	static const TCHAR* MaleBeardPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_01.SK_Hu_M_Beard_01"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_02.SK_Hu_M_Beard_02"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_03.SK_Hu_M_Beard_03"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_05.SK_Hu_M_Beard_05"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_06.SK_Hu_M_Beard_06"),
		TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Customization/Facial/SK_Hu_M_Beard_07.SK_Hu_M_Beard_07")
	};

	return MaleBeardPaths[NormalizeAppearanceIndex(BeardStyleIndex - 1, UE_ARRAY_COUNT(MaleBeardPaths))];
}

const TCHAR* AWUCharacter::GetBodyMaterialPath(EWUCharacterSex Sex, int32 SkinPresetIndex) const
{
	static const TCHAR* FemaleBodyMaterialPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_01.MI_Hu_F_Body_01"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_02.MI_Hu_F_Body_02"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_03.MI_Hu_F_Body_03"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_04.MI_Hu_F_Body_04"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Body/MI_Hu_F_Body_05.MI_Hu_F_Body_05")
	};

	static const TCHAR* MaleBodyMaterialPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_01.MI_Hu_M_Body_01"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_02.MI_Hu_M_Body_02"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_03.MI_Hu_M_Body_03"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_04.MI_Hu_M_Body_04"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Body/MI_Hu_M_Body_05.MI_Hu_M_Body_05")
	};

	if (Sex == EWUCharacterSex::Female)
	{
		return FemaleBodyMaterialPaths[NormalizeAppearanceIndex(SkinPresetIndex, UE_ARRAY_COUNT(FemaleBodyMaterialPaths))];
	}

	return MaleBodyMaterialPaths[NormalizeAppearanceIndex(SkinPresetIndex, UE_ARRAY_COUNT(MaleBodyMaterialPaths))];
}

const TCHAR* AWUCharacter::GetHeadMaterialPath(EWUCharacterSex Sex, int32 HeadPresetIndex) const
{
	static const TCHAR* FemaleHeadMaterialPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_01_A.MI_Hu_F_Head_01_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_02_A.MI_Hu_F_Head_02_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_03_A.MI_Hu_F_Head_03_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_04_A.MI_Hu_F_Head_04_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Female/Head/MI_Hu_F_Head_05_A.MI_Hu_F_Head_05_A")
	};

	static const TCHAR* MaleHeadMaterialPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_01_A.MI_Hu_M_Head_01_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_02_A.MI_Hu_M_Head_02_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_03_A.MI_Hu_M_Head_03_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_04_A.MI_Hu_M_Head_04_A"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Male/Head/MI_Hu_M_Head_05_A.MI_Hu_M_Head_05_A")
	};

	if (Sex == EWUCharacterSex::Female)
	{
		return FemaleHeadMaterialPaths[NormalizeAppearanceIndex(HeadPresetIndex, UE_ARRAY_COUNT(FemaleHeadMaterialPaths))];
	}

	return MaleHeadMaterialPaths[NormalizeAppearanceIndex(HeadPresetIndex, UE_ARRAY_COUNT(MaleHeadMaterialPaths))];
}

const TCHAR* AWUCharacter::GetEyeMaterialPath(int32 EyeColorIndex) const
{
	static const TCHAR* EyeMaterialPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Eye/MI_HU_Eye_Bl.MI_HU_Eye_Bl"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Eye/MI_HU_Eye_Br.MI_HU_Eye_Br"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Eye/MI_HU_Eye_Gn.MI_HU_Eye_Gn"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Eye/MI_HU_Eye_Pe.MI_HU_Eye_Pe")
	};

	return EyeMaterialPaths[NormalizeAppearanceIndex(EyeColorIndex, UE_ARRAY_COUNT(EyeMaterialPaths))];
}

const TCHAR* AWUCharacter::GetHairMaterialPath(int32 HairColorIndex) const
{
	static const TCHAR* HairMaterialPaths[] =
	{
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Hair/MI_HU_Hair_01_Bk.MI_HU_Hair_01_Bk"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Hair/MI_HU_Hair_01_Bd.MI_HU_Hair_01_Bd"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Hair/MI_HU_Hair_01_Br.MI_HU_Hair_01_Br"),
		TEXT("/Game/StylizedCharacter/Materials/Instances/Character/Human/Hair/MI_HU_Hair_01_Gr.MI_HU_Hair_01_Gr")
	};

	return HairMaterialPaths[NormalizeAppearanceIndex(HairColorIndex, UE_ARRAY_COUNT(HairMaterialPaths))];
}

const TCHAR* AWUCharacter::GetAnimationBlueprintPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Animations/Character/Human/Female/ABP_Hu_F.ABP_Hu_F_C")
		: TEXT("/Game/StylizedCharacter/Animations/Character/Human/Male/ABP_Hu_M.ABP_Hu_M_C");
}

const TCHAR* AWUCharacter::GetBackpedalAnimationPath(EWUCharacterSex Sex, float Right) const
{
	if (Sex == EWUCharacterSex::Female)
	{
		if (Right < -0.2f)
		{
			return TEXT("/Game/StylizedCharacter/Animations/Character/Human/Female/A_Hu_F_Walk_BackwardLeft.A_Hu_F_Walk_BackwardLeft");
		}

		if (Right > 0.2f)
		{
			return TEXT("/Game/StylizedCharacter/Animations/Character/Human/Female/A_Hu_F_Walk_BackwardRight.A_Hu_F_Walk_BackwardRight");
		}

		return TEXT("/Game/StylizedCharacter/Animations/Character/Human/Female/A_Hu_F_Walk_Backwards.A_Hu_F_Walk_Backwards");
	}

	if (Right < -0.2f)
	{
		return TEXT("/Game/StylizedCharacter/Animations/Character/Human/Male/A_Hu_M_Walk_BackwardLeft.A_Hu_M_Walk_BackwardLeft");
	}

	if (Right > 0.2f)
	{
		return TEXT("/Game/StylizedCharacter/Animations/Character/Human/Male/A_Hu_M_Walk_BackwardRight.A_Hu_M_Walk_BackwardRight");
	}

	return TEXT("/Game/StylizedCharacter/Animations/Character/Human/Male/A_Hu_M_Walk_Backwards.A_Hu_M_Walk_Backwards");
}

const TCHAR* AWUCharacter::GetTurnInPlaceAnimationPath(EWUCharacterSex Sex, float YawDeltaDegrees) const
{
	const bool bTurnRight = YawDeltaDegrees > 0.0f;

	if (Sex == EWUCharacterSex::Female)
	{
		return bTurnRight
			? TEXT("/Game/StylizedCharacter/Animations/Character/Human/Female/A_Hu_F_Idle_StepRight.A_Hu_F_Idle_StepRight")
			: TEXT("/Game/StylizedCharacter/Animations/Character/Human/Female/A_Hu_F_Idle_StepLeft.A_Hu_F_Idle_StepLeft");
	}

	return bTurnRight
		? TEXT("/Game/StylizedCharacter/Animations/Character/Human/Male/A_Hu_M_Idle_StepRight.A_Hu_M_Idle_StepRight")
		: TEXT("/Game/StylizedCharacter/Animations/Character/Human/Male/A_Hu_M_Idle_StepLeft.A_Hu_M_Idle_StepLeft");
}

const TCHAR* AWUCharacter::GetPantsMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Base/SK_Hu_F_Pants.SK_Hu_F_Pants")
		: TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Base/SK_Hu_M_Pants.SK_Hu_M_Pants");
}

const TCHAR* AWUCharacter::GetHandsMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Base/SK_Hu_F_Hands.SK_Hu_F_Hands")
		: TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Base/SK_Hu_M_Hands.SK_Hu_M_Hands");
}

const TCHAR* AWUCharacter::GetBracersMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Female/Base/SK_Hu_F_Bracers.SK_Hu_F_Bracers")
		: TEXT("/Game/StylizedCharacter/Meshes/Character/Human/Male/Base/SK_Hu_M_Bracers.SK_Hu_M_Bracers");
}

const TCHAR* AWUCharacter::GetStarterChestOutfitMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/Chest/SK_Hu_F_Chest_Peasant_01.SK_Hu_F_Chest_Peasant_01")
		: TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/Chest/SK_Hu_M_Chest_Peasant_01.SK_Hu_M_Chest_Peasant_01");
}

const TCHAR* AWUCharacter::GetStarterChestAddOutfitMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/ChestAdd/SK_Hu_F_ChestAdd_Peasant.SK_Hu_F_ChestAdd_Peasant")
		: TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/ChestAdd/SK_Hu_M_ChestAdd_Peasant.SK_Hu_M_ChestAdd_Peasant");
}

const TCHAR* AWUCharacter::GetStarterBeltOutfitMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/Belt/SK_Hu_F_Belt_Peasant.SK_Hu_F_Belt_Peasant")
		: TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/Belt/SK_Hu_M_Belt_Peasant.SK_Hu_M_Belt_Peasant");
}

const TCHAR* AWUCharacter::GetStarterBootsOutfitMeshPath(EWUCharacterSex Sex) const
{
	return Sex == EWUCharacterSex::Female
		? TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/Boots/SK_Hu_F_Boots_Peasant.SK_Hu_F_Boots_Peasant")
		: TEXT("/Game/StylizedCharacter/Meshes/Item/Equipment/Boots/SK_Hu_M_Boots_Peasant.SK_Hu_M_Boots_Peasant");
}

int32 AWUCharacter::NormalizeAppearanceIndex(int32 Index, int32 Count) const
{
	if (Count <= 0)
	{
		return 0;
	}

	const int32 Mod = Index % Count;
	return Mod < 0 ? Mod + Count : Mod;
}

void AWUCharacter::ServerApplyCharacterAppearance_Implementation(const FWUCharacterAppearance& NewAppearance)
{
	CharacterAppearance = NewAppearance;
	ApplyCharacterAppearanceMeshes();
	ForceNetUpdate();
}

void AWUCharacter::ServerEquipInventorySlot_Implementation(int32 SlotIndex)
{
	EquipInventorySlot(SlotIndex);
}

void AWUCharacter::ServerUnequipEquipmentSlot_Implementation(EWUEquipmentSlot EquipmentSlot)
{
	UnequipEquipmentSlot(EquipmentSlot);
}

void AWUCharacter::StartAttack()
{
	if (bIsDead)
	{
		return;
	}

	ServerAttack();
}

void AWUCharacter::ServerAttack_Implementation()
{
	if (bIsDead)
	{
		return;
	}

	PerformAttackTrace();
}

void AWUCharacter::PerformAttackTrace()
{
	const FVector Start = GetActorLocation();
	const FVector End = Start + (GetActorForwardVector() * 200.0f);

	FHitResult Hit;
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByObjectType(
		Hit,
		Start,
		End,
		ObjectParams,
		Params
	);

	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Red : FColor::Green, false, 1.5f, 0, 2.0f);

	if (bHit)
	{
		AWUCharacter* HitCharacter = Cast<AWUCharacter>(Hit.GetActor());

		if (HitCharacter && HitCharacter != this)
		{
			const float DamageAmount = CalculateDamage();
			const bool bDamageApplied = HitCharacter->ApplyDamage(DamageAmount, this);

			if (GEngine)
			{
				const FString DebugMessage = bDamageApplied
					? FString::Printf(TEXT("Hit player and applied %.1f damage"), DamageAmount)
					: TEXT("Hit dead player - no damage");

				GEngine->AddOnScreenDebugMessage(
					-1,
					2.0f,
					bDamageApplied ? FColor::Red : FColor::Yellow,
					DebugMessage
				);
			}
		}
	}
}

bool AWUCharacter::ApplyDamage(float Amount, AWUCharacter* DamageCauser)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (bIsDead || Health <= 0.0f)
	{
		return false;
	}

	Health -= Amount;
	Health = FMath::Max(Health, 0.0f);

	EnterCombatState();

	if (DamageCauser)
	{
		DamageCauser->EnterCombatState();

		if (AWUPlayerController* AttackerPC = Cast<AWUPlayerController>(DamageCauser->GetController()))
		{
			AttackerPC->AutoTargetDamagedCharacter(this);
		}

		if (AWUPlayerController* VictimPC = Cast<AWUPlayerController>(GetController()))
		{
			VictimPC->AutoTargetDamagedCharacter(DamageCauser);
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Red,
			FString::Printf(TEXT("%s Health = %.1f"), *GetName(), Health)
		);
	}

	if (Health <= 0.0f)
	{
		if (DamageCauser && DamageCauser != this && KillExperienceReward > 0)
		{
			DamageCauser->AwardExperience(KillExperienceReward, EWUExperienceSource::Kill);
		}

		bIsDead = true;
		bHasReleased = false;
		bInCombat = false;
		DeathLocation = GetActorLocation();
		UpdateDeathStateEffects();

		// Spawn corpse marker
		if (!CorpseMarker)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;

			if (CorpseMarkerClass)
			{
				CorpseMarker = GetWorld()->SpawnActor<AActor>(
					CorpseMarkerClass,
					DeathLocation,
					FRotator::ZeroRotator,
					SpawnParams
				);
			}
		}

		GetCharacterMovement()->DisableMovement();

		GetWorldTimerManager().SetTimer(
			ReleaseTimerHandle,
			this,
			&AWUCharacter::HandleAutoRelease,
			15.0f,
			false
		);

		if (IsLocallyControlled() && DeathWidgetClass && !DeathWidget)
		{
			DeathWidget = CreateWidget<UUserWidget>(GetWorld(), DeathWidgetClass);

			if (DeathWidget)
			{
				DeathWidget->AddToViewport();
				Client_SetInputMode(true);
			}
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, TEXT("Player died - 15s to release"));
		}
	}

	return true;
}

void AWUCharacter::HandleAutoRelease()
{
	if (bIsDead && !bHasReleased)
	{
		ReleaseToGraveyard();
	}
}

void AWUCharacter::ReleaseToGraveyard()
{
	if (!HasAuthority())
	{
		return;
	}

	bHasReleased = true;
	UpdateDeathStateEffects();

	// Released players can move again to run back to their corpse
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	FVector GraveyardLocation = GetActorLocation(); // fallback
	bool bFoundGraveyard = false;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

	if (!CurrentGraveyardTag.IsNone())
	{
		for (AActor* Actor : FoundActors)
		{
			if (Actor && Actor->ActorHasTag(CurrentGraveyardTag))
			{
				GraveyardLocation = Actor->GetActorLocation();
				bFoundGraveyard = true;
				break;
			}
		}
	}

	if (!bFoundGraveyard)
	{
		for (AActor* Actor : FoundActors)
		{
			if (Actor && Actor->GetName().Contains(TEXT("BP_Graveyard")))
			{
				GraveyardLocation = Actor->GetActorLocation();
				break;
			}
		}
	}

	SetActorLocation(GraveyardLocation, false, nullptr, ETeleportType::TeleportPhysics);
	ForceNetUpdate();

	if (DeathWidget)
	{
		DeathWidget->RemoveFromParent();
		DeathWidget = nullptr;
		Client_SetInputMode(false);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Blue,
			TEXT("Released to Graveyard")
		);
	}
}

void AWUCharacter::RequestRelease()
{
	if (!bIsDead)
	{
		return;
	}

	// If dead but not yet released, go to graveyard
	if (!bHasReleased)
	{
		ServerRequestRelease();
		return;
	}

	// If already released, allow revive when close enough to corpse
	const float DistanceToCorpse = FVector::Dist(GetActorLocation(), DeathLocation);

	if (DistanceToCorpse <= 200.0f)
	{
		ServerRequestRelease();
	}
}

void AWUCharacter::ServerRequestRelease_Implementation()
{
	if (!bIsDead)
	{
		return;
	}

	// First press while dead: release to graveyard
	if (!bHasReleased)
	{
		ReleaseToGraveyard();
		return;
	}

	// Already released: revive if close enough to corpse
	const float DistanceToCorpse = FVector::Dist(GetActorLocation(), DeathLocation);

	if (DistanceToCorpse <= 200.0f)
	{
		ReviveAtCorpse();
	}
}

void AWUCharacter::ReviveAtCorpse()
{
	if (!HasAuthority())
	{
		return;
	}

	Health = MaxHealth;
	Magic = MaxMagic;
	bIsDead = false;
	bHasReleased = false;
	bInCombat = false;
	UpdateDeathStateEffects();

	SetActorLocation(DeathLocation, false, nullptr, ETeleportType::TeleportPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	ForceNetUpdate();

	if (CorpseMarker)
	{
		CorpseMarker->Destroy();
		CorpseMarker = nullptr;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Green,
			TEXT("Revived at corpse")
		);
	}
}

void AWUCharacter::Client_SetInputMode_Implementation(bool bShowCursor)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	if (AWUPlayerController* WUPC = Cast<AWUPlayerController>(PC))
	{
		if (bShowCursor)
		{
			WUPC->ApplyUIInputMode();
		}
		else
		{
			WUPC->ApplyGameplayInputMode();
		}

		return;
	}

	PC->bShowMouseCursor = bShowCursor;

	if (bShowCursor)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
	else
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}
}
