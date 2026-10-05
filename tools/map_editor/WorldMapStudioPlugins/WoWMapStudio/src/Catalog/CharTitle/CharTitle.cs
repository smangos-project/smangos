namespace WorldMapStudio.WoW;

/// <summary>A `dbc_chartitles` row — the title a quest's `RewardTitle` points at. `Name`/`Name1` are the
/// male/female phrasings (e.g. "%s the Explorer" / "%s the Explorer").</summary>
public sealed class CharTitle : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public string Name { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public string Name1 { get; set; } = string.Empty;

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Char Title {ID}: {Name}";
}
