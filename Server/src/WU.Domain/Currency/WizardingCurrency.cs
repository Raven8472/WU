namespace WU.Domain.Currency;

public static class WizardingCurrency
{
    public const int KnutsPerSickle = 29;
    public const int SicklesPerGalleon = 17;
    public const int KnutsPerGalleon = KnutsPerSickle * SicklesPerGalleon;

    public static WizardingCurrencyBreakdown BreakDown(long balanceKnuts)
    {
        var normalizedBalance = Math.Max(0, balanceKnuts);
        var galleons = normalizedBalance / KnutsPerGalleon;
        var remainderAfterGalleons = normalizedBalance % KnutsPerGalleon;
        var sickles = remainderAfterGalleons / KnutsPerSickle;
        var knuts = remainderAfterGalleons % KnutsPerSickle;

        return new WizardingCurrencyBreakdown(normalizedBalance, galleons, sickles, knuts);
    }
}
