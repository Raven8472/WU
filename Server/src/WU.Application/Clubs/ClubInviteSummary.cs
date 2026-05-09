using WU.Domain.Clubs;

namespace WU.Application.Clubs;

public sealed record ClubInviteSummary(
    Guid InviteId,
    Guid ClubId,
    Guid InvitedCharacterId,
    Guid? InvitedByCharacterId,
    EWuClubInviteStatus Status,
    DateTimeOffset CreatedAt,
    DateTimeOffset? ExpiresAt,
    DateTimeOffset? RespondedAt);
