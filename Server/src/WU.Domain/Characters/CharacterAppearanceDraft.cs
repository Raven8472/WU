namespace WU.Domain.Characters;

public sealed record CharacterAppearanceDraft(
    int SkinPresetIndex = 0,
    int HeadPresetIndex = 0,
    int HairStyleIndex = 0,
    int HairColorIndex = 0,
    int EyeColorIndex = 1,
    int BrowStyleIndex = 0,
    int BeardStyleIndex = 0);
