namespace WU.Infrastructure.Configuration;

public sealed class WuPersistenceOptions
{
    public const string SectionName = "WuPersistence";

    public PostgresOptions Postgres { get; init; } = new();
    public RedisOptions Redis { get; init; } = new();
    public RealmOptions Realm { get; init; } = new();
}

public sealed class PostgresOptions
{
    public string ConnectionString { get; init; } = string.Empty;
}

public sealed class RedisOptions
{
    public string ConnectionString { get; init; } = string.Empty;
}

public sealed class RealmOptions
{
    public string Slug { get; init; } = "local-dev";
    public string DisplayName { get; init; } = "Local Dev";
}
