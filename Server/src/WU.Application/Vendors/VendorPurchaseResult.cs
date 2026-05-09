namespace WU.Application.Vendors;

public sealed record VendorPurchaseResult(
    VendorPurchaseStatus Status,
    VendorPurchaseResponse? Purchase,
    IReadOnlyList<string> Errors)
{
    public static VendorPurchaseResult Purchased(VendorPurchaseResponse purchase)
    {
        return new VendorPurchaseResult(VendorPurchaseStatus.Purchased, purchase, []);
    }

    public static VendorPurchaseResult Invalid(params string[] errors)
    {
        return new VendorPurchaseResult(VendorPurchaseStatus.InvalidRequest, null, errors);
    }

    public static VendorPurchaseResult ItemNotFound()
    {
        return new VendorPurchaseResult(VendorPurchaseStatus.ItemNotFound, null, []);
    }

    public static VendorPurchaseResult CharacterNotFound()
    {
        return new VendorPurchaseResult(VendorPurchaseStatus.CharacterNotFound, null, []);
    }

    public static VendorPurchaseResult InsufficientFunds()
    {
        return new VendorPurchaseResult(VendorPurchaseStatus.InsufficientFunds, null, []);
    }
}
