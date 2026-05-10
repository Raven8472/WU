namespace WU.Application.Clubs;

public enum ClubMemberRemovalStatus
{
    Removed,
    InvalidRequest,
    ClubNotFound,
    ActorNotMember,
    ActorNotAllowed,
    MemberNotFound,
    CannotRemoveSelf,
    CannotRemovePresident
}
