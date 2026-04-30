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

            const int startingLevel = 1;
            return new CharacterSummary(
                characterId,
                command.Name,
                command.Race,
                command.Sex,
                startingLevel,
                CharacterStatRules.Calculate(command.Race, startingLevel),
                new CharacterLocation(0.0f, 0.0f, 0.0f));
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
            SELECT id, name, race, sex, level, position_x, position_y, position_z
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
            characters.Add(ReadCharacterSummary(reader));
        }

        return characters;
    }

    public async Task<CharacterSummary?> UpdateLocationAsync(Guid accountId, Guid realmId, Guid characterId, CharacterLocation location, CancellationToken cancellationToken)
    {
        const string sql = """
            UPDATE characters
            SET position_x = @position_x,
                position_y = @position_y,
                position_z = @position_z,
                updated_at = now()
            WHERE id = @character_id
              AND account_id = @account_id
              AND realm_id = @realm_id
              AND deleted_at IS NULL
            RETURNING id, name, race, sex, level, position_x, position_y, position_z;
            """;

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var dbCommand = new NpgsqlCommand(sql, connection);
        dbCommand.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);
        dbCommand.Parameters.AddWithValue("account_id", NpgsqlDbType.Uuid, accountId);
        dbCommand.Parameters.AddWithValue("realm_id", NpgsqlDbType.Uuid, realmId);
        dbCommand.Parameters.AddWithValue("position_x", NpgsqlDbType.Real, location.X);
        dbCommand.Parameters.AddWithValue("position_y", NpgsqlDbType.Real, location.Y);
        dbCommand.Parameters.AddWithValue("position_z", NpgsqlDbType.Real, location.Z);

        await using var reader = await dbCommand.ExecuteReaderAsync(cancellationToken);
        return await reader.ReadAsync(cancellationToken)
            ? ReadCharacterSummary(reader)
            : null;
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

    private static CharacterSummary ReadCharacterSummary(NpgsqlDataReader reader)
    {
        var race = (EWuCharacterRace)reader.GetInt16(2);
        var level = reader.GetInt32(4);

        return new CharacterSummary(
            CharacterId: reader.GetGuid(0),
            Name: reader.GetString(1),
            Race: race,
            Sex: (EWuCharacterSex)reader.GetInt16(3),
            Level: level,
            Stats: CharacterStatRules.Calculate(race, level),
            Location: new CharacterLocation(
                X: reader.GetFloat(5),
                Y: reader.GetFloat(6),
                Z: reader.GetFloat(7)));
    }
}
