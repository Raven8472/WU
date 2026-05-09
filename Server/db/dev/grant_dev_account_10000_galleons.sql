CREATE EXTENSION IF NOT EXISTS pgcrypto;

CREATE TABLE IF NOT EXISTS character_wallets (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    character_id uuid NOT NULL UNIQUE REFERENCES characters(id) ON DELETE CASCADE,
    balance_knuts bigint NOT NULL DEFAULT 0 CHECK (balance_knuts >= 0),
    created_at timestamptz NOT NULL DEFAULT now(),
    updated_at timestamptz NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS currency_transactions (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    realm_id uuid NOT NULL REFERENCES realms(id) ON DELETE CASCADE,
    from_wallet_type text NOT NULL CHECK (from_wallet_type IN ('Character', 'AccountBank', 'System')),
    from_wallet_id uuid NULL,
    to_wallet_type text NOT NULL CHECK (to_wallet_type IN ('Character', 'AccountBank', 'System')),
    to_wallet_id uuid NULL,
    amount_knuts bigint NOT NULL CHECK (amount_knuts > 0),
    reason text NOT NULL CHECK (reason IN ('Loot', 'VendorPurchase', 'VendorSell', 'BankDeposit', 'BankWithdraw', 'PlayerTrade', 'MailAttachment', 'QuestReward', 'SystemGrant', 'AdminAdjustment')),
    initiated_by_character_id uuid NULL REFERENCES characters(id) ON DELETE SET NULL,
    note text NULL,
    created_at timestamptz NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_currency_transactions_realm_created
    ON currency_transactions (realm_id, created_at DESC);

CREATE INDEX IF NOT EXISTS idx_currency_transactions_from_wallet
    ON currency_transactions (from_wallet_type, from_wallet_id);

CREATE INDEX IF NOT EXISTS idx_currency_transactions_to_wallet
    ON currency_transactions (to_wallet_type, to_wallet_id);

WITH target_characters AS (
    SELECT id, realm_id
    FROM characters
    WHERE account_id = '00000000-0000-0000-0000-000000000101'
      AND deleted_at IS NULL
),
upserted_wallets AS (
    INSERT INTO character_wallets (character_id, balance_knuts)
    SELECT id, 4930000
    FROM target_characters
    ON CONFLICT (character_id) DO UPDATE
        SET balance_knuts = EXCLUDED.balance_knuts,
            updated_at = now()
    RETURNING id, character_id
)
INSERT INTO currency_transactions (
    realm_id,
    from_wallet_type,
    from_wallet_id,
    to_wallet_type,
    to_wallet_id,
    amount_knuts,
    reason,
    initiated_by_character_id,
    note
)
SELECT
    target_characters.realm_id,
    'System',
    NULL,
    'Character',
    upserted_wallets.id,
    4930000,
    'SystemGrant',
    target_characters.id,
    'Dev seed: set character wallet to 10000 galleons'
FROM upserted_wallets
JOIN target_characters ON target_characters.id = upserted_wallets.character_id;

SELECT
    characters.name,
    character_wallets.balance_knuts,
    (character_wallets.balance_knuts / 493) AS galleons,
    ((character_wallets.balance_knuts % 493) / 29) AS sickles,
    (character_wallets.balance_knuts % 29) AS knuts
FROM characters
JOIN character_wallets ON character_wallets.character_id = characters.id
WHERE characters.account_id = '00000000-0000-0000-0000-000000000101'
  AND characters.deleted_at IS NULL
ORDER BY characters.name;
