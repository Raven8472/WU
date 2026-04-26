namespace WU.Application.Auth;

public sealed record AuthAccountSummary(
    Guid AccountId,
    string Username,
    string DisplayName);
