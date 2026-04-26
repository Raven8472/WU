namespace WU.Application.Characters;

public sealed record UpdateCharacterLocationRequest(
    Guid AccountId,
    Guid RealmId,
    CharacterLocation? Location);
