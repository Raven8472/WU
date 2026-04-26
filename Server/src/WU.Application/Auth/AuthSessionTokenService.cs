using System.Security.Cryptography;
using System.Text;

namespace WU.Application.Auth;

public sealed class AuthSessionTokenService
{
    public string CreateToken()
    {
        Span<byte> bytes = stackalloc byte[32];
        RandomNumberGenerator.Fill(bytes);
        return Base64UrlEncode(bytes);
    }

    public string HashToken(string token)
    {
        var bytes = Encoding.UTF8.GetBytes(token);
        var hash = SHA256.HashData(bytes);
        return Convert.ToHexString(hash).ToLowerInvariant();
    }

    private static string Base64UrlEncode(ReadOnlySpan<byte> bytes)
    {
        return Convert.ToBase64String(bytes)
            .TrimEnd('=')
            .Replace('+', '-')
            .Replace('/', '_');
    }
}
