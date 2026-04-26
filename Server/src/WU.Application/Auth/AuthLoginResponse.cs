namespace WU.Application.Auth;

public sealed record AuthLoginResponse(
    string AccessToken,
    DateTimeOffset ExpiresAt,
    AuthAccountSummary Account,
    IReadOnlyList<AuthRealmSummary> Realms);
