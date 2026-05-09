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

COMMENT ON TABLE clubs IS 'Realm-scoped guild/kinship equivalent. Disbanded clubs keep history through disbanded_at.';
COMMENT ON COLUMN clubs.president_character_id IS 'Convenience pointer for the current club president; club_members remains the authoritative membership table.';

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

COMMENT ON COLUMN club_rank_permissions.rank IS '1=Recruit, 2=Member, 3=Officer, 4=President.';
COMMENT ON COLUMN club_rank_permissions.permissions_mask IS 'Bitmask aligned with EWUClubPermission: invite, uninvite, kick, promote, demote, public note, officer note, manage preferences.';

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

COMMENT ON COLUMN club_members.rank IS '1=Recruit, 2=Member, 3=Officer, 4=President.';
COMMENT ON COLUMN club_members.public_note IS 'Member-visible note.';
COMMENT ON COLUMN club_members.officer_note IS 'Officer-visible note.';

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

COMMENT ON COLUMN club_invites.status IS '0=Pending, 1=Accepted, 2=Declined, 3=Revoked, 4=Expired.';

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

COMMENT ON TABLE character_presence IS 'Presence data used by club/social roster views: online state, current zone, path, and last-online display source.';

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

COMMENT ON COLUMN club_chat_messages.channel IS '0=Club chat (/c), 1=Officer chat (/o).';

CREATE INDEX IF NOT EXISTS ix_club_chat_messages_club_channel_created
    ON club_chat_messages(club_id, channel, created_at DESC);
