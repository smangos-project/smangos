namespace WorldMapStudio.WoW;

/// <summary>A `dbc_spellcasttimes` row — a cast-time bracket a spell points at through
/// `dbc_spell.CastingTimeIndex`. A shared lookup: one row backs tens of thousands of spells.</summary>
public sealed class SpellCastTimes : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    /// <summary>Base cast time, milliseconds.</summary>
    [ScriptProperty(Mutable = true)]
    public int Base { get; set; }

    /// <summary>Cast-time change per caster level, milliseconds.</summary>
    [ScriptProperty(Mutable = true)]
    public int PerLevel { get; set; }

    /// <summary>Floor the per-level term can drive the cast time down to, milliseconds.</summary>
    [ScriptProperty(Mutable = true)]
    public int Minimum { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Spell Cast Times {ID}";
}
