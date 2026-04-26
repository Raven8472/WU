namespace WU.Application.Auth;

public sealed record CurrentSession(
    Guid SessionId,
    DateTimeOffset ExpiresAt,
    AuthAccountSummary Account,
    IReadOnlyList<AuthRealmSummary> Realms);
