namespace WU.Application.Auth;

public sealed record AuthLoginResult(
    AuthLoginStatus Status,
    AuthLoginResponse? Response)
{
    public static AuthLoginResult SignedIn(AuthLoginResponse response)
    {
        return new AuthLoginResult(AuthLoginStatus.SignedIn, response);
    }

    public static AuthLoginResult DevAccountMissing()
    {
        return new AuthLoginResult(AuthLoginStatus.DevAccountMissing, null);
    }
}
