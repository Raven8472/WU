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
        await EnsureCharacterSchemaAsync(cancellationToken);

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
                command.Appearance,
                startingLevel,
                0,
                CharacterStatRules.GetExperienceToNextLevel(startingLevel),
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
        await EnsureCharacterSchemaAsync(cancellationToken);

        const string sql = """
            SELECT c.id, c.name, c.race, c.sex, c.level, c.experience, c.position_x, c.position_y, c.position_z,
                   COALESCE(ca.skin_preset_index, 0) AS skin_preset_index,
                   COALESCE(ca.head_preset_index, 0) AS head_preset_index,
                   COALESCE(ca.hair_style_index, 0) AS hair_style_index,
                   COALESCE(ca.hair_color_index, 0) AS hair_color_index,
                   COALESCE(ca.eye_color_index, 1) AS eye_color_index,
                   COALESCE(ca.brow_style_index, 0) AS brow_style_index,
                   COALESCE(ca.beard_style_index, 0) AS beard_style_index
            FROM characters c
            LEFT JOIN character_appearances ca ON ca.character_id = c.id
            WHERE c.account_id = @account_id
              AND c.realm_id = @realm_id
              AND c.deleted_at IS NULL
            ORDER BY c.created_at ASC;
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

    public async Task<bool> DeleteAsync(Guid accountId, Guid realmId, Guid characterId, CancellationToken cancellationToken)
    {
        await EnsureCharacterSchemaAsync(cancellationToken);

        const string sql = """
            UPDATE characters
            SET deleted_at = now(),
                updated_at = now()
            WHERE id = @character_id
              AND account_id = @account_id
              AND realm_id = @realm_id
              AND deleted_at IS NULL;
            """;

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var dbCommand = new NpgsqlCommand(sql, connection);
        dbCommand.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);
        dbCommand.Parameters.AddWithValue("account_id", NpgsqlDbType.Uuid, accountId);
        dbCommand.Parameters.AddWithValue("realm_id", NpgsqlDbType.Uuid, realmId);

        return await dbCommand.ExecuteNonQueryAsync(cancellationToken) > 0;
    }

    public async Task<CharacterSummary?> AwardExperienceAsync(Guid accountId, Guid realmId, Guid characterId, int amount, CancellationToken cancellationToken)
    {
        await EnsureCharacterSchemaAsync(cancellationToken);

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var transaction = await connection.BeginTransactionAsync(cancellationToken);

        var currentCharacter = await LoadCharacterSummaryForUpdateAsync(connection, transaction, accountId, realmId, characterId, cancellationToken);
        if (currentCharacter is null)
        {
            await transaction.RollbackAsync(cancellationToken);
            return null;
        }

        var updatedProgression = CharacterStatRules.ResolveExperienceAward(
            currentCharacter.Level,
            currentCharacter.Experience,
            amount);

        if (updatedProgression.Level != currentCharacter.Level || updatedProgression.Experience != currentCharacter.Experience)
        {
            await UpdateCharacterProgressionAsync(
                connection,
                transaction,
                currentCharacter.CharacterId,
                updatedProgression.Level,
                updatedProgression.Experience,
                cancellationToken);
        }

        await transaction.CommitAsync(cancellationToken);

        return currentCharacter with
        {
            Level = updatedProgression.Level,
            Experience = updatedProgression.Experience,
            ExperienceToNextLevel = updatedProgression.ExperienceToNextLevel,
            Stats = CharacterStatRules.Calculate(currentCharacter.Race, updatedProgression.Level)
        };
    }

    public async Task<CharacterSummary?> UpdateLocationAsync(Guid accountId, Guid realmId, Guid characterId, CharacterLocation location, CancellationToken cancellationToken)
    {
        await EnsureCharacterSchemaAsync(cancellationToken);

        const string sql = """
            WITH updated AS (
                UPDATE characters
                SET position_x = @position_x,
                    position_y = @position_y,
                    position_z = @position_z,
                    updated_at = now()
                WHERE id = @character_id
                  AND account_id = @account_id
                  AND realm_id = @realm_id
                  AND deleted_at IS NULL
                RETURNING id, name, race, sex, level, experience, position_x, position_y, position_z
            )
            SELECT u.id, u.name, u.race, u.sex, u.level, u.experience, u.position_x, u.position_y, u.position_z,
                   COALESCE(ca.skin_preset_index, 0) AS skin_preset_index,
                   COALESCE(ca.head_preset_index, 0) AS head_preset_index,
                   COALESCE(ca.hair_style_index, 0) AS hair_style_index,
                   COALESCE(ca.hair_color_index, 0) AS hair_color_index,
                   COALESCE(ca.eye_color_index, 1) AS eye_color_index,
                   COALESCE(ca.brow_style_index, 0) AS brow_style_index,
                   COALESCE(ca.beard_style_index, 0) AS beard_style_index
            FROM updated u
            LEFT JOIN character_appearances ca ON ca.character_id = u.id;
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
            INSERT INTO character_appearances (
                character_id,
                skin_preset_index,
                head_preset_index,
                hair_style_index,
                hair_color_index,
                eye_color_index,
                brow_style_index,
                beard_style_index)
            VALUES (
                @character_id,
                @skin_preset_index,
                @head_preset_index,
                @hair_style_index,
                @hair_color_index,
                @eye_color_index,
                @brow_style_index,
                @beard_style_index);
            """;

        await using var dbCommand = new NpgsqlCommand(sql, connection);
        dbCommand.Transaction = transaction;
        dbCommand.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);
        dbCommand.Parameters.AddWithValue("skin_preset_index", NpgsqlDbType.Integer, command.Appearance.SkinPresetIndex);
        dbCommand.Parameters.AddWithValue("head_preset_index", NpgsqlDbType.Integer, command.Appearance.HeadPresetIndex);
        dbCommand.Parameters.AddWithValue("hair_style_index", NpgsqlDbType.Integer, command.Appearance.HairStyleIndex);
        dbCommand.Parameters.AddWithValue("hair_color_index", NpgsqlDbType.Integer, command.Appearance.HairColorIndex);
        dbCommand.Parameters.AddWithValue("eye_color_index", NpgsqlDbType.Integer, command.Appearance.EyeColorIndex);
        dbCommand.Parameters.AddWithValue("brow_style_index", NpgsqlDbType.Integer, command.Appearance.BrowStyleIndex);
        dbCommand.Parameters.AddWithValue("beard_style_index", NpgsqlDbType.Integer, command.Appearance.BeardStyleIndex);

        await dbCommand.ExecuteNonQueryAsync(cancellationToken);
    }

    private async Task EnsureCharacterSchemaAsync(CancellationToken cancellationToken)
    {
        const string sql = """
            ALTER TABLE character_appearances
                ADD COLUMN IF NOT EXISTS head_preset_index integer NOT NULL DEFAULT 0,
                ADD COLUMN IF NOT EXISTS eye_color_index integer NOT NULL DEFAULT 1,
                ADD COLUMN IF NOT EXISTS brow_style_index integer NOT NULL DEFAULT 0,
                ADD COLUMN IF NOT EXISTS beard_style_index integer NOT NULL DEFAULT 0;

            ALTER TABLE characters
                ADD COLUMN IF NOT EXISTS experience integer NOT NULL DEFAULT 0;

            ALTER TABLE characters DROP CONSTRAINT IF EXISTS uq_characters_realm_name;
            CREATE UNIQUE INDEX IF NOT EXISTS uq_characters_realm_name_active
                ON characters(realm_id, normalized_name)
                WHERE deleted_at IS NULL;
            """;

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var command = new NpgsqlCommand(sql, connection);
        await command.ExecuteNonQueryAsync(cancellationToken);
    }

    private static bool IsCharacterNameConstraint(PostgresException exception)
    {
        return string.Equals(exception.ConstraintName, "uq_characters_realm_name", StringComparison.Ordinal)
            || string.Equals(exception.ConstraintName, "uq_characters_realm_name_active", StringComparison.Ordinal);
    }

    private static CharacterSummary ReadCharacterSummary(NpgsqlDataReader reader)
    {
        var race = (EWuCharacterRace)reader.GetInt16(2);
        var level = reader.GetInt32(4);
        var experience = reader.GetInt32(5);
        var normalizedProgression = CharacterStatRules.ResolveExperienceAward(level, experience, 0);

        return new CharacterSummary(
            CharacterId: reader.GetGuid(0),
            Name: reader.GetString(1),
            Race: race,
            Sex: (EWuCharacterSex)reader.GetInt16(3),
            Appearance: new CharacterAppearanceDraft(
                SkinPresetIndex: reader.GetInt32(9),
                HeadPresetIndex: reader.GetInt32(10),
                HairStyleIndex: reader.GetInt32(11),
                HairColorIndex: reader.GetInt32(12),
                EyeColorIndex: reader.GetInt32(13),
                BrowStyleIndex: reader.GetInt32(14),
                BeardStyleIndex: reader.GetInt32(15)),
            Level: normalizedProgression.Level,
            Experience: normalizedProgression.Experience,
            ExperienceToNextLevel: normalizedProgression.ExperienceToNextLevel,
            Stats: CharacterStatRules.Calculate(race, normalizedProgression.Level),
            Location: new CharacterLocation(
                X: reader.GetFloat(6),
                Y: reader.GetFloat(7),
                Z: reader.GetFloat(8)));
    }

    private static async Task UpdateCharacterProgressionAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        Guid characterId,
        int level,
        int experience,
        CancellationToken cancellationToken)
    {
        const string sql = """
            UPDATE characters
            SET level = @level,
                experience = @experience,
                updated_at = now()
            WHERE id = @character_id;
            """;

        await using var dbCommand = new NpgsqlCommand(sql, connection);
        dbCommand.Transaction = transaction;
        dbCommand.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);
        dbCommand.Parameters.AddWithValue("level", NpgsqlDbType.Integer, level);
        dbCommand.Parameters.AddWithValue("experience", NpgsqlDbType.Integer, experience);
        await dbCommand.ExecuteNonQueryAsync(cancellationToken);
    }

    private static async Task<CharacterSummary?> LoadCharacterSummaryForUpdateAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        Guid accountId,
        Guid realmId,
        Guid characterId,
        CancellationToken cancellationToken)
    {
        const string sql = """
            SELECT c.id, c.name, c.race, c.sex, c.level, c.experience, c.position_x, c.position_y, c.position_z,
                   COALESCE(ca.skin_preset_index, 0) AS skin_preset_index,
                   COALESCE(ca.head_preset_index, 0) AS head_preset_index,
                   COALESCE(ca.hair_style_index, 0) AS hair_style_index,
                   COALESCE(ca.hair_color_index, 0) AS hair_color_index,
                   COALESCE(ca.eye_color_index, 1) AS eye_color_index,
                   COALESCE(ca.brow_style_index, 0) AS brow_style_index,
                   COALESCE(ca.beard_style_index, 0) AS beard_style_index
            FROM characters c
            LEFT JOIN character_appearances ca ON ca.character_id = c.id
            WHERE c.id = @character_id
              AND c.account_id = @account_id
              AND c.realm_id = @realm_id
              AND c.deleted_at IS NULL
            FOR UPDATE;
            """;

        await using var dbCommand = new NpgsqlCommand(sql, connection);
        dbCommand.Transaction = transaction;
        dbCommand.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);
        dbCommand.Parameters.AddWithValue("account_id", NpgsqlDbType.Uuid, accountId);
        dbCommand.Parameters.AddWithValue("realm_id", NpgsqlDbType.Uuid, realmId);

        await using var reader = await dbCommand.ExecuteReaderAsync(cancellationToken);
        return await reader.ReadAsync(cancellationToken)
            ? ReadCharacterSummary(reader)
            : null;
    }
}
