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

	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("accountId"), Account.AccountId);
	RootObject->SetStringField(TEXT("realmId"), SelectedRealmId);
	RootObject->SetStringField(TEXT("name"), RequestData.CharacterName);
	RootObject->SetStringField(TEXT("race"), WUCharacterCreation::RaceToString(RequestData.Race));
	RootObject->SetStringField(TEXT("sex"), WUCharacterCreation::SexToString(RequestData.Sex));

	TSharedPtr<FJsonObject> AppearanceObject = MakeShared<FJsonObject>();
	AppearanceObject->SetNumberField(TEXT("skinPresetIndex"), RequestData.SkinPresetIndex);
	AppearanceObject->SetNumberField(TEXT("headPresetIndex"), RequestData.HeadPresetIndex);
	AppearanceObject->SetNumberField(TEXT("hairStyleIndex"), RequestData.HairStyleIndex);
	AppearanceObject->SetNumberField(TEXT("hairColorIndex"), RequestData.HairColorIndex);
	AppearanceObject->SetNumberField(TEXT("browStyleIndex"), RequestData.BrowStyleIndex);
	AppearanceObject->SetNumberField(TEXT("beardStyleIndex"), RequestData.BeardStyleIndex);
	RootObject->SetObjectField(TEXT("appearance"), AppearanceObject);

	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("POST"), TEXT("/api/characters"));
	Request->SetContentAsString(Body);
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleCreateCharacterResponse);
	Request->ProcessRequest();
}

void UWUClientSessionSubsystem::SelectCharacter(const FString& CharacterId)
{
	SelectedCharacterId = CharacterId;
}

void UWUClientSessionSubsystem::SaveSelectedCharacterLocation(const FVector& Location)
{
	if (!HasSessionContext() || SelectedCharacterId.IsEmpty())
	{
		return;
	}

	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("accountId"), Account.AccountId);
	RootObject->SetStringField(TEXT("realmId"), SelectedRealmId);

	TSharedPtr<FJsonObject> LocationObject = MakeShared<FJsonObject>();
	LocationObject->SetNumberField(TEXT("x"), Location.X);
	LocationObject->SetNumberField(TEXT("y"), Location.Y);
	LocationObject->SetNumberField(TEXT("z"), Location.Z);
	RootObject->SetObjectField(TEXT("location"), LocationObject);

	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	const FString Path = FString::Printf(TEXT("/api/characters/%s/location"), *SelectedCharacterId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateAuthorizedRequest(TEXT("PUT"), Path);
	Request->SetContentAsString(Body);
	Request->OnProcessRequestComplete().BindUObject(this, &UWUClientSessionSubsystem::HandleSaveCharacterLocationResponse);
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

	for (FWUBackendCharacterSummary& Character : Characters)
	{
		if (Character.CharacterId == UpdatedCharacter.CharacterId)
		{
			Character = UpdatedCharacter;
			return;
		}
	}
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
	double LevelValue = 1.0;
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

	OutCharacter.Level = FMath::Max(1, FMath::RoundToInt(LevelValue));
	OutCharacter.PrimaryStats = WUCharacterStats::CalculatePrimaryStats(OutCharacter.Race, OutCharacter.Level);
	OutCharacter.DerivedStats = WUCharacterStats::CalculateDerivedStats(OutCharacter.PrimaryStats);

	const TSharedPtr<FJsonObject>* LocationObject = nullptr;
	if (JsonObject->TryGetObjectField(TEXT("location"), LocationObject))
	{
		TryParseLocation(*LocationObject, OutCharacter.Location);
	}

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
