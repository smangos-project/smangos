namespace WorldMapStudio.WoW;

/// <summary>A `world_safe_locs` row as a streamed scene entity: a point marker where a ghost can
/// be released.</summary>
public sealed class WowGraveyard : SceneEntity
{
    /// <summary>`world_safe_locs.id`; null until the entity is first committed.</summary>
    public uint? LocId { get; set; }

    public override string DisplayName => $"Graveyard {LocId}";
}
