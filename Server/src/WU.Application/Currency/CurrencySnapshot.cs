namespace WU.Application.Currency;

public sealed record CurrencySnapshot(
    CurrencyWalletSummary CharacterWallet,
    CurrencyWalletSummary AccountBankWallet);
