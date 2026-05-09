namespace WU.Application.Vendors;

public sealed record VendorCatalogTable(
    string VendorTableId,
    string DisplayName,
    IReadOnlyList<VendorCatalogItem> Items);
