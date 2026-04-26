namespace WU.Application.Characters;

public sealed class DuplicateCharacterNameException : Exception
{
    public DuplicateCharacterNameException(string normalizedName)
        : base($"Character name '{normalizedName}' is already taken.")
    {
    }
}
