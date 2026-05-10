// Copyright Epic Games, Inc. All Rights Reserved.

#include "Backend/WUClientSessionSubsystem.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "WU.h"

FVector FWUBackendCharacterLocation::ToVector() const
{
	return FVector(X, Y, Z);
}

bool FWUBackendCharacterLocation::IsNearlyZero() const
{
	return ToVector().IsNearlyZero();
}

FText FWUBackendCurrencyBreakdown::ToDisplayText() const
{
	return FText::Format(
		NSLOCTEXT("WUClientSessionSubsystem", "CurrencyBreakdownDisplay", "{0} G  {1} S  {2} K"),
		FText::AsNumber(Galleons),
		FText::AsNumber(Sickles),
		FText::AsNumber(Knuts));
}

void UWUClientSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BackendBaseUrl.TrimStartAndEndInline();
}

void UWUClientSessionSubsystem::Deinitialize()
{
	ClearSession();
	Super::Deinitialize();
}

void UWUClientSessionSubsystem::DevLogin()
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("POST"), TEXT("/api/auth/dev-login"));
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleDevLoginResponse);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::RefreshCurrentSession()
{
	if (AccessToken.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("No access token is available."));
		return;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("GET"), TEXT("/api/auth/me"));
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleCurrentSessionResponse);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::ListCharacters()
{
	if (!HasSessionContext())
	{
		OnRequestFailed.Broadcast(TEXT("Login and realm selection are required before loading characters."));
		return;
	}

	const FString Path = FString::Printf(
		TEXT("/api/accounts/%s/realms/%s/characters"),
		*Account.AccountId,
		*SelectedRealmId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("GET"), Path);
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleListCharactersResponse);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::CreateCharacter(const FWUCharacterCreateRequest& RequestData)
{
	if (!HasSessionContext())
	{
		OnRequestFailed.Broadcast(TEXT("Login and realm selection are required before creating a character."));
		return;
	}

	TSharedPtr<FJsonObject> RootObject = CreateSessionContextJsonObject();
	RootObject->SetStringField(TEXT("name"), RequestData.CharacterName);
	RootObject->SetStringField(TEXT("race"), WUCharacterCreation::RaceToString(RequestData.Race));
	RootObject->SetStringField(TEXT("sex"), WUCharacterCreation::SexToString(RequestData.Sex));

	TSharedPtr<FJsonObject> AppearanceObject = MakeShared<FJsonObject>();
	AppearanceObject->SetNumberField(TEXT("skinPresetIndex"), RequestData.SkinPresetIndex);
	AppearanceObject->SetNumberField(TEXT("headPresetIndex"), RequestData.HeadPresetIndex);
	AppearanceObject->SetNumberField(TEXT("hairStyleIndex"), RequestData.HairStyleIndex);
	AppearanceObject->SetNumberField(TEXT("hairColorIndex"), RequestData.HairColorIndex);
	AppearanceObject->SetNumberField(TEXT("eyeColorIndex"), RequestData.EyeColorIndex);
	AppearanceObject->SetNumberField(TEXT("browStyleIndex"), RequestData.BrowStyleIndex);
	AppearanceObject->SetNumberField(TEXT("beardStyleIndex"), RequestData.BeardStyleIndex);
	RootObject->SetObjectField(TEXT("appearance"), AppearanceObject);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("POST"), TEXT("/api/characters"));
	SetJsonBody(Request, RootObject);
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleCreateCharacterResponse);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::DeleteCharacter(const FString& CharacterId)
{
	if (!HasSessionContext())
	{
		OnRequestFailed.Broadcast(TEXT("Login and realm selection are required before deleting a character."));
		return;
	}

	if (CharacterId.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("Select a character before deleting."));
		return;
	}

	const FString Path = FString::Printf(
		TEXT("/api/accounts/%s/realms/%s/characters/%s"),
		*Account.AccountId,
		*SelectedRealmId,
		*CharacterId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("DELETE"), Path);
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleDeleteCharacterResponse, CharacterId);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::SelectCharacter(const FString& CharacterId)
{
	if (SelectedCharacterId != CharacterId)
	{
		CurrencySnapshot = FWUBackendCurrencySnapshot();
		bHasCurrencySnapshot = false;
		InventorySnapshot = FWUBackendInventorySnapshot();
		bHasInventorySnapshot = false;
	}

	SelectedCharacterId = CharacterId;
}

void UWUClientSessionSubsystem::SaveSelectedCharacterLocation(const FVector& Location)
{
	if (!HasSessionContext() || SelectedCharacterId.IsEmpty())
	{
		return;
	}

	TSharedPtr<FJsonObject> RootObject = CreateSessionContextJsonObject();

	TSharedPtr<FJsonObject> LocationObject = MakeShared<FJsonObject>();
	LocationObject->SetNumberField(TEXT("x"), Location.X);
	LocationObject->SetNumberField(TEXT("y"), Location.Y);
	LocationObject->SetNumberField(TEXT("z"), Location.Z);
	RootObject->SetObjectField(TEXT("location"), LocationObject);

	const FString Path = FString::Printf(TEXT("/api/characters/%s/location"), *SelectedCharacterId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("PUT"), Path);
	SetJsonBody(Request, RootObject);
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleSaveCharacterLocationResponse);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::AwardSelectedCharacterExperience(int32 Amount, EWUExperienceSource Source)
{
	AwardSelectedCharacterExperience(Amount, Source, FString());
}

void UWUClientSessionSubsystem::AwardSelectedCharacterExperience(int32 Amount, EWUExperienceSource Source, const FString& SourceKey)
{
	if (!HasSessionContext() || SelectedCharacterId.IsEmpty())
	{
		return;
	}

	if (Amount <= 0)
	{
		OnRequestFailed.Broadcast(TEXT("Experience awards must be greater than zero."));
		return;
	}

	TSharedPtr<FJsonObject> RootObject = CreateSessionContextJsonObject();
	RootObject->SetNumberField(TEXT("amount"), Amount);
	RootObject->SetStringField(TEXT("source"), ExperienceSourceToString(Source));
	if (!SourceKey.IsEmpty())
	{
		RootObject->SetStringField(TEXT("sourceKey"), SourceKey);
	}

	const FString Path = FString::Printf(TEXT("/api/characters/%s/experience"), *SelectedCharacterId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("POST"), Path);
	SetJsonBody(Request, RootObject);
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleAwardCharacterExperienceResponse);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::CreateClub(const FString& Name, const FString& Tag, const FString& Description)
{
	if (!HasSessionContext() || SelectedCharacterId.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("Login, realm selection, and a selected character are required before creating a club."));
		return;
	}

	FString TrimmedName = Name;
	TrimmedName.TrimStartAndEndInline();
	if (TrimmedName.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("Club name is required."));
		return;
	}

	TSharedPtr<FJsonObject> RootObject = CreateSessionContextJsonObject();
	RootObject->SetStringField(TEXT("presidentCharacterId"), SelectedCharacterId);
	RootObject->SetStringField(TEXT("name"), TrimmedName);
	RootObject->SetStringField(TEXT("tag"), Tag.TrimStartAndEnd());
	RootObject->SetStringField(TEXT("description"), Description.TrimStartAndEnd());

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("POST"), TEXT("/api/clubs"));
	SetJsonBody(Request, RootObject);
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleCreateClubResponse);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::CreateClubFromSelectedCharter(const FString& Name, int32 CharterSlotIndex, const FString& CharterItemId)
{
	if (!HasSessionContext() || SelectedCharacterId.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("Login, realm selection, and a selected character are required before creating a club."));
		return;
	}

	FString TrimmedName = Name;
	TrimmedName.TrimStartAndEndInline();
	if (TrimmedName.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("Club name is required."));
		return;
	}

	FString TrimmedItemId = CharterItemId;
	TrimmedItemId.TrimStartAndEndInline();
	if (CharterSlotIndex < 0 || TrimmedItemId.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("A club charter item is required."));
		return;
	}

	TSharedPtr<FJsonObject> RootObject = CreateSessionContextJsonObject();
	RootObject->SetStringField(TEXT("presidentCharacterId"), SelectedCharacterId);
	RootObject->SetStringField(TEXT("name"), TrimmedName);
	RootObject->SetStringField(TEXT("tag"), TEXT(""));
	RootObject->SetStringField(TEXT("description"), TEXT(""));
	RootObject->SetNumberField(TEXT("slotIndex"), CharterSlotIndex);
	RootObject->SetStringField(TEXT("itemId"), TrimmedItemId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("POST"), TEXT("/api/clubs/from-charter"));
	SetJsonBody(Request, RootObject);
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleCreateClubResponse);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::InviteCharacterToSelectedClub(const FString& InvitedCharacterId)
{
	if (!HasSessionContext() || SelectedCharacterId.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("Login, realm selection, and a selected character are required before inviting to a club."));
		return;
	}

	if (InvitedCharacterId.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("Choose a character to invite."));
		return;
	}

	if (InvitedCharacterId == SelectedCharacterId)
	{
		OnRequestFailed.Broadcast(TEXT("A character cannot invite itself."));
		return;
	}

	const FWUBackendCharacterSummary* SelectedCharacter = FindCachedCharacter(SelectedCharacterId);
	if (!SelectedCharacter || !SelectedCharacter->Club.HasClub())
	{
		OnRequestFailed.Broadcast(TEXT("The selected character is not in a club."));
		return;
	}

	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("inviterCharacterId"), SelectedCharacterId);
	RootObject->SetStringField(TEXT("invitedCharacterId"), InvitedCharacterId);

	const FString Path = FString::Printf(TEXT("/api/clubs/%s/invites"), *SelectedCharacter->Club.ClubId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("POST"), Path);
	SetJsonBody(Request, RootObject);
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleInviteClubMemberResponse);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::LoadSelectedClubRoster(bool bIncludeOffline)
{
	if (!HasSessionContext() || SelectedCharacterId.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("Login, realm selection, and a selected character are required before loading a club roster."));
		return;
	}

	const FWUBackendCharacterSummary* SelectedCharacter = FindCachedCharacter(SelectedCharacterId);
	if (!SelectedCharacter || !SelectedCharacter->Club.HasClub())
	{
		OnRequestFailed.Broadcast(TEXT("The selected character is not in a club."));
		return;
	}

	const TCHAR* IncludeOfflineValue = bIncludeOffline ? TEXT("true") : TEXT("false");
	const FString Path = FString::Printf(
		TEXT("/api/clubs/%s/roster?viewerCharacterId=%s&includeOffline=%s"),
		*SelectedCharacter->Club.ClubId,
		*SelectedCharacterId,
		IncludeOfflineValue);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("GET"), Path);
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleLoadClubRosterResponse);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::RefreshSelectedCurrencySnapshot()
{
	if (!HasSessionContext() || SelectedCharacterId.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("Login, realm selection, and a selected character are required before loading currency."));
		return;
	}

	const FString Path = FString::Printf(
		TEXT("/api/currency/accounts/%s/realms/%s/characters/%s"),
		*Account.AccountId,
		*SelectedRealmId,
		*SelectedCharacterId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("GET"), Path);
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleRefreshCurrencySnapshotResponse);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::RefreshSelectedInventorySnapshot()
{
	if (!HasSessionContext() || SelectedCharacterId.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("Login, realm selection, and a selected character are required before loading inventory."));
		return;
	}

	const FString Path = FString::Printf(
		TEXT("/api/inventory/accounts/%s/realms/%s/characters/%s"),
		*Account.AccountId,
		*SelectedRealmId,
		*SelectedCharacterId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("GET"), Path);
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleRefreshInventorySnapshotResponse);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::PurchaseSelectedVendorItem(const FString& VendorTableId, const FString& ItemId)
{
	if (!HasSessionContext() || SelectedCharacterId.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("Login, realm selection, and a selected character are required before purchasing from a vendor."));
		return;
	}

	FString TrimmedVendorTableId = VendorTableId;
	TrimmedVendorTableId.TrimStartAndEndInline();
	FString TrimmedItemId = ItemId;
	TrimmedItemId.TrimStartAndEndInline();
	if (TrimmedVendorTableId.IsEmpty() || TrimmedItemId.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("Vendor table and item id are required before purchasing from a vendor."));
		return;
	}

	TSharedPtr<FJsonObject> RootObject = CreateSessionContextJsonObject();
	RootObject->SetStringField(TEXT("characterId"), SelectedCharacterId);
	RootObject->SetStringField(TEXT("vendorTableId"), TrimmedVendorTableId);
	RootObject->SetStringField(TEXT("itemId"), TrimmedItemId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("POST"), TEXT("/api/vendors/purchase"));
	SetJsonBody(Request, RootObject);
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandlePurchaseVendorItemResponse);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::ClearSession()
{
	AccessToken.Reset();
	Account = FWUBackendAccountSummary();
	Realms.Reset();
	SelectedRealmId.Reset();
	Characters.Reset();
	SelectedCharacterId.Reset();
	CurrencySnapshot = FWUBackendCurrencySnapshot();
	bHasCurrencySnapshot = false;
	InventorySnapshot = FWUBackendInventorySnapshot();
	bHasInventorySnapshot = false;
	OnSessionCleared.Broadcast();
}

void UWUClientSessionSubsystem::SetBackendBaseUrl(const FString& NewBackendBaseUrl)
{
	BackendBaseUrl = NewBackendBaseUrl;
	BackendBaseUrl.TrimStartAndEndInline();
}

const FString& UWUClientSessionSubsystem::GetBackendBaseUrl() const
{
	return BackendBaseUrl;
}

bool UWUClientSessionSubsystem::IsAuthenticated() const
{
	return !AccessToken.IsEmpty() && !Account.AccountId.IsEmpty();
}

const FString& UWUClientSessionSubsystem::GetAccessToken() const
{
	return AccessToken;
}

const FWUBackendAccountSummary& UWUClientSessionSubsystem::GetAccount() const
{
	return Account;
}

const TArray<FWUBackendRealmSummary>& UWUClientSessionSubsystem::GetRealms() const
{
	return Realms;
}

const FString& UWUClientSessionSubsystem::GetSelectedRealmId() const
{
	return SelectedRealmId;
}

const TArray<FWUBackendCharacterSummary>& UWUClientSessionSubsystem::GetCharacters() const
{
	return Characters;
}

const FString& UWUClientSessionSubsystem::GetSelectedCharacterId() const
{
	return SelectedCharacterId;
}

const FWUBackendCurrencySnapshot& UWUClientSessionSubsystem::GetCurrencySnapshot() const
{
	return CurrencySnapshot;
}

bool UWUClientSessionSubsystem::HasCurrencySnapshot() const
{
	return bHasCurrencySnapshot;
}

const FWUBackendInventorySnapshot& UWUClientSessionSubsystem::GetInventorySnapshot() const
{
	return InventorySnapshot;
}

bool UWUClientSessionSubsystem::HasInventorySnapshot() const
{
	return bHasInventorySnapshot;
}

void UWUClientSessionSubsystem::HandleDevLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	TSharedPtr<FJsonObject> RootObject;
	FString ErrorMessage;
	if (!bSucceeded || !TryDeserializeObject(Response, RootObject, ErrorMessage))
	{
		OnRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		OnRequestFailed.Broadcast(ExtractBackendError(Response));
		return;
	}

	FString NewAccessToken;
	if (!RootObject->TryGetStringField(TEXT("accessToken"), NewAccessToken) || NewAccessToken.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("Login response did not include an access token."));
		return;
	}

	const TSharedPtr<FJsonObject>* AccountObject = nullptr;
	if (!RootObject->TryGetObjectField(TEXT("account"), AccountObject) || !TryParseAccount(*AccountObject, Account))
	{
		OnRequestFailed.Broadcast(TEXT("Login response did not include a valid account."));
		return;
	}

	Realms.Reset();
	const TArray<TSharedPtr<FJsonValue>>* RealmValues = nullptr;
	if (RootObject->TryGetArrayField(TEXT("realms"), RealmValues))
	{
		for (const TSharedPtr<FJsonValue>& RealmValue : *RealmValues)
		{
			FWUBackendRealmSummary Realm;
			if (TryParseRealm(RealmValue->AsObject(), Realm))
			{
				Realms.Add(Realm);
			}
		}
	}

	if (Realms.IsEmpty())
	{
		OnRequestFailed.Broadcast(TEXT("Login response did not include any realms."));
		return;
	}

	AccessToken = NewAccessToken;
	SelectedRealmId = Realms[0].RealmId;
	Characters.Reset();
	SelectedCharacterId.Reset();
	CurrencySnapshot = FWUBackendCurrencySnapshot();
	bHasCurrencySnapshot = false;

	OnLoginSucceeded.Broadcast();
	ListCharacters();
}

void UWUClientSessionSubsystem::HandleCurrentSessionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	TSharedPtr<FJsonObject> RootObject;
	FString ErrorMessage;
	if (!bSucceeded || !TryDeserializeObject(Response, RootObject, ErrorMessage))
	{
		OnRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		OnRequestFailed.Broadcast(ExtractBackendError(Response));
		return;
	}

	const TSharedPtr<FJsonObject>* AccountObject = nullptr;
	if (!RootObject->TryGetObjectField(TEXT("account"), AccountObject) || !TryParseAccount(*AccountObject, Account))
	{
		OnRequestFailed.Broadcast(TEXT("Session response did not include a valid account."));
		return;
	}

	Realms.Reset();
	const TArray<TSharedPtr<FJsonValue>>* RealmValues = nullptr;
	if (RootObject->TryGetArrayField(TEXT("realms"), RealmValues))
	{
		for (const TSharedPtr<FJsonValue>& RealmValue : *RealmValues)
		{
			FWUBackendRealmSummary Realm;
			if (TryParseRealm(RealmValue->AsObject(), Realm))
			{
				Realms.Add(Realm);
			}
		}
	}

	if (SelectedRealmId.IsEmpty() && !Realms.IsEmpty())
	{
		SelectedRealmId = Realms[0].RealmId;
	}

	OnLoginSucceeded.Broadcast();
}

void UWUClientSessionSubsystem::HandleListCharactersResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	FString ErrorMessage;
	if (!bSucceeded || !Response.IsValid())
	{
		OnRequestFailed.Broadcast(TEXT("Character list request failed."));
		return;
	}

	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		OnRequestFailed.Broadcast(ExtractBackendError(Response));
		return;
	}

	const FString Content = Response->GetContentAsString();
	TArray<TSharedPtr<FJsonValue>> CharacterValues;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
	if (!FJsonSerializer::Deserialize(Reader, CharacterValues))
	{
		OnRequestFailed.Broadcast(TEXT("Character list response was not valid JSON."));
		return;
	}

	Characters.Reset();
	for (const TSharedPtr<FJsonValue>& CharacterValue : CharacterValues)
	{
		FWUBackendCharacterSummary Character;
		if (TryParseCharacter(CharacterValue->AsObject(), Character))
		{
			Characters.Add(Character);
		}
	}

	if (SelectedCharacterId.IsEmpty() && !Characters.IsEmpty())
	{
		SelectedCharacterId = Characters[0].CharacterId;
	}

	OnCharactersLoaded.Broadcast(Characters);
}

void UWUClientSessionSubsystem::HandleCreateCharacterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	TSharedPtr<FJsonObject> RootObject;
	FString ErrorMessage;
	if (!bSucceeded || !TryDeserializeObject(Response, RootObject, ErrorMessage))
	{
		OnRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()) && Response->GetResponseCode() != EHttpResponseCodes::Created)
	{
		OnRequestFailed.Broadcast(ExtractBackendError(Response));
		return;
	}

	FWUBackendCharacterSummary Character;
	if (!TryParseCharacter(RootObject, Character))
	{
		OnRequestFailed.Broadcast(TEXT("Create character response did not include a valid character."));
		return;
	}

	Characters.Add(Character);
	SelectedCharacterId = Character.CharacterId;
	OnCharacterCreated.Broadcast(Character);
	OnCharactersLoaded.Broadcast(Characters);
}

void UWUClientSessionSubsystem::HandleDeleteCharacterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded, FString CharacterId)
{
	if (!bSucceeded || !Response.IsValid())
	{
		OnRequestFailed.Broadcast(TEXT("Character delete request failed."));
		return;
	}

	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()) && Response->GetResponseCode() != 204)
	{
		OnRequestFailed.Broadcast(ExtractBackendError(Response));
		return;
	}

	Characters.RemoveAll([&CharacterId](const FWUBackendCharacterSummary& Character)
	{
		return Character.CharacterId == CharacterId;
	});

	if (SelectedCharacterId == CharacterId)
	{
		SelectedCharacterId = Characters.IsEmpty() ? FString() : Characters[0].CharacterId;
	}

	OnCharactersLoaded.Broadcast(Characters);
	OnCharacterDeleted.Broadcast(CharacterId);
}

void UWUClientSessionSubsystem::HandleSaveCharacterLocationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	TSharedPtr<FJsonObject> RootObject;
	FString ErrorMessage;
	if (!bSucceeded || !TryDeserializeObject(Response, RootObject, ErrorMessage))
	{
		UE_LOG(LogWU, Warning, TEXT("Character location save failed: %s"), *ErrorMessage);
		return;
	}

	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogWU, Warning, TEXT("Character location save failed: %s"), *ExtractBackendError(Response));
		return;
	}

	FWUBackendCharacterSummary UpdatedCharacter;
	if (!TryParseCharacter(RootObject, UpdatedCharacter))
	{
		UE_LOG(LogWU, Warning, TEXT("Character location save response did not include a valid character."));
		return;
	}

	UpdateCachedCharacter(UpdatedCharacter);
}

void UWUClientSessionSubsystem::HandleAwardCharacterExperienceResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	TSharedPtr<FJsonObject> RootObject;
	FString ErrorMessage;
	if (!bSucceeded || !TryDeserializeObject(Response, RootObject, ErrorMessage))
	{
		UE_LOG(LogWU, Warning, TEXT("Character experience award failed: %s"), *ErrorMessage);
		return;
	}

	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogWU, Warning, TEXT("Character experience award failed: %s"), *ExtractBackendError(Response));
		return;
	}

	FWUBackendCharacterSummary UpdatedCharacter;
	if (!TryParseCharacter(RootObject, UpdatedCharacter))
	{
		UE_LOG(LogWU, Warning, TEXT("Character experience award response did not include a valid character."));
		return;
	}

	if (UpdateCachedCharacter(UpdatedCharacter))
	{
		OnCharacterUpdated.Broadcast(UpdatedCharacter);
		OnCharactersLoaded.Broadcast(Characters);
	}
}

void UWUClientSessionSubsystem::HandleCreateClubResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	TSharedPtr<FJsonObject> RootObject;
	FString ErrorMessage;
	if (!bSucceeded || !TryDeserializeObject(Response, RootObject, ErrorMessage))
	{
		OnRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()) && Response->GetResponseCode() != EHttpResponseCodes::Created)
	{
		OnRequestFailed.Broadcast(ExtractBackendError(Response));
		return;
	}

	FWUClubSummary Club;
	if (!TryParseClubSummary(RootObject, Club) || !Club.HasClub())
	{
		OnRequestFailed.Broadcast(TEXT("Create club response did not include a valid club."));
		return;
	}

	if (FWUBackendCharacterSummary* SelectedCharacter = FindMutableCachedCharacter(SelectedCharacterId))
	{
		SelectedCharacter->Club = Club;
		OnCharacterUpdated.Broadcast(*SelectedCharacter);
		OnCharactersLoaded.Broadcast(Characters);
	}

	OnClubCreated.Broadcast(Club);
	RefreshSelectedInventorySnapshot();
}

void UWUClientSessionSubsystem::HandleInviteClubMemberResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	TSharedPtr<FJsonObject> RootObject;
	FString ErrorMessage;
	if (!bSucceeded || !TryDeserializeObject(Response, RootObject, ErrorMessage))
	{
		OnRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()) && Response->GetResponseCode() != EHttpResponseCodes::Created)
	{
		OnRequestFailed.Broadcast(ExtractBackendError(Response));
		return;
	}

	FWUBackendClubInviteSummary Invite;
	if (!TryParseClubInvite(RootObject, Invite))
	{
		OnRequestFailed.Broadcast(TEXT("Club invite response did not include a valid invite."));
		return;
	}

	OnClubInviteCreated.Broadcast(Invite);
}

void UWUClientSessionSubsystem::HandleLoadClubRosterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	TSharedPtr<FJsonObject> RootObject;
	FString ErrorMessage;
	if (!bSucceeded || !TryDeserializeObject(Response, RootObject, ErrorMessage))
	{
		OnRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		OnRequestFailed.Broadcast(ExtractBackendError(Response));
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* MemberValues = nullptr;
	if (!RootObject->TryGetArrayField(TEXT("members"), MemberValues))
	{
		OnRequestFailed.Broadcast(TEXT("Club roster response did not include members."));
		return;
	}

	TArray<FWUClubMemberSummary> Members;
	for (const TSharedPtr<FJsonValue>& MemberValue : *MemberValues)
	{
		FWUClubMemberSummary Member;
		if (TryParseClubMember(MemberValue->AsObject(), Member))
		{
			Members.Add(Member);
		}
	}

	OnClubRosterLoaded.Broadcast(Members);
}

void UWUClientSessionSubsystem::HandleRefreshCurrencySnapshotResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	TSharedPtr<FJsonObject> RootObject;
	FString ErrorMessage;
	if (!bSucceeded || !TryDeserializeObject(Response, RootObject, ErrorMessage))
	{
		OnRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		OnRequestFailed.Broadcast(ExtractBackendError(Response));
		return;
	}

	FWUBackendCurrencySnapshot NewSnapshot;
	if (!TryParseCurrencySnapshot(RootObject, NewSnapshot))
	{
		OnRequestFailed.Broadcast(TEXT("Currency snapshot response did not include valid wallets."));
		return;
	}

	CurrencySnapshot = NewSnapshot;
	bHasCurrencySnapshot = true;
	OnCurrencySnapshotLoaded.Broadcast(CurrencySnapshot);
}

void UWUClientSessionSubsystem::HandleRefreshInventorySnapshotResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	TSharedPtr<FJsonObject> RootObject;
	FString ErrorMessage;
	if (!bSucceeded || !TryDeserializeObject(Response, RootObject, ErrorMessage))
	{
		OnRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		OnRequestFailed.Broadcast(ExtractBackendError(Response));
		return;
	}

	FWUBackendInventorySnapshot NewSnapshot;
	if (!TryParseInventorySnapshot(RootObject, NewSnapshot))
	{
		OnRequestFailed.Broadcast(TEXT("Inventory response did not include valid items."));
		return;
	}

	InventorySnapshot = NewSnapshot;
	bHasInventorySnapshot = true;
	OnInventorySnapshotLoaded.Broadcast(InventorySnapshot);
}

void UWUClientSessionSubsystem::HandlePurchaseVendorItemResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	TSharedPtr<FJsonObject> RootObject;
	FString ErrorMessage;
	if (!bSucceeded || !TryDeserializeObject(Response, RootObject, ErrorMessage))
	{
		OnRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		OnRequestFailed.Broadcast(ExtractBackendError(Response));
		return;
	}

	FWUBackendVendorPurchase Purchase;
	if (!TryParseVendorPurchase(RootObject, Purchase))
	{
		OnRequestFailed.Broadcast(TEXT("Vendor purchase response did not include a valid purchase."));
		return;
	}

	CurrencySnapshot = Purchase.Snapshot;
	bHasCurrencySnapshot = true;
	OnCurrencySnapshotLoaded.Broadcast(CurrencySnapshot);
	if (!Purchase.Inventory.CharacterId.IsEmpty())
	{
		InventorySnapshot = Purchase.Inventory;
		bHasInventorySnapshot = true;
		OnInventorySnapshotLoaded.Broadcast(InventorySnapshot);
	}
	OnVendorPurchaseCompleted.Broadcast(Purchase);
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> UWUClientSessionSubsystem::CreateRequest(const FString& Verb, const FString& Path) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(BuildUrl(Path));
	Request->SetVerb(Verb);
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	return Request;
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> UWUClientSessionSubsystem::CreateAuthorizedRequest(const FString& Verb, const FString& Path) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(Verb, Path);
	if (!AccessToken.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AccessToken));
	}

	return Request;
}

TSharedPtr<FJsonObject> UWUClientSessionSubsystem::CreateSessionContextJsonObject() const
{
	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("accountId"), Account.AccountId);
	RootObject->SetStringField(TEXT("realmId"), SelectedRealmId);
	return RootObject;
}

void UWUClientSessionSubsystem::SetJsonBody(const TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& Request, const TSharedPtr<FJsonObject>& JsonObject)
{
	Request->SetContentAsString(SerializeJsonObject(JsonObject));
}

FString UWUClientSessionSubsystem::SerializeJsonObject(const TSharedPtr<FJsonObject>& JsonObject)
{
	FString Body;
	if (JsonObject.IsValid())
	{
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	}

	return Body;
}

bool UWUClientSessionSubsystem::TryDeserializeObject(FHttpResponsePtr Response, TSharedPtr<FJsonObject>& OutJsonObject, FString& OutErrorMessage) const
{
	if (!Response.IsValid())
	{
		OutErrorMessage = TEXT("Backend did not return a response.");
		return false;
	}

	const FString Content = Response->GetContentAsString();
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
	if (!FJsonSerializer::Deserialize(Reader, OutJsonObject) || !OutJsonObject.IsValid())
	{
		if (Response->GetResponseCode() == EHttpResponseCodes::NotFound)
		{
			OutErrorMessage = TEXT("Backend endpoint was not found. Pull/rebuild the server and restart the API.");
			return false;
		}

		OutErrorMessage = FString::Printf(TEXT("Backend returned invalid JSON with HTTP %d."), Response->GetResponseCode());
		return false;
	}

	return true;
}

FString UWUClientSessionSubsystem::ExtractBackendError(FHttpResponsePtr Response) const
{
	if (!Response.IsValid())
	{
		return TEXT("Backend request failed.");
	}

	TSharedPtr<FJsonObject> ErrorObject;
	FString ErrorMessage;
	if (TryDeserializeObject(Response, ErrorObject, ErrorMessage))
	{
		FString ErrorCode;
		if (ErrorObject->TryGetStringField(TEXT("error"), ErrorCode)
			&& ErrorCode.Equals(TEXT("insufficient_funds"), ESearchCase::IgnoreCase))
		{
			return TEXT("You do not have enough money.");
		}

		FString Message;
		if (ErrorObject->TryGetStringField(TEXT("message"), Message) && !Message.IsEmpty())
		{
			return Message;
		}

		if (ErrorObject->TryGetStringField(TEXT("error"), Message) && !Message.IsEmpty())
		{
			return Message;
		}

		const TArray<TSharedPtr<FJsonValue>>* Messages = nullptr;
		if (ErrorObject->TryGetArrayField(TEXT("messages"), Messages) && Messages->Num() > 0)
		{
			TArray<FString> MessageStrings;
			for (const TSharedPtr<FJsonValue>& Value : *Messages)
			{
				MessageStrings.Add(Value->AsString());
			}

			return FString::Join(MessageStrings, TEXT(" "));
		}
	}

	return FString::Printf(TEXT("Backend request failed with HTTP %d."), Response->GetResponseCode());
}

FString UWUClientSessionSubsystem::BuildUrl(const FString& Path) const
{
	FString BaseUrl = BackendBaseUrl;
	BaseUrl.TrimEndInline();
	while (BaseUrl.EndsWith(TEXT("/")))
	{
		BaseUrl.LeftChopInline(1);
	}

	return Path.StartsWith(TEXT("/"))
		? FString::Printf(TEXT("%s%s"), *BaseUrl, *Path)
		: FString::Printf(TEXT("%s/%s"), *BaseUrl, *Path);
}

bool UWUClientSessionSubsystem::HasSessionContext() const
{
	return !AccessToken.IsEmpty() && !Account.AccountId.IsEmpty() && !SelectedRealmId.IsEmpty();
}

bool UWUClientSessionSubsystem::TryParseAccount(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendAccountSummary& OutAccount)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	return JsonObject->TryGetStringField(TEXT("accountId"), OutAccount.AccountId)
		&& JsonObject->TryGetStringField(TEXT("username"), OutAccount.Username)
		&& JsonObject->TryGetStringField(TEXT("displayName"), OutAccount.DisplayName);
}

bool UWUClientSessionSubsystem::TryParseRealm(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendRealmSummary& OutRealm)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	return JsonObject->TryGetStringField(TEXT("realmId"), OutRealm.RealmId)
		&& JsonObject->TryGetStringField(TEXT("slug"), OutRealm.Slug)
		&& JsonObject->TryGetStringField(TEXT("displayName"), OutRealm.DisplayName)
		&& JsonObject->TryGetStringField(TEXT("status"), OutRealm.Status);
}

bool UWUClientSessionSubsystem::TryParseCharacter(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendCharacterSummary& OutCharacter)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	FString RaceValue;
	FString SexValue;
	FString HouseValue;
	double LevelValue = 1.0;
	double ExperienceValue = 0.0;
	double ExperienceToNextLevelValue = static_cast<double>(WUCharacterStats::GetExperienceToNextLevel(1));
	if (!JsonObject->TryGetStringField(TEXT("characterId"), OutCharacter.CharacterId)
		|| !JsonObject->TryGetStringField(TEXT("name"), OutCharacter.Name)
		|| !JsonObject->TryGetStringField(TEXT("race"), RaceValue)
		|| !JsonObject->TryGetStringField(TEXT("sex"), SexValue)
		|| !JsonObject->TryGetNumberField(TEXT("level"), LevelValue))
	{
		return false;
	}

	if (!TryParseRace(RaceValue, OutCharacter.Race) || !TryParseSex(SexValue, OutCharacter.Sex))
	{
		return false;
	}

	if (JsonObject->TryGetStringField(TEXT("house"), HouseValue) || JsonObject->TryGetStringField(TEXT("houseFaction"), HouseValue))
	{
		if (!TryParseHouseFaction(HouseValue, OutCharacter.HouseFaction))
		{
			return false;
		}
	}
	else
	{
		OutCharacter.HouseFaction = EWUHouseFaction::Unsorted;
	}

	OutCharacter.Appearance.Sex = OutCharacter.Sex;
	OutCharacter.Level = FMath::Max(1, FMath::RoundToInt(LevelValue));
	JsonObject->TryGetNumberField(TEXT("experience"), ExperienceValue);
	OutCharacter.Experience = FMath::Max(0, FMath::RoundToInt(ExperienceValue));
	if (JsonObject->TryGetNumberField(TEXT("experienceToNextLevel"), ExperienceToNextLevelValue))
	{
		OutCharacter.ExperienceToNextLevel = FMath::Max(0, FMath::RoundToInt(ExperienceToNextLevelValue));
	}
	else
	{
		OutCharacter.ExperienceToNextLevel = WUCharacterStats::GetExperienceToNextLevel(OutCharacter.Level);
	}

	const FWUExperienceProgression NormalizedProgression = WUCharacterStats::ResolveExperienceAward(
		OutCharacter.Level,
		OutCharacter.Experience,
		0);
	OutCharacter.Level = NormalizedProgression.Level;
	OutCharacter.Experience = NormalizedProgression.Experience;
	OutCharacter.ExperienceToNextLevel = NormalizedProgression.ExperienceToNextLevel;
	OutCharacter.PrimaryStats = WUCharacterStats::CalculatePrimaryStats(OutCharacter.Race, OutCharacter.Level);
	OutCharacter.DerivedStats = WUCharacterStats::CalculateDerivedStats(OutCharacter.PrimaryStats);

	const TSharedPtr<FJsonObject>* AppearanceObject = nullptr;
	if (JsonObject->TryGetObjectField(TEXT("appearance"), AppearanceObject) && AppearanceObject && AppearanceObject->IsValid())
	{
		double SkinPresetIndex = 0.0;
		double HeadPresetIndex = 0.0;
		double HairStyleIndex = 0.0;
		double HairColorIndex = 0.0;
		double EyeColorIndex = 1.0;
		double BrowStyleIndex = 0.0;
		double BeardStyleIndex = 0.0;

		(*AppearanceObject)->TryGetNumberField(TEXT("skinPresetIndex"), SkinPresetIndex);
		(*AppearanceObject)->TryGetNumberField(TEXT("headPresetIndex"), HeadPresetIndex);
		(*AppearanceObject)->TryGetNumberField(TEXT("hairStyleIndex"), HairStyleIndex);
		(*AppearanceObject)->TryGetNumberField(TEXT("hairColorIndex"), HairColorIndex);
		(*AppearanceObject)->TryGetNumberField(TEXT("eyeColorIndex"), EyeColorIndex);
		(*AppearanceObject)->TryGetNumberField(TEXT("browStyleIndex"), BrowStyleIndex);
		(*AppearanceObject)->TryGetNumberField(TEXT("beardStyleIndex"), BeardStyleIndex);

		OutCharacter.Appearance.SkinPresetIndex = FMath::Max(0, FMath::RoundToInt(SkinPresetIndex));
		OutCharacter.Appearance.HeadPresetIndex = FMath::Max(0, FMath::RoundToInt(HeadPresetIndex));
		OutCharacter.Appearance.HairStyleIndex = FMath::Max(0, FMath::RoundToInt(HairStyleIndex));
		OutCharacter.Appearance.HairColorIndex = FMath::Max(0, FMath::RoundToInt(HairColorIndex));
		OutCharacter.Appearance.EyeColorIndex = FMath::Max(0, FMath::RoundToInt(EyeColorIndex));
		OutCharacter.Appearance.BrowStyleIndex = FMath::Max(0, FMath::RoundToInt(BrowStyleIndex));
		OutCharacter.Appearance.BeardStyleIndex = FMath::Max(0, FMath::RoundToInt(BeardStyleIndex));
	}

	const TSharedPtr<FJsonObject>* ClubObject = nullptr;
	OutCharacter.Club = FWUClubSummary();
	if (JsonObject->TryGetObjectField(TEXT("club"), ClubObject) && ClubObject && ClubObject->IsValid())
	{
		TryParseClubSummary(*ClubObject, OutCharacter.Club);
	}

	const TSharedPtr<FJsonObject>* LocationObject = nullptr;
	if (JsonObject->TryGetObjectField(TEXT("location"), LocationObject))
	{
		TryParseLocation(*LocationObject, OutCharacter.Location);
	}

	return true;
}

bool UWUClientSessionSubsystem::TryParseClubSummary(const TSharedPtr<FJsonObject>& JsonObject, FWUClubSummary& OutClub)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	FString RankValue;
	JsonObject->TryGetStringField(TEXT("clubId"), OutClub.ClubId);
	JsonObject->TryGetStringField(TEXT("name"), OutClub.Name);
	JsonObject->TryGetStringField(TEXT("tag"), OutClub.Tag);
	JsonObject->TryGetStringField(TEXT("publicNote"), OutClub.PublicNote);
	JsonObject->TryGetStringField(TEXT("officerNote"), OutClub.OfficerNote);

	double PermissionsMask = 0.0;
	if (JsonObject->TryGetNumberField(TEXT("permissionsMask"), PermissionsMask))
	{
		OutClub.PermissionsMask = FMath::Max(0, FMath::RoundToInt(PermissionsMask));
	}

	if (JsonObject->TryGetStringField(TEXT("rank"), RankValue))
	{
		return TryParseClubRank(RankValue, OutClub.Rank);
	}

	OutClub.Rank = OutClub.HasClub() ? EWUClubRank::Member : EWUClubRank::None;
	return true;
}

bool UWUClientSessionSubsystem::TryParseClubInvite(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendClubInviteSummary& OutInvite)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	FString CreatedAtValue;
	if (!JsonObject->TryGetStringField(TEXT("inviteId"), OutInvite.InviteId)
		|| !JsonObject->TryGetStringField(TEXT("clubId"), OutInvite.ClubId)
		|| !JsonObject->TryGetStringField(TEXT("invitedCharacterId"), OutInvite.InvitedCharacterId)
		|| !JsonObject->TryGetStringField(TEXT("status"), OutInvite.Status)
		|| !JsonObject->TryGetStringField(TEXT("createdAt"), CreatedAtValue)
		|| !TryParseDateTimeUtc(CreatedAtValue, OutInvite.CreatedAtUtc))
	{
		return false;
	}

	JsonObject->TryGetStringField(TEXT("invitedByCharacterId"), OutInvite.InvitedByCharacterId);

	FString ExpiresAtValue;
	if (JsonObject->TryGetStringField(TEXT("expiresAt"), ExpiresAtValue))
	{
		TryParseDateTimeUtc(ExpiresAtValue, OutInvite.ExpiresAtUtc);
	}

	return true;
}

bool UWUClientSessionSubsystem::TryParseClubMember(const TSharedPtr<FJsonObject>& JsonObject, FWUClubMemberSummary& OutMember)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	FString RankValue;
	FString HouseValue;
	double LevelValue = 1.0;
	if (!JsonObject->TryGetStringField(TEXT("characterId"), OutMember.CharacterId)
		|| !JsonObject->TryGetStringField(TEXT("rank"), RankValue)
		|| !JsonObject->TryGetNumberField(TEXT("level"), LevelValue))
	{
		return false;
	}

	if (!JsonObject->TryGetStringField(TEXT("name"), OutMember.DisplayName))
	{
		JsonObject->TryGetStringField(TEXT("displayName"), OutMember.DisplayName);
	}

	if (!TryParseClubRank(RankValue, OutMember.Rank))
	{
		return false;
	}

	if (JsonObject->TryGetStringField(TEXT("house"), HouseValue) || JsonObject->TryGetStringField(TEXT("houseFaction"), HouseValue))
	{
		TryParseHouseFaction(HouseValue, OutMember.HouseFaction);
	}

	OutMember.Level = FMath::Max(1, FMath::RoundToInt(LevelValue));
	JsonObject->TryGetStringField(TEXT("path"), OutMember.Path);
	JsonObject->TryGetStringField(TEXT("locationDisplayName"), OutMember.LocationDisplayName);
	JsonObject->TryGetBoolField(TEXT("isOnline"), OutMember.bIsOnline);
	JsonObject->TryGetStringField(TEXT("publicNote"), OutMember.PublicNote);
	JsonObject->TryGetStringField(TEXT("officerNote"), OutMember.OfficerNote);

	FString LastOnlineAtValue;
	if (JsonObject->TryGetStringField(TEXT("lastOnlineAt"), LastOnlineAtValue))
	{
		TryParseDateTimeUtc(LastOnlineAtValue, OutMember.LastOnlineUtc);
	}

	return !OutMember.CharacterId.IsEmpty() && !OutMember.DisplayName.IsEmpty();
}

bool UWUClientSessionSubsystem::TryParseVendorPurchase(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendVendorPurchase& OutPurchase)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	double PriceKnuts = 0.0;
	const TSharedPtr<FJsonObject>* SnapshotObject = nullptr;
	const TSharedPtr<FJsonObject>* InventoryObject = nullptr;
	if (!JsonObject->TryGetStringField(TEXT("vendorTableId"), OutPurchase.VendorTableId)
		|| !JsonObject->TryGetStringField(TEXT("itemId"), OutPurchase.ItemId)
		|| !JsonObject->TryGetStringField(TEXT("displayName"), OutPurchase.DisplayName)
		|| !JsonObject->TryGetNumberField(TEXT("priceKnuts"), PriceKnuts)
		|| !JsonObject->TryGetObjectField(TEXT("snapshot"), SnapshotObject)
		|| !TryParseCurrencySnapshot(*SnapshotObject, OutPurchase.Snapshot))
	{
		return false;
	}

	if (JsonObject->TryGetObjectField(TEXT("inventory"), InventoryObject))
	{
		TryParseInventorySnapshot(*InventoryObject, OutPurchase.Inventory);
	}

	OutPurchase.PriceKnuts = FMath::Max<int64>(0, FMath::RoundToInt64(PriceKnuts));
	return true;
}

bool UWUClientSessionSubsystem::TryParseInventorySnapshot(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendInventorySnapshot& OutSnapshot)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	if (!JsonObject->TryGetStringField(TEXT("characterId"), OutSnapshot.CharacterId))
	{
		return false;
	}

	OutSnapshot.Items.Reset();
	const TArray<TSharedPtr<FJsonValue>>* ItemValues = nullptr;
	if (!JsonObject->TryGetArrayField(TEXT("items"), ItemValues))
	{
		return false;
	}

	for (const TSharedPtr<FJsonValue>& ItemValue : *ItemValues)
	{
		const TSharedPtr<FJsonObject> ItemObject = ItemValue.IsValid() ? ItemValue->AsObject() : nullptr;
		FWUBackendInventoryItem Item;
		if (TryParseInventoryItem(ItemObject, Item))
		{
			OutSnapshot.Items.Add(Item);
		}
	}

	return true;
}

bool UWUClientSessionSubsystem::TryParseInventoryItem(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendInventoryItem& OutItem)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	double SlotIndex = 0.0;
	double Quantity = 1.0;
	if (!JsonObject->TryGetNumberField(TEXT("slotIndex"), SlotIndex)
		|| !JsonObject->TryGetStringField(TEXT("itemId"), OutItem.ItemId))
	{
		return false;
	}

	JsonObject->TryGetNumberField(TEXT("quantity"), Quantity);
	OutItem.SlotIndex = FMath::Max(0, FMath::RoundToInt(SlotIndex));
	OutItem.Quantity = FMath::Max(1, FMath::RoundToInt(Quantity));
	return !OutItem.ItemId.IsEmpty();
}

bool UWUClientSessionSubsystem::TryParseCurrencySnapshot(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendCurrencySnapshot& OutSnapshot)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* CharacterWalletObject = nullptr;
	const TSharedPtr<FJsonObject>* AccountBankWalletObject = nullptr;
	return JsonObject->TryGetObjectField(TEXT("characterWallet"), CharacterWalletObject)
		&& JsonObject->TryGetObjectField(TEXT("accountBankWallet"), AccountBankWalletObject)
		&& TryParseCurrencyWallet(*CharacterWalletObject, OutSnapshot.CharacterWallet)
		&& TryParseCurrencyWallet(*AccountBankWalletObject, OutSnapshot.AccountBankWallet);
}

bool UWUClientSessionSubsystem::TryParseCurrencyWallet(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendCurrencyWallet& OutWallet)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* BalanceObject = nullptr;
	return JsonObject->TryGetStringField(TEXT("walletId"), OutWallet.WalletId)
		&& JsonObject->TryGetStringField(TEXT("walletType"), OutWallet.WalletType)
		&& JsonObject->TryGetStringField(TEXT("ownerId"), OutWallet.OwnerId)
		&& JsonObject->TryGetObjectField(TEXT("balance"), BalanceObject)
		&& TryParseCurrencyBreakdown(*BalanceObject, OutWallet.Balance);
}

bool UWUClientSessionSubsystem::TryParseCurrencyBreakdown(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendCurrencyBreakdown& OutBalance)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	double BalanceKnuts = 0.0;
	double Galleons = 0.0;
	double Sickles = 0.0;
	double Knuts = 0.0;
	if (!JsonObject->TryGetNumberField(TEXT("balanceKnuts"), BalanceKnuts)
		|| !JsonObject->TryGetNumberField(TEXT("galleons"), Galleons)
		|| !JsonObject->TryGetNumberField(TEXT("sickles"), Sickles)
		|| !JsonObject->TryGetNumberField(TEXT("knuts"), Knuts))
	{
		return false;
	}

	OutBalance.BalanceKnuts = FMath::Max<int64>(0, FMath::RoundToInt64(BalanceKnuts));
	OutBalance.Galleons = FMath::Max<int64>(0, FMath::RoundToInt64(Galleons));
	OutBalance.Sickles = FMath::Max<int64>(0, FMath::RoundToInt64(Sickles));
	OutBalance.Knuts = FMath::Max<int64>(0, FMath::RoundToInt64(Knuts));
	return true;
}

bool UWUClientSessionSubsystem::TryParseLocation(const TSharedPtr<FJsonObject>& JsonObject, FWUBackendCharacterLocation& OutLocation)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	double X = 0.0;
	double Y = 0.0;
	double Z = 0.0;
	if (!JsonObject->TryGetNumberField(TEXT("x"), X)
		|| !JsonObject->TryGetNumberField(TEXT("y"), Y)
		|| !JsonObject->TryGetNumberField(TEXT("z"), Z))
	{
		return false;
	}

	OutLocation.X = static_cast<float>(X);
	OutLocation.Y = static_cast<float>(Y);
	OutLocation.Z = static_cast<float>(Z);
	return true;
}

bool UWUClientSessionSubsystem::TryParseRace(const FString& Value, EWUCharacterRace& OutRace)
{
	if (Value.Equals(TEXT("Halfblood"), ESearchCase::IgnoreCase))
	{
		OutRace = EWUCharacterRace::Halfblood;
		return true;
	}

	if (Value.Equals(TEXT("Pureblood"), ESearchCase::IgnoreCase))
	{
		OutRace = EWUCharacterRace::Pureblood;
		return true;
	}

	if (Value.Equals(TEXT("Mudblood"), ESearchCase::IgnoreCase))
	{
		OutRace = EWUCharacterRace::Mudblood;
		return true;
	}

	return false;
}

bool UWUClientSessionSubsystem::TryParseSex(const FString& Value, EWUCharacterSex& OutSex)
{
	if (Value.Equals(TEXT("Male"), ESearchCase::IgnoreCase))
	{
		OutSex = EWUCharacterSex::Male;
		return true;
	}

	if (Value.Equals(TEXT("Female"), ESearchCase::IgnoreCase))
	{
		OutSex = EWUCharacterSex::Female;
		return true;
	}

	return false;
}

bool UWUClientSessionSubsystem::TryParseHouseFaction(const FString& Value, EWUHouseFaction& OutHouseFaction)
{
	return WUIdentity::TryParseHouseFaction(Value, OutHouseFaction);
}

bool UWUClientSessionSubsystem::TryParseClubRank(const FString& Value, EWUClubRank& OutRank)
{
	return WUIdentity::TryParseClubRank(Value, OutRank);
}

bool UWUClientSessionSubsystem::TryParseDateTimeUtc(const FString& Value, FDateTime& OutDateTime)
{
	if (Value.IsEmpty())
	{
		return false;
	}

	return FDateTime::ParseIso8601(*Value, OutDateTime);
}

FString UWUClientSessionSubsystem::ExperienceSourceToString(EWUExperienceSource Source)
{
	switch (Source)
	{
	case EWUExperienceSource::Exploration:
		return TEXT("Exploration");

	case EWUExperienceSource::QuestTurnIn:
		return TEXT("QuestTurnIn");

	case EWUExperienceSource::Kill:
		return TEXT("Kill");

	default:
		return TEXT("Kill");
	}
}

FWUBackendCharacterSummary* UWUClientSessionSubsystem::FindMutableCachedCharacter(const FString& CharacterId)
{
	for (FWUBackendCharacterSummary& Character : Characters)
	{
		if (Character.CharacterId == CharacterId)
		{
			return &Character;
		}
	}

	return nullptr;
}

const FWUBackendCharacterSummary* UWUClientSessionSubsystem::FindCachedCharacter(const FString& CharacterId) const
{
	for (const FWUBackendCharacterSummary& Character : Characters)
	{
		if (Character.CharacterId == CharacterId)
		{
			return &Character;
		}
	}

	return nullptr;
}

bool UWUClientSessionSubsystem::UpdateCachedCharacter(const FWUBackendCharacterSummary& UpdatedCharacter)
{
	for (FWUBackendCharacterSummary& Character : Characters)
	{
		if (Character.CharacterId == UpdatedCharacter.CharacterId)
		{
			Character = UpdatedCharacter;
			return true;
		}
	}

	return false;
}
