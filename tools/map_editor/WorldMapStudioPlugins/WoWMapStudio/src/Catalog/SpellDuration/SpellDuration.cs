namespace WorldMapStudio.WoW;

/// <summary>A `dbc_spellduration` row — an effect-duration bracket a spell points at through
/// `dbc_spell.DurationIndex`. A shared lookup: one row backs many spells.</summary>
public sealed class SpellDuration : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    /// <summary>Base duration, milliseconds (-1 = permanent in the client's reading).</summary>
    [ScriptProperty(Mutable = true)]
    public int Duration { get; set; }

    /// <summary>Duration change per caster level, milliseconds.</summary>
    [ScriptProperty(Mutable = true)]
    public int DurationPerLevel { get; set; }

    /// <summary>Ceiling the per-level term can raise the duration to, milliseconds.</summary>
    [ScriptProperty(Mutable = true)]
    public int MaxDuration { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Spell Duration {ID}";
}
