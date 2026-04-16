// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WUCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UUserWidget;
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

	/** Reference to the spawned corpse marker actor */
	UPROPERTY()
	AActor* CorpseMarker;

	/** Corpse marker Blueprint class */
	UPROPERTY(EditAnywhere, Category = "Death")
	TSubclassOf<AActor> CorpseMarkerClass;

public:

	/** Constructor */
	AWUCharacter();

	/** Current health of the character (replicated from server) */
	UPROPERTY(Replicated)
	float Health;

	/** Whether the character is dead (replicated from server) */
	UPROPERTY(Replicated)
	bool bIsDead;

	/** Location where the character died (replicated from server) */
	UPROPERTY(Replicated)
	FVector DeathLocation;

	/** Whether the character has released to graveyard (replicated from server) */
	UPROPERTY(Replicated)
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
	bool ApplyDamage(float Amount);

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

	/** Revives the player at their death location (corpse) */
	void ReviveAtCorpse();

protected:

	virtual void BeginPlay() override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

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

