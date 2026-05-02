namespace WU.Application.Characters;

public sealed class CharacterExperienceService(ICharacterRepository repository)
{
    public async Task<CharacterExperienceResult> AwardAsync(Guid characterId, AwardCharacterExperienceRequest request, CancellationToken cancellationToken)
    {
        var validationErrors = Validate(characterId, request);
        if (validationErrors.Count > 0)
        {
            return CharacterExperienceResult.Invalid(validationErrors.ToArray());
        }

        var character = await repository.AwardExperienceAsync(
            request.AccountId,
            request.RealmId,
            characterId,
            request.Amount,
            cancellationToken);

        return character is null
            ? CharacterExperienceResult.NotFound()
            : CharacterExperienceResult.Awarded(character);
    }

    private static List<string> Validate(Guid characterId, AwardCharacterExperienceRequest request)
    {
        List<string> errors = [];

        if (characterId == Guid.Empty)
        {
            errors.Add("characterId is required.");
        }

        if (request.AccountId == Guid.Empty)
        {
            errors.Add("accountId is required.");
        }

        if (request.RealmId == Guid.Empty)
        {
            errors.Add("realmId is required.");
        }

        if (request.Amount <= 0)
        {
            errors.Add("amount must be greater than zero.");
        }

        return errors;
    }
}
