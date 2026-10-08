namespace WorldMapStudio.WoW;

/// <summary>A `dbc_spellvisual` row.</summary>
public sealed class SpellVisual : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public float MissileCastOffset1 { get; set; }

    [ScriptProperty(Mutable = true)]
    public float MissileCastOffset2 { get; set; }

    [ScriptProperty(Mutable = true)]
    public float MissileCastOffset3 { get; set; }

    [ScriptProperty(Mutable = true)]
    public float MissileImpactOffset1 { get; set; }

    [ScriptProperty(Mutable = true)]
    public float MissileImpactOffset2 { get; set; }

    [ScriptProperty(Mutable = true)]
    public float MissileImpactOffset3 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int AnimEventSoundID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Flags { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MissileAttachment { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MissileDestinationAttachment { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MissileCastPositionerID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MissileImpactPositionerID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MissileTargetingKit { get; set; }

    [ScriptProperty(Mutable = true)]
    public int HostileSpellVisualID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int CasterSpellVisualID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int SpellVisualMissileSetID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int DamageNumberDelay { get; set; }

    [ScriptProperty(Mutable = true)]
    public int LowViolenceSpellVisualID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int RaidSpellVisualMissileSetID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int ReducedUnexpectedCameraMovementSpellVisualID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int AreaModel { get; set; }

    [ScriptProperty(Mutable = true)]
    public int HasMissile { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Spell Visual {ID}";
}
