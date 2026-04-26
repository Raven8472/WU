using WU.Application.Backend;
using WU.Application.Characters;
using WU.Domain.Characters;
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

app.Run();
