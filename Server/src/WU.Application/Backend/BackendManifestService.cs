namespace WU.Application.Backend;

public sealed class BackendManifestService
{
    public BackendManifest GetManifest()
    {
        return new BackendManifest(
            ServiceName: "WU.Persistence",
            Version: "0.1.0",
            RuntimeMode: "modular-monolith",
            Modules:
            [
                "Auth",
                "Characters",
                "Realms",
                "Zones",
                "Inventory",
                "Bank",
                "Mail",
                "Guilds",
                "Auction",
                "Quests",
                "Crafting"
            ]);
    }
}
