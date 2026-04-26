namespace WU.Application.Auth;

public interface IAuthRepository
{
    Task<AuthDevLoginContext?> GetDevelopmentLoginContextAsync(CancellationToken cancellationToken);

    Task CreateSessionAsync(Guid accountId, string tokenHash, DateTimeOffset expiresAt, CancellationToken cancellationToken);

    Task<CurrentSession?> GetSessionByTokenHashAsync(string tokenHash, CancellationToken cancellationToken);
}
