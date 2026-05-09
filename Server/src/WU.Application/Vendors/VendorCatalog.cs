namespace WU.Application.Vendors;

public static class VendorCatalog
{
    public const string ClubVendorTableId = "vendor_table_01";
    public const string ClubCharterItemId = "club_charter";
    public const long ClubCharterPriceKnuts = 563;

    private static readonly VendorCatalogTable ClubVendorTable = new(
        ClubVendorTableId,
        "Club Vendor",
        [
            new VendorCatalogItem(
                ClubCharterItemId,
                "Club Charter",
                ClubCharterPriceKnuts)
        ]);

    public static IReadOnlyList<VendorCatalogTable> Tables { get; } = [ClubVendorTable];

    public static VendorCatalogTable? FindTable(string? vendorTableId)
    {
        var normalizedVendorTableId = NormalizeId(vendorTableId);
        return Tables.FirstOrDefault(table => string.Equals(table.VendorTableId, normalizedVendorTableId, StringComparison.Ordinal));
    }

    public static VendorCatalogItem? FindItem(string? vendorTableId, string? itemId)
    {
        var table = FindTable(vendorTableId);
        var normalizedItemId = NormalizeId(itemId);
        return table?.Items.FirstOrDefault(item => string.Equals(item.ItemId, normalizedItemId, StringComparison.Ordinal));
    }

    private static string NormalizeId(string? value)
    {
        return (value ?? string.Empty).Trim().ToLowerInvariant();
    }
}
