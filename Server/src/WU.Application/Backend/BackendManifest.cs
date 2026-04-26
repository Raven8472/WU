namespace WU.Application.Backend;

public sealed record BackendManifest(
    string ServiceName,
    string Version,
    string RuntimeMode,
    IReadOnlyList<string> Modules);
