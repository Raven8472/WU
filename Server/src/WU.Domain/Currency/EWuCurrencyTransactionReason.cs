namespace WU.Domain.Currency;

public enum EWuCurrencyTransactionReason
{
    BankDeposit,
    BankWithdrawal,
    PlayerTransfer,
    VendorPurchase,
    VendorSale,
    SystemGrant,
    AdminAdjustment
}
