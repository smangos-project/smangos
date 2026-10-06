namespace WorldMapStudio.WoW;

/// <summary>A `creature_template` row, the palette entry creature spawns point at. `ID` is the template entry.</summary>
public sealed class CreatureTemplate : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public string Name { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public string Subname { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public int LevelMin { get; set; }

    [ScriptProperty(Mutable = true)]
    public int LevelMax { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Faction { get; set; }

    [ScriptProperty(Mutable = true)]
    public int NpcFlags { get; set; }

    [ScriptProperty(Mutable = true)]
    public int DisplayId1 { get; set; }

    [ScriptProperty(Mutable = true)]
    public float DisplayScale1 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Type { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Rank { get; set; }

    [ScriptProperty(Mutable = true)]
    public int LootId { get; set; }

    [ScriptProperty(Mutable = true)]
    public int VendorId { get; set; }

    [ScriptProperty(Mutable = true)]
    public int TrainerId { get; set; }

    [ScriptProperty(Mutable = true)]
    public int GossipMenuId { get; set; }

    [ScriptProperty(Mutable = true)]
    public string AiName { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public string ScriptName { get; set; } = string.Empty;

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Creature {ID}: {Name}";
}
