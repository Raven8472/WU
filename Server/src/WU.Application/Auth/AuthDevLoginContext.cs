namespace WU.Application.Auth;

public sealed record AuthDevLoginContext(
    AuthAccountSummary Account,
    IReadOnlyList<AuthRealmSummary> Realms);
