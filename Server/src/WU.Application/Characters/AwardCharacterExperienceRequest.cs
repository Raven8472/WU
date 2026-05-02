using WU.Domain.Characters;

namespace WU.Application.Characters;

public sealed record AwardCharacterExperienceRequest(
    Guid AccountId,
    Guid RealmId,
    int Amount,
    CharacterExperienceSource Source);
