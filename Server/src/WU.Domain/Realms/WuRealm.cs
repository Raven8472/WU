namespace WU.Domain.Realms;

public sealed record WuRealm(
    Guid Id,
    string Slug,
    string DisplayName,
    string Status);
