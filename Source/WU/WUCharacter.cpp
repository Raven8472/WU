// Copyright Epic Games, Inc. All Rights Reserved.

#include "WUCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
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
#include "Kismet/GameplayStatics.h"
#include "WUPlayerController.h"

namespace
{
	FWUInventoryItem MakeStarterItem(FName ItemId, const TCHAR* DisplayName, EWUEquipmentSlot EquipmentSlot, const FLinearColor& ItemTint)
	{
		FWUInventoryItem Item;
		Item.ItemId = ItemId;
		Item.DisplayName = DisplayName;
		Item.EquipmentSlot = EquipmentSlot;
		Item.bEquippable = true;
		Item.ItemTint = ItemTint;
		return Item;
	}
}

AWUCharacter::AWUCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

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

	bReplicates = true;

	PrimaryStats = WUCharacterStats::CalculatePrimaryStats(BloodStatus, CharacterLevel);
	DerivedStats = WUCharacterStats::CalculateDerivedStats(PrimaryStats);
	MaxHealth = DerivedStats.MaxHealth;
	MaxMagic = DerivedStats.MaxMagic;
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
	DOREPLIFETIME(AWUCharacter, BloodStatus);
	DOREPLIFETIME(AWUCharacter, CharacterLevel);
	DOREPLIFETIME(AWUCharacter, PrimaryStats);
	DOREPLIFETIME(AWUCharacter, DerivedStats);
	DOREPLIFETIME(AWUCharacter, InventorySlots);
	DOREPLIFETIME(AWUCharacter, EquipmentSlots);
}

void AWUCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ApplyCharacterProgressionInternal(BloodStatus, CharacterLevel, true);
		bIsDead = false;
		bHasReleased = false;
		InitializeInventoryStorage();
		SeedStarterInventory();
		ForceNetUpdate();
	}

	DeathWidget = nullptr;

	// Apply the initial alive/dead collision and movement state on spawn so
	// freshly spawned players match the same rules used after later state changes.
	UpdateDeathStateEffects();
}

void AWUCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

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

		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AWUCharacter::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AWUCharacter::Look);
		//attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AWUCharacter::StartAttack);
		//release
		EnhancedInputComponent->BindAction(ReleaseAction, ETriggerEvent::Started, this, &AWUCharacter::RequestRelease);

		PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AWUCharacter::TargetUnderCursorInput);
		PlayerInputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AWUCharacter::TargetNextInput);
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

void AWUCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
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

void AWUCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
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
	ApplyCharacterProgressionInternal(NewBloodStatus, NewLevel, true);

	if (!HasAuthority())
	{
		ServerApplyCharacterProgression(NewBloodStatus, NewLevel);
	}
}

void AWUCharacter::ServerApplyCharacterProgression_Implementation(EWUCharacterRace NewBloodStatus, int32 NewLevel)
{
	ApplyCharacterProgressionInternal(NewBloodStatus, NewLevel, true);
	ForceNetUpdate();
}

void AWUCharacter::ApplyCharacterProgressionInternal(EWUCharacterRace NewBloodStatus, int32 NewLevel, bool bResetResources)
{
	BloodStatus = NewBloodStatus;
	CharacterLevel = WUCharacterStats::ClampCharacterLevel(NewLevel);
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
	Health = FMath::Clamp(Health, 0.0f, MaxHealth);
	Magic = FMath::Clamp(Magic, 0.0f, MaxMagic);
}

void AWUCharacter::OnRep_InventoryChanged()
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

FWUPrimaryStats AWUCharacter::GetPrimaryStats() const
{
	return PrimaryStats;
}

FWUDerivedStats AWUCharacter::GetDerivedStats() const
{
	return DerivedStats;
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
		return;
	}

	AddItemToInventory(MakeStarterItem(TEXT("starter_wand"), TEXT("Holly Wand"), EWUEquipmentSlot::Wand, FLinearColor(0.75f, 0.55f, 0.28f, 1.0f)));
	AddItemToInventory(MakeStarterItem(TEXT("starter_robes"), TEXT("First-Year Robes"), EWUEquipmentSlot::ChestRobes, FLinearColor(0.18f, 0.25f, 0.36f, 1.0f)));
	AddItemToInventory(MakeStarterItem(TEXT("starter_shirt"), TEXT("Linen Shirt"), EWUEquipmentSlot::Shirt, FLinearColor(0.82f, 0.78f, 0.68f, 1.0f)));
	AddItemToInventory(MakeStarterItem(TEXT("starter_shoes"), TEXT("School Shoes"), EWUEquipmentSlot::Shoes, FLinearColor(0.22f, 0.18f, 0.14f, 1.0f)));
	AddItemToInventory(MakeStarterItem(TEXT("starter_hat"), TEXT("Wool Hat"), EWUEquipmentSlot::Hat, FLinearColor(0.32f, 0.27f, 0.22f, 1.0f)));
	AddItemToInventory(MakeStarterItem(TEXT("starter_ring"), TEXT("Copper Ring"), EWUEquipmentSlot::Ring1, FLinearColor(0.86f, 0.46f, 0.22f, 1.0f)));
	AddItemToInventory(MakeStarterItem(TEXT("starter_bracelet"), TEXT("Woven Bracelet"), EWUEquipmentSlot::Bracelet1, FLinearColor(0.40f, 0.58f, 0.34f, 1.0f)));
	AddItemToInventory(MakeStarterItem(TEXT("starter_nicnak"), TEXT("Tiny Nicnak"), EWUEquipmentSlot::Nicnak1, FLinearColor(0.45f, 0.64f, 0.88f, 1.0f)));
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

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (Actor && Actor->GetName().Contains(TEXT("BP_Graveyard")))
		{
			GraveyardLocation = Actor->GetActorLocation();
			break;
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
