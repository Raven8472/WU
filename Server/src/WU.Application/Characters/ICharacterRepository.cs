namespace WU.Application.Characters;

public interface ICharacterRepository
{
    Task<CharacterSummary> CreateAsync(CreateCharacterCommand command, CancellationToken cancellationToken);

    Task<IReadOnlyList<CharacterSummary>> ListForAccountRealmAsync(Guid accountId, Guid realmId, CancellationToken cancellationToken);

    Task<bool> DeleteAsync(Guid accountId, Guid realmId, Guid characterId, CancellationToken cancellationToken);

    Task<CharacterSummary?> UpdateLocationAsync(Guid accountId, Guid realmId, Guid characterId, CharacterLocation location, CancellationToken cancellationToken);

    Task<CharacterSummary?> AwardExperienceAsync(Guid accountId, Guid realmId, Guid characterId, int amount, CancellationToken cancellationToken);
}
