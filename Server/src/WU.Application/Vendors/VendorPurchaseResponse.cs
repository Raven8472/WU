using WU.Application.Currency;

namespace WU.Application.Vendors;

public sealed record VendorPurchaseResponse(
    string VendorTableId,
    string ItemId,
    string DisplayName,
    long PriceKnuts,
    CurrencySnapshot Snapshot,
    CurrencyTransactionSummary Transaction);
