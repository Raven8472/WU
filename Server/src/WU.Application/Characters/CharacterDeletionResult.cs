namespace WU.Application.Characters;

public sealed record CharacterDeletionResult(
    CharacterDeletionStatus Status,
    IReadOnlyList<string> Errors)
{
    public static CharacterDeletionResult Deleted()
    {
        return new CharacterDeletionResult(CharacterDeletionStatus.Deleted, []);
    }

    public static CharacterDeletionResult NotFound()
    {
        return new CharacterDeletionResult(CharacterDeletionStatus.NotFound, []);
    }

    public static CharacterDeletionResult Invalid(params string[] errors)
    {
        return new CharacterDeletionResult(CharacterDeletionStatus.InvalidRequest, errors);
    }
}
