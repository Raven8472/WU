using Npgsql;
using NpgsqlTypes;
using WU.Application.Inventory;

namespace WU.Infrastructure.Inventory;

public sealed class PostgresCharacterInventoryRepository(NpgsqlDataSource dataSource) : ICharacterInventoryRepository
{
    public async Task<CharacterInventorySnapshot?> GetSnapshotAsync(Guid accountId, Guid realmId, Guid characterId, CancellationToken cancellationToken)
    {
        await EnsureInventorySchemaAsync(cancellationToken);

        const string sql = """
            SELECT c.id,
                   i.slot_index,
                   i.item_id,
                   i.quantity
            FROM characters c
            LEFT JOIN character_inventory_items i ON i.character_id = c.id
            WHERE c.id = @character_id
              AND c.account_id = @account_id
              AND c.realm_id = @realm_id
              AND c.deleted_at IS NULL
            ORDER BY i.slot_index ASC;
            """;

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var command = new NpgsqlCommand(sql, connection);
        command.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);
        command.Parameters.AddWithValue("account_id", NpgsqlDbType.Uuid, accountId);
        command.Parameters.AddWithValue("realm_id", NpgsqlDbType.Uuid, realmId);

        Guid? foundCharacterId = null;
        List<CharacterInventoryItemSummary> items = [];
        await using var reader = await command.ExecuteReaderAsync(cancellationToken);
        while (await reader.ReadAsync(cancellationToken))
        {
            foundCharacterId ??= reader.GetGuid(0);

            if (!reader.IsDBNull(2))
            {
                items.Add(new CharacterInventoryItemSummary(
                    SlotIndex: reader.GetInt32(1),
                    ItemId: reader.GetString(2),
                    Quantity: reader.GetInt32(3)));
            }
        }

        return foundCharacterId is Guid id
            ? new CharacterInventorySnapshot(id, items)
            : null;
    }

    public async Task<CharacterInventorySnapshot?> AddPurchasedItemAsync(Guid accountId, Guid realmId, Guid characterId, string itemId, CancellationToken cancellationToken)
    {
        await EnsureInventorySchemaAsync(cancellationToken);

        var trimmedItemId = itemId.Trim();
        if (trimmedItemId.Length == 0)
        {
            return null;
        }

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var transaction = await connection.BeginTransactionAsync(cancellationToken);

        const string characterSql = """
            SELECT id
            FROM characters
            WHERE id = @character_id
              AND account_id = @account_id
              AND realm_id = @realm_id
              AND deleted_at IS NULL
            FOR UPDATE;
            """;

        await using (var characterCommand = new NpgsqlCommand(characterSql, connection, transaction))
        {
            characterCommand.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);
            characterCommand.Parameters.AddWithValue("account_id", NpgsqlDbType.Uuid, accountId);
            characterCommand.Parameters.AddWithValue("realm_id", NpgsqlDbType.Uuid, realmId);

            if (await characterCommand.ExecuteScalarAsync(cancellationToken) is not Guid)
            {
                await transaction.RollbackAsync(cancellationToken);
                return null;
            }
        }

        const string nextSlotSql = """
            SELECT COALESCE(MAX(slot_index), -1) + 1
            FROM character_inventory_items
            WHERE character_id = @character_id;
            """;

        var nextSlotIndex = 0;
        await using (var slotCommand = new NpgsqlCommand(nextSlotSql, connection, transaction))
        {
            slotCommand.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);
            var scalar = await slotCommand.ExecuteScalarAsync(cancellationToken);
            nextSlotIndex = scalar is int value ? value : Convert.ToInt32(scalar);
        }

        const string insertSql = """
            INSERT INTO character_inventory_items (character_id, slot_index, item_id, quantity)
            VALUES (@character_id, @slot_index, @item_id, 1);
            """;

        await using (var insertCommand = new NpgsqlCommand(insertSql, connection, transaction))
        {
            insertCommand.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);
            insertCommand.Parameters.AddWithValue("slot_index", NpgsqlDbType.Integer, nextSlotIndex);
            insertCommand.Parameters.AddWithValue("item_id", NpgsqlDbType.Text, trimmedItemId);
            await insertCommand.ExecuteNonQueryAsync(cancellationToken);
        }

        await transaction.CommitAsync(cancellationToken);
        return await GetSnapshotAsync(accountId, realmId, characterId, cancellationToken);
    }

    private async Task EnsureInventorySchemaAsync(CancellationToken cancellationToken)
    {
        const string sql = """
            CREATE EXTENSION IF NOT EXISTS pgcrypto;

            CREATE TABLE IF NOT EXISTS character_inventory_items (
                id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
                character_id uuid NOT NULL REFERENCES characters(id) ON DELETE CASCADE,
                slot_index integer NOT NULL CHECK (slot_index >= 0),
                item_id text NOT NULL,
                quantity integer NOT NULL DEFAULT 1 CHECK (quantity > 0),
                created_at timestamptz NOT NULL DEFAULT now(),
                updated_at timestamptz NOT NULL DEFAULT now(),
                UNIQUE (character_id, slot_index)
            );

            CREATE INDEX IF NOT EXISTS idx_character_inventory_items_character
                ON character_inventory_items (character_id, slot_index);
            """;

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var command = new NpgsqlCommand(sql, connection);
        await command.ExecuteNonQueryAsync(cancellationToken);
    }
}
