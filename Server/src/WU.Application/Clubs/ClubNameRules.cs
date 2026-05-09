namespace WU.Application.Clubs;

public static class ClubNameRules
{
    public const int MinNameLength = 3;
    public const int MaxNameLength = 32;
    public const int MinTagLength = 2;
    public const int MaxTagLength = 6;
    public const int MaxDescriptionLength = 512;

    public static string NormalizeName(string value)
    {
        return value.Trim().ToLowerInvariant();
    }

    public static string NormalizeTag(string? value)
    {
        return (value ?? string.Empty).Trim().ToLowerInvariant();
    }

    public static bool IsValidName(string value)
    {
        var trimmed = value.Trim();
        return trimmed.Length is >= MinNameLength and <= MaxNameLength;
    }

    public static bool IsValidTag(string? value)
    {
        var trimmed = (value ?? string.Empty).Trim();
        return trimmed.Length == 0 || trimmed.Length is >= MinTagLength and <= MaxTagLength;
    }
}
