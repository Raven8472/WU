namespace WU.Application.Clubs;

public sealed class ClubService(IClubRepository repository)
{
    public async Task<ClubCreateResult> CreateAsync(CreateClubRequest request, CancellationToken cancellationToken)
    {
        var validationErrors = ValidateCreate(request);
        if (validationErrors.Count > 0)
        {
            return ClubCreateResult.Invalid(validationErrors.ToArray());
        }

        var name = request.Name.Trim();
        var tag = (request.Tag ?? string.Empty).Trim();
        var description = (request.Description ?? string.Empty).Trim();

        var command = new CreateClubCommand(
            request.AccountId,
            request.RealmId,
            request.PresidentCharacterId,
            name,
            ClubNameRules.NormalizeName(name),
            tag,
            ClubNameRules.NormalizeTag(tag),
            description);

        return await repository.CreateAsync(command, cancellationToken);
    }

    public async Task<ClubInviteResult> InviteAsync(Guid clubId, InviteClubMemberRequest request, CancellationToken cancellationToken)
    {
        List<string> errors = [];

        if (clubId == Guid.Empty)
        {
            errors.Add("clubId is required.");
        }

        if (request.InviterCharacterId == Guid.Empty)
        {
            errors.Add("inviterCharacterId is required.");
        }

        if (request.InvitedCharacterId == Guid.Empty)
        {
            errors.Add("invitedCharacterId is required.");
        }

        if (request.InviterCharacterId == request.InvitedCharacterId)
        {
            errors.Add("a character cannot invite itself.");
        }

        if (errors.Count > 0)
        {
            return ClubInviteResult.Invalid(errors.ToArray());
        }

        return await repository.InviteAsync(clubId, request.InviterCharacterId, request.InvitedCharacterId, cancellationToken);
    }

    public async Task<ClubRosterResult> GetRosterAsync(Guid clubId, Guid viewerCharacterId, bool includeOffline, CancellationToken cancellationToken)
    {
        List<string> errors = [];

        if (clubId == Guid.Empty)
        {
            errors.Add("clubId is required.");
        }

        if (viewerCharacterId == Guid.Empty)
        {
            errors.Add("viewerCharacterId is required.");
        }

        if (errors.Count > 0)
        {
            return ClubRosterResult.Invalid(errors.ToArray());
        }

        return await repository.GetRosterAsync(clubId, viewerCharacterId, includeOffline, cancellationToken);
    }

    private static List<string> ValidateCreate(CreateClubRequest request)
    {
        List<string> errors = [];

        if (request.AccountId == Guid.Empty)
        {
            errors.Add("accountId is required.");
        }

        if (request.RealmId == Guid.Empty)
        {
            errors.Add("realmId is required.");
        }

        if (request.PresidentCharacterId == Guid.Empty)
        {
            errors.Add("presidentCharacterId is required.");
        }

        if (string.IsNullOrWhiteSpace(request.Name) || !ClubNameRules.IsValidName(request.Name))
        {
            errors.Add($"name must be {ClubNameRules.MinNameLength}-{ClubNameRules.MaxNameLength} characters.");
        }

        if (!ClubNameRules.IsValidTag(request.Tag))
        {
            errors.Add($"tag must be empty or {ClubNameRules.MinTagLength}-{ClubNameRules.MaxTagLength} characters.");
        }

        if ((request.Description ?? string.Empty).Trim().Length > ClubNameRules.MaxDescriptionLength)
        {
            errors.Add($"description must be {ClubNameRules.MaxDescriptionLength} characters or fewer.");
        }

        return errors;
    }
}
