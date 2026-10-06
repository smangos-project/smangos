namespace WorldMapStudio.WoW;

/// <summary>A `quest_template` row. `ID` is the quest entry.</summary>
public sealed class QuestTemplate : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public string Title { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public string Details { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public string Objectives { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public int ZoneOrSort { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MinLevel { get; set; }

    [ScriptProperty(Mutable = true)]
    public int QuestLevel { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Type { get; set; }

    [ScriptProperty(Mutable = true)]
    public int RequiredClasses { get; set; }

    [ScriptProperty(Mutable = true)]
    public int RequiredRaces { get; set; }

    [ScriptProperty(Mutable = true)]
    public int PrevQuestId { get; set; }

    [ScriptProperty(Mutable = true)]
    public int NextQuestId { get; set; }

    [ScriptProperty(Mutable = true)]
    public int NextQuestInChain { get; set; }

    [ScriptProperty(Mutable = true)]
    public int QuestFlags { get; set; }

    [ScriptProperty(Mutable = true)]
    public int SrcItemId { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Quest {ID}: {Title}";
}
