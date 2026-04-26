using WU.Domain.Characters;

namespace WU.Application.Characters;

public sealed record CreateCharacterCommand(
    Guid AccountId,
    Guid RealmId,
    string Name,
    string NormalizedName,
    EWuCharacterRace Race,
    EWuCharacterSex Sex,
    CharacterAppearanceDraft Appearance);
