namespace WU.Application.Clubs;

public sealed record ClubRosterResponse(
    ClubInfo Club,
    IReadOnlyList<ClubMemberSummary> Members,
    bool IncludeOffline);
