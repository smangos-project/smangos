namespace WorldMapStudio.WoW;

/// <summary>
/// A `spell_target_position` row as a streamed scene entity — the destination a spell teleports or
/// aims at. The row is keyed by <c>(id, target_map)</c>, so its identity is the spell it belongs to
/// within the map it sits on.
/// </summary>
public sealed class WowSpellTargetPosition : SceneEntity
{
    /// <summary>`spell_target_position.id` — the spell entry. Fixed at creation: changing it after a
    /// save would need delete-and-reinsert, which the factory does not implement.</summary>
    public uint SpellId { get; set; }

    /// <summary>Whether a row already exists for this spell on this map.</summary>
    public bool IsSaved { get; set; }

    public override string DisplayName => $"Spell {SpellId} Target";
}
