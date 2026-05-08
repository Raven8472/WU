namespace WU.Application.Clubs;

public sealed record InviteClubMemberRequest(
    Guid InviterCharacterId,
    Guid InvitedCharacterId);
