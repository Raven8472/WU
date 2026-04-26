using Npgsql;
using NpgsqlTypes;
using WU.Application.Auth;

namespace WU.Infrastructure.Auth;

public sealed class PostgresAuthRepository(NpgsqlDataSource dataSource) : IAuthRepository
{
    private const string DevelopmentUsername = "dev";
    private const string DevelopmentRealmSlug = "local-dev";

    public async Task<AuthDevLoginContext?> GetDevelopmentLoginContextAsync(CancellationToken cancellationToken)
    {
        await EnsureSessionSchemaAsync(cancellationToken);

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        var account = await GetAccountByUsernameAsync(connection, DevelopmentUsername, cancellationToken);
        if (account is null)
        {
            return null;
        }

        var realms = await ListRealmsAsync(connection, cancellationToken);
        if (realms.Count == 0)
        {
            return null;
        }

        return new AuthDevLoginContext(account, realms);
    }

    public async Task CreateSessionAsync(Guid accountId, string tokenHash, DateTimeOffset expiresAt, CancellationToken cancellationToken)
    {
        await EnsureSessionSchemaAsync(cancellationToken);

        const string sql = """
            INSERT INTO account_sessions (account_id, token_hash, expires_at)
            VALUES (@account_id, @token_hash, @expires_at);
            """;

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var command = new NpgsqlCommand(sql, connection);
        command.Parameters.AddWithValue("account_id", NpgsqlDbType.Uuid, accountId);
        command.Parameters.AddWithValue("token_hash", NpgsqlDbType.Text, tokenHash);
        command.Parameters.AddWithValue("expires_at", NpgsqlDbType.TimestampTz, expiresAt);

        await command.ExecuteNonQueryAsync(cancellationToken);
    }

    public async Task<CurrentSession?> GetSessionByTokenHashAsync(string tokenHash, CancellationToken cancellationToken)
    {
        await EnsureSessionSchemaAsync(cancellationToken);

        const string sessionSql = """
            SELECT s.id, s.expires_at, a.id, a.username, a.display_name
            FROM account_sessions s
            INNER JOIN accounts a ON a.id = s.account_id
            WHERE s.token_hash = @token_hash
              AND s.revoked_at IS NULL
              AND s.expires_at > now()
              AND a.disabled_at IS NULL;
            """;

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var command = new NpgsqlCommand(sessionSql, connection);
        command.Parameters.AddWithValue("token_hash", NpgsqlDbType.Text, tokenHash);

        await using var reader = await command.ExecuteReaderAsync(cancellationToken);
        if (!await reader.ReadAsync(cancellationToken))
        {
            return null;
        }

        var sessionId = reader.GetGuid(0);
        var expiresAt = reader.GetFieldValue<DateTimeOffset>(1);
        var account = new AuthAccountSummary(
            AccountId: reader.GetGuid(2),
            Username: reader.GetString(3),
            DisplayName: reader.GetString(4));

        await reader.DisposeAsync();

        var realms = await ListRealmsAsync(connection, cancellationToken);
        return new CurrentSession(sessionId, expiresAt, account, realms);
    }

    private async Task EnsureSessionSchemaAsync(CancellationToken cancellationToken)
    {
        const string sql = """
            CREATE TABLE IF NOT EXISTS account_sessions (
                id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
                account_id uuid NOT NULL REFERENCES accounts(id) ON DELETE CASCADE,
                token_hash text NOT NULL UNIQUE,
                created_at timestamptz NOT NULL DEFAULT now(),
                expires_at timestamptz NOT NULL,
                revoked_at timestamptz NULL
            );

            CREATE INDEX IF NOT EXISTS ix_account_sessions_account ON account_sessions(account_id);
            CREATE INDEX IF NOT EXISTS ix_account_sessions_expires ON account_sessions(expires_at);
            """;

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var command = new NpgsqlCommand(sql, connection);
        await command.ExecuteNonQueryAsync(cancellationToken);
    }

    private static async Task<AuthAccountSummary?> GetAccountByUsernameAsync(NpgsqlConnection connection, string username, CancellationToken cancellationToken)
    {
        const string sql = """
            SELECT id, username, display_name
            FROM accounts
            WHERE username = @username
              AND disabled_at IS NULL;
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Parameters.AddWithValue("username", NpgsqlDbType.Text, username);

        await using var reader = await command.ExecuteReaderAsync(cancellationToken);
        if (!await reader.ReadAsync(cancellationToken))
        {
            return null;
        }

        return new AuthAccountSummary(
            AccountId: reader.GetGuid(0),
            Username: reader.GetString(1),
            DisplayName: reader.GetString(2));
    }

    private static async Task<IReadOnlyList<AuthRealmSummary>> ListRealmsAsync(NpgsqlConnection connection, CancellationToken cancellationToken)
    {
        const string sql = """
            SELECT id, slug, display_name, status
            FROM realms
            WHERE slug = @slug
            ORDER BY display_name ASC;
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Parameters.AddWithValue("slug", NpgsqlDbType.Text, DevelopmentRealmSlug);

        List<AuthRealmSummary> realms = [];
        await using var reader = await command.ExecuteReaderAsync(cancellationToken);
        while (await reader.ReadAsync(cancellationToken))
        {
            realms.Add(new AuthRealmSummary(
                RealmId: reader.GetGuid(0),
                Slug: reader.GetString(1),
                DisplayName: reader.GetString(2),
                Status: reader.GetString(3)));
        }

        return realms;
    }
}
