namespace WU.Application.Characters;

public sealed record CharacterLocationResult(
    CharacterLocationStatus Status,
    CharacterSummary? Character,
    IReadOnlyList<string> Errors)
{
    public static CharacterLocationResult Updated(CharacterSummary character)
    {
        return new CharacterLocationResult(CharacterLocationStatus.Updated, character, []);
    }

    public static CharacterLocationResult NotFound()
    {
        return new CharacterLocationResult(CharacterLocationStatus.NotFound, null, []);
    }

    public static CharacterLocationResult Invalid(params string[] errors)
    {
        return new CharacterLocationResult(CharacterLocationStatus.InvalidRequest, null, errors);
    }
}
