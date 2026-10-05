namespace WorldMapStudio.WoW;

/// <summary>A `dbc_skillline` row — the skill a quest's `RequiredSkillID` (and its `RewardSkillId`)
/// point at.</summary>
public sealed class SkillLine : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public string Name { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public int CategoryID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int SpellIconFileID { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Skill Line {ID}: {Name}";
}
