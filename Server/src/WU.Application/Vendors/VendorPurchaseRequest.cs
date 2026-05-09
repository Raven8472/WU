namespace WU.Application.Vendors;

public sealed record VendorPurchaseRequest(
    Guid AccountId,
    Guid RealmId,
    Guid CharacterId,
    string VendorTableId,
    string ItemId);
