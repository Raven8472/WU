using System.Globalization;

namespace WU.Application.World;

public sealed class WorldTimeService
{
    private const double WorldDayLengthSeconds = 24.0 * 60.0 * 60.0;
    private static readonly TimeZoneInfo CentralTimeZone = ResolveCentralTimeZone();

    public WorldTimeResponse GetCurrent()
    {
        var utc = DateTimeOffset.UtcNow;
        var serverLocal = TimeZoneInfo.ConvertTime(utc, CentralTimeZone);
        var worldSecondsOfDay = serverLocal.TimeOfDay.TotalSeconds;
        var worldDayProgress = worldSecondsOfDay / WorldDayLengthSeconds;

        return new WorldTimeResponse(
            utc,
            serverLocal,
            CentralTimeZone.Id,
            CentralTimeZone.DisplayName,
            Math.Clamp(worldDayProgress, 0.0, 1.0),
            worldSecondsOfDay,
            WorldDayLengthSeconds,
            serverLocal.ToString("HH:mm", CultureInfo.InvariantCulture));
    }

    private static TimeZoneInfo ResolveCentralTimeZone()
    {
        return TryFindTimeZone("Central Standard Time")
            ?? TryFindTimeZone("America/Chicago")
            ?? TimeZoneInfo.Local;
    }

    private static TimeZoneInfo? TryFindTimeZone(string id)
    {
        try
        {
            return TimeZoneInfo.FindSystemTimeZoneById(id);
        }
        catch (TimeZoneNotFoundException)
        {
            return null;
        }
        catch (InvalidTimeZoneException)
        {
            return null;
        }
    }
}
