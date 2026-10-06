using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json.Nodes;
using System.Threading.Tasks;
using Godot;
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
    private string _newTiles = "";

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
        ImGui.InputText("New tiles (x,y;x,y)", ref _newTiles, 256);
    }

    public JsonObject SaveSettings() => new()
    {
        ["sourceMap"] = _sourceMap,
        ["targetMap"] = _targetMap,
        ["newTiles"] = _newTiles,
    };

    public void LoadSettings(JsonObject settings)
    {
        _sourceMap = settings["sourceMap"]?.GetValue<string>() ?? "";
        _targetMap = settings["targetMap"]?.GetValue<string>() ?? "";
        _newTiles = settings["newTiles"]?.GetValue<string>() ?? "";
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

        WowFileDataIds ids = WowFileDataIds.For(context.Editor);
        List<(int FileX, int FileY)> newTiles = ParseTiles(context.Settings["newTiles"]?.GetValue<string>() ?? "");
        foreach ((int fileX, int fileY) in newTiles)
        {
            foreach (string path in NewTilePaths(sourceMap, fileX, fileY))
            {
                ids.Allocate(path);
            }
        }

        var fs = WowClientFiles.OpenIsolated(content);
        WowWdt wdt = await Task.Run(() => WowWdt.Read(fs, sourceMap)).ConfigureAwait(false);
        List<AdtImportTileRecord> rows = (await AdtImportTileStore.LoadForMapAsync(storage, catalog.Map.Value).ConfigureAwait(false))
            .Where(row => row.SourceId == sourceMap)
            .OrderBy(row => (row.FileY, row.FileX))
            .ToList();
        newTiles.RemoveAll(tile => wdt.Has(tile.FileX, tile.FileY));
        if (rows.Count == 0 && newTiles.Count == 0)
        {
            context.Report("Nothing to export — the map has no imported tiles.");
            return;
        }

        var ledger = new Dictionary<long, long>(await WowExportEntityIds.LoadAsync(storage).ConfigureAwait(false));
        List<EditorPlacement> placements = await StoredPlacements.LoadAsync(storage, context.Editor.Assets, catalog.Map.Value).ConfigureAwait(false);
        var newUids = new Dictionary<long, long>();
        uint nextUid = (uint)Math.Max(PlacementExtractor.CustomUidBase, ledger.Count == 0 ? 0 : ledger.Values.Max() + 1);

        uint ResolveTexture(string path) =>
            ids.IdOf(path)
            ?? (path.StartsWith("unknown/", StringComparison.Ordinal) && uint.TryParse(Path.GetFileNameWithoutExtension(path), out uint raw) ? raw : ids.Allocate(path));

        var grid = new LandscapeGrid(catalog.Settings);
        var allRows = rows.Concat(newTiles.Select(tile => new AdtImportTileRecord { SourceId = sourceMap, FileX = tile.FileX, FileY = tile.FileY })).ToList();
        Dictionary<(int FileX, int FileY), List<WaterSurfacePatch>> waterPatches = await GatherWaterAsync(context, work, catalog.Map, grid, sourceMap, allRows).ConfigureAwait(false);
        // With no authored water anywhere, MH2O is left exactly as the client has it (water import may be off).
        bool anyWater = waterPatches.Values.Any(patches => patches.Count > 0);

        int written = 0;
        var edited = new List<ImportedTile>();
        var created = new List<((int FileX, int FileY), WowTileFiles)>();
        (int DonorX, int DonorY)? donor = null;
        for (int i = 0; i < allRows.Count; i++)
        {
            work.ThrowIfCancellationRequested();
            var id = new WowTileId(sourceMap, allRows[i].FileX, allRows[i].FileY);
            bool isNew = i >= rows.Count;
            context.Step($"Tile {i + 1}/{allRows.Count}: {id.Stem}");
            context.Progress(i / (float)allRows.Count);

            ImportedTile template = isNew
                ? WowBlankTile.Create(new ImportedTileId(sourceMap, id.FileX, id.FileY), ids.IdOf("tileset/generic/black.blp") ?? 0)
                : await Task.Run(() => WowAdtTileReader.Read(fs, wdt, new ImportedTileId(sourceMap, id.FileX, id.FileY))).ConfigureAwait(false);
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
                path => ids.Allocate(path));
            tile = ImportedTileExtractor.WithPlacements(tile, doodads, objects);
            if (anyWater)
            {
                tile = ImportedTileExtractor.WithLiquid(
                    tile,
                    WowWaterTileBuilder.Build(waterPatches[(id.FileX, id.FileY)], id, grid, tile, template.Chunks.Select(chunk => chunk.Liquid).ToList()));
            }

            string? difference = isNew ? "new tile" : ImportedTileComparer.FirstDifference(template, tile);
            if (difference == null)
            {
                continue;
            }

            context.Report($"{id.Stem}: changed ({difference}).");
            Godot.GD.Print($"[Export] {id.Stem}: changed ({difference})");

            if (isNew)
            {
                donor ??= FindDonor(wdt) ?? throw new InvalidOperationException("The map has no existing tile to build new tiles from.");
                (int donorX, int donorY) = donor.Value;
                await Task.Run(() => WowAdtTileWriter.WriteNew(fs, wdt, sourceMap, donorX, donorY, tile)).ConfigureAwait(false);
                string[] paths = NewTilePaths(sourceMap, id.FileX, id.FileY);
                created.Add(((id.FileX, id.FileY), new WowTileFiles(ids.Allocate(paths[0]), ids.Allocate(paths[1]), 0, ids.Allocate(paths[2]), 0, 0, 0, 0)));
            }
            else
            {
                await Task.Run(() => WowAdtTileWriter.Write(fs, wdt, tile)).ConfigureAwait(false);
            }

            edited.Add(tile);
            written++;
        }

        if (created.Count > 0)
        {
            await Task.Run(() => WowWdtWriter.AddTiles(fs, sourceMap, created)).ConfigureAwait(false);
        }

        if (edited.Count > 0)
        {
            await Task.Run(() => WowWdlWriter.Write(fs, sourceMap, edited.ToArray())).ConfigureAwait(false);
        }

        if (newUids.Count > 0)
        {
            await WowExportEntityIds.UpsertAsync(storage, newUids).ConfigureAwait(false);
        }

        ids.ExportIndex(content);
        context.Report($"Exported {written} edited tile(s) of {allRows.Count} to {content}.");
    }

    private static string[] NewTilePaths(string map, int fileX, int fileY)
    {
        string stem = $"world/maps/{map}/{map}_{fileX}_{fileY}";
        return [$"{stem}.adt", $"{stem}_obj0.adt", $"{stem}_tex0.adt"];
    }

    private static (int, int)? FindDonor(WowWdt wdt)
    {
        for (int i = 0; i < wdt.HasAdt.Length; i++)
        {
            if (wdt.HasAdt[i])
            {
                return (i % WowWdt.GridEdge, i / WowWdt.GridEdge);
            }
        }

        return null;
    }

    private static List<(int FileX, int FileY)> ParseTiles(string text)
    {
        var tiles = new List<(int, int)>();
        foreach (string part in text.Split(';', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
        {
            string[] xy = part.Split(',', StringSplitOptions.TrimEntries);
            if (xy.Length == 2 && int.TryParse(xy[0], out int x) && int.TryParse(xy[1], out int y) && x is >= 0 and < 64 && y is >= 0 and < 64 && !tiles.Contains((x, y)))
            {
                tiles.Add((x, y));
            }
        }

        return tiles;
    }

    /// <summary>Each tile's water patches: the scene is scanned in the background, the built meshes are read on the main thread.</summary>
    private static async Task<Dictionary<(int FileX, int FileY), List<WaterSurfacePatch>>> GatherWaterAsync(
        BatchContext context, WorkContext work, MapId map, LandscapeGrid grid, string sourceMap, IReadOnlyList<AdtImportTileRecord> rows)
    {
        var scans = new List<((int FileX, int FileY) Key, IReadOnlyList<SceneEntity> Hits)>();
        foreach (AdtImportTileRecord row in rows)
        {
            Aabb bounds = WowTileAddressing.EnumerateChunks(new WowTileId(sourceMap, row.FileX, row.FileY))
                .Select(entry => grid.BoundsOf(entry.Coord))
                .Aggregate((a, b) => a.Merge(b));
            scans.Add(((row.FileX, row.FileY), await context.ScanSceneAsync(map, bounds).ConfigureAwait(false)));
        }

        var result = new Dictionary<(int, int), List<WaterSurfacePatch>>();
        await work.SwitchToMain();
        try
        {
            foreach (((int FileX, int FileY) key, IReadOnlyList<SceneEntity> hits) in scans)
            {
                result[key] = hits.SelectMany(entity => WowWaterPatches.Gather(context.Editor, entity)).ToList();
            }
        }
        finally
        {
            await work.SwitchToBackground();
        }

        return result;
    }
}
