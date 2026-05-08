CREATE TABLE IF NOT EXISTS character_wallets (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    character_id uuid NOT NULL UNIQUE REFERENCES characters(id) ON DELETE CASCADE,
    balance_knuts bigint NOT NULL DEFAULT 0,
    created_at timestamptz NOT NULL DEFAULT now(),
    updated_at timestamptz NOT NULL DEFAULT now(),
    CONSTRAINT ck_character_wallets_balance CHECK (balance_knuts >= 0)
);

COMMENT ON TABLE character_wallets IS 'Spendable carried currency for one character, stored in Knuts.';
COMMENT ON COLUMN character_wallets.balance_knuts IS '1 Sickle = 29 Knuts. 1 Galleon = 17 Sickles = 493 Knuts.';

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

COMMENT ON TABLE account_bank_wallets IS 'Account-wide banked currency per realm, stored in Knuts.';
COMMENT ON COLUMN account_bank_wallets.balance_knuts IS 'Shared savings wallet; characters can deposit and withdraw from this bank balance.';

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

COMMENT ON TABLE currency_transactions IS 'Append-only currency ledger for transfers, deposits, withdrawals, purchases, and grants.';

CREATE INDEX IF NOT EXISTS ix_currency_transactions_realm_created
    ON currency_transactions(realm_id, created_at DESC);

CREATE INDEX IF NOT EXISTS ix_currency_transactions_from_wallet
    ON currency_transactions(from_wallet_type, from_wallet_id, created_at DESC);

CREATE INDEX IF NOT EXISTS ix_currency_transactions_to_wallet
    ON currency_transactions(to_wallet_type, to_wallet_id, created_at DESC);

CREATE INDEX IF NOT EXISTS ix_currency_transactions_initiated_by
    ON currency_transactions(initiated_by_character_id, created_at DESC);
