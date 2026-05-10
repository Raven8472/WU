namespace WU.Application.Clubs;

public interface IClubRepository
{
    Task<ClubCreateResult> CreateAsync(CreateClubCommand command, CancellationToken cancellationToken);

    Task<ClubCreateResult> CreateFromCharterAsync(
        CreateClubCommand command,
        int charterSlotIndex,
        string charterItemId,
        CancellationToken cancellationToken);

    Task<ClubInviteResult> InviteAsync(Guid clubId, Guid inviterCharacterId, Guid invitedCharacterId, CancellationToken cancellationToken);

    Task<ClubRosterResult> GetRosterAsync(Guid clubId, Guid viewerCharacterId, bool includeOffline, CancellationToken cancellationToken);
}
