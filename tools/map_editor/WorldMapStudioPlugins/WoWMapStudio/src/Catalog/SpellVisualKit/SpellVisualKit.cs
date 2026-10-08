namespace WorldMapStudio.WoW;

/// <summary>A `dbc_spellvisualkit` row.</summary>
public sealed class SpellVisualKit : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int FallbackSpellVisualKitID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int DelayMin { get; set; }

    [ScriptProperty(Mutable = true)]
    public int DelayMax { get; set; }

    [ScriptProperty(Mutable = true)]
    public float FallbackPriority { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Flags1 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Flags2 { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Spell Visual Kit {ID}";
}
