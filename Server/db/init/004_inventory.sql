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
