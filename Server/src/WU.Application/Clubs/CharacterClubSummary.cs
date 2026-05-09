using WU.Domain.Clubs;

namespace WU.Application.Clubs;

public sealed record CharacterClubSummary(
    Guid ClubId,
    string Name,
    string Tag,
    EWuClubRank Rank,
    int PermissionsMask,
    string PublicNote,
    string OfficerNote);
