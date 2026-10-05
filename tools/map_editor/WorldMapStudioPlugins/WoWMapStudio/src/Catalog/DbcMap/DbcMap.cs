namespace WorldMapStudio.WoW;

/// <summary>
/// A Map db2 row as a catalog entity — the WoW client's map table (id ↔ ADT directory ↔ display
/// name). Read-only reference data: nothing in this editor authors map rows, so
/// the catalog factory loads it whole for browsing and lookups but gives it no create/edit
/// surface. Its reason to exist is letting map bindings turn the open editor
/// map's name into the WoW map id.
/// </summary>
public sealed class DbcMap : CatalogEntity
{
    [ScriptProperty]
    public int MapId { get; set; }

    /// <summary>The ADT folder name, e.g. `Azeroth` — what <c>WowAdtBatchOperation</c> keys export off.</summary>
    [ScriptProperty]
    public string Directory { get; set; } = string.Empty;

    /// <summary>The localized display name, e.g. `Eastern Kingdoms`.</summary>
    [ScriptProperty]
    public string MapName { get; set; } = string.Empty;

    [ScriptProperty]
    public int InstanceType { get; set; }

    [ScriptProperty]
    public int ParentMapId { get; set; }

    public override string DisplayName => $"Map {MapId}: {(Directory.Length > 0 ? Directory : MapName)}";
}
