namespace WU.Application.Currency;

public sealed class CurrencyService(ICurrencyRepository repository)
{
    public async Task<CurrencySnapshotResult> GetSnapshotAsync(Guid accountId, Guid realmId, Guid characterId, CancellationToken cancellationToken)
    {
        var errors = ValidateContext(accountId, realmId, characterId);
        if (errors.Count > 0)
        {
            return CurrencySnapshotResult.Invalid(errors.ToArray());
        }

        return await repository.GetSnapshotAsync(accountId, realmId, characterId, cancellationToken);
    }

    public async Task<CurrencyOperationResult> DepositToBankAsync(BankCurrencyTransferRequest request, CancellationToken cancellationToken)
    {
        var errors = ValidateBankTransfer(request);
        if (errors.Count > 0)
        {
            return CurrencyOperationResult.Invalid(errors.ToArray());
        }

        return await repository.DepositToBankAsync(
            request.AccountId,
            request.RealmId,
            request.CharacterId,
            request.AmountKnuts,
            NormalizeNote(request.Note),
            cancellationToken);
    }

    public async Task<CurrencyOperationResult> WithdrawFromBankAsync(BankCurrencyTransferRequest request, CancellationToken cancellationToken)
    {
        var errors = ValidateBankTransfer(request);
        if (errors.Count > 0)
        {
            return CurrencyOperationResult.Invalid(errors.ToArray());
        }

        return await repository.WithdrawFromBankAsync(
            request.AccountId,
            request.RealmId,
            request.CharacterId,
            request.AmountKnuts,
            NormalizeNote(request.Note),
            cancellationToken);
    }

    public async Task<CurrencyOperationResult> TransferToCharacterAsync(PlayerCurrencyTransferRequest request, CancellationToken cancellationToken)
    {
        var errors = ValidatePlayerTransfer(request);
        if (errors.Count > 0)
        {
            return CurrencyOperationResult.Invalid(errors.ToArray());
        }

        return await repository.TransferToCharacterAsync(
            request.AccountId,
            request.RealmId,
            request.FromCharacterId,
            request.ToCharacterId,
            request.AmountKnuts,
            NormalizeNote(request.Note),
            cancellationToken);
    }

    private static List<string> ValidateContext(Guid accountId, Guid realmId, Guid characterId)
    {
        List<string> errors = [];

        if (accountId == Guid.Empty)
        {
            errors.Add("accountId is required.");
        }

        if (realmId == Guid.Empty)
        {
            errors.Add("realmId is required.");
        }

        if (characterId == Guid.Empty)
        {
            errors.Add("characterId is required.");
        }

        return errors;
    }

    private static List<string> ValidateBankTransfer(BankCurrencyTransferRequest request)
    {
        var errors = ValidateContext(request.AccountId, request.RealmId, request.CharacterId);
        ValidateAmountAndNote(request.AmountKnuts, request.Note, errors);
        return errors;
    }

    private static List<string> ValidatePlayerTransfer(PlayerCurrencyTransferRequest request)
    {
        var errors = ValidateContext(request.AccountId, request.RealmId, request.FromCharacterId);

        if (request.ToCharacterId == Guid.Empty)
        {
            errors.Add("toCharacterId is required.");
        }

        if (request.FromCharacterId == request.ToCharacterId)
        {
            errors.Add("fromCharacterId and toCharacterId must be different.");
        }

        ValidateAmountAndNote(request.AmountKnuts, request.Note, errors);
        return errors;
    }

    private static void ValidateAmountAndNote(long amountKnuts, string? note, List<string> errors)
    {
        if (amountKnuts <= 0)
        {
            errors.Add("amountKnuts must be greater than zero.");
        }

        if ((note ?? string.Empty).Trim().Length > 256)
        {
            errors.Add("note must be 256 characters or fewer.");
        }
    }

    private static string NormalizeNote(string? note)
    {
        return (note ?? string.Empty).Trim();
    }
}
