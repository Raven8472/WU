CREATE EXTENSION IF NOT EXISTS pgcrypto;

CREATE TABLE IF NOT EXISTS character_experience_awards (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    character_id uuid NOT NULL REFERENCES characters(id) ON DELETE CASCADE,
    source smallint NOT NULL,
    source_key text NOT NULL,
    amount integer NOT NULL,
    awarded_at timestamptz NOT NULL DEFAULT now(),
    CONSTRAINT ck_character_experience_awards_amount CHECK (amount > 0),
    CONSTRAINT uq_character_experience_awards_source UNIQUE (character_id, source, source_key)
);

CREATE INDEX IF NOT EXISTS ix_character_experience_awards_character
    ON character_experience_awards(character_id);
