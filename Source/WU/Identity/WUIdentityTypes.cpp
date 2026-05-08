// Copyright Epic Games, Inc. All Rights Reserved.

#include "Identity/WUIdentityTypes.h"

bool FWUClubSummary::HasClub() const
{
	return !ClubId.IsEmpty();
}

bool FWUClubSummary::HasPermission(EWUClubPermission Permission) const
{
	return WUIdentity::HasPermission(PermissionsMask, Permission);
}

bool FWUClubRankPermissions::HasPermission(EWUClubPermission Permission) const
{
	return WUIdentity::HasPermission(PermissionsMask, Permission);
}

namespace WUIdentity
{
	FString HouseFactionToString(EWUHouseFaction HouseFaction)
	{
		switch (HouseFaction)
		{
		case EWUHouseFaction::Unsorted:
			return TEXT("Unsorted");
		case EWUHouseFaction::Gryffindor:
			return TEXT("Gryffindor");
		case EWUHouseFaction::Hufflepuff:
			return TEXT("Hufflepuff");
		case EWUHouseFaction::Ravenclaw:
			return TEXT("Ravenclaw");
		case EWUHouseFaction::Slytherin:
			return TEXT("Slytherin");
		default:
			return TEXT("Unsorted");
		}
	}

	bool TryParseHouseFaction(const FString& Value, EWUHouseFaction& OutHouseFaction)
	{
		if (Value.Equals(TEXT("Unsorted"), ESearchCase::IgnoreCase))
		{
			OutHouseFaction = EWUHouseFaction::Unsorted;
			return true;
		}

		if (Value.Equals(TEXT("Gryffindor"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("Griffindor"), ESearchCase::IgnoreCase))
		{
			OutHouseFaction = EWUHouseFaction::Gryffindor;
			return true;
		}

		if (Value.Equals(TEXT("Hufflepuff"), ESearchCase::IgnoreCase))
		{
			OutHouseFaction = EWUHouseFaction::Hufflepuff;
			return true;
		}

		if (Value.Equals(TEXT("Ravenclaw"), ESearchCase::IgnoreCase))
		{
			OutHouseFaction = EWUHouseFaction::Ravenclaw;
			return true;
		}

		if (Value.Equals(TEXT("Slytherin"), ESearchCase::IgnoreCase))
		{
			OutHouseFaction = EWUHouseFaction::Slytherin;
			return true;
		}

		return false;
	}

	FString ClubRankToString(EWUClubRank Rank)
	{
		switch (Rank)
		{
		case EWUClubRank::None:
			return TEXT("None");
		case EWUClubRank::Recruit:
			return TEXT("Recruit");
		case EWUClubRank::Member:
			return TEXT("Member");
		case EWUClubRank::Officer:
			return TEXT("Officer");
		case EWUClubRank::President:
			return TEXT("President");
		default:
			return TEXT("None");
		}
	}

	bool TryParseClubRank(const FString& Value, EWUClubRank& OutRank)
	{
		if (Value.Equals(TEXT("None"), ESearchCase::IgnoreCase))
		{
			OutRank = EWUClubRank::None;
			return true;
		}

		if (Value.Equals(TEXT("Recruit"), ESearchCase::IgnoreCase))
		{
			OutRank = EWUClubRank::Recruit;
			return true;
		}

		if (Value.Equals(TEXT("Member"), ESearchCase::IgnoreCase))
		{
			OutRank = EWUClubRank::Member;
			return true;
		}

		if (Value.Equals(TEXT("Officer"), ESearchCase::IgnoreCase))
		{
			OutRank = EWUClubRank::Officer;
			return true;
		}

		if (Value.Equals(TEXT("President"), ESearchCase::IgnoreCase))
		{
			OutRank = EWUClubRank::President;
			return true;
		}

		return false;
	}

	int32 PermissionMask(EWUClubPermission Permission)
	{
		return static_cast<int32>(Permission);
	}

	bool HasPermission(int32 PermissionsMask, EWUClubPermission Permission)
	{
		const int32 RequestedMask = PermissionMask(Permission);
		return RequestedMask != 0 && (PermissionsMask & RequestedMask) == RequestedMask;
	}
}
