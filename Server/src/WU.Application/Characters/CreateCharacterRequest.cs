using WU.Domain.Characters;

namespace WU.Application.Characters;

public sealed record CreateCharacterRequest(
    Guid AccountId,
    Guid RealmId,
    string Name,
    EWuCharacterRace Race,
    EWuCharacterSex Sex,
    CharacterAppearanceDraft? Appearance);
