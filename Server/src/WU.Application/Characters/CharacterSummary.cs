using WU.Domain.Characters;
using WU.Application.Clubs;

namespace WU.Application.Characters;

public sealed record CharacterSummary(
    Guid CharacterId,
    string Name,
    EWuCharacterRace Race,
    EWuCharacterSex Sex,
    EWuHouse House,
    CharacterAppearanceDraft Appearance,
    CharacterClubSummary? Club,
    int Level,
    int Experience,
    int ExperienceToNextLevel,
    CharacterStats Stats,
    CharacterLocation Location);
