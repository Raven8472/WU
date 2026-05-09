namespace WU.Application.Clubs;

public sealed record CreateClubRequest(
    Guid AccountId,
    Guid RealmId,
    Guid PresidentCharacterId,
    string Name,
    string? Tag,
    string? Description);
