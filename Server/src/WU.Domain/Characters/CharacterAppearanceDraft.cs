namespace WU.Domain.Characters;

public sealed record CharacterAppearanceDraft(
    int SkinPresetIndex,
    int HairStyleIndex,
    int HairColorIndex);
