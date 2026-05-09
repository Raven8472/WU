namespace WU.Application.Vendors;

public sealed record VendorCatalogItem(
    string ItemId,
    string DisplayName,
    long PriceKnuts);
