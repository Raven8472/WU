using Npgsql;
using NpgsqlTypes;
using WU.Application.Currency;
using WU.Domain.Currency;

namespace WU.Infrastructure.Currency;

public sealed class PostgresCurrencyRepository(NpgsqlDataSource dataSource) : ICurrencyRepository
{
    public async Task<CurrencySnapshotResult> GetSnapshotAsync(Guid accountId, Guid realmId, Guid characterId, CancellationToken cancellationToken)
    {
        await EnsureCurrencySchemaAsync(cancellationToken);

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var transaction = await connection.BeginTransactionAsync(cancellationToken);

        if (!await CharacterExistsForAccountRealmAsync(connection, transaction, accountId, realmId, characterId, cancellationToken))
        {
            await transaction.RollbackAsync(cancellationToken);
            return CurrencySnapshotResult.CharacterNotFound();
        }

        var characterWallet = await GetOrCreateCharacterWalletAsync(connection, transaction, characterId, cancellationToken);
        var bankWallet = await GetOrCreateAccountBankWalletAsync(connection, transaction, accountId, realmId, cancellationToken);
        await transaction.CommitAsync(cancellationToken);

        return CurrencySnapshotResult.Found(BuildSnapshot(accountId, characterId, characterWallet, bankWallet));
    }

    public async Task<CurrencyOperationResult> DepositToBankAsync(Guid accountId, Guid realmId, Guid characterId, long amountKnuts, string note, CancellationToken cancellationToken)
    {
        await EnsureCurrencySchemaAsync(cancellationToken);

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var transaction = await connection.BeginTransactionAsync(cancellationToken);

        if (!await CharacterExistsForAccountRealmAsync(connection, transaction, accountId, realmId, characterId, cancellationToken))
        {
            await transaction.RollbackAsync(cancellationToken);
            return CurrencyOperationResult.CharacterNotFound();
        }

        var characterWallet = await GetOrCreateCharacterWalletAsync(connection, transaction, characterId, cancellationToken);
        var bankWallet = await GetOrCreateAccountBankWalletAsync(connection, transaction, accountId, realmId, cancellationToken);

        if (characterWallet.BalanceKnuts < amountKnuts)
        {
            await transaction.RollbackAsync(cancellationToken);
            return CurrencyOperationResult.InsufficientFunds();
        }

        characterWallet = characterWallet with { BalanceKnuts = characterWallet.BalanceKnuts - amountKnuts };
        bankWallet = bankWallet with { BalanceKnuts = bankWallet.BalanceKnuts + amountKnuts };

        await UpdateCharacterWalletBalanceAsync(connection, transaction, characterWallet, cancellationToken);
        await UpdateAccountBankWalletBalanceAsync(connection, transaction, bankWallet, cancellationToken);

        var ledgerEntry = await InsertTransactionAsync(
            connection,
            transaction,
            realmId,
            EWuCurrencyWalletType.Character,
            characterWallet.WalletId,
            EWuCurrencyWalletType.AccountBank,
            bankWallet.WalletId,
            amountKnuts,
            EWuCurrencyTransactionReason.BankDeposit,
            characterId,
            note,
            cancellationToken);

        await transaction.CommitAsync(cancellationToken);

        return CurrencyOperationResult.Completed(
            BuildSnapshot(accountId, characterId, characterWallet, bankWallet),
            ledgerEntry);
    }

    public async Task<CurrencyOperationResult> WithdrawFromBankAsync(Guid accountId, Guid realmId, Guid characterId, long amountKnuts, string note, CancellationToken cancellationToken)
    {
        await EnsureCurrencySchemaAsync(cancellationToken);

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var transaction = await connection.BeginTransactionAsync(cancellationToken);

        if (!await CharacterExistsForAccountRealmAsync(connection, transaction, accountId, realmId, characterId, cancellationToken))
        {
            await transaction.RollbackAsync(cancellationToken);
            return CurrencyOperationResult.CharacterNotFound();
        }

        var characterWallet = await GetOrCreateCharacterWalletAsync(connection, transaction, characterId, cancellationToken);
        var bankWallet = await GetOrCreateAccountBankWalletAsync(connection, transaction, accountId, realmId, cancellationToken);

        if (bankWallet.BalanceKnuts < amountKnuts)
        {
            await transaction.RollbackAsync(cancellationToken);
            return CurrencyOperationResult.InsufficientFunds();
        }

        bankWallet = bankWallet with { BalanceKnuts = bankWallet.BalanceKnuts - amountKnuts };
        characterWallet = characterWallet with { BalanceKnuts = characterWallet.BalanceKnuts + amountKnuts };

        await UpdateCharacterWalletBalanceAsync(connection, transaction, characterWallet, cancellationToken);
        await UpdateAccountBankWalletBalanceAsync(connection, transaction, bankWallet, cancellationToken);

        var ledgerEntry = await InsertTransactionAsync(
            connection,
            transaction,
            realmId,
            EWuCurrencyWalletType.AccountBank,
            bankWallet.WalletId,
            EWuCurrencyWalletType.Character,
            characterWallet.WalletId,
            amountKnuts,
            EWuCurrencyTransactionReason.BankWithdrawal,
            characterId,
            note,
            cancellationToken);

        await transaction.CommitAsync(cancellationToken);

        return CurrencyOperationResult.Completed(
            BuildSnapshot(accountId, characterId, characterWallet, bankWallet),
            ledgerEntry);
    }

    public async Task<CurrencyOperationResult> TransferToCharacterAsync(Guid accountId, Guid realmId, Guid fromCharacterId, Guid toCharacterId, long amountKnuts, string note, CancellationToken cancellationToken)
    {
        await EnsureCurrencySchemaAsync(cancellationToken);

        await using var connection = await dataSource.OpenConnectionAsync(cancellationToken);
        await using var transaction = await connection.BeginTransactionAsync(cancellationToken);

        if (!await CharacterExistsForAccountRealmAsync(connection, transaction, accountId, realmId, fromCharacterId, cancellationToken))
        {
            await transaction.RollbackAsync(cancellationToken);
            return CurrencyOperationResult.CharacterNotFound();
        }

        if (!await CharacterExistsInRealmAsync(connection, transaction, realmId, toCharacterId, cancellationToken))
        {
            await transaction.RollbackAsync(cancellationToken);
            return CurrencyOperationResult.TargetCharacterNotFound();
        }

        var fromCharacterWallet = await GetOrCreateCharacterWalletAsync(connection, transaction, fromCharacterId, cancellationToken);
        var toCharacterWallet = await GetOrCreateCharacterWalletAsync(connection, transaction, toCharacterId, cancellationToken);
        var bankWallet = await GetOrCreateAccountBankWalletAsync(connection, transaction, accountId, realmId, cancellationToken);

        if (fromCharacterWallet.BalanceKnuts < amountKnuts)
        {
            await transaction.RollbackAsync(cancellationToken);
            return CurrencyOperationResult.InsufficientFunds();
        }

        fromCharacterWallet = fromCharacterWallet with { BalanceKnuts = fromCharacterWallet.BalanceKnuts - amountKnuts };
        toCharacterWallet = toCharacterWallet with { BalanceKnuts = toCharacterWallet.BalanceKnuts + amountKnuts };

        await UpdateCharacterWalletBalanceAsync(connection, transaction, fromCharacterWallet, cancellationToken);
        await UpdateCharacterWalletBalanceAsync(connection, transaction, toCharacterWallet, cancellationToken);

        var ledgerEntry = await InsertTransactionAsync(
            connection,
            transaction,
            realmId,
            EWuCurrencyWalletType.Character,
            fromCharacterWallet.WalletId,
            EWuCurrencyWalletType.Character,
            toCharacterWallet.WalletId,
            amountKnuts,
            EWuCurrencyTransactionReason.PlayerTransfer,
            fromCharacterId,
            note,
            cancellationToken);

        await transaction.CommitAsync(cancellationToken);

        return CurrencyOperationResult.Completed(
            BuildSnapshot(accountId, fromCharacterId, fromCharacterWallet, bankWallet),
            ledgerEntry,
            BuildCharacterWalletSummary(toCharacterId, toCharacterWallet));
    }

    private async Task EnsureCurrencySchemaAsync(CancellationToken cancellationToken)
    {
        const string sql = """
            CREATE EXTENSION IF NOT EXISTS pgcrypto;

            CREATE TABLE IF NOT EXISTS character_wallets (
                id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
                character_id uuid NOT NULL UNIQUE REFERENCES characters(id) ON DELETE CASCADE,
                balance_knuts bigint NOT NULL DEFAULT 0,
                created_at timestamptz NOT NULL DEFAULT now(),
                updated_at timestamptz NOT NULL DEFAULT now(),
                CONSTRAINT ck_character_wallets_balance CHECK (balance_knuts >= 0)
            );

            CREATE INDEX IF NOT EXISTS ix_character_wallets_character
                ON character_wallets(character_id);

            CREATE TABLE IF NOT EXISTS account_bank_wallets (
                id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
                account_id uuid NOT NULL REFERENCES accounts(id) ON DELETE CASCADE,
                realm_id uuid NOT NULL REFERENCES realms(id) ON DELETE CASCADE,
                balance_knuts bigint NOT NULL DEFAULT 0,
                created_at timestamptz NOT NULL DEFAULT now(),
                updated_at timestamptz NOT NULL DEFAULT now(),
                UNIQUE (account_id, realm_id),
                CONSTRAINT ck_account_bank_wallets_balance CHECK (balance_knuts >= 0)
            );

            CREATE INDEX IF NOT EXISTS ix_account_bank_wallets_account_realm
                ON account_bank_wallets(account_id, realm_id);

            CREATE TABLE IF NOT EXISTS currency_transactions (
                id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
                realm_id uuid NOT NULL REFERENCES realms(id) ON DELETE CASCADE,
                from_wallet_type text NOT NULL,
                from_wallet_id uuid NULL,
                to_wallet_type text NOT NULL,
                to_wallet_id uuid NULL,
                amount_knuts bigint NOT NULL,
                reason text NOT NULL,
                initiated_by_character_id uuid NULL REFERENCES characters(id) ON DELETE SET NULL,
                note text NOT NULL DEFAULT '',
                created_at timestamptz NOT NULL DEFAULT now(),
                CONSTRAINT ck_currency_transactions_amount CHECK (amount_knuts > 0),
                CONSTRAINT ck_currency_transactions_from_wallet_type CHECK (from_wallet_type IN ('Character', 'AccountBank', 'System')),
                CONSTRAINT ck_currency_transactions_to_wallet_type CHECK (to_wallet_type IN ('Character', 'AccountBank', 'System')),
                CONSTRAINT ck_currency_transactions_reason CHECK (reason IN ('BankDeposit', 'BankWithdrawal', 'PlayerTransfer', 'VendorPurchase', 'VendorSale', 'SystemGrant', 'AdminAdjustment')),
                CONSTRAINT ck_currency_transactions_note_length CHECK (char_length(note) <= 256)
            );

            CREATE INDEX IF NOT EXISTS ix_currency_transactions_realm_created
                ON currency_transactions(realm_id, created_at DESC);

            CREATE INDEX IF NOT EXISTS ix_currency_transactions_from_wallet
                ON currency_transactions(from_wallet_type, from_wallet_id, created_at DESC);

            CREATE INDEX IF NOT EXISTS ix_currency_transactions_to_wallet
                ON currency_transactions(to_wallet_type, to_wallet_id, created_at DESC);

            CREATE INDEX IF NOT EXISTS ix_currency_transactions_initiated_by
                ON currency_transactions(initiated_by_character_id, created_at DESC);
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

    private static async Task<WalletRow> GetOrCreateCharacterWalletAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        Guid characterId,
        CancellationToken cancellationToken)
    {
        const string insertSql = """
            INSERT INTO character_wallets (character_id)
            VALUES (@character_id)
            ON CONFLICT (character_id) DO NOTHING;
            """;

        await using (var insertCommand = new NpgsqlCommand(insertSql, connection))
        {
            insertCommand.Transaction = transaction;
            insertCommand.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);
            await insertCommand.ExecuteNonQueryAsync(cancellationToken);
        }

        const string selectSql = """
            SELECT id, balance_knuts
            FROM character_wallets
            WHERE character_id = @character_id
            FOR UPDATE;
            """;

        await using var selectCommand = new NpgsqlCommand(selectSql, connection);
        selectCommand.Transaction = transaction;
        selectCommand.Parameters.AddWithValue("character_id", NpgsqlDbType.Uuid, characterId);

        await using var reader = await selectCommand.ExecuteReaderAsync(cancellationToken);
        if (!await reader.ReadAsync(cancellationToken))
        {
            throw new InvalidOperationException("PostgreSQL did not return a character wallet row.");
        }

        return new WalletRow(reader.GetGuid(0), reader.GetInt64(1));
    }

    private static async Task<WalletRow> GetOrCreateAccountBankWalletAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        Guid accountId,
        Guid realmId,
        CancellationToken cancellationToken)
    {
        const string insertSql = """
            INSERT INTO account_bank_wallets (account_id, realm_id)
            VALUES (@account_id, @realm_id)
            ON CONFLICT (account_id, realm_id) DO NOTHING;
            """;

        await using (var insertCommand = new NpgsqlCommand(insertSql, connection))
        {
            insertCommand.Transaction = transaction;
            insertCommand.Parameters.AddWithValue("account_id", NpgsqlDbType.Uuid, accountId);
            insertCommand.Parameters.AddWithValue("realm_id", NpgsqlDbType.Uuid, realmId);
            await insertCommand.ExecuteNonQueryAsync(cancellationToken);
        }

        const string selectSql = """
            SELECT id, balance_knuts
            FROM account_bank_wallets
            WHERE account_id = @account_id
              AND realm_id = @realm_id
            FOR UPDATE;
            """;

        await using var selectCommand = new NpgsqlCommand(selectSql, connection);
        selectCommand.Transaction = transaction;
        selectCommand.Parameters.AddWithValue("account_id", NpgsqlDbType.Uuid, accountId);
        selectCommand.Parameters.AddWithValue("realm_id", NpgsqlDbType.Uuid, realmId);

        await using var reader = await selectCommand.ExecuteReaderAsync(cancellationToken);
        if (!await reader.ReadAsync(cancellationToken))
        {
            throw new InvalidOperationException("PostgreSQL did not return an account bank wallet row.");
        }

        return new WalletRow(reader.GetGuid(0), reader.GetInt64(1));
    }

    private static async Task UpdateCharacterWalletBalanceAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        WalletRow wallet,
        CancellationToken cancellationToken)
    {
        const string sql = """
            UPDATE character_wallets
            SET balance_knuts = @balance_knuts,
                updated_at = now()
            WHERE id = @wallet_id;
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Transaction = transaction;
        command.Parameters.AddWithValue("wallet_id", NpgsqlDbType.Uuid, wallet.WalletId);
        command.Parameters.AddWithValue("balance_knuts", NpgsqlDbType.Bigint, wallet.BalanceKnuts);
        await command.ExecuteNonQueryAsync(cancellationToken);
    }

    private static async Task UpdateAccountBankWalletBalanceAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        WalletRow wallet,
        CancellationToken cancellationToken)
    {
        const string sql = """
            UPDATE account_bank_wallets
            SET balance_knuts = @balance_knuts,
                updated_at = now()
            WHERE id = @wallet_id;
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Transaction = transaction;
        command.Parameters.AddWithValue("wallet_id", NpgsqlDbType.Uuid, wallet.WalletId);
        command.Parameters.AddWithValue("balance_knuts", NpgsqlDbType.Bigint, wallet.BalanceKnuts);
        await command.ExecuteNonQueryAsync(cancellationToken);
    }

    private static async Task<CurrencyTransactionSummary> InsertTransactionAsync(
        NpgsqlConnection connection,
        NpgsqlTransaction transaction,
        Guid realmId,
        EWuCurrencyWalletType fromWalletType,
        Guid? fromWalletId,
        EWuCurrencyWalletType toWalletType,
        Guid? toWalletId,
        long amountKnuts,
        EWuCurrencyTransactionReason reason,
        Guid? initiatedByCharacterId,
        string note,
        CancellationToken cancellationToken)
    {
        const string sql = """
            INSERT INTO currency_transactions (
                realm_id,
                from_wallet_type,
                from_wallet_id,
                to_wallet_type,
                to_wallet_id,
                amount_knuts,
                reason,
                initiated_by_character_id,
                note)
            VALUES (
                @realm_id,
                @from_wallet_type,
                @from_wallet_id,
                @to_wallet_type,
                @to_wallet_id,
                @amount_knuts,
                @reason,
                @initiated_by_character_id,
                @note)
            RETURNING id, realm_id, from_wallet_type, from_wallet_id, to_wallet_type, to_wallet_id, amount_knuts, reason, initiated_by_character_id, note, created_at;
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        command.Transaction = transaction;
        command.Parameters.AddWithValue("realm_id", NpgsqlDbType.Uuid, realmId);
        command.Parameters.AddWithValue("from_wallet_type", NpgsqlDbType.Text, fromWalletType.ToString());
        command.Parameters.AddWithValue("from_wallet_id", NpgsqlDbType.Uuid, (object?)fromWalletId ?? DBNull.Value);
        command.Parameters.AddWithValue("to_wallet_type", NpgsqlDbType.Text, toWalletType.ToString());
        command.Parameters.AddWithValue("to_wallet_id", NpgsqlDbType.Uuid, (object?)toWalletId ?? DBNull.Value);
        command.Parameters.AddWithValue("amount_knuts", NpgsqlDbType.Bigint, amountKnuts);
        command.Parameters.AddWithValue("reason", NpgsqlDbType.Text, reason.ToString());
        command.Parameters.AddWithValue("initiated_by_character_id", NpgsqlDbType.Uuid, (object?)initiatedByCharacterId ?? DBNull.Value);
        command.Parameters.AddWithValue("note", NpgsqlDbType.Text, note);

        await using var reader = await command.ExecuteReaderAsync(cancellationToken);
        if (!await reader.ReadAsync(cancellationToken))
        {
            throw new InvalidOperationException("PostgreSQL did not return a currency transaction row.");
        }

        return new CurrencyTransactionSummary(
            TransactionId: reader.GetGuid(0),
            RealmId: reader.GetGuid(1),
            FromWalletType: Enum.Parse<EWuCurrencyWalletType>(reader.GetString(2)),
            FromWalletId: reader.IsDBNull(3) ? null : reader.GetGuid(3),
            ToWalletType: Enum.Parse<EWuCurrencyWalletType>(reader.GetString(4)),
            ToWalletId: reader.IsDBNull(5) ? null : reader.GetGuid(5),
            Amount: WizardingCurrency.BreakDown(reader.GetInt64(6)),
            Reason: Enum.Parse<EWuCurrencyTransactionReason>(reader.GetString(7)),
            InitiatedByCharacterId: reader.IsDBNull(8) ? null : reader.GetGuid(8),
            Note: reader.GetString(9),
            CreatedAt: reader.GetFieldValue<DateTimeOffset>(10));
    }

    private static CurrencySnapshot BuildSnapshot(Guid accountId, Guid characterId, WalletRow characterWallet, WalletRow bankWallet)
    {
        return new CurrencySnapshot(
            CharacterWallet: BuildCharacterWalletSummary(characterId, characterWallet),
            AccountBankWallet: new CurrencyWalletSummary(
                WalletId: bankWallet.WalletId,
                WalletType: EWuCurrencyWalletType.AccountBank,
                OwnerId: accountId,
                Balance: WizardingCurrency.BreakDown(bankWallet.BalanceKnuts)));
    }

    private static CurrencyWalletSummary BuildCharacterWalletSummary(Guid characterId, WalletRow wallet)
    {
        return new CurrencyWalletSummary(
            WalletId: wallet.WalletId,
            WalletType: EWuCurrencyWalletType.Character,
            OwnerId: characterId,
            Balance: WizardingCurrency.BreakDown(wallet.BalanceKnuts));
    }

    private readonly record struct WalletRow(Guid WalletId, long BalanceKnuts);
}
