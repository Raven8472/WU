// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterStats/WUCharacterStats.h"

namespace WUCharacterStats
{
	int32 ClampCharacterLevel(int32 Level)
	{
		return FMath::Clamp(Level, MinLevel, MaxLevel);
	}

	int32 GetHumanBaselineStatForLevel(int32 Level)
	{
		return HumanBaselineStat + (ClampCharacterLevel(Level) - MinLevel);
	}

	int32 GetExperienceToNextLevel(int32 CurrentLevel)
	{
		const int32 Level = ClampCharacterLevel(CurrentLevel);
		if (Level >= MaxLevel)
		{
			return 0;
		}

		return BaseExperienceToNextLevel + (Level * Level * ExperienceQuadraticFactor);
	}

	int32 GetTotalExperienceToReachLevel(int32 TargetLevel)
	{
		const int32 ClampedTargetLevel = ClampCharacterLevel(TargetLevel);
		int32 TotalExperience = 0;

		for (int32 Level = MinLevel; Level < ClampedTargetLevel; ++Level)
		{
			TotalExperience += GetExperienceToNextLevel(Level);
		}

		return TotalExperience;
	}

	FWUExperienceProgression ResolveExperienceAward(int32 CurrentLevel, int32 CurrentExperience, int32 AwardedExperience)
	{
		FWUExperienceProgression Progression;
		Progression.Level = ClampCharacterLevel(CurrentLevel);
		int64 RemainingExperience = FMath::Max<int64>(0, static_cast<int64>(CurrentExperience))
			+ FMath::Max<int64>(0, static_cast<int64>(AwardedExperience));

		while (Progression.Level < MaxLevel)
		{
			Progression.ExperienceToNextLevel = GetExperienceToNextLevel(Progression.Level);
			if (RemainingExperience < Progression.ExperienceToNextLevel)
			{
				Progression.Experience = static_cast<int32>(RemainingExperience);
				return Progression;
			}

			RemainingExperience -= Progression.ExperienceToNextLevel;
			Progression.Level++;
			Progression.LevelsGained++;
		}

		Progression.Level = MaxLevel;
		Progression.Experience = 0;
		Progression.ExperienceToNextLevel = 0;
		Progression.bReachedLevelCap = true;
		return Progression;
	}

	FWUPrimaryStats CalculatePrimaryStats(EWUCharacterRace BloodStatus, int32 Level)
	{
		const int32 Baseline = GetHumanBaselineStatForLevel(Level);

		FWUPrimaryStats Stats;
		Stats.Nerve = Baseline;
		Stats.Ambition = Baseline;
		Stats.Wit = Baseline;
		Stats.Patience = Baseline;
		Stats.Cunning = Baseline;
		Stats.Knowledge = Baseline;

		switch (BloodStatus)
		{
		case EWUCharacterRace::Pureblood:
			Stats.Ambition += 2;
			Stats.Patience -= 1;
			Stats.Cunning += 1;
			break;

		case EWUCharacterRace::Halfblood:
			Stats.Ambition -= 1;
			Stats.Wit += 2;
			Stats.Patience += 1;
			break;

		case EWUCharacterRace::Mudblood:
			Stats.Nerve += 1;
			Stats.Patience += 2;
			Stats.Cunning -= 1;
			break;

		default:
			break;
		}

		return Stats;
	}

	FWUDerivedStats CalculateDerivedStats(const FWUPrimaryStats& PrimaryStats)
	{
		FWUDerivedStats Stats;
		Stats.MaxHealth = static_cast<float>(PrimaryStats.Nerve) * 10.0f;
		Stats.MaxMagic = static_cast<float>(PrimaryStats.Wit) * 15.0f;
		Stats.HealthRegenOutOfCombatPerSecond = Stats.MaxHealth * (static_cast<float>(PrimaryStats.Patience) * 0.0005f);
		Stats.HealthRegenInCombatPerSecond = Stats.MaxHealth * (static_cast<float>(PrimaryStats.Patience) * 0.0001f);
		Stats.MagicRegenOutOfCombatPerSecond = static_cast<float>(PrimaryStats.Knowledge) * 0.25f;
		Stats.MagicRegenInCombatPerSecond = static_cast<float>(PrimaryStats.Knowledge) * 0.0833f;
		Stats.CriticalChancePercent = static_cast<float>(PrimaryStats.Ambition) * 0.05f;
		Stats.CriticalDamageMultiplier = 1.5f;
		Stats.SpellPowerPercent = static_cast<float>(PrimaryStats.Wit) * 0.2f;
		Stats.MagicCostReductionPercent = static_cast<float>(PrimaryStats.Cunning) * 0.1f;
		return Stats;
	}

	FWUDerivedStats CalculateDerivedStats(EWUCharacterRace BloodStatus, int32 Level)
	{
		return CalculateDerivedStats(CalculatePrimaryStats(BloodStatus, Level));
	}
}
