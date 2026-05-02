// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"
#include "WUCharacterStats.generated.h"

USTRUCT(BlueprintType)
struct FWUPrimaryStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	int32 Nerve = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	int32 Ambition = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	int32 Wit = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	int32 Patience = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	int32 Cunning = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	int32 Knowledge = 10;
};

USTRUCT(BlueprintType)
struct FWUDerivedStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	float MaxMagic = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	float HealthRegenOutOfCombatPerSecond = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	float HealthRegenInCombatPerSecond = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	float MagicRegenOutOfCombatPerSecond = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	float MagicRegenInCombatPerSecond = 0.833f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	float CriticalChancePercent = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	float CriticalDamageMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	float SpellPowerPercent = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	float MagicCostReductionPercent = 1.0f;
};

UENUM(BlueprintType)
enum class EWUExperienceSource : uint8
{
	Exploration,
	QuestTurnIn,
	Kill
};

USTRUCT(BlueprintType)
struct FWUExperienceProgression
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	int32 Experience = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	int32 ExperienceToNextLevel = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	int32 LevelsGained = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|Stats")
	bool bReachedLevelCap = false;
};

namespace WUCharacterStats
{
	constexpr int32 MinLevel = 1;
	constexpr int32 MaxLevel = 80;
	constexpr int32 HumanBaselineStat = 10;
	constexpr int32 BaseExperienceToNextLevel = 500;
	constexpr int32 ExperienceQuadraticFactor = 55;

	int32 ClampCharacterLevel(int32 Level);
	int32 GetHumanBaselineStatForLevel(int32 Level);
	int32 GetExperienceToNextLevel(int32 CurrentLevel);
	int32 GetTotalExperienceToReachLevel(int32 TargetLevel);
	FWUExperienceProgression ResolveExperienceAward(int32 CurrentLevel, int32 CurrentExperience, int32 AwardedExperience);
	FWUPrimaryStats CalculatePrimaryStats(EWUCharacterRace BloodStatus, int32 Level);
	FWUDerivedStats CalculateDerivedStats(const FWUPrimaryStats& PrimaryStats);
	FWUDerivedStats CalculateDerivedStats(EWUCharacterRace BloodStatus, int32 Level);
}
