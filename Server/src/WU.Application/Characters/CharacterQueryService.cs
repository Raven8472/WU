namespace WU.Application.Characters;

public sealed class CharacterQueryService(ICharacterRepository repository)
{
    public async Task<CharacterListResult> ListForAccountRealmAsync(Guid accountId, Guid realmId, CancellationToken cancellationToken)
    {
        if (accountId == Guid.Empty)
        {
            return CharacterListResult.Invalid("accountId is required.");
        }

        if (realmId == Guid.Empty)
        {
            return CharacterListResult.Invalid("realmId is required.");
        }

        var characters = await repository.ListForAccountRealmAsync(accountId, realmId, cancellationToken);
        return CharacterListResult.Found(characters);
    }
}
