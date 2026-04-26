// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WUCharacterCreationTypes.generated.h"

UENUM(BlueprintType)
enum class EWUCharacterRace : uint8
{
	Halfblood,
	Pureblood,
	Mudblood
};

UENUM(BlueprintType)
enum class EWUCharacterSex : uint8
{
	Male,
	Female
};

USTRUCT(BlueprintType)
struct FWUCharacterCreateRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Creation")
	FString CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Creation")
	EWUCharacterRace Race = EWUCharacterRace::Halfblood;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Creation")
	EWUCharacterSex Sex = EWUCharacterSex::Male;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Creation")
	int32 SkinPresetIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Creation")
	int32 HeadPresetIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Creation")
	int32 HairStyleIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Creation")
	int32 HairColorIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Creation")
	int32 BrowStyleIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Creation")
	int32 BeardStyleIndex = 0;
};

namespace WUCharacterCreation
{
	FString RaceToString(EWUCharacterRace Race);
	FString SexToString(EWUCharacterSex Sex);
}
