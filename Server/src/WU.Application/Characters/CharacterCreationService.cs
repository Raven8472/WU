using WU.Domain.Characters;

namespace WU.Application.Characters;

public sealed class CharacterCreationService(ICharacterRepository repository)
{
    public async Task<CharacterCreationResult> CreateAsync(CreateCharacterRequest request, CancellationToken cancellationToken)
    {
        var validationErrors = Validate(request);
        if (validationErrors.Count > 0)
        {
            return new CharacterCreationResult(CharacterCreationStatus.InvalidRequest, null, validationErrors);
        }

        var normalizedName = CharacterNameRules.Normalize(request.Name);
        var command = new CreateCharacterCommand(
            request.AccountId,
            request.RealmId,
            request.Name.Trim(),
            normalizedName,
            request.Race,
            request.Sex,
            request.Appearance!);

        try
        {
            var character = await repository.CreateAsync(command, cancellationToken);
            return CharacterCreationResult.Created(character);
        }
        catch (DuplicateCharacterNameException)
        {
            return CharacterCreationResult.DuplicateName();
        }
    }

    private static List<string> Validate(CreateCharacterRequest request)
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

        var name = request.Name.Trim();
        if (!CharacterNameRules.IsValid(name))
        {
            errors.Add($"name must be {CharacterNameRules.MinLength}-{CharacterNameRules.MaxLength} letters.");
        }

        if (!Enum.IsDefined(request.Race))
        {
            errors.Add("race is invalid.");
        }

        if (!Enum.IsDefined(request.Sex))
        {
            errors.Add("sex is invalid.");
        }

        if (request.Appearance is null)
        {
            errors.Add("appearance is required.");
            return errors;
        }

        if (request.Appearance.SkinPresetIndex < 0)
        {
            errors.Add("skinPresetIndex must be zero or greater.");
        }

        if (request.Appearance.HeadPresetIndex < 0)
        {
            errors.Add("headPresetIndex must be zero or greater.");
        }

        if (request.Appearance.HairStyleIndex < 0)
        {
            errors.Add("hairStyleIndex must be zero or greater.");
        }

        if (request.Appearance.HairColorIndex < 0)
        {
            errors.Add("hairColorIndex must be zero or greater.");
        }

        if (request.Appearance.EyeColorIndex < 0)
        {
            errors.Add("eyeColorIndex must be zero or greater.");
        }

        if (request.Appearance.BrowStyleIndex < 0)
        {
            errors.Add("browStyleIndex must be zero or greater.");
        }

        if (request.Appearance.BeardStyleIndex < 0)
        {
            errors.Add("beardStyleIndex must be zero or greater.");
        }

        return errors;
    }
}
