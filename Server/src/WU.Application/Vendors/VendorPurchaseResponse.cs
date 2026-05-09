using WU.Application.Currency;
using WU.Application.Inventory;

namespace WU.Application.Vendors;

public sealed record VendorPurchaseResponse(
    string VendorTableId,
    string ItemId,
    string DisplayName,
    long PriceKnuts,
    CurrencySnapshot Snapshot,
    CurrencyTransactionSummary Transaction,
    CharacterInventorySnapshot Inventory);
