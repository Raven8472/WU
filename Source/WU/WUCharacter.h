// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStats/WUCharacterStats.h"
#include "Inventory/WUInventoryTypes.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WUCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UAnimationAsset;
class UMaterialInterface;
class USkeletalMesh;
class USkeletalMeshComponent;
class UTexture2D;
class UUserWidget;
class AActor;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class WU_API AWUCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = 100.0f, Units = "cm"))
	float MinCameraBoomLength = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = 100.0f, Units = "cm"))
	float MaxCameraBoomLength = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = 1.0f, Units = "cm"))
	float MouseWheelZoomStep = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = 1.0f, Units = "cm/s"))
	float BackpedalMaxWalkSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = 1.0f, Units = "deg/s"))
	float KeyboardTurnRateDegreesPerSecond = 185.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = 1.0f, Units = "deg/s"))
	float MouseFacingTurnRateDegreesPerSecond = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = 0.0f, Units = "deg"))
	float TurnInPlaceActivationAngleDegrees = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = 0.0f, Units = "s"))
	float TurnInPlaceAnimationMinSeconds = 0.32f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = 0.1f, ClampMax = 2.0f))
	float TurnInPlaceAnimationPlayRate = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (ClampMin = 0.0))
	float ClickTargetDragThreshold = 2.0f;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	/** Release Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ReleaseAction;

	/** Timer handle used for auto-release after death */
	FTimerHandle ReleaseTimerHandle;

	/** Widget class used for the death screen */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> DeathWidgetClass;

	/** Runtime instance of the death screen widget */
	UPROPERTY()
	UUserWidget* DeathWidget;

	/** Default mapping context used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Reference to the spawned corpse marker actor */
	UPROPERTY()
	AActor* CorpseMarker;

	/** Corpse marker Blueprint class */
	UPROPERTY(EditAnywhere, Category = "Death")
	TSubclassOf<AActor> CorpseMarkerClass;

	/** Optional display name used by HUD frames when this character is targeted */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	FText DisplayName;

	/** Optional portrait texture used by HUD frames when this character is targeted */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	TObjectPtr<UTexture2D> PortraitTexture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> HeadMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> HairMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> BrowsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> BeardMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> PantsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> HandsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> BracersMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> ChestOutfitMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> ChestAddOutfitMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> BeltOutfitMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> BootsOutfitMeshComponent;

public:

	/** Constructor */
	AWUCharacter();

	virtual void Tick(float DeltaSeconds) override;

	/** Base attack damage (will later be modified by stats/gear) */
	UPROPERTY(EditAnywhere, Category = "Combat")
	float BaseAttackDamage = 10.0f;

	/** Maximum health calculated from Nerve */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
	float MaxHealth = 100.0f;

	/** Current health of the character (replicated from server) */
	UPROPERTY(Replicated)
	float Health;

	/** Maximum Magic calculated from Wit */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
	float MaxMagic = 150.0f;

	/** Current Magic of the character (replicated from server) */
	UPROPERTY(Replicated)
	float Magic;

	/** Whether the character is dead (replicated from server) */
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	bool bIsDead;

	/** Location where the character died (replicated from server) */
	UPROPERTY(Replicated)
	FVector DeathLocation;

	/** Whether the character has released to graveyard (replicated from server) */
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	bool bHasReleased;

	/** True while recent damage or attacks keep the character in combat */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Combat")
	bool bInCombat = false;

	/** Seconds without dealing or receiving damage before leaving combat */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = 0.0f, Units = "s"))
	float CombatTimeoutSeconds = 10.0f;

	/** Total starter inventory slots. Kept aligned with the current inventory UI capacity. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 1))
	int32 MaxInventorySlots = 60;

	/** Bag inventory slots replicated from the server. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_InventoryChanged, Category = "Inventory")
	TArray<FWUInventorySlot> InventorySlots;

	/** Equipped item slots replicated from the server. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_InventoryChanged, Category = "Inventory")
	TArray<FWUEquipmentSlotEntry> EquipmentSlots;

	/** Modular humanoid appearance used by the live gameplay pawn. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CharacterAppearance, Category = "Appearance")
	FWUCharacterAppearance CharacterAppearance;

	/** Blood status currently stored in the character creation race field */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CharacterStats, Category = "Stats")
	EWUCharacterRace BloodStatus = EWUCharacterRace::Halfblood;

	/** Character level used to derive primary stats */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CharacterStats, Category = "Stats", meta = (ClampMin = 1, ClampMax = 80))
	int32 CharacterLevel = 1;

	/** Current experience progress within the active level */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CharacterStats, Category = "Stats", meta = (ClampMin = 0))
	int32 CharacterExperience = 0;

	/** Primary stats after human baseline, level growth, and blood-status modifiers */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CharacterStats, Category = "Stats")
	FWUPrimaryStats PrimaryStats;

	/** Derived combat values calculated from primary stats */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CharacterStats, Category = "Stats")
	FWUDerivedStats DerivedStats;

	/** Kill reward granted to the attacker when this character dies */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression", meta = (ClampMin = 0))
	int32 KillExperienceReward = 120;

	/** Sets up which variables replicate */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Starts a local attack input and forwards it to the server */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartAttack();

	/** Server-authoritative attack entry point */
	UFUNCTION(Server, Reliable)
	void ServerAttack();

	/** Performs the forward attack trace on the server */
	void PerformAttackTrace();

	/** Applies damage on the server, returns true if damage was actually applied */
	bool ApplyDamage(float Amount, AWUCharacter* DamageCauser = nullptr);

	/** Calculates outgoing damage (placeholder for future stat system) */
	float CalculateDamage() const;

	/** Applies level and blood status, then recalculates primary and derived stats */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ApplyCharacterProgression(EWUCharacterRace NewBloodStatus, int32 NewLevel);

	/** Applies level, experience, and blood status, then recalculates primary and derived stats */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ApplyCharacterProgressionState(EWUCharacterRace NewBloodStatus, int32 NewLevel, int32 NewExperience);

	/** Awards experience and resolves level-up progression on the authority side */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AwardExperience(int32 Amount, EWUExperienceSource Source);

	/** Returns health normalized to the max health for HUD usage */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetHealthPercent() const;

	/** Returns the current health value for HUD usage */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetCurrentHealth() const;

	/** Returns the maximum health value for HUD usage */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetMaxHealth() const;

	/** Returns Magic normalized to the max Magic for HUD usage */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetMagicPercent() const;

	/** Returns the current Magic value for HUD usage */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetCurrentMagic() const;

	/** Returns the maximum Magic value for HUD usage */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetMaxMagic() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInCombat() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<FWUInventorySlot> GetInventorySlots() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<FWUEquipmentSlotEntry> GetEquipmentSlots() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool GetInventorySlot(int32 SlotIndex, FWUInventorySlot& OutSlot) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool GetEquippedItem(EWUEquipmentSlot EquipmentSlot, FWUInventoryItem& OutItem) const;

	UFUNCTION(BlueprintPure, Category = "Appearance")
	FWUCharacterAppearance GetCharacterAppearance() const;

	UFUNCTION(BlueprintCallable, Category = "Appearance")
	void ApplyCharacterAppearance(const FWUCharacterAppearance& NewAppearance);

	/** Equips the item in the requested bag slot, swapping with occupied equipment when needed. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool EquipInventorySlot(int32 SlotIndex);

	/** Unequips the requested equipment slot into the first available bag slot. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UnequipEquipmentSlot(EWUEquipmentSlot EquipmentSlot);

	UFUNCTION(BlueprintPure, Category = "Stats")
	EWUCharacterRace GetBloodStatus() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetCharacterExperience() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetExperienceToNextLevel() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetExperiencePercent() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	FWUPrimaryStats GetPrimaryStats() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	FWUDerivedStats GetDerivedStats() const;

	/** Returns the display name used by HUD frames */
	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetDisplayName() const;

	/** Sets the display name used by HUD frames */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetDisplayName(const FText& NewDisplayName);

	/** Returns the portrait texture used by HUD frames */
	UFUNCTION(BlueprintPure, Category = "HUD")
	UTexture2D* GetPortraitTexture() const;

	/** Returns true if the character is dead */
	UFUNCTION(BlueprintPure, Category = "Death")
	bool IsDead() const;

	/** Returns true if the character has released to spirit form */
	UFUNCTION(BlueprintPure, Category = "Death")
	bool HasReleased() const;

	/** Returns true if the player is in released spirit form */
	UFUNCTION(BlueprintPure, Category = "Death")
	bool IsReleasedSpirit() const;

	/** Returns true if the released spirit is close enough to revive at the corpse */
	UFUNCTION(BlueprintPure, Category = "Death")
	bool CanReviveAtCorpse() const;

	/** Handles auto-release when the death timer expires */
	void HandleAutoRelease();

	/** Releases the player to a graveyard location */
	void ReleaseToGraveyard();

	/** Player manually requests release (client-side input) */
	UFUNCTION(BlueprintCallable, Category = "Death")
	void RequestRelease();

	/** Server handles release request */
	UFUNCTION(Server, Reliable)
	void ServerRequestRelease();

	/** Client switches between gameplay input and UI input */
	UFUNCTION(Client, Reliable)
	void Client_SetInputMode(bool bShowCursor);

	/** Revives the player at their death location (corpse) */
	void ReviveAtCorpse();

	/** Called when replicated death/release state changes on clients */
	UFUNCTION()
	void OnRep_DeathState();

	UFUNCTION()
	void OnRep_CharacterStats();

	UFUNCTION()
	void OnRep_InventoryChanged();

	UFUNCTION()
	void OnRep_CharacterAppearance();

	/** Applies movement/collision/visual behavior for alive, dead, and spirit states */
	void UpdateDeathStateEffects();

	void EnterCombatState();
	void UpdateCombatState();
	void RegenerateResources(float DeltaSeconds);
	void InitializeInventoryStorage();
	void SeedStarterInventory();
	void BeginBackpedal(float Right);
	void EndBackpedal();
	int32 FindEquipmentEntryIndex(EWUEquipmentSlot EquipmentSlot) const;
	int32 FindFirstFreeInventorySlot() const;
	bool AddItemToInventory(const FWUInventoryItem& Item);
	void ConfigureModularMeshComponent(USkeletalMeshComponent* MeshComponent) const;
	void ApplyCharacterAppearanceMeshes();
	USkeletalMesh* LoadSkeletalMeshForPath(const TCHAR* AssetPath) const;
	UMaterialInterface* LoadMaterialForPath(const TCHAR* AssetPath) const;
	UAnimationAsset* LoadAnimationAssetForPath(const TCHAR* AssetPath) const;
	UClass* LoadAnimClassForPath(const TCHAR* AssetPath) const;
	const TCHAR* GetBodyMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetHeadMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetHairMeshPath(EWUCharacterSex Sex, int32 HairStyleIndex) const;
	const TCHAR* GetBrowsMeshPath(EWUCharacterSex Sex, int32 BrowStyleIndex) const;
	const TCHAR* GetBeardMeshPath(EWUCharacterSex Sex, int32 BeardStyleIndex) const;
	const TCHAR* GetBodyMaterialPath(EWUCharacterSex Sex, int32 SkinPresetIndex) const;
	const TCHAR* GetHeadMaterialPath(EWUCharacterSex Sex, int32 HeadPresetIndex) const;
	const TCHAR* GetEyeMaterialPath(int32 EyeColorIndex) const;
	const TCHAR* GetHairMaterialPath(int32 HairColorIndex) const;
	const TCHAR* GetAnimationBlueprintPath(EWUCharacterSex Sex) const;
	const TCHAR* GetBackpedalAnimationPath(EWUCharacterSex Sex, float Right) const;
	const TCHAR* GetTurnInPlaceAnimationPath(EWUCharacterSex Sex, float YawDeltaDegrees) const;
	const TCHAR* GetPantsMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetHandsMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetBracersMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetStarterChestOutfitMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetStarterChestAddOutfitMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetStarterBeltOutfitMeshPath(EWUCharacterSex Sex) const;
	const TCHAR* GetStarterBootsOutfitMeshPath(EWUCharacterSex Sex) const;
	int32 NormalizeAppearanceIndex(int32 Index, int32 Count) const;
	void ApplyCharacterProgressionInternal(EWUCharacterRace NewBloodStatus, int32 NewLevel, int32 NewExperience, bool bResetResources);

	UFUNCTION(Server, Reliable)
	void ServerApplyCharacterProgression(EWUCharacterRace NewBloodStatus, int32 NewLevel);

	UFUNCTION(Server, Reliable)
	void ServerApplyCharacterProgressionState(EWUCharacterRace NewBloodStatus, int32 NewLevel, int32 NewExperience);

	UFUNCTION(Server, Reliable)
	void ServerApplyCharacterAppearance(const FWUCharacterAppearance& NewAppearance);

	UFUNCTION(Server, Reliable)
	void ServerEquipInventorySlot(int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerUnequipEquipmentSlot(EWUEquipmentSlot EquipmentSlot);

protected:

	virtual void BeginPlay() override;

	float LastCombatEventTimeSeconds = -1000.0f;
	float PreBackpedalMaxWalkSpeed = 500.0f;
	float DefaultRotationRateDegreesPerSecond = 500.0f;
	float PendingLeftClickDragDistance = 0.0f;
	float TurnInPlaceAnimationHoldUntilSeconds = 0.0f;
	bool bBackpedaling = false;
	bool bBackpedalAnimationActive = false;
	bool bTurnInPlaceAnimationActive = false;
	bool bKeyboardTurnInPlaceActive = false;
	bool bLeftMouseLookHeld = false;
	bool bRightMouseLookHeld = false;
	bool bSuppressLeftClickTargetOnRelease = false;
	FString CurrentBackpedalAnimationPath;
	FString CurrentTurnInPlaceAnimationPath;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Restores normal movement-facing behavior after movement input ends */
	void StopMove(const FInputActionValue& Value);

	/** Called for controller/thumbstick looking input */
	void Look(const FInputActionValue& Value);

	/** Called for mouse-delta look input while orbit/steer buttons are held */
	void MouseLook(const FInputActionValue& Value);

	/** Called for raw mouse-wheel camera zoom input */
	void ZoomCamera(float AxisValue);

	void OnLeftMousePressed();
	void OnLeftMouseReleased();
	void OnRightMousePressed();
	void OnRightMouseReleased();

	/** Fallback target-select input routed through the possessed pawn input component */
	void TargetUnderCursorInput();

	/** Fallback target-cycle input routed through the possessed pawn input component */
	void TargetNextInput();

	void UpdateMouseSteering(float DeltaSeconds);
	void BeginTurnInPlace(float YawDeltaDegrees);
	void EndTurnInPlace(bool bForce = false);
	void RestoreDefaultLocomotionAnimation();
	bool IsGameplayMouseInputAllowed() const;
	bool IsMouseCameraOrbitActive() const;
	bool IsMouseFacingControlActive() const;
	bool IsDualMouseDriveActive() const;

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	/** Handles jump released inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

