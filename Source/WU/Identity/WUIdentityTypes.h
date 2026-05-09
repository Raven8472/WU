// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WUIdentityTypes.generated.h"

UENUM(BlueprintType)
enum class EWUHouseFaction : uint8
{
	Unsorted,
	Gryffindor,
	Hufflepuff,
	Ravenclaw,
	Slytherin
};

UENUM(BlueprintType)
enum class EWUClubRank : uint8
{
	None,
	Recruit,
	Member,
	Officer,
	President
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EWUClubPermission : uint8
{
	None = 0 UMETA(Hidden),
	Invite = 1 << 0,
	Uninvite = 1 << 1,
	Kick = 1 << 2,
	Promote = 1 << 3,
	Demote = 1 << 4,
	EditPublicNote = 1 << 5,
	EditOfficerNote = 1 << 6,
	ManagePreferences = 1 << 7
};
ENUM_CLASS_FLAGS(EWUClubPermission);

USTRUCT(BlueprintType)
struct FWUClubSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString ClubId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString Tag;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	EWUClubRank Rank = EWUClubRank::None;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club", meta = (Bitmask, BitmaskEnum = "/Script/WU.EWUClubPermission"))
	int32 PermissionsMask = 0;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString PublicNote;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString OfficerNote;

	bool HasClub() const;
	bool HasPermission(EWUClubPermission Permission) const;
};

USTRUCT(BlueprintType)
struct FWUClubMemberSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString CharacterId;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	EWUClubRank Rank = EWUClubRank::Member;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	int32 Level = 1;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString Path;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	EWUHouseFaction HouseFaction = EWUHouseFaction::Unsorted;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString LocationDisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	bool bIsOnline = false;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FDateTime LastOnlineUtc;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString PublicNote;

	UPROPERTY(BlueprintReadOnly, Category = "WU|Club")
	FString OfficerNote;
};

USTRUCT(BlueprintType)
struct FWUClubRankPermissions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Club")
	EWUClubRank Rank = EWUClubRank::Member;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Club", meta = (Bitmask, BitmaskEnum = "/Script/WU.EWUClubPermission"))
	int32 PermissionsMask = 0;

	bool HasPermission(EWUClubPermission Permission) const;
};

namespace WUIdentity
{
	FString HouseFactionToString(EWUHouseFaction HouseFaction);
	bool TryParseHouseFaction(const FString& Value, EWUHouseFaction& OutHouseFaction);
	FString ClubRankToString(EWUClubRank Rank);
	bool TryParseClubRank(const FString& Value, EWUClubRank& OutRank);
	int32 PermissionMask(EWUClubPermission Permission);
	bool HasPermission(int32 PermissionsMask, EWUClubPermission Permission);
}
