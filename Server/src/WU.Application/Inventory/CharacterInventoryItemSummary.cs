namespace WU.Application.Inventory;

public sealed record CharacterInventoryItemSummary(
    int SlotIndex,
    string ItemId,
    int Quantity);
