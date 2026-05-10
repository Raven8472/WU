namespace WU.Application.Clubs;

public sealed record ClubMemberRemovalResult(
    ClubMemberRemovalStatus Status,
    IReadOnlyList<string> Errors)
{
    public static ClubMemberRemovalResult Removed()
    {
        return new ClubMemberRemovalResult(ClubMemberRemovalStatus.Removed, []);
    }

    public static ClubMemberRemovalResult Invalid(params string[] errors)
    {
        return new ClubMemberRemovalResult(ClubMemberRemovalStatus.InvalidRequest, errors);
    }

    public static ClubMemberRemovalResult ClubNotFound()
    {
        return new ClubMemberRemovalResult(ClubMemberRemovalStatus.ClubNotFound, []);
    }

    public static ClubMemberRemovalResult ActorNotMember()
    {
        return new ClubMemberRemovalResult(ClubMemberRemovalStatus.ActorNotMember, []);
    }

    public static ClubMemberRemovalResult ActorNotAllowed()
    {
        return new ClubMemberRemovalResult(ClubMemberRemovalStatus.ActorNotAllowed, []);
    }

    public static ClubMemberRemovalResult MemberNotFound()
    {
        return new ClubMemberRemovalResult(ClubMemberRemovalStatus.MemberNotFound, []);
    }

    public static ClubMemberRemovalResult CannotRemoveSelf()
    {
        return new ClubMemberRemovalResult(ClubMemberRemovalStatus.CannotRemoveSelf, []);
    }

    public static ClubMemberRemovalResult CannotRemovePresident()
    {
        return new ClubMemberRemovalResult(ClubMemberRemovalStatus.CannotRemovePresident, []);
    }
}
