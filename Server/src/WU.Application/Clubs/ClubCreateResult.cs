namespace WU.Application.Clubs;

public sealed record ClubCreateResult(
    ClubCreateStatus Status,
    CharacterClubSummary? Club,
    IReadOnlyList<string> Errors)
{
    public static ClubCreateResult Created(CharacterClubSummary club)
    {
        return new ClubCreateResult(ClubCreateStatus.Created, club, []);
    }

    public static ClubCreateResult Invalid(params string[] errors)
    {
        return new ClubCreateResult(ClubCreateStatus.InvalidRequest, null, errors);
    }

    public static ClubCreateResult PresidentNotFound()
    {
        return new ClubCreateResult(ClubCreateStatus.PresidentNotFound, null, []);
    }

    public static ClubCreateResult PresidentAlreadyInClub()
    {
        return new ClubCreateResult(ClubCreateStatus.PresidentAlreadyInClub, null, []);
    }

    public static ClubCreateResult NameTaken()
    {
        return new ClubCreateResult(ClubCreateStatus.NameTaken, null, []);
    }

    public static ClubCreateResult TagTaken()
    {
        return new ClubCreateResult(ClubCreateStatus.TagTaken, null, []);
    }
}
