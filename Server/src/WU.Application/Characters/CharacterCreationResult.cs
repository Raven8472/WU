namespace WU.Application.Characters;

public sealed record CharacterCreationResult(
    CharacterCreationStatus Status,
    CharacterSummary? Character,
    IReadOnlyList<string> Errors)
{
    public static CharacterCreationResult Created(CharacterSummary character)
    {
        return new CharacterCreationResult(CharacterCreationStatus.Created, character, []);
    }

    public static CharacterCreationResult DuplicateName()
    {
        return new CharacterCreationResult(CharacterCreationStatus.DuplicateName, null, []);
    }

    public static CharacterCreationResult Invalid(params string[] errors)
    {
        return new CharacterCreationResult(CharacterCreationStatus.InvalidRequest, null, errors);
    }
}
