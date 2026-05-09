namespace WU.Application.Inventory;

public sealed class CharacterInventoryService(ICharacterInventoryRepository repository)
{
    public async Task<CharacterInventorySnapshot?> GetSnapshotAsync(Guid accountId, Guid realmId, Guid characterId, CancellationToken cancellationToken)
    {
        if (accountId == Guid.Empty || realmId == Guid.Empty || characterId == Guid.Empty)
        {
            return null;
        }

        return await repository.GetSnapshotAsync(accountId, realmId, characterId, cancellationToken);
    }
}
