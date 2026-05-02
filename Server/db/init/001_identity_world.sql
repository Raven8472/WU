CREATE EXTENSION IF NOT EXISTS pgcrypto;

CREATE TABLE IF NOT EXISTS realms (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    slug text NOT NULL UNIQUE,
    display_name text NOT NULL,
    status text NOT NULL DEFAULT 'online',
    created_at timestamptz NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS zones (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    realm_id uuid NOT NULL REFERENCES realms(id) ON DELETE CASCADE,
    zone_key text NOT NULL,
    display_name text NOT NULL,
    unreal_map_name text NOT NULL,
    max_players integer NOT NULL DEFAULT 200,
    is_instanced boolean NOT NULL DEFAULT false,
    created_at timestamptz NOT NULL DEFAULT now(),
    UNIQUE (realm_id, zone_key)
);

CREATE TABLE IF NOT EXISTS accounts (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    username text NOT NULL UNIQUE,
    display_name text NOT NULL,
    created_at timestamptz NOT NULL DEFAULT now(),
    disabled_at timestamptz NULL
);

CREATE TABLE IF NOT EXISTS characters (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    account_id uuid NOT NULL REFERENCES accounts(id) ON DELETE CASCADE,
    realm_id uuid NOT NULL REFERENCES realms(id) ON DELETE CASCADE,
    name text NOT NULL,
    normalized_name text NOT NULL,
    race smallint NOT NULL,
    sex smallint NOT NULL,
    house smallint NULL,
    level integer NOT NULL DEFAULT 1,
    current_zone_id uuid NULL REFERENCES zones(id),
    position_x real NOT NULL DEFAULT 0,
    position_y real NOT NULL DEFAULT 0,
    position_z real NOT NULL DEFAULT 0,
    created_at timestamptz NOT NULL DEFAULT now(),
    updated_at timestamptz NOT NULL DEFAULT now(),
    deleted_at timestamptz NULL,
    CONSTRAINT ck_characters_name_length CHECK (char_length(name) BETWEEN 3 AND 16),
    CONSTRAINT ck_characters_name_pattern CHECK (name ~ '^[A-Za-z]+$'),
    CONSTRAINT ck_characters_normalized_name CHECK (normalized_name = lower(name)),
    CONSTRAINT ck_characters_race CHECK (race BETWEEN 0 AND 2),
    CONSTRAINT ck_characters_sex CHECK (sex BETWEEN 0 AND 1),
    CONSTRAINT ck_characters_house CHECK (house IS NULL OR house BETWEEN 0 AND 3),
    CONSTRAINT ck_characters_level CHECK (level >= 1)
);

CREATE TABLE IF NOT EXISTS character_appearances (
    character_id uuid PRIMARY KEY REFERENCES characters(id) ON DELETE CASCADE,
    skin_preset_index integer NOT NULL DEFAULT 0,
    head_preset_index integer NOT NULL DEFAULT 0,
    hair_style_index integer NOT NULL DEFAULT 0,
    hair_color_index integer NOT NULL DEFAULT 0,
    eye_color_index integer NOT NULL DEFAULT 1,
    brow_style_index integer NOT NULL DEFAULT 0,
    beard_style_index integer NOT NULL DEFAULT 0,
    updated_at timestamptz NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS ix_characters_account_realm ON characters(account_id, realm_id);
CREATE INDEX IF NOT EXISTS ix_characters_zone ON characters(current_zone_id);
CREATE UNIQUE INDEX IF NOT EXISTS uq_characters_realm_name_active ON characters(realm_id, normalized_name) WHERE deleted_at IS NULL;

INSERT INTO realms (id, slug, display_name)
VALUES ('00000000-0000-0000-0000-000000000001', 'local-dev', 'Local Dev')
ON CONFLICT (slug) DO NOTHING;

INSERT INTO accounts (id, username, display_name)
VALUES ('00000000-0000-0000-0000-000000000101', 'dev', 'Dev Player')
ON CONFLICT (username) DO NOTHING;
