namespace WU.Application.Clubs;

public sealed record ClubInfo(
    Guid ClubId,
    Guid RealmId,
    string Name,
    string Tag,
    string Description,
    Guid? PresidentCharacterId,
    DateTimeOffset CreatedAt);
