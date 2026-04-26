namespace WU.Application.Auth;

public sealed record AuthRealmSummary(
    Guid RealmId,
    string Slug,
    string DisplayName,
    string Status);
