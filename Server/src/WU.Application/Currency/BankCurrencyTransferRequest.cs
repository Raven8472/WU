namespace WU.Application.Currency;

public sealed record BankCurrencyTransferRequest(
    Guid AccountId,
    Guid RealmId,
    Guid CharacterId,
    long AmountKnuts,
    string? Note);
