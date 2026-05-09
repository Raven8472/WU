namespace WU.Application.Inventory;

public sealed record CharacterInventorySnapshot(
    Guid CharacterId,
    IReadOnlyList<CharacterInventoryItemSummary> Items);
