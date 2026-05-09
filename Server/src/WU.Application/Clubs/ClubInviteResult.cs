namespace WU.Application.Clubs;

public sealed record ClubInviteResult(
    ClubInviteResultStatus Status,
    ClubInviteSummary? Invite,
    IReadOnlyList<string> Errors)
{
    public static ClubInviteResult Invited(ClubInviteSummary invite)
    {
        return new ClubInviteResult(ClubInviteResultStatus.Invited, invite, []);
    }

    public static ClubInviteResult Invalid(params string[] errors)
    {
        return new ClubInviteResult(ClubInviteResultStatus.InvalidRequest, null, errors);
    }

    public static ClubInviteResult ClubNotFound()
    {
        return new ClubInviteResult(ClubInviteResultStatus.ClubNotFound, null, []);
    }

    public static ClubInviteResult InviterNotMember()
    {
        return new ClubInviteResult(ClubInviteResultStatus.InviterNotMember, null, []);
    }

    public static ClubInviteResult InviterNotAllowed()
    {
        return new ClubInviteResult(ClubInviteResultStatus.InviterNotAllowed, null, []);
    }

    public static ClubInviteResult InviteeNotFound()
    {
        return new ClubInviteResult(ClubInviteResultStatus.InviteeNotFound, null, []);
    }

    public static ClubInviteResult InviteeAlreadyInClub()
    {
        return new ClubInviteResult(ClubInviteResultStatus.InviteeAlreadyInClub, null, []);
    }

    public static ClubInviteResult InviteAlreadyPending()
    {
        return new ClubInviteResult(ClubInviteResultStatus.InviteAlreadyPending, null, []);
    }
}
