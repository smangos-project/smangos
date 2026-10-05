namespace WorldMapStudio.WoW;

/// <summary>
/// A `game_tele` row as a streamed scene entity — a named destination marker, plain and point-shaped
/// like <see cref="WowTaxiNode"/>.: shares <see cref="WowPointMarker"/> with
/// <see cref="WowSpellTargetPosition"/> rather than each growing its own near-identical marker.
/// </summary>
public sealed class WowGameTele : SceneEntity
{
    /// <summary>`tc_game_tele.id`. Null until this entity has been committed at least once —
    /// `game_tele.id` is assigned by the database on insert,
    /// the same convention `WowCreatureSpawn.Guid` uses.</summary>
    public uint? TeleId { get; set; }

    // Base Name maps directly onto tc_game_tele.name — a real, user-facing label (1530 distinct names
    // over 1531 rows), so it is worth showing here rather than adding a second, redundant field, the
    // same choice WowSoundEmitter already makes for its own Name column.
    public override string DisplayName => Name.Length > 0 ? Name : $"Game Tele {TeleId}";
}
