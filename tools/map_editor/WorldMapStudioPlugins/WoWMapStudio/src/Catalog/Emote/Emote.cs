namespace WorldMapStudio.WoW;

/// <summary>A `dbc_emotes` row — the emote a quest's dialogue slots (`tc_quest_details.Emote1..4`, etc.)
/// point at. Has no localized name; `SlashCommand` (e.g. `EMOTE_ONESHOT_CHEER`) is the closest thing to
/// one and doubles as the search key.</summary>
public sealed class Emote : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public string SlashCommand { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public int AnimID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int EmoteFlags { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Emote {ID}: {SlashCommand}";
}
