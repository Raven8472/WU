namespace WU.Application.World;

public sealed record WorldTimeResponse(
    DateTimeOffset Utc,
    DateTimeOffset ServerLocal,
    string ServerTimeZoneId,
    string ServerTimeZoneDisplayName,
    double WorldDayProgress,
    double WorldSecondsOfDay,
    double WorldDayLengthSeconds,
    string WorldClock);
