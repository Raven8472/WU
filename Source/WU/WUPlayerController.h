// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Backend/WUClientSessionSubsystem.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"
#include "CharacterStats/WUCharacterStats.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/WUInventoryTypes.h"
#include "WUPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class AActor;
class AWUCharacterCreatorPreviewActor;
class AWUCharacter;
class UWUCharacterCreatorWidget;
class UWUCharacterPanelWidget;
class UWUChatWidget;
class UWUClubCharterPromptWidget;
class UWUExperienceBarWidget;
class UWUInventoryWidget;
class UWUPlayerFrameWidget;
class UWUSocialWidget;
class UWUTargetFrameWidget;
class UWUVendorWidget;
class UWUWorldHoverTooltipWidget;
class UWUZoneNameWidget;
class AWUNpcCharacter;

UENUM(BlueprintType)
enum class EWUChatChannel : uint8
{
	Say,
	General,
	Club,
	Officer
};

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

	/** Deprecated: left mouse world clicks are routed by the possessed character after drag detection. */
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
	FVector2D PlayerFrameViewportSize = FVector2D(260.0f, 82.0f);

	/** Hides the legacy Blueprint player frame when the native unit frame is active. */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	bool bHideLegacyPlayerFrameWidget = true;

	/** Native player experience bar widget shown above the action bar. */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	TSubclassOf<UWUExperienceBarWidget> ExperienceBarWidgetClass;

	/** Pointer to the native player experience bar widget. */
	UPROPERTY()
	TObjectPtr<UWUExperienceBarWidget> ExperienceBarWidget;

	/** Viewport position for the player experience bar. */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	FVector2D ExperienceBarViewportPosition = FVector2D(0.0f, -52.0f);

	/** Viewport size for the player experience bar. */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	FVector2D ExperienceBarViewportSize = FVector2D(560.0f, 26.0f);

	/** Native zone name widget shown beneath the minimap. */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	TSubclassOf<UWUZoneNameWidget> ZoneNameWidgetClass;

	/** Pointer to the native zone name widget. */
	UPROPERTY()
	TObjectPtr<UWUZoneNameWidget> ZoneNameWidget;

	/** Viewport position for the zone name readout. */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	FVector2D ZoneNameViewportPosition = FVector2D(-24.0f, 218.0f);

	/** Viewport size for the zone name readout. */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	FVector2D ZoneNameViewportSize = FVector2D(280.0f, 30.0f);

	/** Bottom-right world hover tooltip widget. */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	TSubclassOf<UWUWorldHoverTooltipWidget> WorldHoverTooltipWidgetClass;

	/** Pointer to the native world hover tooltip widget. */
	UPROPERTY()
	TObjectPtr<UWUWorldHoverTooltipWidget> WorldHoverTooltipWidget;

	/** Viewport position for the world hover tooltip. */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	FVector2D WorldHoverTooltipViewportPosition = FVector2D(-28.0f, -184.0f);

	/** Viewport size for the world hover tooltip. */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	FVector2D WorldHoverTooltipViewportSize = FVector2D(300.0f, 86.0f);

	/** Native player chat widget to spawn for the local player */
	UPROPERTY(EditAnywhere, Category = "UI|Chat")
	TSubclassOf<UWUChatWidget> ChatWidgetClass;

	/** Pointer to the native player chat widget */
	UPROPERTY()
	TObjectPtr<UWUChatWidget> ChatWidget;

	/** Viewport position for the player chat widget */
	UPROPERTY(EditAnywhere, Category = "UI|Chat")
	FVector2D ChatViewportPosition = FVector2D(24.0f, -220.0f);

	/** Viewport size for the player chat widget */
	UPROPERTY(EditAnywhere, Category = "UI|Chat")
	FVector2D ChatViewportSize = FVector2D(520.0f, 220.0f);

	/** Maximum number of characters accepted in a single chat message. */
	UPROPERTY(EditAnywhere, Category = "UI|Chat", meta = (ClampMin = 1, ClampMax = 512))
	int32 MaxChatMessageLength = 160;

	/** Minimum time between chat messages from the same controller. */
	UPROPERTY(EditAnywhere, Category = "UI|Chat", meta = (ClampMin = 0.0f, Units = "s"))
	float ChatMessageCooldownSeconds = 0.25f;

	/** Native social window used for club roster and social tabs. */
	UPROPERTY(EditAnywhere, Category = "UI|Social")
	TSubclassOf<UWUSocialWidget> SocialWidgetClass;

	/** Pointer to the native social window. */
	UPROPERTY()
	TObjectPtr<UWUSocialWidget> SocialWidget;

	/** Viewport position for the social window. */
	UPROPERTY(EditAnywhere, Category = "UI|Social")
	FVector2D SocialViewportPosition = FVector2D(24.0f, 128.0f);

	/** Viewport size for the social window. */
	UPROPERTY(EditAnywhere, Category = "UI|Social")
	FVector2D SocialViewportSize = FVector2D(620.0f, 560.0f);

	/** Native inventory shell widget to spawn for the local player */
	UPROPERTY(EditAnywhere, Category = "UI|Inventory")
	TSubclassOf<UWUInventoryWidget> InventoryWidgetClass;

	/** Pointer to the native inventory shell widget */
	UPROPERTY()
	TObjectPtr<UWUInventoryWidget> InventoryWidget;

	/** Viewport position for the inventory shell widget */
	UPROPERTY(EditAnywhere, Category = "UI|Inventory")
	FVector2D InventoryViewportPosition = FVector2D(-24.0f, -150.0f);

	/** Viewport size for the inventory shell widget */
	UPROPERTY(EditAnywhere, Category = "UI|Inventory")
	FVector2D InventoryViewportSize = FVector2D(620.0f, 420.0f);

	/** Temporary dev prompt used when clicking a Club Charter inventory item. */
	UPROPERTY(EditAnywhere, Category = "UI|Club")
	TSubclassOf<UWUClubCharterPromptWidget> ClubCharterPromptWidgetClass;

	/** Pointer to the native club charter prompt widget. */
	UPROPERTY()
	TObjectPtr<UWUClubCharterPromptWidget> ClubCharterPromptWidget;

	/** Viewport position for the club charter prompt. */
	UPROPERTY(EditAnywhere, Category = "UI|Club")
	FVector2D ClubCharterPromptViewportPosition = FVector2D(0.0f, 0.0f);

	/** Viewport size for the club charter prompt. */
	UPROPERTY(EditAnywhere, Category = "UI|Club")
	FVector2D ClubCharterPromptViewportSize = FVector2D(360.0f, 210.0f);

	/** Native vendor shell widget to spawn for NPC shops */
	UPROPERTY(EditAnywhere, Category = "UI|Vendor")
	TSubclassOf<UWUVendorWidget> VendorWidgetClass;

	/** Pointer to the native vendor shell widget */
	UPROPERTY()
	TObjectPtr<UWUVendorWidget> VendorWidget;

	/** Viewport position for the vendor shell widget */
	UPROPERTY(EditAnywhere, Category = "UI|Vendor")
	FVector2D VendorViewportPosition = FVector2D(24.0f, 150.0f);

	/** Viewport size for the vendor shell widget */
	UPROPERTY(EditAnywhere, Category = "UI|Vendor")
	FVector2D VendorViewportSize = FVector2D(460.0f, 560.0f);

	/** Native character panel widget to spawn for the local player */
	UPROPERTY(EditAnywhere, Category = "UI|Character")
	TSubclassOf<UWUCharacterPanelWidget> CharacterPanelWidgetClass;

	/** Pointer to the native character panel widget */
	UPROPERTY()
	TObjectPtr<UWUCharacterPanelWidget> CharacterPanelWidget;

	/** Viewport position for the character panel widget */
	UPROPERTY(EditAnywhere, Category = "UI|Character")
	FVector2D CharacterPanelViewportPosition = FVector2D(0.0f, 0.0f);

	/** Viewport size for the character panel widget */
	UPROPERTY(EditAnywhere, Category = "UI|Character")
	FVector2D CharacterPanelViewportSize = FVector2D(560.0f, 520.0f);

	/** Native character creator shell widget to spawn for the local player */
	UPROPERTY(EditAnywhere, Category = "UI|Character Creation")
	TSubclassOf<UWUCharacterCreatorWidget> CharacterCreatorWidgetClass;

	/** Pointer to the native character creator shell widget */
	UPROPERTY()
	TObjectPtr<UWUCharacterCreatorWidget> CharacterCreatorWidget;

	/** World preview actor used by the local character creator shell */
	UPROPERTY(EditAnywhere, Category = "UI|Character Creation")
	TSubclassOf<AWUCharacterCreatorPreviewActor> CharacterCreatorPreviewActorClass;

	/** Pointer to the local-only character creator preview actor */
	UPROPERTY()
	TObjectPtr<AWUCharacterCreatorPreviewActor> CharacterCreatorPreviewActor;

	/** Viewport position for the character creator shell widget */
	UPROPERTY(EditAnywhere, Category = "UI|Character Creation")
	FVector2D CharacterCreatorViewportPosition = FVector2D(32.0f, 96.0f);

	/** Viewport size for the character creator shell widget */
	UPROPERTY(EditAnywhere, Category = "UI|Character Creation")
	FVector2D CharacterCreatorViewportSize = FVector2D(430.0f, 520.0f);

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
	FVector2D TargetFrameViewportSize = FVector2D(260.0f, 82.0f);

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

	/** Maximum distance for direct NPC interaction. */
	UPROPERTY(EditAnywhere, Category = "Input|NPC", meta = (ClampMin = 100, Units = "cm"))
	float NpcInteractionDistance = 450.0f;

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

	/** Handles primary world click, preferring NPC interaction before combat targeting. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HandlePrimaryWorldClick();

	/** Interacts with the NPC under the cursor or the nearest interactable NPC. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void InteractWithNpc();

	/** Cycles to the next valid target in front of the camera. */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void TargetNextCharacter();

	/** Restores normal gameplay cursor/input mode. */
	UFUNCTION(BlueprintCallable, Category = "Input|Cursor")
	void ApplyGameplayInputMode();

	/** Applies a cursor-friendly UI input mode, used by death/release UI. */
	UFUNCTION(BlueprintCallable, Category = "Input|Cursor")
	void ApplyUIInputMode();

	/** True while a UI overlay should block gameplay mouse-look handling. */
	UFUNCTION(BlueprintPure, Category = "Input|Cursor")
	bool HasInteractiveOverlayOpen() const;

	/** Opens the chat input line and gives it keyboard focus. */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void OpenChatInput();

	/** Closes the chat input line and restores gameplay input. */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void CloseChatInput();

	/** Sends a chat message through the current server transport. */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SubmitChatMessage(const FString& RawMessage);

	/** Toggles the social/club roster window. */
	UFUNCTION(BlueprintCallable, Category = "Social")
	void ToggleSocialPanel();

	/** Shows the social/club roster window. */
	UFUNCTION(BlueprintCallable, Category = "Social")
	void ShowSocialPanel();

	/** Hides the social/club roster window. */
	UFUNCTION(BlueprintCallable, Category = "Social")
	void HideSocialPanel();

	/** Toggles the inventory shell window. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ToggleInventory();

	/** Shows the inventory shell window. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ShowInventory();

	/** Hides the inventory shell window. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void HideInventory();

	/** Shows a vendor window for the requested NPC. */
	UFUNCTION(BlueprintCallable, Category = "Vendor")
	void ShowVendorForNpc(AWUNpcCharacter* NpcCharacter);

	/** Hides the vendor window. */
	UFUNCTION(BlueprintCallable, Category = "Vendor")
	void HideVendor();

	/** Toggles the character panel window. */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void ToggleCharacterPanel();

	/** Shows the character panel window. */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void ShowCharacterPanel();

	/** Hides the character panel window. */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void HideCharacterPanel();

	/** Toggles the local character creator shell. */
	UFUNCTION(BlueprintCallable, Category = "Character Creation")
	void ToggleCharacterCreator();

	/** Shows the local character creator shell. */
	UFUNCTION(BlueprintCallable, Category = "Character Creation")
	void ShowCharacterCreator();

	/** Hides the local character creator shell. */
	UFUNCTION(BlueprintCallable, Category = "Character Creation")
	void HideCharacterCreator();

	/** Applies a draft character create request to the local preview actor. */
	UFUNCTION(BlueprintCallable, Category = "Character Creation")
	void PreviewCharacterCreateRequest(const FWUCharacterCreateRequest& Request);

	/** Rotates the local character creator preview actor. */
	UFUNCTION(BlueprintCallable, Category = "Character Creation")
	void RotateCharacterCreatorPreview(float YawDelta);

	/** Handles the draft create request until backend persistence exists. */
	UFUNCTION(BlueprintCallable, Category = "Character Creation")
	void SubmitCharacterCreateRequest(const FWUCharacterCreateRequest& Request);

	/** Routes a progression award through the gameplay authority. */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RequestSelectedCharacterExperience(int32 Amount, EWUExperienceSource Source);

	/** Awards exploration XP through the shared progression path. */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void GrantExplorationExperience(int32 Amount);

	/** Awards quest turn-in XP through the shared progression path. */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void GrantQuestTurnInExperience(int32 Amount);

	/** Owning client persists awarded XP to the account backend. */
	UFUNCTION(Client, Reliable)
	void Client_HandleExperienceAward(int32 Amount, EWUExperienceSource Source, const FString& SourceKey);

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Updates cursor-driven world hover UI. */
	virtual void PlayerTick(float DeltaTime) override;

	/** Saves selected character location when the controller leaves gameplay. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Applies selected character identity after server-side possession. */
	virtual void OnPossess(APawn* InPawn) override;

	/** Applies selected character identity after client-side possession ack. */
	virtual void AcknowledgePossession(APawn* P) override;

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

	/** Returns true while the chat text entry is active. */
	bool IsChatInputOpen() const;

	/** Returns true while the inventory shell window is open. */
	bool IsInventoryOpen() const;

	/** Returns true while the vendor shell window is open. */
	bool IsVendorOpen() const;

	/** Returns true while the character panel window is open. */
	bool IsCharacterPanelOpen() const;

	/** Returns true while the character creator shell window is open. */
	bool IsCharacterCreatorOpen() const;

	/** Returns true while the club charter naming prompt is open. */
	bool IsClubCharterPromptOpen() const;

	/** Returns true while the social window is open. */
	bool IsSocialPanelOpen() const;

	/** Finds an interactable NPC under the cursor. */
	AWUNpcCharacter* FindNpcUnderCursor() const;

	/** Finds the nearest interactable NPC close enough to the controlled pawn. */
	AWUNpcCharacter* FindNearestInteractableNpc() const;

	/** Returns true when the NPC is close enough for interaction. */
	bool IsNpcInInteractionRange(const AWUNpcCharacter* NpcCharacter) const;

	/** Updates the bottom-right world hover tooltip from the actor under the cursor. */
	void UpdateWorldHoverTooltip();

	/** Finds a player character or NPC beneath the cursor without interaction range gating. */
	AActor* FindWorldHoverActorUnderCursor() const;

	/** Ensures a local-only preview actor exists for the character creator. */
	AWUCharacterCreatorPreviewActor* EnsureCharacterCreatorPreviewActor();

	/** Places the local-only preview actor near the controlled pawn. */
	void PositionCharacterCreatorPreviewActor();

	/** Sanitizes player-entered character names for the draft request. */
	FString SanitizeCharacterName(const FString& RawName) const;

	/** Sanitizes raw player chat before local/server handling. */
	FString SanitizeChatMessage(const FString& RawMessage) const;

	/** Returns the display name used for outgoing chat. */
	FString GetChatDisplayName() const;

	/** Parses slash commands and prepares a channel-aware outgoing chat message. */
	bool TryPrepareOutgoingChatMessage(const FString& RawMessage, FString& OutMessage, EWUChatChannel& OutChannel) const;

	/** Returns the selected character's cached club summary, when present. */
	bool TryGetSelectedCharacterClub(FWUClubSummary& OutClub) const;

	/** Sends selected-character identity to the server-side chat router. */
	void UpdateServerChatIdentityFromSession();

	/** Applies the selected login character to this gameplay controller/pawn. */
	void ApplySelectedCharacterSessionContext();

	/** Persists the selected character's current world location. */
	void SaveSelectedCharacterLocation();

	UFUNCTION()
	void HandleInventorySnapshotLoaded(const FWUBackendInventorySnapshot& Snapshot);

	UFUNCTION()
	void HandleSessionCharacterUpdated(const FWUBackendCharacterSummary& UpdatedCharacter);

	UFUNCTION()
	void HandleInventoryItemUseRequested(int32 SlotIndex, FWUInventoryItem Item);

	UFUNCTION()
	void HandleClubCharterSubmitted(const FString& ClubName, int32 SlotIndex, FWUInventoryItem Item);

	UFUNCTION()
	void HandleClubCharterPromptCancelled();

	UFUNCTION()
	void HandleClubCreated(const FWUClubSummary& Club);

	UFUNCTION()
	void HandleClubInviteCreated(const FWUBackendClubInviteSummary& Invite);

	UFUNCTION()
	void HandleClubMemberRemoved(const FString& CharacterId);

	UFUNCTION()
	void HandleClubRosterLoaded(const TArray<FWUClubMemberSummary>& Members);

	UFUNCTION()
	void HandleSocialIncludeOfflineChanged(bool bIncludeOffline);

	UFUNCTION()
	void HandleSocialRefreshRequested();

	UFUNCTION()
	void HandleSocialInviteRequested(const FString& CharacterNameOrId);

	UFUNCTION()
	void HandleSocialKickRequested(const FString& CharacterId);

	UFUNCTION()
	void HandleSessionRequestFailed(const FString& ErrorMessage);

	void ShowClubCharterPrompt(int32 SlotIndex, const FWUInventoryItem& Item);
	void HideClubCharterPrompt();
	void RestoreInputAfterModalPrompt();

	/** Applies a loaded backend inventory snapshot to the currently possessed pawn. */
	bool ApplyInventorySnapshotToCurrentPawn(const FWUBackendInventorySnapshot& Snapshot);

	/** Server-authoritative experience award entry point. */
	UFUNCTION(Server, Reliable)
	void Server_RequestSelectedCharacterExperience(int32 Amount, EWUExperienceSource Source);

	/** Server-authoritative idempotent exploration award for a specific zone. */
	UFUNCTION(Server, Reliable)
	void Server_RequestZoneExplorationExperience(int32 Amount, FName ZoneId);

	/** Server-authoritative saved location restore for the selected character. */
	UFUNCTION(Server, Reliable)
	void Server_ApplySelectedCharacterSpawnLocation(FVector SavedLocation);

	/** Server-side player chat entry point. */
	UFUNCTION(Server, Reliable)
	void Server_SendChatMessage(const FString& Message, EWUChatChannel Channel, const FString& ClubId);

	/** Server-side identity used by the prototype chat router. */
	UFUNCTION(Server, Reliable)
	void Server_UpdateChatIdentity(const FString& CharacterId, const FString& DisplayName, const FString& ClubId, EWUClubRank ClubRank);

	/** Handles replicated target updates from server-authoritative systems */
	UFUNCTION()
	void OnRep_CurrentTarget();

	/** Clears the target if the selected actor is destroyed. */
	UFUNCTION()
	void OnCurrentTargetDestroyed(AActor* DestroyedActor);

private:

	/** Client-side chat delivery. */
	UFUNCTION(Client, Reliable)
	void Client_ReceiveChatMessage(const FString& SenderName, const FString& Message);

	/** Client-side target update used when server-authoritative damage selects a target. */
	UFUNCTION(Client, Reliable)
	void Client_AutoTargetDamagedCharacter(AWUCharacter* DamagedCharacter);

	float LastChatMessageServerTime = -1000.0f;
	FString AppliedSessionCharacterId;
	FString AppliedSessionSpawnCharacterId;
	FString AppliedInventoryCharacterId;
	FString ServerChatCharacterId;
	FString ServerChatDisplayName;
	FString ServerChatClubId;
	EWUClubRank ServerChatClubRank = EWUClubRank::None;
	int32 PendingClubCharterSlotIndex = INDEX_NONE;
	bool bSocialIncludeOffline = false;
	FTimerHandle CharacterLocationSaveTimerHandle;

};
