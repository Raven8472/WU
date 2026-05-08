namespace WU.Application.Clubs;

public enum ClubInviteResultStatus
{
    Invited,
    InvalidRequest,
    ClubNotFound,
    InviterNotMember,
    InviterNotAllowed,
    InviteeNotFound,
    InviteeAlreadyInClub,
    InviteAlreadyPending
}
