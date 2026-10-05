namespace WorldMapStudio.WoW;

/// <summary>
/// Resolves the editor's <see cref="MapId"/> to WoW's own integer map id, through the project map's
/// explicit <see cref="WowMapBindings"/> binding. Null when the map is unbound: a scan then loads
/// nothing rather than another map's rows, and a save refuses rather than writing a bogus map column.
/// Shared by every <c>[Subsystem(nameof(WorldStorage))]</c> factory that needs the lookup.
/// </summary>
public static class WowMapResolution
{
    public static int? Resolve(WorldStorage storage, MapId editorMap) =>
        WowMapBindings.For(storage.Context).Resolve(editorMap);
}
