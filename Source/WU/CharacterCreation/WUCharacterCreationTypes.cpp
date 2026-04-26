// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterCreation/WUCharacterCreationTypes.h"

namespace WUCharacterCreation
{
	FString RaceToString(EWUCharacterRace Race)
	{
		switch (Race)
		{
		case EWUCharacterRace::Halfblood:
			return TEXT("Halfblood");
		case EWUCharacterRace::Pureblood:
			return TEXT("Pureblood");
		case EWUCharacterRace::Mudblood:
			return TEXT("Mudblood");
		default:
			return TEXT("Unknown");
		}
	}

	FString SexToString(EWUCharacterSex Sex)
	{
		switch (Sex)
		{
		case EWUCharacterSex::Male:
			return TEXT("Male");
		case EWUCharacterSex::Female:
			return TEXT("Female");
		default:
			return TEXT("Unknown");
		}
	}
}
