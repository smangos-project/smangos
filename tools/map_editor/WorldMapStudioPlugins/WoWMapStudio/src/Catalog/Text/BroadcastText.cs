namespace WorldMapStudio.WoW;

/// <summary>A `broadcast_text` row. `ID` is the entry.</summary>
public sealed class BroadcastText : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public string MaleText { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public string FemaleText { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public int ChatType { get; set; }

    [ScriptProperty(Mutable = true)]
    public int SoundId { get; set; }

    [ScriptProperty(Mutable = true)]
    public int LanguageId { get; set; }

    [ScriptProperty(Mutable = true)]
    public int EmoteId1 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int EmoteId2 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int EmoteId3 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int EmoteDelay1 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int EmoteDelay2 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int EmoteDelay3 { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Broadcast Text {ID}: {MaleText}";
}

/// <summary>An `npc_text` row: up to eight weighted broadcast texts a gossip menu can show.</summary>
public sealed class NpcText : CatalogEntity, IDbcCatalogEntity
{
    public const int Slots = 8;

    [ScriptProperty]
    public int ID { get; set; }

    public int[] BroadcastTextIds { get; } = new int[Slots];

    public float[] Probabilities { get; } = new float[Slots];

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Npc Text {ID}";
}
