namespace WU.Application.Characters;

public sealed record CharacterListResult(
    CharacterListStatus Status,
    IReadOnlyList<CharacterSummary> Characters,
    IReadOnlyList<string> Errors)
{
    public static CharacterListResult Found(IReadOnlyList<CharacterSummary> characters)
    {
        return new CharacterListResult(CharacterListStatus.Found, characters, []);
    }

    public static CharacterListResult Invalid(params string[] errors)
    {
        return new CharacterListResult(CharacterListStatus.InvalidRequest, [], errors);
    }
}
