namespace WU.Application.Currency;

public sealed record PlayerCurrencyTransferRequest(
    Guid AccountId,
    Guid RealmId,
    Guid FromCharacterId,
    Guid ToCharacterId,
    long AmountKnuts,
    string? Note);
