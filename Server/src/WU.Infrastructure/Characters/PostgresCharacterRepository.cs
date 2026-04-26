using Npgsql;
using NpgsqlTypes;
using WU.Application.Characters;
using WU.Domain.Characters;

namespace WU.Infrastructure.Characters;

public sealed class PostgresCharacterRepository(NpgsqlDataSource dataSource) : ICharacterRepository
{
    private const string UniqueViolationSqlState = "23505";

    public async Task<CharacterSummary> CreateAsync(CreateCharacterCommand command, CancellationToken cancellationToken)
    {
        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var transaction = await connection.BeginTransactionAsync(cancellationToken);

        try
        {
            var characterId = await InsertCharacterAsync(connection, transaction, command, cancellationToken);
            await InsertAppearanceAsync(connection, transaction, characterId, command, cancellationToken);
            await transaction.CommitAsync(cancellationToken);

            return new CharacterSummary(characterId, command.Name, command.Race, command.Sex, Level: 1);
        }
        catch (PostgresException exception) when (exception.SqlState == UniqueViolationSqlState && IsCharacterNameConstraint(exception))
        {
            await transaction.RollbackAsync(cancellationToken);
            throw new DuplicateCharacterNameException(command.NormalizedName);
        }
        catch
        {
            await transaction.RollbackAsync(cancellationToken);
            throw;
        }
    }

    public async Task<IReadOnlyList<CharacterSummary>> ListForAccountRealmAsync(Guid accountId, Guid realmId, CancellationToken cancellationToken)
    {
        const string sql = """
            SELECT id, name, race, sex, level
            FROM characters
            WHERE account_id = @account_id
              AND realm_id = @realm_id
              AND deleted_at IS NULL
            ORDER BY created_at ASC;
            """;

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var dbCommand = new NpgsqlCommand(sql, connection);
        dbCommand.Parameters.AddWithValue("account_id", NpgsqlDbType.Uuid, accountId);
        dbCommand.Parameters.AddWithValue("realm_id", NpgsqlDbType.Uuid, realmId);

        List<CharacterSummary> characters = [];
        await using var reader = await dbCommand.ExecuteReaderAsync(cancellationToken);
        while (await reader.ReadAsync(cancellationToken))
        {
            characters.Add(new CharacterSummary(
                CharacterId: reader.GetGuid(0),
                Name: reader.GetString(1),
                Race: (EWuCharacterRace)reader.GetInt16(2),
                Sex: (EWuCharacterSex)reader.GetInt16(3),
                Level: reader.GetInt32(4)));
        }

        return characters;
    }

    private static async Task<Guid> InsertCharacterAsync(NpgsqlConnection connection, NpgsqlTransaction transaction, CreateCharacterCommand command, CancellationToken cancellationToken)
    {
        const string sql = """
            INSERT INTO characters (account_id, realm_id, name, normalized_name, race, sex)
            VALUES (@account_id, @realm_id, @name, @normalized_name, @race, @sex)
            RETURNING id;
            """;

        await using var dbCommand = new NpgsqlCommand(sql, connection);
        dbCommand.Transaction = transaction;
        dbCommand.Parameters.AddWithValue("account_id", NpgsqlDbType.Uuid, command.AccountId);
        dbCommand.Parameters.AddWithValue("realm_id", NpgsqlDbType.Uuid, command.RealmId);
        dbCommand.Parameters.AddWithValue("name", NpgsqlDbType.Text, command.Name);
        dbCommand.Parameters.AddWithValue("normalized_name", NpgsqlDbType.Text, command.NormalizedName);
        dbCommand.Parameters.AddWithValue("race", NpgsqlDbType.Smallint, (short)command.Race);
        dbCommand.Parameters.AddWithValue("sex", NpgsqlDbType.Smallint, (short)command.Sex);

        var result = await dbCommand.ExecuteScalarAsync(cancellationToken);
        return result is Guid id ? id : throw new InvalidOperationException("PostgreSQL did not return a character id.");
    }

    private static async Task InsertAppearanceAsync(NpgsqlConnection connection, NpgsqlTransaction transaction, Guid characterId, CreateCharacterCommand command, CancellationToken cancellationToken)
    {
        const string sql = """
            INSERT INTO character_appearances (character_id, skin_preset_index, hair_style_index, hair_color_index)
            VALUES (@character_id, @skin_preset_index, @hair_style_index, @hair_color_index);
            """;

        await using var dbCommand = new NpgsqlCommand(sql, connection);
        dbCommand.Transaction = transaction;
        dbCommand.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);
        dbCommand.Parameters.AddWithValue("skin_preset_index", NpgsqlDbType.Integer, command.Appearance.SkinPresetIndex);
        dbCommand.Parameters.AddWithValue("hair_style_index", NpgsqlDbType.Integer, command.Appearance.HairStyleIndex);
        dbCommand.Parameters.AddWithValue("hair_color_index", NpgsqlDbType.Integer, command.Appearance.HairColorIndex);

        await dbCommand.ExecuteNonQueryAsync(cancellationToken);
    }

    private static bool IsCharacterNameConstraint(PostgresException exception)
    {
        return string.Equals(exception.ConstraintName, "uq_characters_realm_name", StringComparison.Ordinal);
    }
}
