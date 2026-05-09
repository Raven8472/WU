using Npgsql;
using NpgsqlTypes;
using WU.Application.Clubs;
using WU.Domain.Characters;
using WU.Domain.Clubs;

namespace WU.Infrastructure.Clubs;

public sealed class PostgresClubRepository(NpgsqlDataSource dataSource) : IClubRepository
{
    private const string UniqueViolationSqlState = "23505";
    private const int AllClubPermissionsMask =
        (int)EWuClubPermission.Invite
        | (int)EWuClubPermission.Uninvite
        | (int)EWuClubPermission.Kick
        | (int)EWuClubPermission.Promote
        | (int)EWuClubPermission.Demote
        | (int)EWuClubPermission.EditPublicNote
        | (int)EWuClubPermission.EditOfficerNote
        | (int)EWuClubPermission.ManagePreferences;

    public async Task<ClubCreateResult> CreateAsync(CreateClubCommand command, CancellationToken cancellationToken)
    {
        await EnsureClubSchemaAsync(cancellationToken);

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var transaction = await connection.BeginTransactionAsync(cancellationToken);

        try
        {
            if (!await CharacterExistsForAccountRealmAsync(connection, transaction, command.AccountId, command.RealmId, command.PresidentCharacterId, cancellationToken))
            {
                await transaction.RollbackAsync(cancellationToken);
                return ClubCreateResult.PresidentNotFound();
            }

            if (await CharacterHasClubAsync(connection, transaction, command.PresidentCharacterId, cancellationToken))
            {
                await transaction.RollbackAsync(cancellationToken);
                return ClubCreateResult.PresidentAlreadyInClub();
            }

            var conflict = await FindActiveClubConflictAsync(connection, transaction, command.RealmId, command.NormalizedName, command.NormalizedTag, cancellationToken);
            if (conflict == ClubNameConflict.Name)
            {
                await transaction.RollbackAsync(cancellationToken);
                return ClubCreateResult.NameTaken();
            }

            if (conflict == ClubNameConflict.Tag)
            {
                await transaction.RollbackAsync(cancellationToken);
                return ClubCreateResult.TagTaken();
            }

            var club = await InsertClubAsync(connection, transaction, command, cancellationToken);
            await InsertDefaultRankPermissionsAsync(connection, transaction, club.ClubId, cancellationToken);
            await InsertClubMemberAsync(connection, transaction, club.ClubId, command.PresidentCharacterId, EWuClubRank.President, cancellationToken);

            await transaction.CommitAsync(cancellationToken);

            return ClubCreateResult.Created(new CharacterClubSummary(
                club.ClubId,
                club.Name,
                club.Tag,
                EWuClubRank.President,
                AllClubPermissionsMask,
                string.Empty,
                string.Empty));
        }
        catch (PostgresException exception) when (exception.SqlState == UniqueViolationSqlState && IsClubNameConstraint(exception))
        {
            await transaction.RollbackAsync(cancellationToken);
            return ClubCreateResult.NameTaken();
        }
        catch (PostgresException exception) when (exception.SqlState == UniqueViolationSqlState && IsClubTagConstraint(exception))
        {
            await transaction.RollbackAsync(cancellationToken);
            return ClubCreateResult.TagTaken();
        }
        catch
        {
            await transaction.RollbackAsync(cancellationToken);
            throw;
        }
    }

    public async Task<ClubInviteResult> InviteAsync(Guid clubId, Guid inviterCharacterId, Guid invitedCharacterId, CancellationToken cancellationToken)
    {
        await EnsureClubSchemaAsync(cancellationToken);

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var transaction = await connection.BeginTransactionAsync(cancellationToken);

        try
        {
            var club = await GetClubInfoAsync(connection, transaction, clubId, cancellationToken);
            if (club is null)
            {
                await transaction.RollbackAsync(cancellationToken);
                return ClubInviteResult.ClubNotFound();
            }

            var inviterMembership = await GetClubMembershipAsync(connection, transaction, clubId, inviterCharacterId, cancellationToken);
            if (inviterMembership is null)
            {
                await transaction.RollbackAsync(cancellationToken);
                return ClubInviteResult.InviterNotMember();
            }

            if (!CanInvite(inviterMembership.Value.Rank, inviterMembership.Value.PermissionsMask))
            {
                await transaction.RollbackAsync(cancellationToken);
                return ClubInviteResult.InviterNotAllowed();
            }

            if (!await CharacterExistsInRealmAsync(connection, transaction, club.RealmId, invitedCharacterId, cancellationToken))
            {
                await transaction.RollbackAsync(cancellationToken);
                return ClubInviteResult.InviteeNotFound();
            }

            if (await CharacterHasClubAsync(connection, transaction, invitedCharacterId, cancellationToken))
            {
                await transaction.RollbackAsync(cancellationToken);
                return ClubInviteResult.InviteeAlreadyInClub();
            }

            if (await PendingInviteExistsAsync(connection, transaction, clubId, invitedCharacterId, cancellationToken))
            {
                await transaction.RollbackAsync(cancellationToken);
                return ClubInviteResult.InviteAlreadyPending();
            }

            var invite = await InsertInviteAsync(connection, transaction, clubId, inviterCharacterId, invitedCharacterId, cancellationToken);
            await transaction.CommitAsync(cancellationToken);

            return ClubInviteResult.Invited(invite);
        }
        catch (PostgresException exception) when (exception.SqlState == UniqueViolationSqlState && IsPendingInviteConstraint(exception))
        {
            await transaction.RollbackAsync(cancellationToken);
            return ClubInviteResult.InviteAlreadyPending();
        }
        catch
        {
            await transaction.RollbackAsync(cancellationToken);
            throw;
        }
    }

    public async Task<ClubRosterResult> GetRosterAsync(Guid clubId, Guid viewerCharacterId, bool includeOffline, CancellationToken cancellationToken)
    {
        await EnsureClubSchemaAsync(cancellationToken);

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        var club = await GetClubInfoAsync(connection, null, clubId, cancellationToken);
        if (club is null)
        {
            return ClubRosterResult.ClubNotFound();
        }

        var viewerMembership = await GetClubMembershipAsync(connection, null, clubId, viewerCharacterId, cancellationToken);
        if (viewerMembership is null)
        {
            return ClubRosterResult.ViewerNotMember();
        }

        var members = await ListRosterMembersAsync(connection, clubId, includeOffline, cancellationToken);
        return ClubRosterResult.Found(new ClubRosterResponse(club, members, includeOffline));
    }

    private static bool CanInvite(EWuClubRank rank, int permissionsMask)
    {
        return rank == EWuClubRank.President || (permissionsMask & (int)EWuClubPermission.Invite) == (int)EWuClubPermission.Invite;
    }

    private async Task EnsureClubSchemaAsync(CancellationToken cancellationToken)
    {
        const string sql = """
            CREATE EXTENSION IF NOT EXISTS pgcrypto;

            CREATE TABLE IF NOT EXISTS clubs (
                id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
                realm_id uuid NOT NULL REFERENCES realms(id) ON DELETE CASCADE,
                name text NOT NULL,
                normalized_name text NOT NULL,
                tag text NOT NULL DEFAULT '',
                normalized_tag text NOT NULL DEFAULT '',
                description text NOT NULL DEFAULT '',
                president_character_id uuid NULL REFERENCES characters(id) ON DELETE SET NULL,
                created_at timestamptz NOT NULL DEFAULT now(),
                updated_at timestamptz NOT NULL DEFAULT now(),
                disbanded_at timestamptz NULL,
                CONSTRAINT ck_clubs_name_length CHECK (char_length(name) BETWEEN 3 AND 32),
                CONSTRAINT ck_clubs_normalized_name CHECK (normalized_name = lower(name)),
                CONSTRAINT ck_clubs_tag_length CHECK (tag = '' OR char_length(tag) BETWEEN 2 AND 6),
                CONSTRAINT ck_clubs_normalized_tag CHECK (normalized_tag = lower(tag))
            );

            CREATE UNIQUE INDEX IF NOT EXISTS ux_clubs_realm_name_active
                ON clubs(realm_id, normalized_name)
                WHERE disbanded_at IS NULL;

            CREATE UNIQUE INDEX IF NOT EXISTS ux_clubs_realm_tag_active
                ON clubs(realm_id, normalized_tag)
                WHERE disbanded_at IS NULL AND normalized_tag <> '';

            CREATE INDEX IF NOT EXISTS ix_clubs_realm_active
                ON clubs(realm_id)
                WHERE disbanded_at IS NULL;

            CREATE TABLE IF NOT EXISTS club_rank_permissions (
                club_id uuid NOT NULL REFERENCES clubs(id) ON DELETE CASCADE,
                rank smallint NOT NULL,
                permissions_mask integer NOT NULL DEFAULT 0,
                updated_at timestamptz NOT NULL DEFAULT now(),
                PRIMARY KEY (club_id, rank),
                CONSTRAINT ck_club_rank_permissions_rank CHECK (rank BETWEEN 1 AND 4),
                CONSTRAINT ck_club_rank_permissions_mask CHECK (permissions_mask BETWEEN 0 AND 255)
            );

            CREATE TABLE IF NOT EXISTS club_members (
                club_id uuid NOT NULL REFERENCES clubs(id) ON DELETE CASCADE,
                character_id uuid NOT NULL REFERENCES characters(id) ON DELETE CASCADE,
                rank smallint NOT NULL DEFAULT 2,
                public_note text NOT NULL DEFAULT '',
                officer_note text NOT NULL DEFAULT '',
                joined_at timestamptz NOT NULL DEFAULT now(),
                updated_at timestamptz NOT NULL DEFAULT now(),
                PRIMARY KEY (club_id, character_id),
                CONSTRAINT ck_club_members_rank CHECK (rank BETWEEN 1 AND 4),
                CONSTRAINT ck_club_members_public_note_length CHECK (char_length(public_note) <= 512),
                CONSTRAINT ck_club_members_officer_note_length CHECK (char_length(officer_note) <= 512)
            );

            CREATE UNIQUE INDEX IF NOT EXISTS ux_club_members_character
                ON club_members(character_id);

            CREATE INDEX IF NOT EXISTS ix_club_members_club_rank
                ON club_members(club_id, rank);

            CREATE TABLE IF NOT EXISTS club_invites (
                id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
                club_id uuid NOT NULL REFERENCES clubs(id) ON DELETE CASCADE,
                invited_character_id uuid NOT NULL REFERENCES characters(id) ON DELETE CASCADE,
                invited_by_character_id uuid NULL REFERENCES characters(id) ON DELETE SET NULL,
                status smallint NOT NULL DEFAULT 0,
                created_at timestamptz NOT NULL DEFAULT now(),
                expires_at timestamptz NULL,
                responded_at timestamptz NULL,
                CONSTRAINT ck_club_invites_status CHECK (status BETWEEN 0 AND 4)
            );

            CREATE UNIQUE INDEX IF NOT EXISTS ux_club_invites_pending
                ON club_invites(club_id, invited_character_id)
                WHERE status = 0;

            CREATE INDEX IF NOT EXISTS ix_club_invites_invited_character
                ON club_invites(invited_character_id, status);

            CREATE INDEX IF NOT EXISTS ix_club_invites_club_status
                ON club_invites(club_id, status);

            CREATE TABLE IF NOT EXISTS character_presence (
                character_id uuid PRIMARY KEY REFERENCES characters(id) ON DELETE CASCADE,
                realm_id uuid NOT NULL REFERENCES realms(id) ON DELETE CASCADE,
                current_zone_id uuid NULL REFERENCES zones(id) ON DELETE SET NULL,
                path text NOT NULL DEFAULT '',
                is_online boolean NOT NULL DEFAULT false,
                last_online_at timestamptz NULL,
                updated_at timestamptz NOT NULL DEFAULT now(),
                CONSTRAINT ck_character_presence_path_length CHECK (char_length(path) <= 64)
            );

            CREATE INDEX IF NOT EXISTS ix_character_presence_realm_online
                ON character_presence(realm_id, is_online);

            CREATE INDEX IF NOT EXISTS ix_character_presence_zone
                ON character_presence(current_zone_id);

            CREATE TABLE IF NOT EXISTS club_chat_messages (
                id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
                club_id uuid NOT NULL REFERENCES clubs(id) ON DELETE CASCADE,
                sender_character_id uuid NULL REFERENCES characters(id) ON DELETE SET NULL,
                channel smallint NOT NULL,
                message text NOT NULL,
                created_at timestamptz NOT NULL DEFAULT now(),
                CONSTRAINT ck_club_chat_messages_channel CHECK (channel BETWEEN 0 AND 1),
                CONSTRAINT ck_club_chat_messages_length CHECK (char_length(message) BETWEEN 1 AND 512)
            );

            CREATE INDEX IF NOT EXISTS ix_club_chat_messages_club_channel_created
                ON club_chat_messages(club_id, channel, created_at DESC);
            """;

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var command = new NpgsqlCommand(sql, connection);
        await command.ExecuteNonQueryAsync(cancellationToken);
    }

    private static async Task<bool> CharacterExistsForAccountRealmAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        Guid accountId,
        Guid realmId,
        Guid characterId,
        CancellationToken cancellationToken)
    {
        const string sql = """
            SELECT 1
            FROM characters
            WHERE id = @character_id
              AND account_id = @account_id
              AND realm_id = @realm_id
              AND deleted_at IS NULL;
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Transaction = transaction;
        command.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);
        command.Parameters.AddWithValue("account_id", NpgsqlDbType.Uuid, accountId);
        command.Parameters.AddWithValue("realm_id", NpgsqlDbType.Uuid, realmId);

        return await command.ExecuteScalarAsync(cancellationToken) is not null;
    }

    private static async Task<bool> CharacterExistsInRealmAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        Guid realmId,
        Guid characterId,
        CancellationToken cancellationToken)
    {
        const string sql = """
            SELECT 1
            FROM characters
            WHERE id = @character_id
              AND realm_id = @realm_id
              AND deleted_at IS NULL;
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Transaction = transaction;
        command.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);
        command.Parameters.AddWithValue("realm_id", NpgsqlDbType.Uuid, realmId);

        return await command.ExecuteScalarAsync(cancellationToken) is not null;
    }

    private static async Task<bool> CharacterHasClubAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        Guid characterId,
        CancellationToken cancellationToken)
    {
        const string sql = """
            SELECT 1
            FROM club_members
            WHERE character_id = @character_id;
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Transaction = transaction;
        command.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);

        return await command.ExecuteScalarAsync(cancellationToken) is not null;
    }

    private static async Task<ClubNameConflict> FindActiveClubConflictAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        Guid realmId,
        string normalizedName,
        string normalizedTag,
        CancellationToken cancellationToken)
    {
        const string sql = """
            SELECT normalized_name, normalized_tag
            FROM clubs
            WHERE realm_id = @realm_id
              AND disbanded_at IS NULL
              AND (
                    normalized_name = @normalized_name
                    OR (@normalized_tag <> '' AND normalized_tag = @normalized_tag)
                  )
            LIMIT 1;
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Transaction = transaction;
        command.Parameters.AddWithValue("realm_id", NpgsqlDbType.Uuid, realmId);
        command.Parameters.AddWithValue("normalized_name", NpgsqlDbType.Text, normalizedName);
        command.Parameters.AddWithValue("normalized_tag", NpgsqlDbType.Text, normalizedTag);

        await using var reader = await command.ExecuteReaderAsync(cancellationToken);
        if (!await reader.ReadAsync(cancellationToken))
        {
            return ClubNameConflict.None;
        }

        return string.Equals(reader.GetString(0), normalizedName, StringComparison.Ordinal)
            ? ClubNameConflict.Name
            : ClubNameConflict.Tag;
    }

    private static async Task<ClubInfo> InsertClubAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        CreateClubCommand command,
        CancellationToken cancellationToken)
    {
        const string sql = """
            INSERT INTO clubs (
                realm_id,
                name,
                normalized_name,
                tag,
                normalized_tag,
                description,
                president_character_id)
            VALUES (
                @realm_id,
                @name,
                @normalized_name,
                @tag,
                @normalized_tag,
                @description,
                @president_character_id)
            RETURNING id, realm_id, name, tag, description, president_character_id, created_at;
            """;

        await using var dbCommand = new NpgsqlCommand(sql, connection);
        dbCommand.Transaction = transaction;
        dbCommand.Parameters.AddWithValue("realm_id", NpgsqlDbType.Uuid, command.RealmId);
        dbCommand.Parameters.AddWithValue("name", NpgsqlDbType.Text, command.Name);
        dbCommand.Parameters.AddWithValue("normalized_name", NpgsqlDbType.Text, command.NormalizedName);
        dbCommand.Parameters.AddWithValue("tag", NpgsqlDbType.Text, command.Tag);
        dbCommand.Parameters.AddWithValue("normalized_tag", NpgsqlDbType.Text, command.NormalizedTag);
        dbCommand.Parameters.AddWithValue("description", NpgsqlDbType.Text, command.Description);
        dbCommand.Parameters.AddWithValue("president_character_id", NpgsqlDbType.Uuid, command.PresidentCharacterId);

        await using var reader = await dbCommand.ExecuteReaderAsync(cancellationToken);
        if (!await reader.ReadAsync(cancellationToken))
        {
            throw new InvalidOperationException("PostgreSQL did not return a club row.");
        }

        return ReadClubInfo(reader);
    }

    private static async Task InsertDefaultRankPermissionsAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        Guid clubId,
        CancellationToken cancellationToken)
    {
        const string sql = """
            INSERT INTO club_rank_permissions (club_id, rank, permissions_mask)
            VALUES
                (@club_id, 1, 0),
                (@club_id, 2, 0),
                (@club_id, 3, @officer_permissions),
                (@club_id, 4, @president_permissions)
            ON CONFLICT (club_id, rank) DO UPDATE
            SET permissions_mask = EXCLUDED.permissions_mask,
                updated_at = now();
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Transaction = transaction;
        command.Parameters.AddWithValue("club_id", NpgsqlDbType.Uuid, clubId);
        command.Parameters.AddWithValue("officer_permissions", NpgsqlDbType.Integer, (int)EWuClubPermission.Invite);
        command.Parameters.AddWithValue("president_permissions", NpgsqlDbType.Integer, AllClubPermissionsMask);
        await command.ExecuteNonQueryAsync(cancellationToken);
    }

    private static async Task InsertClubMemberAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        Guid clubId,
        Guid characterId,
        EWuClubRank rank,
        CancellationToken cancellationToken)
    {
        const string sql = """
            INSERT INTO club_members (club_id, character_id, rank)
            VALUES (@club_id, @character_id, @rank);
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Transaction = transaction;
        command.Parameters.AddWithValue("club_id", NpgsqlDbType.Uuid, clubId);
        command.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);
        command.Parameters.AddWithValue("rank", NpgsqlDbType.Smallint, (short)rank);
        await command.ExecuteNonQueryAsync(cancellationToken);
    }

    private static async Task<ClubInfo?> GetClubInfoAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction? transaction,
        Guid clubId,
        CancellationToken cancellationToken)
    {
        const string sql = """
            SELECT id, realm_id, name, tag, description, president_character_id, created_at
            FROM clubs
            WHERE id = @club_id
              AND disbanded_at IS NULL;
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Transaction = transaction;
        command.Parameters.AddWithValue("club_id", NpgsqlDbType.Uuid, clubId);

        await using var reader = await command.ExecuteReaderAsync(cancellationToken);
        return await reader.ReadAsync(cancellationToken)
            ? ReadClubInfo(reader)
            : null;
    }

    private static async Task<ClubMembershipAccess?> GetClubMembershipAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction? transaction,
        Guid clubId,
        Guid characterId,
        CancellationToken cancellationToken)
    {
        const string sql = """
            SELECT cm.rank, COALESCE(crp.permissions_mask, 0)
            FROM club_members cm
            LEFT JOIN club_rank_permissions crp ON crp.club_id = cm.club_id AND crp.rank = cm.rank
            WHERE cm.club_id = @club_id
              AND cm.character_id = @character_id;
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Transaction = transaction;
        command.Parameters.AddWithValue("club_id", NpgsqlDbType.Uuid, clubId);
        command.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);

        await using var reader = await command.ExecuteReaderAsync(cancellationToken);
        return await reader.ReadAsync(cancellationToken)
            ? new ClubMembershipAccess((EWuClubRank)reader.GetInt16(0), reader.GetInt32(1))
            : null;
    }

    private static async Task<bool> PendingInviteExistsAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        Guid clubId,
        Guid characterId,
        CancellationToken cancellationToken)
    {
        const string sql = """
            SELECT 1
            FROM club_invites
            WHERE club_id = @club_id
              AND invited_character_id = @character_id
              AND status = 0;
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Transaction = transaction;
        command.Parameters.AddWithValue("club_id", NpgsqlDbType.Uuid, clubId);
        command.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);

        return await command.ExecuteScalarAsync(cancellationToken) is not null;
    }

    private static async Task<ClubInviteSummary> InsertInviteAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        Guid clubId,
        Guid inviterCharacterId,
        Guid invitedCharacterId,
        CancellationToken cancellationToken)
    {
        const string sql = """
            INSERT INTO club_invites (
                club_id,
                invited_character_id,
                invited_by_character_id,
                expires_at)
            VALUES (
                @club_id,
                @invited_character_id,
                @invited_by_character_id,
                now() + interval '14 days')
            RETURNING id, club_id, invited_character_id, invited_by_character_id, status, created_at, expires_at, responded_at;
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Transaction = transaction;
        command.Parameters.AddWithValue("club_id", NpgsqlDbType.Uuid, clubId);
        command.Parameters.AddWithValue("invited_character_id", NpgsqlDbType.Uuid, invitedCharacterId);
        command.Parameters.AddWithValue("invited_by_character_id", NpgsqlDbType.Uuid, inviterCharacterId);

        await using var reader = await command.ExecuteReaderAsync(cancellationToken);
        if (!await reader.ReadAsync(cancellationToken))
        {
            throw new InvalidOperationException("PostgreSQL did not return a club invite row.");
        }

        return ReadClubInviteSummary(reader);
    }

    private static async Task<IReadOnlyList<ClubMemberSummary>> ListRosterMembersAsync(
        NpgsqlConnection connection,
        Guid clubId,
        bool includeOffline,
        CancellationToken cancellationToken)
    {
        const string sql = """
            SELECT cm.character_id,
                   c.name,
                   c.house,
                   c.level,
                   cm.rank,
                   COALESCE(cp.path, '') AS path,
                   COALESCE(cp.is_online, false) AS is_online,
                   cp.current_zone_id,
                   COALESCE(z.display_name, '') AS location_display_name,
                   cp.last_online_at,
                   cm.public_note,
                   cm.officer_note,
                   cm.joined_at
            FROM club_members cm
            INNER JOIN characters c ON c.id = cm.character_id
            LEFT JOIN character_presence cp ON cp.character_id = cm.character_id
            LEFT JOIN zones z ON z.id = cp.current_zone_id
            WHERE cm.club_id = @club_id
              AND c.deleted_at IS NULL
              AND (@include_offline OR COALESCE(cp.is_online, false))
            ORDER BY COALESCE(cp.is_online, false) DESC, cm.rank DESC, c.name ASC;
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Parameters.AddWithValue("club_id", NpgsqlDbType.Uuid, clubId);
        command.Parameters.AddWithValue("include_offline", NpgsqlDbType.Boolean, includeOffline);

        List<ClubMemberSummary> members = [];
        await using var reader = await command.ExecuteReaderAsync(cancellationToken);
        while (await reader.ReadAsync(cancellationToken))
        {
            members.Add(ReadClubMemberSummary(reader));
        }

        return members;
    }

    private static ClubInfo ReadClubInfo(NpgsqlDataReader reader)
    {
        return new ClubInfo(
            ClubId: reader.GetGuid(0),
            RealmId: reader.GetGuid(1),
            Name: reader.GetString(2),
            Tag: reader.GetString(3),
            Description: reader.GetString(4),
            PresidentCharacterId: reader.IsDBNull(5) ? null : reader.GetGuid(5),
            CreatedAt: reader.GetFieldValue<DateTimeOffset>(6));
    }

    private static ClubInviteSummary ReadClubInviteSummary(NpgsqlDataReader reader)
    {
        return new ClubInviteSummary(
            InviteId: reader.GetGuid(0),
            ClubId: reader.GetGuid(1),
            InvitedCharacterId: reader.GetGuid(2),
            InvitedByCharacterId: reader.IsDBNull(3) ? null : reader.GetGuid(3),
            Status: (EWuClubInviteStatus)reader.GetInt16(4),
            CreatedAt: reader.GetFieldValue<DateTimeOffset>(5),
            ExpiresAt: reader.IsDBNull(6) ? null : reader.GetFieldValue<DateTimeOffset>(6),
            RespondedAt: reader.IsDBNull(7) ? null : reader.GetFieldValue<DateTimeOffset>(7));
    }

    private static ClubMemberSummary ReadClubMemberSummary(NpgsqlDataReader reader)
    {
        return new ClubMemberSummary(
            CharacterId: reader.GetGuid(0),
            Name: reader.GetString(1),
            House: (EWuHouse)reader.GetInt16(2),
            Level: reader.GetInt32(3),
            Rank: (EWuClubRank)reader.GetInt16(4),
            Path: reader.GetString(5),
            IsOnline: reader.GetBoolean(6),
            CurrentZoneId: reader.IsDBNull(7) ? null : reader.GetGuid(7),
            LocationDisplayName: reader.GetString(8),
            LastOnlineAt: reader.IsDBNull(9) ? null : reader.GetFieldValue<DateTimeOffset>(9),
            PublicNote: reader.GetString(10),
            OfficerNote: reader.GetString(11),
            JoinedAt: reader.GetFieldValue<DateTimeOffset>(12));
    }

    private static bool IsClubNameConstraint(PostgresException exception)
    {
        return string.Equals(exception.ConstraintName, "ux_clubs_realm_name_active", StringComparison.Ordinal);
    }

    private static bool IsClubTagConstraint(PostgresException exception)
    {
        return string.Equals(exception.ConstraintName, "ux_clubs_realm_tag_active", StringComparison.Ordinal);
    }

    private static bool IsPendingInviteConstraint(PostgresException exception)
    {
        return string.Equals(exception.ConstraintName, "ux_club_invites_pending", StringComparison.Ordinal);
    }

    private readonly record struct ClubMembershipAccess(EWuClubRank Rank, int PermissionsMask);

    private enum ClubNameConflict
    {
        None,
        Name,
        Tag
    }
}
