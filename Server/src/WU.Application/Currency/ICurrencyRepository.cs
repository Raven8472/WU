namespace WU.Application.Currency;

public interface ICurrencyRepository
{
    Task<CurrencySnapshotResult> GetSnapshotAsync(Guid accountId, Guid realmId, Guid characterId, CancellationToken cancellationToken);

    Task<CurrencyOperationResult> DepositToBankAsync(Guid accountId, Guid realmId, Guid characterId, long amountKnuts, string note, CancellationToken cancellationToken);

    Task<CurrencyOperationResult> WithdrawFromBankAsync(Guid accountId, Guid realmId, Guid characterId, long amountKnuts, string note, CancellationToken cancellationToken);

    Task<CurrencyOperationResult> TransferToCharacterAsync(Guid accountId, Guid realmId, Guid fromCharacterId, Guid toCharacterId, long amountKnuts, string note, CancellationToken cancellationToken);

    Task<CurrencyOperationResult> SpendFromCharacterAsync(Guid accountId, Guid realmId, Guid characterId, long amountKnuts, string note, CancellationToken cancellationToken);
}
