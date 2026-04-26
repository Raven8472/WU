namespace WU.Application.Characters;

public sealed class CharacterLocationService(ICharacterRepository repository)
{
    public async Task<CharacterLocationResult> UpdateAsync(Guid characterId, UpdateCharacterLocationRequest request, CancellationToken cancellationToken)
    {
        var validationErrors = Validate(characterId, request);
        if (validationErrors.Count > 0)
        {
            return CharacterLocationResult.Invalid(validationErrors.ToArray());
        }

        var character = await repository.UpdateLocationAsync(
            request.AccountId,
            request.RealmId,
            characterId,
            request.Location!,
            cancellationToken);

        return character is null
            ? CharacterLocationResult.NotFound()
            : CharacterLocationResult.Updated(character);
    }

    private static List<string> Validate(Guid characterId, UpdateCharacterLocationRequest request)
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

        if (request.Location is null)
        {
            errors.Add("location is required.");
            return errors;
        }

        if (!float.IsFinite(request.Location.X) || !float.IsFinite(request.Location.Y) || !float.IsFinite(request.Location.Z))
        {
            errors.Add("location coordinates must be finite numbers.");
        }

        return errors;
    }
}
