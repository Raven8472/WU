using WU.Domain.Currency;

namespace WU.Application.Currency;

public sealed record CurrencyWalletSummary(
    Guid WalletId,
    EWuCurrencyWalletType WalletType,
    Guid OwnerId,
    WizardingCurrencyBreakdown Balance);
