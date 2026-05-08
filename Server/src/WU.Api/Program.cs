using WU.Application.Backend;
using WU.Application.Auth;
using WU.Application.Characters;
using WU.Application.Clubs;
using WU.Application.Currency;
using WU.Domain.Characters;
using WU.Infrastructure.Auth;
using WU.Infrastructure.Characters;
using WU.Infrastructure.Clubs;
using WU.Infrastructure.Configuration;
using WU.Infrastructure.Currency;
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
builder.Services.AddScoped<CharacterExperienceService>();
builder.Services.AddScoped<CharacterDeletionService>();
builder.Services.AddScoped<IClubRepository, PostgresClubRepository>();
builder.Services.AddScoped<ClubService>();
builder.Services.AddScoped<ICurrencyRepository, PostgresCurrencyRepository>();
builder.Services.AddScoped<CurrencyService>();
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

app.MapPost("/api/characters/{characterId:guid}/experience", async (Guid characterId, AwardCharacterExperienceRequest request, CharacterExperienceService service, CancellationToken cancellationToken) =>
{
    var result = await service.AwardAsync(characterId, request, cancellationToken);

    return result.Status switch
    {
        CharacterExperienceStatus.Awarded => Results.Ok(result.Character),
        CharacterExperienceStatus.NotFound => Results.NotFound(new { error = "character_not_found", message = "The character could not be found for that account and realm." }),
        CharacterExperienceStatus.InvalidRequest => Results.BadRequest(new { error = "invalid_character_experience_request", messages = result.Errors }),
        _ => Results.Problem("The character experience could not be updated.")
    };
});

app.MapPost("/api/clubs", async (CreateClubRequest request, ClubService service, CancellationToken cancellationToken) =>
{
    var result = await service.CreateAsync(request, cancellationToken);

    return result.Status switch
    {
        ClubCreateStatus.Created => Results.Created($"/api/clubs/{result.Club!.ClubId}", result.Club),
        ClubCreateStatus.PresidentNotFound => Results.NotFound(new { error = "president_not_found", message = "The president character could not be found for that account and realm." }),
        ClubCreateStatus.PresidentAlreadyInClub => Results.Conflict(new { error = "president_already_in_club", message = "That character is already in a club." }),
        ClubCreateStatus.NameTaken => Results.Conflict(new { error = "club_name_taken", message = "That club name is already taken on this realm." }),
        ClubCreateStatus.TagTaken => Results.Conflict(new { error = "club_tag_taken", message = "That club tag is already taken on this realm." }),
        ClubCreateStatus.InvalidRequest => Results.BadRequest(new { error = "invalid_club_create_request", messages = result.Errors }),
        _ => Results.Problem("The club could not be created.")
    };
});

app.MapPost("/api/clubs/{clubId:guid}/invites", async (Guid clubId, InviteClubMemberRequest request, ClubService service, CancellationToken cancellationToken) =>
{
    var result = await service.InviteAsync(clubId, request, cancellationToken);

    return result.Status switch
    {
        ClubInviteResultStatus.Invited => Results.Created($"/api/clubs/{clubId}/invites/{result.Invite!.InviteId}", result.Invite),
        ClubInviteResultStatus.ClubNotFound => Results.NotFound(new { error = "club_not_found", message = "The club could not be found." }),
        ClubInviteResultStatus.InviterNotMember => Results.StatusCode(StatusCodes.Status403Forbidden),
        ClubInviteResultStatus.InviterNotAllowed => Results.StatusCode(StatusCodes.Status403Forbidden),
        ClubInviteResultStatus.InviteeNotFound => Results.NotFound(new { error = "invitee_not_found", message = "The invited character could not be found on this realm." }),
        ClubInviteResultStatus.InviteeAlreadyInClub => Results.Conflict(new { error = "invitee_already_in_club", message = "That character is already in a club." }),
        ClubInviteResultStatus.InviteAlreadyPending => Results.Conflict(new { error = "club_invite_pending", message = "That character already has a pending invite to this club." }),
        ClubInviteResultStatus.InvalidRequest => Results.BadRequest(new { error = "invalid_club_invite_request", messages = result.Errors }),
        _ => Results.Problem("The club invite could not be created.")
    };
});

app.MapGet("/api/clubs/{clubId:guid}/roster", async (Guid clubId, Guid viewerCharacterId, bool includeOffline, ClubService service, CancellationToken cancellationToken) =>
{
    var result = await service.GetRosterAsync(clubId, viewerCharacterId, includeOffline, cancellationToken);

    return result.Status switch
    {
        ClubRosterStatus.Found => Results.Ok(result.Roster),
        ClubRosterStatus.ClubNotFound => Results.NotFound(new { error = "club_not_found", message = "The club could not be found." }),
        ClubRosterStatus.ViewerNotMember => Results.StatusCode(StatusCodes.Status403Forbidden),
        ClubRosterStatus.InvalidRequest => Results.BadRequest(new { error = "invalid_club_roster_request", messages = result.Errors }),
        _ => Results.Problem("The club roster could not be loaded.")
    };
});

app.MapGet("/api/currency/accounts/{accountId:guid}/realms/{realmId:guid}/characters/{characterId:guid}", async (Guid accountId, Guid realmId, Guid characterId, CurrencyService service, CancellationToken cancellationToken) =>
{
    var result = await service.GetSnapshotAsync(accountId, realmId, characterId, cancellationToken);

    return result.Status switch
    {
        CurrencySnapshotStatus.Found => Results.Ok(result.Snapshot),
        CurrencySnapshotStatus.CharacterNotFound => Results.NotFound(new { error = "character_not_found", message = "The character could not be found for that account and realm." }),
        CurrencySnapshotStatus.InvalidRequest => Results.BadRequest(new { error = "invalid_currency_snapshot_request", messages = result.Errors }),
        _ => Results.Problem("The currency snapshot could not be loaded.")
    };
});

app.MapPost("/api/currency/bank/deposit", async (BankCurrencyTransferRequest request, CurrencyService service, CancellationToken cancellationToken) =>
{
    var result = await service.DepositToBankAsync(request, cancellationToken);

    return result.Status switch
    {
        CurrencyOperationStatus.Completed => Results.Ok(new { result.Snapshot, result.Transaction }),
        CurrencyOperationStatus.CharacterNotFound => Results.NotFound(new { error = "character_not_found", message = "The character could not be found for that account and realm." }),
        CurrencyOperationStatus.InsufficientFunds => Results.Conflict(new { error = "insufficient_funds", message = "The character wallet does not have enough Knuts." }),
        CurrencyOperationStatus.InvalidRequest => Results.BadRequest(new { error = "invalid_currency_deposit_request", messages = result.Errors }),
        _ => Results.Problem("The currency deposit could not be completed.")
    };
});

app.MapPost("/api/currency/bank/withdraw", async (BankCurrencyTransferRequest request, CurrencyService service, CancellationToken cancellationToken) =>
{
    var result = await service.WithdrawFromBankAsync(request, cancellationToken);

    return result.Status switch
    {
        CurrencyOperationStatus.Completed => Results.Ok(new { result.Snapshot, result.Transaction }),
        CurrencyOperationStatus.CharacterNotFound => Results.NotFound(new { error = "character_not_found", message = "The character could not be found for that account and realm." }),
        CurrencyOperationStatus.InsufficientFunds => Results.Conflict(new { error = "insufficient_funds", message = "The account bank wallet does not have enough Knuts." }),
        CurrencyOperationStatus.InvalidRequest => Results.BadRequest(new { error = "invalid_currency_withdraw_request", messages = result.Errors }),
        _ => Results.Problem("The currency withdrawal could not be completed.")
    };
});

app.MapPost("/api/currency/transfers/player", async (PlayerCurrencyTransferRequest request, CurrencyService service, CancellationToken cancellationToken) =>
{
    var result = await service.TransferToCharacterAsync(request, cancellationToken);

    return result.Status switch
    {
        CurrencyOperationStatus.Completed => Results.Ok(new { result.Snapshot, result.TargetCharacterWallet, result.Transaction }),
        CurrencyOperationStatus.CharacterNotFound => Results.NotFound(new { error = "character_not_found", message = "The sending character could not be found for that account and realm." }),
        CurrencyOperationStatus.TargetCharacterNotFound => Results.NotFound(new { error = "target_character_not_found", message = "The receiving character could not be found on that realm." }),
        CurrencyOperationStatus.InsufficientFunds => Results.Conflict(new { error = "insufficient_funds", message = "The sending character wallet does not have enough Knuts." }),
        CurrencyOperationStatus.InvalidRequest => Results.BadRequest(new { error = "invalid_currency_transfer_request", messages = result.Errors }),
        _ => Results.Problem("The currency transfer could not be completed.")
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
