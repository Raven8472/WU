using WU.Domain.Characters;

namespace WU.Application.Characters;

public sealed record CharacterSummary(
    Guid CharacterId,
    string Name,
    EWuCharacterRace Race,
    EWuCharacterSex Sex,
    CharacterAppearanceDraft Appearance,
    int Level,
    int Experience,
    int ExperienceToNextLevel,
    CharacterStats Stats,
    CharacterLocation Location);
