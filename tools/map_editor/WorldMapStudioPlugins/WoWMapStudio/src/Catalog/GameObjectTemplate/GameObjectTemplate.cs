namespace WorldMapStudio.WoW;

/// <summary>A `gameobject_template` row, the palette entry gameobject spawns point at. `ID` is the template entry.</summary>
public sealed class GameObjectTemplate : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public string Name { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public int Type { get; set; }

    [ScriptProperty(Mutable = true)]
    public int DisplayId { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Faction { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Flags { get; set; }

    [ScriptProperty(Mutable = true)]
    public float Size { get; set; } = 1f;

    [ScriptProperty(Mutable = true)]
    public string ScriptName { get; set; } = string.Empty;

    public bool IsSaved { get; set; }

    public override string DisplayName => $"GameObject {ID}: {Name}";
}
