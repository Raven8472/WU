namespace WU.Application.Auth;

public sealed class AuthService(IAuthRepository repository, AuthSessionTokenService tokenService)
{
    private static readonly TimeSpan DevelopmentSessionLifetime = TimeSpan.FromHours(12);

    public async Task<AuthLoginResult> CreateDevelopmentLoginAsync(CancellationToken cancellationToken)
    {
        var context = await repository.GetDevelopmentLoginContextAsync(cancellationToken);
        if (context is null)
        {
            return AuthLoginResult.DevAccountMissing();
        }

        var token = tokenService.CreateToken();
        var tokenHash = tokenService.HashToken(token);
        var expiresAt = DateTimeOffset.UtcNow.Add(DevelopmentSessionLifetime);

        await repository.CreateSessionAsync(context.Account.AccountId, tokenHash, expiresAt, cancellationToken);

        return AuthLoginResult.SignedIn(new AuthLoginResponse(
            AccessToken: token,
            ExpiresAt: expiresAt,
            Account: context.Account,
            Realms: context.Realms));
    }

    public async Task<CurrentSessionResult> GetCurrentSessionAsync(string accessToken, CancellationToken cancellationToken)
    {
        if (string.IsNullOrWhiteSpace(accessToken))
        {
            return CurrentSessionResult.InvalidToken();
        }

        var tokenHash = tokenService.HashToken(accessToken);
        var session = await repository.GetSessionByTokenHashAsync(tokenHash, cancellationToken);

        return session is null
            ? CurrentSessionResult.InvalidToken()
            : CurrentSessionResult.Found(session);
    }
}
