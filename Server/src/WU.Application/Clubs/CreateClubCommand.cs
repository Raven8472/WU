namespace WU.Application.Clubs;

public sealed record CreateClubCommand(
    Guid AccountId,
    Guid RealmId,
    Guid PresidentCharacterId,
    string Name,
    string NormalizedName,
    string Tag,
    string NormalizedTag,
    string Description);
