using WU.Application.Backend;
using WU.Application.Auth;
using WU.Application.Characters;
using WU.Domain.Characters;
using WU.Infrastructure.Auth;
using WU.Infrastructure.Characters;
using WU.Infrastructure.Configuration;
using Npgsql;
using System.Text.Json.Serialization;

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddSingleton<BackendManifestService>();
builder.Services.ConfigureHttpJsonOptions(options =>
{
    options.SerializerOptions.Converters.Add(new JsonStringEnumConverter());
});
builder.Services.Configure<WuPersistenceOptions>(builder.Configuration.GetSection(WuPersistenceOptions.SectionName));
builder.Services.AddSingleton(sp =>
{
    var configuration = sp.GetRequiredService<IConfiguration>();
    var options = configuration.GetSection(WuPersistenceOptions.SectionName).Get<WuPersistenceOptions>() ?? new WuPersistenceOptions();
    return NpgsqlDataSource.Create(options.Postgres.ConnectionString);
});
builder.Services.AddScoped<ICharacterRepository, PostgresCharacterRepository>();
builder.Services.AddScoped<CharacterCreationService>();
builder.Services.AddScoped<CharacterQueryService>();
builder.Services.AddScoped<CharacterLocationService>();
builder.Services.AddScoped<CharacterDeletionService>();
builder.Services.AddScoped<IAuthRepository, PostgresAuthRepository>();
builder.Services.AddScoped<AuthSessionTokenService>();
builder.Services.AddScoped<AuthService>();

var app = builder.Build();

app.MapGet("/", () => Results.Redirect("/health/live"));

app.MapGet("/health/live", () => Results.Ok(new
{
    status = "ok",
    service = "WU.Persistence",
    utc = DateTimeOffset.UtcNow
}));

app.MapGet("/health/ready", (IConfiguration configuration) =>
{
    var settings = configuration.GetSection(WuPersistenceOptions.SectionName).Get<WuPersistenceOptions>() ?? new WuPersistenceOptions();
    var hasPostgres = !string.IsNullOrWhiteSpace(settings.Postgres.ConnectionString);
    var hasRedis = !string.IsNullOrWhiteSpace(settings.Redis.ConnectionString);

    return Results.Ok(new
    {
        status = hasPostgres && hasRedis ? "ready" : "needs-configuration",
        postgresConfigured = hasPostgres,
        redisConfigured = hasRedis
    });
});

app.MapGet("/api/backend/manifest", (BackendManifestService manifestService) =>
{
    return Results.Ok(manifestService.GetManifest());
});

app.MapPost("/api/auth/dev-login", async (AuthService service, CancellationToken cancellationToken) =>
{
    var result = await service.CreateDevelopmentLoginAsync(cancellationToken);

    return result.Status switch
    {
        AuthLoginStatus.SignedIn => Results.Ok(result.Response),
        AuthLoginStatus.DevAccountMissing => Results.Problem("The development account or realm seed data is missing."),
        _ => Results.Problem("Development login failed.")
    };
});

app.MapGet("/api/auth/me", async (HttpRequest request, AuthService service, CancellationToken cancellationToken) =>
{
    var token = BearerTokenReader.Read(request);
    if (string.IsNullOrWhiteSpace(token))
    {
        return Results.Unauthorized();
    }

    var result = await service.GetCurrentSessionAsync(token, cancellationToken);

    return result.Status switch
    {
        CurrentSessionStatus.Found => Results.Ok(result.Response),
        CurrentSessionStatus.InvalidToken => Results.Unauthorized(),
        _ => Results.Problem("The current session could not be loaded.")
    };
});

app.MapGet("/api/characters/schema", () => Results.Ok(new CharacterCreationSchema(
    MinNameLength: CharacterNameRules.MinLength,
    MaxNameLength: CharacterNameRules.MaxLength,
    AllowedNamePattern: CharacterNameRules.AllowedPattern,
    Races: Enum.GetNames<EWuCharacterRace>(),
    Sexes: Enum.GetNames<EWuCharacterSex>())));

app.MapPost("/api/characters", async (CreateCharacterRequest request, CharacterCreationService service, CancellationToken cancellationToken) =>
{
    var result = await service.CreateAsync(request, cancellationToken);

    return result.Status switch
    {
        CharacterCreationStatus.Created => Results.Created($"/api/characters/{result.Character!.CharacterId}", result.Character),
        CharacterCreationStatus.DuplicateName => Results.Conflict(new { error = "character_name_taken", message = "That character name is already taken on this realm." }),
        CharacterCreationStatus.InvalidRequest => Results.BadRequest(new { error = "invalid_character_request", messages = result.Errors }),
        _ => Results.Problem("The character could not be created.")
    };
});

app.MapGet("/api/accounts/{accountId:guid}/realms/{realmId:guid}/characters", async (Guid accountId, Guid realmId, CharacterQueryService service, CancellationToken cancellationToken) =>
{
    var result = await service.ListForAccountRealmAsync(accountId, realmId, cancellationToken);

    return result.Status switch
    {
        CharacterListStatus.Found => Results.Ok(result.Characters),
        CharacterListStatus.InvalidRequest => Results.BadRequest(new { error = "invalid_character_list_request", messages = result.Errors }),
        _ => Results.Problem("Characters could not be loaded.")
    };
});

app.MapDelete("/api/accounts/{accountId:guid}/realms/{realmId:guid}/characters/{characterId:guid}", async (Guid accountId, Guid realmId, Guid characterId, CharacterDeletionService service, CancellationToken cancellationToken) =>
{
    var result = await service.DeleteAsync(accountId, realmId, characterId, cancellationToken);

    return result.Status switch
    {
        CharacterDeletionStatus.Deleted => Results.NoContent(),
        CharacterDeletionStatus.NotFound => Results.NotFound(new { error = "character_not_found", message = "The character could not be found for that account and realm." }),
        CharacterDeletionStatus.InvalidRequest => Results.BadRequest(new { error = "invalid_character_delete_request", messages = result.Errors }),
        _ => Results.Problem("The character could not be deleted.")
    };
});

app.MapPut("/api/characters/{characterId:guid}/location", async (Guid characterId, UpdateCharacterLocationRequest request, CharacterLocationService service, CancellationToken cancellationToken) =>
{
    var result = await service.UpdateAsync(characterId, request, cancellationToken);

    return result.Status switch
    {
        CharacterLocationStatus.Updated => Results.Ok(result.Character),
        CharacterLocationStatus.NotFound => Results.NotFound(new { error = "character_not_found", message = "The character could not be found for that account and realm." }),
        CharacterLocationStatus.InvalidRequest => Results.BadRequest(new { error = "invalid_character_location_request", messages = result.Errors }),
        _ => Results.Problem("The character location could not be updated.")
    };
});

app.Run();

internal static class BearerTokenReader
{
    public static string? Read(HttpRequest request)
    {
        var header = request.Headers.Authorization.ToString();
        if (string.IsNullOrWhiteSpace(header))
        {
            return null;
        }

        const string prefix = "Bearer ";
        return header.StartsWith(prefix, StringComparison.OrdinalIgnoreCase)
            ? header[prefix.Length..].Trim()
            : null;
    }
}
