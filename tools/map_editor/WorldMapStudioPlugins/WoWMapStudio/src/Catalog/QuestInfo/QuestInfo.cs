namespace WorldMapStudio.WoW;

/// <summary>A `dbc_questinfo` row — the type label `tc_quest_template.QuestInfoID` points at (e.g.
/// "Group Quest", "PvP Quest"). Tiny table, fully modelled.</summary>
public sealed class QuestInfo : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public string Name { get; set; } = string.Empty;

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Quest Info {ID}: {Name}";
}
