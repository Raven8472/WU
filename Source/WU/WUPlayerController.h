// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WUPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class AActor;
class AWUCharacter;
class UWUPlayerFrameWidget;
class UWUTargetFrameWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUTargetChangedSignature, AWUCharacter*, NewTarget);

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class AWUPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Click/select target action. If unset, left mouse click is bound directly as a fallback. */
	UPROPERTY(EditAnywhere, Category = "Input|Targeting")
	UInputAction* ClickTargetAction;

	/** Tab target action. If unset, Tab is bound directly as a fallback. */
	UPROPERTY(EditAnywhere, Category = "Input|Targeting")
	UInputAction* TabTargetAction;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Main gameplay HUD widget to spawn for the local player */
	UPROPERTY(EditAnywhere, Category="UI|HUD")
	TSubclassOf<UUserWidget> PlayerHUDWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Pointer to the main gameplay HUD widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> PlayerHUDWidget;

	/** Player unit frame widget to spawn for the local player */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	TSubclassOf<UWUPlayerFrameWidget> PlayerFrameWidgetClass;

	/** Pointer to the native player unit frame widget */
	UPROPERTY()
	TObjectPtr<UWUPlayerFrameWidget> PlayerFrameWidget;

	/** Viewport position for the player unit frame */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	FVector2D PlayerFrameViewportPosition = FVector2D(-84.0f, -124.0f);

	/** Viewport size for the player unit frame */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	FVector2D PlayerFrameViewportSize = FVector2D(260.0f, 64.0f);

	/** Hides the legacy Blueprint player frame when the native unit frame is active. */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	bool bHideLegacyPlayerFrameWidget = true;

	/** Enemy target frame widget to spawn for the local player */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	TSubclassOf<UWUTargetFrameWidget> TargetFrameWidgetClass;

	/** Pointer to the enemy target frame widget */
	UPROPERTY()
	TObjectPtr<UWUTargetFrameWidget> TargetFrameWidget;

	/** Viewport position for the enemy target frame */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	FVector2D TargetFrameViewportPosition = FVector2D(84.0f, -124.0f);

	/** Viewport size for the enemy target frame */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	FVector2D TargetFrameViewportSize = FVector2D(260.0f, 64.0f);

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Show a normal cursor during gameplay so players can click targets. */
	UPROPERTY(EditAnywhere, Category = "Input|Cursor")
	bool bShowGameplayCursor = true;

	/** Maximum distance for click and tab targeting traces. */
	UPROPERTY(EditAnywhere, Category = "Input|Targeting", meta = (ClampMin = 1000, Units = "cm"))
	float TargetTraceDistance = 100000.0f;

	/** Maximum distance for Tab target candidates. */
	UPROPERTY(EditAnywhere, Category = "Input|Targeting", meta = (ClampMin = 0, Units = "cm"))
	float TabTargetMaxDistance = 5000.0f;

	/** Soft angle from camera forward used to order Tab target candidates. */
	UPROPERTY(EditAnywhere, Category = "Input|Targeting", meta = (ClampMin = 0, ClampMax = 180, Units = "deg"))
	float TabTargetPreferredAngleDegrees = 120.0f;

	/** Extra cursor-ray tolerance for click targeting when collision does not report a pawn hit. */
	UPROPERTY(EditAnywhere, Category = "Input|Targeting", meta = (ClampMin = 0, Units = "cm"))
	float ClickTargetRayTolerance = 140.0f;

	/** Shows short on-screen messages for target input and selection while tuning the prototype. */
	UPROPERTY(EditAnywhere, Category = "Input|Targeting")
	bool bShowTargetingDebugMessages = true;

	/** Current selected target for HUD and future combat routing. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentTarget, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AWUCharacter> CurrentTarget;

public:

	/** Constructor */
	AWUPlayerController();

	/** Sets up replicated controller state */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Broadcast when the selected target changes. */
	UPROPERTY(BlueprintAssignable, Category = "Targeting")
	FWUTargetChangedSignature OnTargetChanged;

	/** Returns the currently selected target, if any. */
	UFUNCTION(BlueprintPure, Category = "Targeting")
	AWUCharacter* GetCurrentTarget() const;

	/** True if a selectable target is currently selected. */
	UFUNCTION(BlueprintPure, Category = "Targeting")
	bool HasCurrentTarget() const;

	/** Sets the selected target. Invalid targets clear the current target. */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void SetCurrentTarget(AWUCharacter* NewTarget);

	/** Selects a character because damage connected. Safe to call from server gameplay code. */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void AutoTargetDamagedCharacter(AWUCharacter* DamagedCharacter);

	/** Clears the selected target. */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void ClearCurrentTarget();

	/** Selects the character under the mouse cursor, if one is present. */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void TargetUnderCursor();

	/** Cycles to the next valid target in front of the camera. */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void TargetNextCharacter();

	/** Restores normal gameplay cursor/input mode. */
	UFUNCTION(BlueprintCallable, Category = "Input|Cursor")
	void ApplyGameplayInputMode();

	/** Applies a cursor-friendly UI input mode, used by death/release UI. */
	UFUNCTION(BlueprintCallable, Category = "Input|Cursor")
	void ApplyUIInputMode();

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	/** Returns true if the actor can be selected as a target. */
	bool IsSelectableTarget(const AWUCharacter* Candidate) const;

	/** Shows a prototype targeting debug message when enabled. */
	void ShowTargetingDebugMessage(const FString& Message, const FColor& Color = FColor::Cyan) const;

	/** Finds the nearest selectable target close to a world-space ray. */
	AWUCharacter* FindSelectableTargetNearRay(const FVector& RayOrigin, const FVector& RayDirection, float MaxDistance, float RayTolerance) const;

	/** Client-side target update used when server-authoritative damage selects a target. */
	UFUNCTION(Client, Reliable)
	void Client_AutoTargetDamagedCharacter(AWUCharacter* DamagedCharacter);

	/** Handles replicated target updates from server-authoritative systems */
	UFUNCTION()
	void OnRep_CurrentTarget();

	/** Clears the target if the selected actor is destroyed. */
	UFUNCTION()
	void OnCurrentTargetDestroyed(AActor* DestroyedActor);

};
