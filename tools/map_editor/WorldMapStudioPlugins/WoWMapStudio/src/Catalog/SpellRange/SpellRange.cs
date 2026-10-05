namespace WorldMapStudio.WoW;

/// <summary>A `dbc_spellrange` row — a min/max range bracket a spell points at through
/// `dbc_spell.RangeIndex`. A shared lookup: one row backs tens of thousands of spells.</summary>
public sealed class SpellRange : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    /// <summary>Minimum range against a hostile target, yards.</summary>
    [ScriptProperty(Mutable = true)]
    public float RangeMin1 { get; set; }

    /// <summary>Minimum range against a friendly target, yards.</summary>
    [ScriptProperty(Mutable = true)]
    public float RangeMin2 { get; set; }

    /// <summary>Maximum range against a hostile target, yards.</summary>
    [ScriptProperty(Mutable = true)]
    public float RangeMax1 { get; set; }

    /// <summary>Maximum range against a friendly target, yards.</summary>
    [ScriptProperty(Mutable = true)]
    public float RangeMax2 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Flags { get; set; }

    [ScriptProperty(Mutable = true)]
    public string DisplayNameLang { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public string DisplayNameShortLang { get; set; } = string.Empty;

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Spell Range {ID}";
}
