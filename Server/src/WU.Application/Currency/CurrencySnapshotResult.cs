namespace WU.Application.Currency;

public sealed record CurrencySnapshotResult(
    CurrencySnapshotStatus Status,
    CurrencySnapshot? Snapshot,
    IReadOnlyList<string> Errors)
{
    public static CurrencySnapshotResult Found(CurrencySnapshot snapshot)
    {
        return new CurrencySnapshotResult(CurrencySnapshotStatus.Found, snapshot, []);
    }

    public static CurrencySnapshotResult Invalid(params string[] errors)
    {
        return new CurrencySnapshotResult(CurrencySnapshotStatus.InvalidRequest, null, errors);
    }

    public static CurrencySnapshotResult CharacterNotFound()
    {
        return new CurrencySnapshotResult(CurrencySnapshotStatus.CharacterNotFound, null, []);
    }
}
