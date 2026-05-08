namespace WU.Application.Currency;

public sealed record CurrencyOperationResult(
    CurrencyOperationStatus Status,
    CurrencySnapshot? Snapshot,
    CurrencyWalletSummary? TargetCharacterWallet,
    CurrencyTransactionSummary? Transaction,
    IReadOnlyList<string> Errors)
{
    public static CurrencyOperationResult Completed(
        CurrencySnapshot snapshot,
        CurrencyTransactionSummary transaction,
        CurrencyWalletSummary? targetCharacterWallet = null)
    {
        return new CurrencyOperationResult(CurrencyOperationStatus.Completed, snapshot, targetCharacterWallet, transaction, []);
    }

    public static CurrencyOperationResult Invalid(params string[] errors)
    {
        return new CurrencyOperationResult(CurrencyOperationStatus.InvalidRequest, null, null, null, errors);
    }

    public static CurrencyOperationResult CharacterNotFound()
    {
        return new CurrencyOperationResult(CurrencyOperationStatus.CharacterNotFound, null, null, null, []);
    }

    public static CurrencyOperationResult TargetCharacterNotFound()
    {
        return new CurrencyOperationResult(CurrencyOperationStatus.TargetCharacterNotFound, null, null, null, []);
    }

    public static CurrencyOperationResult InsufficientFunds()
    {
        return new CurrencyOperationResult(CurrencyOperationStatus.InsufficientFunds, null, null, null, []);
    }
}
