namespace WU.Domain.Characters;

public sealed record CharacterPrimaryStats(
    int Nerve,
    int Ambition,
    int Wit,
    int Patience,
    int Cunning,
    int Knowledge);

public sealed record CharacterDerivedStats(
    float MaxHealth,
    float MaxMagic,
    float HealthRegenOutOfCombatPerSecond,
    float HealthRegenInCombatPerSecond,
    float MagicRegenOutOfCombatPerSecond,
    float MagicRegenInCombatPerSecond,
    float CriticalChancePercent,
    float CriticalDamageMultiplier,
    float SpellPowerPercent,
    float MagicCostReductionPercent);

public sealed record CharacterStats(
    CharacterPrimaryStats Primary,
    CharacterDerivedStats Derived);

public sealed record CharacterExperienceProgression(
    int Level,
    int Experience,
    int ExperienceToNextLevel,
    int LevelsGained,
    bool ReachedLevelCap);

public enum CharacterExperienceSource
{
    Exploration,
    QuestTurnIn,
    Kill
}

public static class CharacterStatRules
{
    public const int MinLevel = 1;
    public const int MaxLevel = 80;
    public const int HumanBaselineStat = 10;
    public const int BaseExperienceToNextLevel = 500;
    public const int ExperienceQuadraticFactor = 55;

    public static CharacterStats Calculate(EWuCharacterRace bloodStatus, int level)
    {
        var primary = CalculatePrimaryStats(bloodStatus, level);
        return new CharacterStats(primary, CalculateDerivedStats(primary));
    }

    public static CharacterPrimaryStats CalculatePrimaryStats(EWuCharacterRace bloodStatus, int level)
    {
        var baseline = GetHumanBaselineStatForLevel(level);
        var nerve = baseline;
        var ambition = baseline;
        var wit = baseline;
        var patience = baseline;
        var cunning = baseline;
        var knowledge = baseline;

        switch (bloodStatus)
        {
            case EWuCharacterRace.Pureblood:
                ambition += 2;
                patience -= 1;
                cunning += 1;
                break;
            case EWuCharacterRace.Halfblood:
                ambition -= 1;
                wit += 2;
                patience += 1;
                break;
            case EWuCharacterRace.Mudblood:
                nerve += 1;
                patience += 2;
                cunning -= 1;
                break;
        }

        return new CharacterPrimaryStats(nerve, ambition, wit, patience, cunning, knowledge);
    }

    public static CharacterDerivedStats CalculateDerivedStats(CharacterPrimaryStats primary)
    {
        var maxHealth = primary.Nerve * 10.0f;
        var maxMagic = primary.Wit * 15.0f;

        return new CharacterDerivedStats(
            MaxHealth: maxHealth,
            MaxMagic: maxMagic,
            HealthRegenOutOfCombatPerSecond: maxHealth * (primary.Patience * 0.0005f),
            HealthRegenInCombatPerSecond: maxHealth * (primary.Patience * 0.0001f),
            MagicRegenOutOfCombatPerSecond: primary.Knowledge * 0.25f,
            MagicRegenInCombatPerSecond: primary.Knowledge * 0.0833f,
            CriticalChancePercent: primary.Ambition * 0.05f,
            CriticalDamageMultiplier: 1.5f,
            SpellPowerPercent: primary.Wit * 0.2f,
            MagicCostReductionPercent: primary.Cunning * 0.1f);
    }

    public static int ClampCharacterLevel(int level)
    {
        return Math.Clamp(level, MinLevel, MaxLevel);
    }

    public static int GetHumanBaselineStatForLevel(int level)
    {
        return HumanBaselineStat + (ClampCharacterLevel(level) - MinLevel);
    }

    public static int GetExperienceToNextLevel(int currentLevel)
    {
        var level = ClampCharacterLevel(currentLevel);
        if (level >= MaxLevel)
        {
            return 0;
        }

        return checked(BaseExperienceToNextLevel + (level * level * ExperienceQuadraticFactor));
    }

    public static int GetTotalExperienceToReachLevel(int targetLevel)
    {
        var clampedTargetLevel = ClampCharacterLevel(targetLevel);
        var totalExperience = 0;

        for (var level = MinLevel; level < clampedTargetLevel; level++)
        {
            totalExperience = checked(totalExperience + GetExperienceToNextLevel(level));
        }

        return totalExperience;
    }

    public static CharacterExperienceProgression ResolveExperienceAward(int currentLevel, int currentExperience, int awardedExperience)
    {
        var level = ClampCharacterLevel(currentLevel);
        long remainingExperience = Math.Max(0L, currentExperience) + Math.Max(0L, awardedExperience);
        var levelsGained = 0;

        while (level < MaxLevel)
        {
            var experienceToNextLevel = GetExperienceToNextLevel(level);
            if (remainingExperience < experienceToNextLevel)
            {
                return new CharacterExperienceProgression(
                    Level: level,
                    Experience: (int)remainingExperience,
                    ExperienceToNextLevel: experienceToNextLevel,
                    LevelsGained: levelsGained,
                    ReachedLevelCap: false);
            }

            remainingExperience -= experienceToNextLevel;
            level++;
            levelsGained++;
        }

        return new CharacterExperienceProgression(
            Level: MaxLevel,
            Experience: 0,
            ExperienceToNextLevel: 0,
            LevelsGained: levelsGained,
            ReachedLevelCap: true);
    }
}
