namespace WU.Application.Characters;

public interface ICharacterRepository
{
    Task<CharacterSummary> CreateAsync(CreateCharacterCommand command, CancellationToken cancellationToken);
}
