namespace WU.Application.Characters;

public sealed record CharacterExperienceResult(
    CharacterExperienceStatus Status,
    CharacterSummary? Character,
    IReadOnlyList<string> Errors)
{
    public static CharacterExperienceResult Awarded(CharacterSummary character)
    {
        return new CharacterExperienceResult(CharacterExperienceStatus.Awarded, character, []);
    }

    public static CharacterExperienceResult NotFound()
    {
        return new CharacterExperienceResult(CharacterExperienceStatus.NotFound, null, []);
    }

    public static CharacterExperienceResult Invalid(params string[] errors)
    {
        return new CharacterExperienceResult(CharacterExperienceStatus.InvalidRequest, null, errors);
    }
}
