namespace WU.Application.Characters;

public sealed class CharacterDeletionService(ICharacterRepository repository)
{
    public async Task<CharacterDeletionResult> DeleteAsync(Guid accountId, Guid realmId, Guid characterId, CancellationToken cancellationToken)
    {
        List<string> errors = [];

        if (accountId == Guid.Empty)
        {
            errors.Add("accountId is required.");
        }

        if (realmId == Guid.Empty)
        {
            errors.Add("realmId is required.");
        }

        if (characterId == Guid.Empty)
        {
            errors.Add("characterId is required.");
        }

        if (errors.Count > 0)
        {
            return CharacterDeletionResult.Invalid(errors.ToArray());
        }

        return await repository.DeleteAsync(accountId, realmId, characterId, cancellationToken)
            ? CharacterDeletionResult.Deleted()
            : CharacterDeletionResult.NotFound();
    }
}
