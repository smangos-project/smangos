using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>A `world_safe_locs` row as a streamed scene entity: a point marker where a ghost can
/// be released.</summary>
public sealed class WowGraveyard : SceneEntity
{
    /// <summary>`world_safe_locs.id`; null until the entity is first committed.</summary>
    public uint? LocId { get; set; }

    /// <summary>The `game_graveyard_zone` rows linking this location to the zones whose ghosts it serves.</summary>
    public List<WowGraveyardZone> Zones { get; set; } = [];

    public override string DisplayName => $"Graveyard {LocId}";
}

/// <summary>A `game_graveyard_zone` link; <see cref="Faction"/> 0 serves both factions.</summary>
public readonly record struct WowGraveyardZone(int GhostZone, int Faction);
