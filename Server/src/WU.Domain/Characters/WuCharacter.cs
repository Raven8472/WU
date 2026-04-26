namespace WU.Domain.Characters;

public sealed record WuCharacter(
    Guid Id,
    Guid AccountId,
    Guid RealmId,
    string Name,
    EWuCharacterRace Race,
    EWuCharacterSex Sex,
    EWuHouse? House,
    int Level);
