using WU.Application.Currency;
using WU.Application.Inventory;

namespace WU.Application.Vendors;

public sealed class VendorService(ICurrencyRepository currencyRepository, ICharacterInventoryRepository inventoryRepository)
{
    public VendorCatalogTable? FindTable(string vendorTableId)
    {
        return VendorCatalog.FindTable(vendorTableId);
    }

    public async Task<VendorPurchaseResult> PurchaseAsync(VendorPurchaseRequest request, CancellationToken cancellationToken)
    {
        var errors = ValidatePurchase(request);
        if (errors.Count > 0)
        {
            return VendorPurchaseResult.Invalid(errors.ToArray());
        }

        var item = VendorCatalog.FindItem(request.VendorTableId, request.ItemId);
        if (item is null)
        {
            return VendorPurchaseResult.ItemNotFound();
        }

        var currencyResult = await currencyRepository.SpendFromCharacterAsync(
            request.AccountId,
            request.RealmId,
            request.CharacterId,
            item.PriceKnuts,
            $"Vendor purchase: {item.ItemId}",
            cancellationToken);

        if (currencyResult.Status == CurrencyOperationStatus.Completed)
        {
            var inventory = await inventoryRepository.AddPurchasedItemAsync(
                request.AccountId,
                request.RealmId,
                request.CharacterId,
                item.ItemId,
                cancellationToken);

            if (inventory is null)
            {
                return VendorPurchaseResult.CharacterNotFound();
            }

            return VendorPurchaseResult.Purchased(new VendorPurchaseResponse(
                request.VendorTableId.Trim().ToLowerInvariant(),
                item.ItemId,
                item.DisplayName,
                item.PriceKnuts,
                currencyResult.Snapshot!,
                currencyResult.Transaction!,
                inventory));
        }

        return currencyResult.Status switch
        {
            CurrencyOperationStatus.CharacterNotFound => VendorPurchaseResult.CharacterNotFound(),
            CurrencyOperationStatus.InsufficientFunds => VendorPurchaseResult.InsufficientFunds(),
            CurrencyOperationStatus.InvalidRequest => VendorPurchaseResult.Invalid(currencyResult.Errors.ToArray()),
            _ => VendorPurchaseResult.Invalid("vendor purchase could not be completed.")
        };
    }

    private static List<string> ValidatePurchase(VendorPurchaseRequest request)
    {
        List<string> errors = [];

        if (request.AccountId == Guid.Empty)
        {
            errors.Add("accountId is required.");
        }

        if (request.RealmId == Guid.Empty)
        {
            errors.Add("realmId is required.");
        }

        if (request.CharacterId == Guid.Empty)
        {
            errors.Add("characterId is required.");
        }

        if (string.IsNullOrWhiteSpace(request.VendorTableId))
        {
            errors.Add("vendorTableId is required.");
        }

        if (string.IsNullOrWhiteSpace(request.ItemId))
        {
            errors.Add("itemId is required.");
        }

        return errors;
    }
}
