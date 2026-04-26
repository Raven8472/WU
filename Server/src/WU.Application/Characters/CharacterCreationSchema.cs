namespace WU.Application.Characters;

public sealed record CharacterCreationSchema(
    int MinNameLength,
    int MaxNameLength,
    string AllowedNamePattern,
    IReadOnlyList<string> Races,
    IReadOnlyList<string> Sexes);
