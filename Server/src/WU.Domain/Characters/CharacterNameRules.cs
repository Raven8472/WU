using System.Text.RegularExpressions;

namespace WU.Domain.Characters;

public static partial class CharacterNameRules
{
    public const int MinLength = 3;
    public const int MaxLength = 16;
    public const string AllowedPattern = "^[A-Za-z]+$";

    public static bool IsValid(string name)
    {
        return name.Length is >= MinLength and <= MaxLength && CharacterNameRegex().IsMatch(name);
    }

    public static string Normalize(string name)
    {
        return name.Trim().ToLowerInvariant();
    }

    [GeneratedRegex(AllowedPattern)]
    private static partial Regex CharacterNameRegex();
}
