namespace WU.Domain.Realms;

public sealed record WuZone(
    Guid Id,
    Guid RealmId,
    string ZoneKey,
    string DisplayName,
    string UnrealMapName,
    int MaxPlayers,
    bool IsInstanced);
