using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json.Nodes;
using System.Threading.Tasks;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>
/// Writes the imported tiles of a project map back to client ADTs under the project's client content
/// overlay, rebuilding each from the map-wide terrain images and the tile as the client last had it.
/// </summary>
[Subsystem(nameof(BatchSystem))]
public sealed class WowAdtExportBatchOperation : IBatchOperation
{
    private string _sourceMap = "";
    private string _targetMap = "";

    public string Id => "wow.export.adt";

    public string DisplayName => "WoW ADT Export (Cataclysm)";

    public string Description =>
        "Writes the terrain of a project map's imported tiles into the client's content overlay as ADT files.";

    public WowAdtExportBatchOperation(BatchSystem batch)
    {
    }

    public void DrawSettings()
    {
        ImGui.InputText("Client map folder", ref _sourceMap, 128);
        ImGui.InputText("Project map", ref _targetMap, 128);
    }

    public JsonObject SaveSettings() => new()
    {
        ["sourceMap"] = _sourceMap,
        ["targetMap"] = _targetMap,
    };

    public void LoadSettings(JsonObject settings)
    {
        _sourceMap = settings["sourceMap"]?.GetValue<string>() ?? "";
        _targetMap = settings["targetMap"]?.GetValue<string>() ?? "";
    }

    public async Task RunAsync(BatchContext context, WorkContext work)
    {
        string sourceMap = context.Settings["sourceMap"]?.GetValue<string>() ?? "";
        string targetMap = context.Settings["targetMap"]?.GetValue<string>() ?? "";
        if (sourceMap.Length == 0 || targetMap.Length == 0)
        {
            throw new InvalidOperationException("Both a client map folder and a project map name are required.");
        }

        string content = context.Editor.WowClientPaths.Content
            ?? throw new InvalidOperationException(WowClientPaths.MissingContentMessage);
        EditorStorage storage = context.Editor.Database.EditorStorage;

        context.Step("Preparing map");
        AdtImportCatalog catalog = await AdtImportCatalog.EnsureAsync(context, work, targetMap, sourceMap, PaintImageStorageKind.Database).ConfigureAwait(false);
        PaintImage[] images = new[]
        {
            catalog.HeightImageId, catalog.AlphaImageId, catalog.HoleImageId, catalog.AreaImageId,
            catalog.FlagsImageId, catalog.VertexColorImageId, catalog.VertexLightImageId,
        }
        .Select(id => context.Editor.Images.FindImage(id) ?? throw new InvalidOperationException($"Terrain image #{id} is missing."))
        .ToArray();

        var fs = WowClientFiles.OpenIsolated(content);
        WowWdt wdt = await Task.Run(() => WowWdt.Read(fs, sourceMap)).ConfigureAwait(false);
        List<AdtImportTileRecord> rows = (await AdtImportTileStore.LoadForMapAsync(storage, catalog.Map.Value).ConfigureAwait(false))
            .Where(row => row.SourceId == sourceMap)
            .OrderBy(row => (row.FileY, row.FileX))
            .ToList();
        if (rows.Count == 0)
        {
            context.Report("Nothing to export — the map has no imported tiles.");
            return;
        }

        WowFileDataIds.Shared.AttachRegistry(content);

        var ledger = new Dictionary<long, long>(await WowExportEntityIds.LoadAsync(storage).ConfigureAwait(false));
        List<EditorPlacement> placements = await StoredPlacements.LoadAsync(storage, context.Editor.Assets, catalog.Map.Value).ConfigureAwait(false);
        var newUids = new Dictionary<long, long>();
        uint nextUid = (uint)Math.Max(PlacementExtractor.CustomUidBase, ledger.Count == 0 ? 0 : ledger.Values.Max() + 1);

        uint ResolveTexture(string path) =>
            WowFileDataIds.Shared.IdOf(path)
            ?? (path.StartsWith("unknown/", StringComparison.Ordinal) && uint.TryParse(Path.GetFileNameWithoutExtension(path), out uint raw) ? raw : WowFileDataIds.Shared.Allocate(path));

        int written = 0;
        var edited = new List<ImportedTile>();
        for (int i = 0; i < rows.Count; i++)
        {
            work.ThrowIfCancellationRequested();
            var id = new WowTileId(sourceMap, rows[i].FileX, rows[i].FileY);
            context.Step($"Tile {i + 1}/{rows.Count}: {id.Stem}");
            context.Progress(i / (float)rows.Count);

            ImportedTile template = await Task.Run(() => WowAdtTileReader.Read(fs, wdt, new ImportedTileId(sourceMap, id.FileX, id.FileY))).ConfigureAwait(false);
            StoredTerrainImages stored = await StoredTerrainImages.LoadAsync(storage, images, catalog, id).ConfigureAwait(false);
            var slotTextures = await StoredSlotTextures.LoadAsync(storage, catalog.Map.Value, id.FileX, id.FileY, ResolveTexture).ConfigureAwait(false);
            ImportedTile tile = ImportedTileExtractor.Extract(id, template, stored, slotTextures);
            var (doodads, objects) = PlacementExtractor.Apply(
                template,
                placements,
                recordId => ledger.TryGetValue(recordId, out long uid) ? (uint)uid : null,
                recordId =>
                {
                    uint uid = nextUid++;
                    ledger[recordId] = uid;
                    newUids[recordId] = uid;
                    return uid;
                },
                path => WowFileDataIds.Shared.Allocate(path));
            tile = ImportedTileExtractor.WithPlacements(tile, doodads, objects);
            string? difference = ImportedTileComparer.FirstDifference(template, tile);
            if (difference == null)
            {
                continue;
            }

            context.Report($"{id.Stem}: changed ({difference}).");
            Godot.GD.Print($"[Export] {id.Stem}: changed ({difference})");

            await Task.Run(() => WowAdtTileWriter.Write(fs, wdt, tile)).ConfigureAwait(false);
            edited.Add(tile);
            written++;
        }

        if (edited.Count > 0)
        {
            await Task.Run(() => WowWdlWriter.Write(fs, sourceMap, edited.ToArray())).ConfigureAwait(false);
        }

        if (newUids.Count > 0)
        {
            await WowExportEntityIds.UpsertAsync(storage, newUids).ConfigureAwait(false);
        }

        context.Report($"Exported {written} edited tile(s) of {rows.Count} to {content}.");
    }
}
