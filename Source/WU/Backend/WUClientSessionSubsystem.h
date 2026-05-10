// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStats/WUCharacterStats.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"
#include "Identity/WUIdentityTypes.h"
#include "HttpFwd.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WUClientSessionSubsystem.generated.h"

class FJsonObject;

USTRUCT(BlueprintType)
struct FWUBackendAccountSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FString AccountId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FString Username;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FString DisplayName;
};

USTRUCT(BlueprintType)
struct FWUBackendRealmSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FString RealmId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FString Slug;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FString Status;
};

USTRUCT(BlueprintType)
struct FWUBackendCharacterLocation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	float X = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	float Y = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	float Z = 0.0f;

	FVector ToVector() const;
	bool IsNearlyZero() const;
};

USTRUCT(BlueprintType)
struct FWUBackendCharacterSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FString CharacterId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	EWUCharacterRace Race = EWUCharacterRace::Halfblood;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	EWUCharacterSex Sex = EWUCharacterSex::Male;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	EWUHouseFaction HouseFaction = EWUHouseFaction::Unsorted;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FWUCharacterAppearance Appearance;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FWUClubSummary Club;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	int32 Level = 1;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	int32 Experience = 0;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	int32 ExperienceToNextLevel = 500;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FWUPrimaryStats PrimaryStats;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FWUDerivedStats DerivedStats;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FWUBackendCharacterLocation Location;
};

USTRUCT(BlueprintType)
struct FWUBackendClubInviteSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString InviteId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString ClubId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString InvitedCharacterId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString InvitedByCharacterId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString Status;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FDateTime CreatedAtUtc;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FDateTime ExpiresAtUtc;
};

USTRUCT(BlueprintType)
struct FWUBackendCurrencyBreakdown
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "WU|Currency")
	int64 BalanceKnuts = 0;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Currency")
	int64 Galleons = 0;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Currency")
	int64 Sickles = 0;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Currency")
	int64 Knuts = 0;

	FText ToDisplayText() const;
};

USTRUCT(BlueprintType)
struct FWUBackendCurrencyWallet
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "WU|Currency")
	FString WalletId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Currency")
	FString WalletType;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Currency")
	FString OwnerId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Currency")
	FWUBackendCurrencyBreakdown Balance;
};

USTRUCT(BlueprintType)
struct FWUBackendCurrencySnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "WU|Currency")
	FWUBackendCurrencyWallet CharacterWallet;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Currency")
	FWUBackendCurrencyWallet AccountBankWallet;
};

USTRUCT(BlueprintType)
struct FWUBackendInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "WU|Inventory")
	int32 SlotIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Inventory")
	FString ItemId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Inventory")
	int32 Quantity = 1;
};

USTRUCT(BlueprintType)
struct FWUBackendInventorySnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "WU|Inventory")
	FString CharacterId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Inventory")
	TArray<FWUBackendInventoryItem> Items;
};

USTRUCT(BlueprintType)
struct FWUBackendVendorPurchase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "WU|Vendor")
	FString VendorTableId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Vendor")
	FString ItemId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Vendor")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Vendor")
	int64 PriceKnuts = 0;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Vendor")
	FWUBackendCurrencySnapshot Snapshot;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Vendor")
	FWUBackendInventorySnapshot Inventory;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWUClientSessionSimpleSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionErrorSignature, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionCharactersSignature, const TArray<FWUBackendCharacterSummary>&, Characters);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionCharacterSignature, const FWUBackendCharacterSummary&, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionCharacterDeletedSignature, const FString&, CharacterId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionClubSignature, const FWUClubSummary&, Club);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionClubInviteSignature, const FWUBackendClubInviteSummary&, Invite);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionClubMemberRemovedSignature, const FString&, CharacterId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionClubMembersSignature, const TArray<FWUClubMemberSummary>&, Members);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionCurrencySnapshotSignature, const FWUBackendCurrencySnapshot&, Snapshot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionInventorySnapshotSignature, const FWUBackendInventorySnapshot&, Snapshot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionVendorPurchaseSignature, const FWUBackendVendorPurchase&, Purchase);

/**
 * Client-side bridge to the WU persistence backend.
 * Owns the temporary auth token and character-list data used by login and character select UI.
 */
UCLASS(Config = Game)
class WU_API UWUClientSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "WU|Session")
	FWUClientSessionSimpleSignature OnLoginSucceeded;

	UPROPERTY(BlueprintAssignable, Category = "WU|Session")
	FWUClientSessionSimpleSignature OnSessionCleared;

	UPROPERTY(BlueprintAssignable, Category = "WU|Session")
	FWUClientSessionCharactersSignature OnCharactersLoaded;

	UPROPERTY(BlueprintAssignable, Category = "WU|Session")
	FWUClientSessionCharacterSignature OnCharacterCreated;

	UPROPERTY(BlueprintAssignable, Category = "WU|Session")
	FWUClientSessionCharacterSignature OnCharacterUpdated;

	UPROPERTY(BlueprintAssignable, Category = "WU|Session")
	FWUClientSessionCharacterDeletedSignature OnCharacterDeleted;

	UPROPERTY(BlueprintAssignable, Category = "WU|Club")
	FWUClientSessionClubSignature OnClubCreated;

	UPROPERTY(BlueprintAssignable, Category = "WU|Club")
	FWUClientSessionClubInviteSignature OnClubInviteCreated;

	UPROPERTY(BlueprintAssignable, Category = "WU|Club")
	FWUClientSessionClubMemberRemovedSignature OnClubMemberRemoved;

	UPROPERTY(BlueprintAssignable, Category = "WU|Club")
	FWUClientSessionClubMembersSignature OnClubRosterLoaded;

	UPROPERTY(BlueprintAssignable, Category = "WU|Currency")
	FWUClientSessionCurrencySnapshotSignature OnCurrencySnapshotLoaded;

	UPROPERTY(BlueprintAssignable, Category = "WU|Inventory")
	FWUClientSessionInventorySnapshotSignature OnInventorySnapshotLoaded;

	UPROPERTY(BlueprintAssignable, Category = "WU|Vendor")
	FWUClientSessionVendorPurchaseSignature OnVendorPurchaseCompleted;

	UPROPERTY(BlueprintAssignable, Category = "WU|Session")
	FWUClientSessionErrorSignature OnRequestFailed;

	UFUNCTION(BlueprintCallable, Category = "WU|Session")
	void DevLogin();

	UFUNCTION(BlueprintCallable, Category = "WU|Session")
	void RefreshCurrentSession();

	UFUNCTION(BlueprintCallable, Category = "WU|Session")
	void ListCharacters();

	UFUNCTION(BlueprintCallable, Category = "WU|Session")
	void CreateCharacter(const FWUCharacterCreateRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "WU|Session")
	void DeleteCharacter(const FString& CharacterId);

	UFUNCTION(BlueprintCallable, Category = "WU|Session")
	void SelectCharacter(const FString& CharacterId);

	UFUNCTION(BlueprintCallable, Category = "WU|Session")
	void SaveSelectedCharacterLocation(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "WU|Session")
	void AwardSelectedCharacterExperience(int32 Amount, EWUExperienceSource Source);

	void AwardSelectedCharacterExperience(int32 Amount, EWUExperienceSource Source, const FString& SourceKey);

	UFUNCTION(BlueprintCallable, Category = "WU|Club")
	void CreateClub(const FString& Name, const FString& Tag, const FString& Description);

	UFUNCTION(BlueprintCallable, Category = "WU|Club")
	void CreateClubFromSelectedCharter(const FString& Name, int32 CharterSlotIndex, const FString& CharterItemId);

	UFUNCTION(BlueprintCallable, Category = "WU|Club")
	void InviteCharacterToSelectedClub(const FString& InvitedCharacterNameOrId);

	UFUNCTION(BlueprintCallable, Category = "WU|Club")
	void KickCharacterFromSelectedClub(const FString& MemberCharacterId);

	UFUNCTION(BlueprintCallable, Category = "WU|Club")
	void LoadSelectedClubRoster(bool bIncludeOffline);

	UFUNCTION(BlueprintCallable, Category = "WU|Currency")
	void RefreshSelectedCurrencySnapshot();

	UFUNCTION(BlueprintCallable, Category = "WU|Inventory")
	void RefreshSelectedInventorySnapshot();

	UFUNCTION(BlueprintCallable, Category = "WU|Vendor")
	void PurchaseSelectedVendorItem(const FString& VendorTableId, const FString& ItemId);

	UFUNCTION(BlueprintCallable, Category = "WU|Session")
	void ClearSession();

	UFUNCTION(BlueprintCallable, Category = "WU|Session")
	void SetBackendBaseUrl(const FString& NewBackendBaseUrl);

	UFUNCTION(BlueprintPure, Category = "WU|Session")
	const FString& GetBackendBaseUrl() const;

	UFUNCTION(BlueprintPure, Category = "WU|Session")
	bool IsAuthenticated() const;

	UFUNCTION(BlueprintPure, Category = "WU|Session")
	const FString& GetAccessToken() const;

	UFUNCTION(BlueprintPure, Category = "WU|Session")
	const FWUBackendAccountSummary& GetAccount() const;

	UFUNCTION(BlueprintPure, Category = "WU|Session")
	const TArray<FWUBackendRealmSummary>& GetRealms() const;

	UFUNCTION(BlueprintPure, Category = "WU|Session")
	const FString& GetSelectedRealmId() const;

	UFUNCTION(BlueprintPure, Category = "WU|Session")
	const TArray<FWUBackendCharacterSummary>& GetCharacters() const;

	UFUNCTION(BlueprintPure, Category = "WU|Session")
	const FString& GetSelectedCharacterId() const;

	UFUNCTION(BlueprintPure, Category = "WU|Currency")
	const FWUBackendCurrencySnapshot& GetCurrencySnapshot() const;

	UFUNCTION(BlueprintPure, Category = "WU|Currency")
	bool HasCurrencySnapshot() const;

	UFUNCTION(BlueprintPure, Category = "WU|Inventory")
	const FWUBackendInventorySnapshot& GetInventorySnapshot() const;

	UFUNCTION(BlueprintPure, Category = "WU|Inventory")
	bool HasInventorySnapshot() const;

protected:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "WU|Session")
	FString BackendBaseUrl = TEXT("http://127.0.0.1:5080");

private:
	void HandleDevLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleCurrentSessionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleListCharactersResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleCreateCharacterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleDeleteCharacterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded, FString CharacterId);
	void HandleSaveCharacterLocationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleAwardCharacterExperienceResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleCreateClubResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleInviteClubMemberResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleKickClubMemberResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleLoadClubRosterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleRefreshCurrencySnapshotResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleRefreshInventorySnapshotResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandlePurchaseVendorItemResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateRequest(const FString& Verb, const FString& Path) const;
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateAuthorizedRequest(const FString& Verb, const FString& Path) const;
	TSharedPtr<FJsonObject> CreateSessionContextJsonObject() const;

	static void SetJsonBody(const TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& Request, const TSharedPtr<FJsonObject>& JsonObject);
	static FString SerializeJsonObject(const TSharedPtr<FJsonObject>& JsonObject);
	bool TryDeserializeObject(FHttpResponsePtr Response, TSharedPtr<FJsonObject>& OutJsonObject, FString& OutErrorMessage) const;
	FString ExtractBackendError(FHttpResponsePtr Response) const;
	FString BuildUrl(const FString& Path) const;
	bool HasSessionContext() const;

	static bool TryParseAccount(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendAccountSummary& OutAccount);
	static bool TryParseRealm(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendRealmSummary& OutRealm);
	static bool TryParseCharacter(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendCharacterSummary& OutCharacter);
	static bool TryParseClubSummary(const TSharedPtr<FJsonObject>& JsonObject, FWUClubSummary& OutClub);
	static bool TryParseClubInvite(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendClubInviteSummary& OutInvite);
	static bool TryParseClubMember(const TSharedPtr<FJsonObject>& JsonObject, FWUClubMemberSummary& OutMember);
	static bool TryParseVendorPurchase(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendVendorPurchase& OutPurchase);
	static bool TryParseInventorySnapshot(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendInventorySnapshot& OutSnapshot);
	static bool TryParseInventoryItem(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendInventoryItem& OutItem);
	static bool TryParseCurrencySnapshot(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendCurrencySnapshot& OutSnapshot);
	static bool TryParseCurrencyWallet(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendCurrencyWallet& OutWallet);
	static bool TryParseCurrencyBreakdown(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendCurrencyBreakdown& OutBalance);
	static bool TryParseLocation(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendCharacterLocation& OutLocation);
	static bool TryParseRace(const FString& Value, EWUCharacterRace& OutRace);
	static bool TryParseSex(const FString& Value, EWUCharacterSex& OutSex);
	static bool TryParseHouseFaction(const FString& Value, EWUHouseFaction& OutHouseFaction);
	static bool TryParseClubRank(const FString& Value, EWUClubRank& OutRank);
	static bool TryParseDateTimeUtc(const FString& Value, FDateTime& OutDateTime);
	static FString ExperienceSourceToString(EWUExperienceSource Source);
	FWUBackendCharacterSummary* FindMutableCachedCharacter(const FString& CharacterId);
	const FWUBackendCharacterSummary* FindCachedCharacter(const FString& CharacterId) const;
	bool UpdateCachedCharacter(const FWUBackendCharacterSummary& UpdatedCharacter);

private:
	FString AccessToken;
	FWUBackendAccountSummary Account;
	TArray<FWUBackendRealmSummary> Realms;
	FString SelectedRealmId;
	TArray<FWUBackendCharacterSummary> Characters;
	FString SelectedCharacterId;
	FWUBackendCurrencySnapshot CurrencySnapshot;
	bool bHasCurrencySnapshot = false;
	FWUBackendInventorySnapshot InventorySnapshot;
	bool bHasInventorySnapshot = false;
	FString PendingClubKickCharacterId;
};
