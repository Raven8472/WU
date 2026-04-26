namespace WU.Application.Auth;

public sealed record CurrentSessionResult(
    CurrentSessionStatus Status,
    CurrentSession? Response)
{
    public static CurrentSessionResult Found(CurrentSession session)
    {
        return new CurrentSessionResult(CurrentSessionStatus.Found, session);
    }

    public static CurrentSessionResult InvalidToken()
    {
        return new CurrentSessionResult(CurrentSessionStatus.InvalidToken, null);
    }
}
