namespace WorldMapStudio.WoW;

/// <summary>A `dbc_faction` row — the reputation-bearing faction a quest's `RequiredFactionId1/2` and
/// `RewardFactionID1..5` point at. Minimal: id, name, and the columns worth a link target having —
/// elaborating the per-race/class reputation slots is out of scope here.</summary>
public sealed class Faction : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public string Name { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public int ReputationIndex { get; set; }

    /// <summary>The faction this one's reputation rolls up into, or 0 for none.</summary>
    [ScriptProperty(Mutable = true)]
    public int ParentFactionID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Expansion { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Faction {ID}: {Name}";
}
