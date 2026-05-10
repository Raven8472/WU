namespace WU.Application.Clubs;

public sealed record CreateClubFromCharterRequest(
    Guid AccountId,
    Guid RealmId,
    Guid PresidentCharacterId,
    string Name,
    string? Tag,
    string? Description,
    int SlotIndex,
    string ItemId);
