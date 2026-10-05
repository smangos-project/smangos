namespace WorldMapStudio.WoW;

/// <summary>A `dbc_questsort` row — one of the negative values `tc_quest_template.QuestSortID` can take
/// (a log category like "Seasonal" rather than an AreaTable zone). Tiny table, fully modelled.</summary>
public sealed class QuestSort : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public string Name { get; set; } = string.Empty;

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Quest Sort {ID}: {Name}";
}
