namespace WU.Application.Inventory;

public interface ICharacterInventoryRepository
{
    Task<CharacterInventorySnapshot?> GetSnapshotAsync(
        Guid accountId,
        Guid realmId,
        Guid characterId,
        CancellationToken cancellationToken);

    Task<CharacterInventorySnapshot?> AddPurchasedItemAsync(
        Guid accountId,
        Guid realmId,
        Guid characterId,
        string itemId,
        CancellationToken cancellationToken);
}
