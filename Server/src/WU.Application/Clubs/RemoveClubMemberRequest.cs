namespace WU.Application.Clubs;

public sealed record RemoveClubMemberRequest(
    Guid ActorCharacterId,
    Guid MemberCharacterId);
