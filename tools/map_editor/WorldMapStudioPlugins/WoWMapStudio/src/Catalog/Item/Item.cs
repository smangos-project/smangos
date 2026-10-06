namespace WorldMapStudio.WoW;

/// <summary>A `dbc_itemsparse` row, the item's stats and name. Editing only; a new item also needs its `dbc_item` row.</summary>
public sealed class Item : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public string Name { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public string Description { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public int Quality { get; set; }

    [ScriptProperty(Mutable = true)]
    public int ItemLevel { get; set; }

    [ScriptProperty(Mutable = true)]
    public int RequiredLevel { get; set; }

    [ScriptProperty(Mutable = true)]
    public int InventoryType { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Bonding { get; set; }

    [ScriptProperty(Mutable = true)]
    public int BuyPrice { get; set; }

    [ScriptProperty(Mutable = true)]
    public int SellPrice { get; set; }

    [ScriptProperty(Mutable = true)]
    public int StartQuestID { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Item {ID}: {Name}";
}
