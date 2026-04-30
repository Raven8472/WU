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

public static class CharacterStatRules
{
    public const int MinLevel = 1;
    public const int MaxLevel = 80;
    public const int HumanBaselineStat = 10;

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
}
