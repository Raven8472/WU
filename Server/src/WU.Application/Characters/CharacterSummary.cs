using WU.Domain.Characters;

namespace WU.Application.Characters;

public sealed record CharacterSummary(
    Guid CharacterId,
    string Name,
    EWuCharacterRace Race,
    EWuCharacterSex Sex,
    int Level,
    CharacterStats Stats,
    CharacterLocation Location);
