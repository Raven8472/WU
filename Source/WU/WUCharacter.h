// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WUCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
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

public:

	/** Constructor */
	AWUCharacter();

	/** Base attack damage (will later be modified by stats/gear) */
	UPROPERTY(EditAnywhere, Category = "Combat")
	float BaseAttackDamage = 10.0f;

	/** Maximum health used by the base prototype character */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float MaxHealth = 100.0f;

	/** Current health of the character (replicated from server) */
	UPROPERTY(Replicated)
	float Health;

	/** Whether the character is dead (replicated from server) */
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	bool bIsDead;

	/** Location where the character died (replicated from server) */
	UPROPERTY(Replicated)
	FVector DeathLocation;

	/** Whether the character has released to graveyard (replicated from server) */
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	bool bHasReleased;

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

	/** Returns health normalized to the max health for HUD usage */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetHealthPercent() const;

	/** Returns the current health value for HUD usage */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetCurrentHealth() const;

	/** Returns the maximum health value for HUD usage */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetMaxHealth() const;

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

	/** Applies movement/collision/visual behavior for alive, dead, and spirit states */
	void UpdateDeathStateEffects();

protected:

	virtual void BeginPlay() override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Fallback target-select input routed through the possessed pawn input component */
	void TargetUnderCursorInput();

	/** Fallback target-cycle input routed through the possessed pawn input component */
	void TargetNextInput();

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

