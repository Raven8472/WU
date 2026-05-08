namespace WU.Domain.Clubs;

[Flags]
public enum EWuClubPermission
{
    None = 0,
    Invite = 1 << 0,
    Uninvite = 1 << 1,
    Kick = 1 << 2,
    Promote = 1 << 3,
    Demote = 1 << 4,
    EditPublicNote = 1 << 5,
    EditOfficerNote = 1 << 6,
    ManagePreferences = 1 << 7
}
