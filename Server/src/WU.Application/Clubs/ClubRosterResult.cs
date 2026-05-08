namespace WU.Application.Clubs;

public sealed record ClubRosterResult(
    ClubRosterStatus Status,
    ClubRosterResponse? Roster,
    IReadOnlyList<string> Errors)
{
    public static ClubRosterResult Found(ClubRosterResponse roster)
    {
        return new ClubRosterResult(ClubRosterStatus.Found, roster, []);
    }

    public static ClubRosterResult Invalid(params string[] errors)
    {
        return new ClubRosterResult(ClubRosterStatus.InvalidRequest, null, errors);
    }

    public static ClubRosterResult ClubNotFound()
    {
        return new ClubRosterResult(ClubRosterStatus.ClubNotFound, null, []);
    }

    public static ClubRosterResult ViewerNotMember()
    {
        return new ClubRosterResult(ClubRosterStatus.ViewerNotMember, null, []);
    }
}
