using WU.Domain.Currency;

namespace WU.Application.Currency;

public sealed record CurrencyTransactionSummary(
    Guid TransactionId,
    Guid RealmId,
    EWuCurrencyWalletType FromWalletType,
    Guid? FromWalletId,
    EWuCurrencyWalletType ToWalletType,
    Guid? ToWalletId,
    WizardingCurrencyBreakdown Amount,
    EWuCurrencyTransactionReason Reason,
    Guid? InitiatedByCharacterId,
    string Note,
    DateTimeOffset CreatedAt);
