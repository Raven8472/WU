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
