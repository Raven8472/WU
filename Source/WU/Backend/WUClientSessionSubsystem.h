// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStats/WUCharacterStats.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"
#include "HttpFwd.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WUClientSessionSubsystem.generated.h"

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
	FWUCharacterAppearance Appearance;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	int32 Level = 1;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FWUPrimaryStats PrimaryStats;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FWUDerivedStats DerivedStats;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Session")
	FWUBackendCharacterLocation Location;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWUClientSessionSimpleSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionErrorSignature, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionCharactersSignature, const TArray<FWUBackendCharacterSummary>&, Characters);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionCharacterSignature, const FWUBackendCharacterSummary&, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUClientSessionCharacterDeletedSignature, const FString&, CharacterId);

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
	FWUClientSessionCharacterDeletedSignature OnCharacterDeleted;

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

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateRequest(const FString& Verb, const FString& Path) const;
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateAuthorizedRequest(const FString& Verb, const FString& Path) const;

	bool TryDeserializeObject(FHttpResponsePtr Response, TSharedPtr<FJsonObject>& OutJsonObject, FString& OutErrorMessage) const;
	FString ExtractBackendError(FHttpResponsePtr Response) const;
	FString BuildUrl(const FString& Path) const;
	bool HasSessionContext() const;

	static bool TryParseAccount(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendAccountSummary& OutAccount);
	static bool TryParseRealm(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendRealmSummary& OutRealm);
	static bool TryParseCharacter(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendCharacterSummary& OutCharacter);
	static bool TryParseLocation(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendCharacterLocation& OutLocation);
	static bool TryParseRace(const FString& Value, EWUCharacterRace& OutRace);
	static bool TryParseSex(const FString& Value, EWUCharacterSex& OutSex);

private:
	FString AccessToken;
	FWUBackendAccountSummary Account;
	TArray<FWUBackendRealmSummary> Realms;
	FString SelectedRealmId;
	TArray<FWUBackendCharacterSummary> Characters;
	FString SelectedCharacterId;
};
