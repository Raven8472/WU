using WU.Domain.Characters;
using WU.Domain.Clubs;

namespace WU.Application.Clubs;

public sealed record ClubMemberSummary(
    Guid CharacterId,
    string Name,
    EWuHouse House,
    int Level,
    EWuClubRank Rank,
    string Path,
    bool IsOnline,
    Guid? CurrentZoneId,
    string LocationDisplayName,
    DateTimeOffset? LastOnlineAt,
    string PublicNote,
    string OfficerNote,
    DateTimeOffset JoinedAt);
