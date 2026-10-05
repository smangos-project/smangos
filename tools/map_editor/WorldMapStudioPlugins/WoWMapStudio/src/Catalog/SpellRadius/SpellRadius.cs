namespace WorldMapStudio.WoW;

/// <summary>A `dbc_spellradius` row — an area-of-effect radius a spell effect points at through
/// `dbc_spelleffect.EffectRadiusIndex1/2`. A shared lookup: one row backs many effects.</summary>
public sealed class SpellRadius : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    /// <summary>Base radius, yards.</summary>
    [ScriptProperty(Mutable = true)]
    public float Radius { get; set; }

    /// <summary>Radius change per caster level, yards.</summary>
    [ScriptProperty(Mutable = true)]
    public float RadiusPerLevel { get; set; }

    /// <summary>Ceiling the per-level term can raise the radius to, yards.</summary>
    [ScriptProperty(Mutable = true)]
    public float RadiusMax { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Spell Radius {ID}";
}
