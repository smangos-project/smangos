using System;
using System.Collections.Generic;
using System.Data.Common;
using System.Linq;
using System.Security.Cryptography;
using System.Text.Json.Nodes;
using System.Threading.Tasks;
using Godot;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Storage;
using NVector2 = System.Numerics.Vector2;
using NVector4 = System.Numerics.Vector4;

namespace WorldMapStudio.WoW;

/// <summary>One image chunk a single tile contributes. <paramref name="Owned"/> marks, per pixel, the
/// texels that are the tile's own — the rest are apron, an edge replica that must never displace what
/// a neighbouring tile has already written.</summary>
internal sealed record ImportedImageChunk(ImageChunkCoord Coord, byte[] Pixels, bool[] Owned);

/// <summary>
/// Imports existing Cataclysm ADT tiles into a project map — the mirror of
/// <see cref="WowAdtBatchOperation"/>. Per tile it reads the root / <c>_tex0</c> / <c>_obj0</c> files
/// through <see cref="CataAdtTileReader"/>, re-authors the terrain heights as per-vertex texels in
/// the map-wide <c>adt_height</c> image (<see cref="HeightMapper"/>), records the tile's per-chunk
/// facts on an <see cref="AdtTileComponent"/>, and writes an <see cref="AdtImportTileRecord"/> so the
/// run resumes where it left off. Everything is written straight to storage, outside the edit session;
/// <see cref="BatchContext.RequireReload"/> brings the loaded world back in line at the end.
///
/// Heights, textures, holes, vertex colours/light, placements and water are mapped. Water becomes
/// one <c>wow.water.surface</c> <see cref="ProceduralModel"/> plus a <see cref="ProceduralComponent"/>
/// per merged body (<see cref="WaterMapper"/>).
/// </summary>
[Subsystem(nameof(BatchSystem))]
public sealed class WowAdtImportBatchOperation : IBatchOperation
{
    /// <summary>Bumped whenever a change here would make an already-imported tile wrong. It is stored
    /// on every tile's row, so bumping it makes a re-run redo the map.
    /// v2: slot-material alpha bindings used "." instead of ":" as the channel swizzle separator, so
    /// no alpha layer resolved.
    /// v3: a tile's apron replaced a neighbouring tile's whole height / vertex chunk instead of
    /// merging into it, so every tile but the last one imported came out flat.
    /// v4: the placement dedup set was seeded from the whole project-wide export uid ledger, so
    /// importing a source map's tiles into a second target map skipped every model and WMO.
    /// v5: placements and textures are keyed by FileDataID.
    /// v6: MH2O is imported as editable water (a wow.water.surface model per body).</summary>
    public const int ImporterVersion = 6;

    /// <summary>The map's most-negative landscape chunk — a full 64×64-tile map's origin, which the
    /// map-wide height image's pixel (0,0) is aligned to (<see cref="AdtImportCatalog"/>).</summary>
    internal static readonly ChunkCoord MapMinChunk = new(
        -(WowTileAddressing.TilesPerMapEdge / 2) * WowTileAddressing.ChunksPerTileEdge,
        -(WowTileAddressing.TilesPerMapEdge / 2) * WowTileAddressing.ChunksPerTileEdge);

    private const int MapEdge = WowTileAddressing.TilesPerMapEdge;

    /// <summary>MODF flag marking the <c>scale</c> field as meaningful. Unset in every pre-Shadowlands
    /// client, where the field is left zero and the WMO is always placed at 1.0.</summary>
    private const ushort ModfHasScaleFlag = 0x4;

    private readonly BatchSystem _batch;

    private string _sourceMap = "";
    private string _targetMap = "";
    private bool _diskStorage;
    private bool _importWater = true;
    private int _minX = 32;
    private int _minY = 32;
    private int _maxX = 32;
    private int _maxY = 32;

    // Which of the source map's 64×64 tiles have an ADT, for the visual picker — loaded from the WDT
    // on demand and kept until the source map name changes.
    private bool[]? _tilePresence;
    private string _tilePresenceFor = "\0";
    private (int X, int Y)? _dragAnchor;

    public string Id => "wow.import.adt";

    public string DisplayName => "WoW ADT Import (Cataclysm)";

    public string Description =>
        "Reads existing Cataclysm ADT tiles from the configured client data into a project map's terrain, one tile at a time, resuming where a previous run stopped.";

    public WowAdtImportBatchOperation(BatchSystem batch)
    {
        _batch = batch;
    }

    public void DrawSettings()
    {
        ImGui.InputText("Source map folder", ref _sourceMap, 128);
        ImGui.SameLine();
        if (ImGui.Button("Browse...##sourceMap") && NativeFileDialog.IsSupported)
        {
            NativeFileDialog.PickDirectory("Choose a World/Maps/<Map> folder", "", path =>
            {
                if (!string.IsNullOrEmpty(path))
                {
                    _sourceMap = System.IO.Path.GetFileName(path.TrimEnd('/', '\\'));
                }
            });
        }

        ImGui.InputText("Target project map", ref _targetMap, 128);
        ImGui.Checkbox("Store heights on disk", ref _diskStorage);
        ImGui.Checkbox("Import water (MH2O)", ref _importWater);

        ImGui.InputInt("Tile min X", ref _minX);
        ImGui.InputInt("Tile min Y", ref _minY);
        ImGui.InputInt("Tile max X", ref _maxX);
        ImGui.InputInt("Tile max Y", ref _maxY);
        ClampTileRect();

        if (ImGui.Button("Select tiles on a grid..."))
        {
            ImGui.OpenPopup("adt-import-tiles");
        }

        ImGui.SameLine();
        ImGui.TextDisabled($"{ExistingTilesInRect()} tile(s) in [{_minX}-{_maxX}] × [{_minY}-{_maxY}]");

        DrawTileGridPopup();
        DrawReimportWarning();
    }

    // Tiles in the rectangle that a re-import would rewrite although they were edited after their last
    // import. Looked up off the UI thread and again only when the target or rectangle changes.
    private Task<IReadOnlyList<(int X, int Y)>>? _editedTiles;
    private (string Source, string Target, int MinX, int MinY, int MaxX, int MaxY) _editedTilesKey;

    private void DrawReimportWarning()
    {
        var key = (_sourceMap, _targetMap, _minX, _minY, _maxX, _maxY);
        if (_editedTiles == null || key != _editedTilesKey)
        {
            _editedTilesKey = key;
            Map? map = _batch.Context.Maps.Maps.FirstOrDefault(candidate =>
                string.Equals(candidate.DisplayName, _targetMap, StringComparison.OrdinalIgnoreCase));
            _editedTiles = map == null
                ? Task.FromResult<IReadOnlyList<(int X, int Y)>>([])
                : FindEditedTilesAsync(map.Id, key.Item1, key.Item3, key.Item4, key.Item5, key.Item6);
        }

        if (_editedTiles.IsCompletedSuccessfully && _editedTiles.Result.Count > 0)
        {
            ImGui.TextColored(
                new System.Numerics.Vector4(1.0f, 0.55f, 0.2f, 1.0f),
                $"{_editedTiles.Result.Count} tile(s) here were edited after they were imported; importing them again overwrites those edits ({AdtReimportGuard.Describe(_editedTiles.Result)}).");
        }
    }

    private async Task<IReadOnlyList<(int X, int Y)>> FindEditedTilesAsync(MapId map, string source, int minX, int minY, int maxX, int maxY)
    {
        EditorStorage storage = _batch.Context.Database.EditorStorage;
        IReadOnlyList<AdtImportTileRecord> rows = await AdtImportTileStore.LoadForMapAsync(storage, map.Value).ConfigureAwait(false);
        IEnumerable<AdtImportTileRecord> stale = rows.Where(row =>
            row.SourceId == source && row.ImporterVersion < ImporterVersion &&
            row.FileX >= minX && row.FileX <= maxX && row.FileY >= minY && row.FileY <= maxY);
        return await AdtReimportGuard.EditedTilesAsync(storage, map, stale).ConfigureAwait(false);
    }

    private void ClampTileRect()
    {
        _minX = Math.Clamp(_minX, 0, MapEdge - 1);
        _minY = Math.Clamp(_minY, 0, MapEdge - 1);
        _maxX = Math.Clamp(_maxX, _minX, MapEdge - 1);
        _maxY = Math.Clamp(_maxY, _minY, MapEdge - 1);
    }

    private int ExistingTilesInRect()
    {
        if (_tilePresence is not { } presence)
        {
            return 0;
        }

        int count = 0;
        for (int y = _minY; y <= _maxY; y++)
        {
            for (int x = _minX; x <= _maxX; x++)
            {
                if (presence[(y * MapEdge) + x])
                {
                    count++;
                }
            }
        }

        return count;
    }

    /// <summary>A 64×64 grid of the source map's tiles: filled cells exist on disk, and a click-drag
    /// paints the min/max rectangle. Reads the WDT once per source map.</summary>
    private void DrawTileGridPopup()
    {
        if (!ImGui.BeginPopup("adt-import-tiles"))
        {
            return;
        }

        EnsureTilePresence();

        ImGui.TextUnformatted(_sourceMap.Length == 0
            ? "Set a source map folder first."
            : $"{_sourceMap}: drag to select the tile rectangle.");
        ImGui.SameLine();
        if (ImGui.SmallButton("Reload"))
        {
            _tilePresenceFor = "\0";
        }

        const float cell = 9.0f;
        ImDrawListPtr draw = ImGui.GetWindowDrawList();
        NVector2 origin = ImGui.GetCursorScreenPos();

        uint colMissing = ImGui.GetColorU32(new NVector4(0.16f, 0.16f, 0.16f, 1.0f));
        uint colExists = ImGui.GetColorU32(new NVector4(0.55f, 0.55f, 0.55f, 1.0f));
        uint colSelected = ImGui.GetColorU32(new NVector4(0.27f, 0.67f, 1.0f, 1.0f));
        uint colSelectedExists = ImGui.GetColorU32(new NVector4(0.45f, 0.85f, 1.0f, 1.0f));

        for (int y = 0; y < MapEdge; y++)
        {
            for (int x = 0; x < MapEdge; x++)
            {
                bool exists = _tilePresence is { } p && p[(y * MapEdge) + x];
                bool selected = x >= _minX && x <= _maxX && y >= _minY && y <= _maxY;
                var p0 = new NVector2(origin.X + (x * cell), origin.Y + (y * cell));
                var p1 = new NVector2(p0.X + cell - 1.0f, p0.Y + cell - 1.0f);
                draw.AddRectFilled(p0, p1, selected ? (exists ? colSelectedExists : colSelected) : (exists ? colExists : colMissing));
            }
        }

        ImGui.InvisibleButton("grid", new NVector2(MapEdge * cell, MapEdge * cell));
        if (ImGui.IsItemActive())
        {
            NVector2 mouse = ImGui.GetMousePos();
            int tx = Math.Clamp((int)((mouse.X - origin.X) / cell), 0, MapEdge - 1);
            int ty = Math.Clamp((int)((mouse.Y - origin.Y) / cell), 0, MapEdge - 1);

            if (ImGui.IsMouseClicked(ImGuiMouseButton.Left))
            {
                _dragAnchor = (tx, ty);
            }

            if (_dragAnchor is { } anchor)
            {
                _minX = Math.Min(anchor.X, tx);
                _maxX = Math.Max(anchor.X, tx);
                _minY = Math.Min(anchor.Y, ty);
                _maxY = Math.Max(anchor.Y, ty);
            }
        }

        if (ImGui.IsMouseReleased(ImGuiMouseButton.Left))
        {
            _dragAnchor = null;
        }

        ImGui.Dummy(new NVector2(0.0f, 4.0f));
        ImGui.TextDisabled($"X {_minX}–{_maxX}   Y {_minY}–{_maxY}   ({ExistingTilesInRect()} present)");
        if (ImGui.Button("Done"))
        {
            ImGui.CloseCurrentPopup();
        }

        ImGui.EndPopup();
    }

    private void EnsureTilePresence()
    {
        if (_tilePresenceFor == _sourceMap)
        {
            return;
        }

        _tilePresenceFor = _sourceMap;
        _tilePresence = new bool[MapEdge * MapEdge];
        if (_sourceMap.Length == 0)
        {
            return;
        }

        try
        {
            WowWdt wdt = WowWdt.Read(WowClientFiles.Shared, _sourceMap);
            for (int y = 0; y < MapEdge; y++)
            {
                for (int x = 0; x < MapEdge; x++)
                {
                    _tilePresence[(y * MapEdge) + x] = wdt.Has(x, y);
                }
            }
        }
        catch (Exception e)
        {
            GD.PushWarning($"[WoW ADT Import] Could not read WDT for '{_sourceMap}': {e.Message}");
        }
    }

    public JsonObject SaveSettings() => new()
    {
        ["sourceMap"] = _sourceMap,
        ["targetMap"] = _targetMap,
        ["diskStorage"] = _diskStorage,
        ["importWater"] = _importWater,
        ["minX"] = _minX,
        ["minY"] = _minY,
        ["maxX"] = _maxX,
        ["maxY"] = _maxY,
    };

    public void LoadSettings(JsonObject settings)
    {
        _sourceMap = settings["sourceMap"]?.GetValue<string>() ?? "";
        _targetMap = settings["targetMap"]?.GetValue<string>() ?? "";
        _diskStorage = settings["diskStorage"]?.GetValue<bool>() ?? false;
        _importWater = settings["importWater"]?.GetValue<bool>() ?? true;
        _minX = settings["minX"]?.GetValue<int>() ?? 32;
        _minY = settings["minY"]?.GetValue<int>() ?? 32;
        _maxX = settings["maxX"]?.GetValue<int>() ?? 32;
        _maxY = settings["maxY"]?.GetValue<int>() ?? 32;
    }

    public async Task RunAsync(BatchContext context, WorkContext work)
    {
        JsonObject settings = context.Settings;
        string sourceMap = settings["sourceMap"]?.GetValue<string>() ?? "";
        string targetMap = settings["targetMap"]?.GetValue<string>() ?? "";
        if (sourceMap.Length == 0 || targetMap.Length == 0)
        {
            throw new InvalidOperationException("Both a source map folder and a target project map name are required.");
        }

        PaintImageStorageKind storageKind = (settings["diskStorage"]?.GetValue<bool>() ?? false)
            ? PaintImageStorageKind.Disk
            : PaintImageStorageKind.Database;

        bool importWater = settings["importWater"]?.GetValue<bool>() ?? true;

        int minX = Math.Min(settings["minX"]?.GetValue<int>() ?? 0, settings["maxX"]?.GetValue<int>() ?? 0);
        int maxX = Math.Max(settings["minX"]?.GetValue<int>() ?? 0, settings["maxX"]?.GetValue<int>() ?? 0);
        int minY = Math.Min(settings["minY"]?.GetValue<int>() ?? 0, settings["maxY"]?.GetValue<int>() ?? 0);
        int maxY = Math.Max(settings["minY"]?.GetValue<int>() ?? 0, settings["maxY"]?.GetValue<int>() ?? 0);

        AssetSystem assets = context.Editor.Assets;
        EditorStorage storage = context.Editor.Database.EditorStorage;

        context.Step("Preparing map");
        AdtImportCatalog catalog;
        using (context.Timings.Measure("prepare map catalog"))
        {
            try
            {
                catalog = await AdtImportCatalog.EnsureAsync(context, work, targetMap, sourceMap, storageKind).ConfigureAwait(false);
            }
            catch (Exception e)
            {
                context.Log($"Preparing map catalog failed: {e}");
                throw;
            }
        }
        var grid = new LandscapeGrid(catalog.Settings);
        context.Log($"Map '{targetMap}' is #{catalog.Map.Value} — switch to it after the run. " +
            $"channels h#{catalog.HeightChannelId} a#{catalog.AlphaChannelId}; layers terrain#{catalog.HeightLayerId} " +
            $"base#{catalog.SlotLayerIds.ElementAtOrDefault(0)}; images h#{catalog.HeightImageId} a#{catalog.AlphaImageId}; " +
            $"height material #{catalog.HeightMaterialId}.");

        PaintImage heightImage = context.Editor.Images.FindImage(catalog.HeightImageId)
            ?? throw new InvalidOperationException("The height image is missing after catalog setup.");
        PaintImage alphaImage = context.Editor.Images.FindImage(catalog.AlphaImageId)
            ?? throw new InvalidOperationException("The alpha image is missing after catalog setup.");
        PaintImage holeImage = context.Editor.Images.FindImage(catalog.HoleImageId)
            ?? throw new InvalidOperationException("The hole image is missing after catalog setup.");
        PaintImage vColorImage = context.Editor.Images.FindImage(catalog.VertexColorImageId)
            ?? throw new InvalidOperationException("The vertex colour image is missing after catalog setup.");
        PaintImage vLightImage = context.Editor.Images.FindImage(catalog.VertexLightImageId)
            ?? throw new InvalidOperationException("The vertex light image is missing after catalog setup.");
        PaintImage areaImage = context.Editor.Images.FindImage(catalog.AreaImageId)
            ?? throw new InvalidOperationException("The area image is missing after catalog setup.");
        PaintImage flagsImage = context.Editor.Images.FindImage(catalog.FlagsImageId)
            ?? throw new InvalidOperationException("The MCNK-flags image is missing after catalog setup.");

        WowWdt wdt;
        using (context.Timings.Measure("read WDT"))
        {
            wdt = await Task.Run(() => WowWdt.Read(WowClientFiles.Shared, sourceMap)).ConfigureAwait(false);
        }

        Dictionary<(int, int), AdtImportTileRecord> existingRows;
        using (context.Timings.Measure("load imported-tile rows"))
        {
            existingRows = (await AdtImportTileStore.LoadForMapAsync(storage, catalog.Map.Value).ConfigureAwait(false))
                .Where(row => row.SourceId == sourceMap)
                .ToDictionary(row => (row.FileX, row.FileY));
        }

        var tiles = new List<(int X, int Y)>();
        var reimport = new HashSet<(int X, int Y)>();
        for (int y = minY; y <= maxY; y++)
        {
            for (int x = minX; x <= maxX; x++)
            {
                if (!wdt.Has(x, y))
                {
                    continue;
                }

                if (!existingRows.TryGetValue((x, y), out AdtImportTileRecord? row))
                {
                    tiles.Add((x, y));
                }
                else if (row.ImporterVersion < ImporterVersion)
                {
                    // A version bump makes this tile's data stale — rewrite it, dropping its old
                    // AdtTileComponent entity first. Its image chunks are overwritten in place and its
                    // placements match on uniqueId, so nothing else needs clearing.
                    tiles.Add((x, y));
                    reimport.Add((x, y));
                }
            }
        }

        if (tiles.Count == 0)
        {
            context.Report("Nothing to import — every requested tile is absent or already imported.");
            return;
        }

        if (reimport.Count > 0)
        {
            IReadOnlyList<(int X, int Y)> edited = await AdtReimportGuard.EditedTilesAsync(
                storage, catalog.Map, reimport.Select(tile => existingRows[tile])).ConfigureAwait(false);
            if (edited.Count > 0)
            {
                context.Log($"[WoW] WARNING: {edited.Count} tile(s) being re-imported were edited after they were imported, and those edits are overwritten: {AdtReimportGuard.Describe(edited)}.");
            }
        }

        context.RequireReload("WoW ADT import");
                int imported = 0;

        // Placements are deduplicated by MDDF/MODF uniqueId — an object straddling a tile border is
        // listed in every tile it touches, and a re-import (a version bump) re-reads tiles whose
        // placement entities are still present. The dedup set is the uids already allocated to
        // placements in this target map only: the ledger is project-wide, so seeding from all of it
        // makes importing the same source tiles into a second map skip every placement.
        // Model bounds are loaded once per distinct path.
        IReadOnlyCollection<long> mapUids;
        using (context.Timings.Measure("load uid ledger"))
        {
            mapUids = await WowExportEntityIds.LoadForMapAsync(storage, catalog.Map.Value).ConfigureAwait(false);
        }
        var knownUids = new HashSet<long>(mapUids);
        var m2BoundsCache = new Dictionary<string, Aabb?>(StringComparer.OrdinalIgnoreCase);
        var wmoRootCache = new Dictionary<string, WmoRoot?>(StringComparer.OrdinalIgnoreCase);
        var assetProblems = new List<Problem>();
        var reportedMissing = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        var newUids = new Dictionary<long, long>();

        for (int i = 0; i < tiles.Count; i++)
        {
            work.ThrowIfCancellationRequested();
            (int fx, int fy) = tiles[i];
            var tileId = new WowTileId(sourceMap, fx, fy);
            context.Step($"Tile {i + 1}/{tiles.Count}: {tileId.Stem}");

            // Reported before the tile, not after: reporting (i + 1) here would leave the bar at 100%
            // for the whole of the last tile plus everything after the loop.
            context.Progress(i / (float)tiles.Count);

            ImportedTile parsed;
            try
            {
                using (context.Timings.Measure("parse ADT"))
                {
                    parsed = await Task.Run(() => WowAdtTileReader.Read(WowClientFiles.Shared, wdt, new ImportedTileId(sourceMap, fx, fy))).ConfigureAwait(false);
                }
            }
            catch (Exception e)
            {
                context.Log($"{tileId.Stem}: could not read — {e.Message}; skipped.");
                continue;
            }

            if (reimport.Contains((fx, fy)))
            {
                using (context.Timings.Measure("delete previous tile"))
                {
                    await DeleteExistingTileEntityAsync(storage, catalog.Map, fx, fy).ConfigureAwait(false);
                    await DeleteExistingWaterAsync(context, work, catalog.Map, grid, tileId).ConfigureAwait(false);
                }
            }

            IReadOnlyDictionary<(uint Texture, int Slot), int> materials;
            using (context.Timings.Measure("slot materials"))
            {
                materials = await AdtImportCatalog
                    .EnsureSlotMaterialsAsync(context, work, catalog.Map, TextureMapper.RequiredMaterials(parsed).ToList())
                    .ConfigureAwait(false);
            }

            IReadOnlyList<ImportedImageChunk> heightChunks;
            IReadOnlyList<(ImageChunkCoord Coord, byte[] Pixels)> alphaChunks;
            IReadOnlyList<(ImageChunkCoord Coord, byte[] Pixels)> holeChunks;
            IReadOnlyList<(ImageChunkCoord Coord, byte[] Pixels)> areaChunks;
            IReadOnlyList<(ImageChunkCoord Coord, byte[] Pixels)> flagsChunks;
            IReadOnlyList<ImportedImageChunk> vColorChunks;
            IReadOnlyList<ImportedImageChunk> vLightChunks;
            try
            {
                using (context.Timings.Measure("build image chunks"))
                {
                    heightChunks = BuildHeightChunks(tileId, parsed, heightImage.ChunkSize);
                    alphaChunks = BuildAlphaChunks(tileId, parsed, alphaImage.ChunkSize);
                    holeChunks = BuildHoleChunks(tileId, parsed, holeImage.ChunkSize);
                    areaChunks = BuildScalarChunks(tileId, parsed, areaImage.ChunkSize, AreaMapper.AreaValue);
                    flagsChunks = BuildScalarChunks(tileId, parsed, flagsImage.ChunkSize, AreaMapper.FlagsValue);
                    vColorChunks = BuildVertexChunks(tileId, parsed, vColorImage.ChunkSize, vertexColour: true);
                    vLightChunks = BuildVertexChunks(tileId, parsed, vLightImage.ChunkSize, vertexColour: false);
                }
            }
            catch (Exception e)
            {
                context.Log($"{tileId.Stem}: building image chunks failed: {e}");
                throw;
            }

            // All seven of a tile's images are read and merged together, rather than one context per
            // image: that collapses the read from three round trips to one. The actual write is folded
            // into the single transaction below, alongside the tile's other writes.
            IReadOnlyDictionary<PaintImage, IReadOnlyList<(ImageChunkCoord Coord, byte[] Pixels)>> imageWrites;
            try
            {
                using (context.Timings.Measure("write image chunks"))
                {
                    imageWrites = await BuildMergedImageWritesAsync(
                        storage,
                        new Dictionary<PaintImage, IReadOnlyList<ImportedImageChunk>>
                        {
                            [heightImage] = heightChunks,
                            [vColorImage] = vColorChunks,
                            [vLightImage] = vLightChunks,
                        },
                        new Dictionary<PaintImage, IReadOnlyList<(ImageChunkCoord Coord, byte[] Pixels)>>
                        {
                            [alphaImage] = alphaChunks,
                            [holeImage] = holeChunks,
                            [areaImage] = areaChunks,
                            [flagsImage] = flagsChunks,
                        }).ConfigureAwait(false);
                }
            }
            catch (Exception e)
            {
                context.Log($"{tileId.Stem}: merging image writes failed: {e}");
                throw;
            }

            if (i == 0)
            {
                float[] firstOuter = parsed.Chunks[0].OuterHeights;
                float rawMin = float.MaxValue, rawMax = float.MinValue;
                foreach (float h in firstOuter)
                {
                    rawMin = Math.Min(rawMin, h);
                    rawMax = Math.Max(rawMax, h);
                }

                float blockMin = float.MaxValue, blockMax = float.MinValue;
                if (heightChunks.Count > 0)
                {
                    var floats = new float[heightChunks[0].Pixels.Length / 4];
                    Buffer.BlockCopy(heightChunks[0].Pixels, 0, floats, 0, heightChunks[0].Pixels.Length);
                    foreach (float f in floats)
                    {
                        blockMin = Math.Min(blockMin, f);
                        blockMax = Math.Max(blockMax, f);
                    }
                }

                context.Log($"{tileId.Stem}: wrote {heightChunks.Count} height + {alphaChunks.Count} alpha image chunk(s) " +
                    $"({(heightImage.IsDiskBacked ? "disk" : "database")}); " +
                    $"first height chunk at {(heightChunks.Count > 0 ? heightChunks[0].Coord : default)}; " +
                    $"chunk[0] MCVT range [{rawMin:F1}, {rawMax:F1}]; height image pixels range [{blockMin:F1}, {blockMax:F1}].");
            }
            SceneEntity entity = BuildTileEntity(catalog, grid, tileId, parsed, materials);

            List<(SceneEntity Entity, long UniqueId)> placements;
            using (context.Timings.Measure("build placements"))
            {
                placements = await BuildPlacementEntitiesAsync(
                    context, assets, catalog.Map, parsed, knownUids, m2BoundsCache, wmoRootCache, assetProblems, reportedMissing).ConfigureAwait(false);
            }

            // Nothing here references a row that only a write-back would produce, so the tile entity,
            // placements, water models and water entities can all go in the same commit.
            List<IEntity> waterModels = [];
            Vector3[] waterOrigins = [];
            int waterBodyCount = 0;
            if (importWater)
            {
                (waterModels, waterOrigins, waterBodyCount) = await BuildWaterModelsAsync(context, work, grid, tileId, parsed).ConfigureAwait(false);
            }

            List<IEntity> waterEntities = waterModels.Count > 0
                ? BuildWaterEntities(context.Editor, catalog.Map, waterModels, waterOrigins)
                : [];

            var firstBatch = new List<IEntity> { entity };
            firstBatch.AddRange(placements.Select(p => (IEntity)p.Entity));
            firstBatch.AddRange(waterModels);
            firstBatch.AddRange(waterEntities);

            List<ChunkCoord> chunkCoords = WowTileAddressing.EnumerateChunks(tileId).Select(entry => entry.Coord).ToList();
            List<(int Map, int X, int Y)> chunkChangeRows = chunkCoords.Select(coord => (catalog.Map.Value, coord.X, coord.Y)).ToList();

            var tileRecord = new AdtImportTileRecord
            {
                ProjectMap = catalog.Map.Value,
                FileX = fx,
                FileY = fy,
                SourceId = sourceMap,
                SourceTileHash = Convert.ToHexString(SHA256.HashData(System.Text.Encoding.UTF8.GetBytes(wdt.Files?[(fy * MapEdge) + fx].Root.ToString() ?? tileId.Stem)))[..16],
                ImportedAtUtc = DateTime.UtcNow,
                ImporterVersion = ImporterVersion,
            };

            // One transaction for everything the tile writes: the entity commit, the image chunks, the
            // chunk-change stamps and the tracking row. Each keeps its own timing label; sharing one
            // transaction means the transaction floor is paid once.
            try
            {
                await storage.CommitTransactionAsync(async db =>
                {
                    using (context.Timings.Measure("commit tile"))
                    {
                        await storage.CommitEntitiesWithinTransactionAsync(db, firstBatch, []).ConfigureAwait(false);
                    }

                    DbTransaction tx = db.Database.CurrentTransaction!.GetDbTransaction();

                    using (context.Timings.Measure("write image chunks"))
                    {
                        if (imageWrites.Count > 0)
                        {
                            await storage.UpsertImageChunksWithinTransactionAsync(db, tx, imageWrites).ConfigureAwait(false);
                        }
                    }

                    using (context.Timings.Measure("mark chunks built"))
                    {
                        await storage.UpsertChunkChangesWithinTransactionAsync(db, tx, chunkChangeRows).ConfigureAwait(false);
                    }

                    using (context.Timings.Measure("tile record"))
                    {
                        // After the chunk stamps above, so a later edit is the only thing newer than this.
                        tileRecord.ImportedAtUtc = DateTime.UtcNow;
                        await AdtImportTileStore.UpsertWithinTransactionAsync(db, tileRecord).ConfigureAwait(false);
                    }
                }).ConfigureAwait(false);
            }
            catch (Exception e)
            {
                context.Log($"{tileId.Stem}: committing tile failed: {e}");
                throw;
            }

            foreach ((SceneEntity placed, long uniqueId) in placements)
            {
                if (placed.RecordId is { } recordId)
                {
                    newUids[recordId] = uniqueId;
                }
            }

            if (waterBodyCount > 0)
            {
                context.Log($"{tileId.Stem}: {waterBodyCount} water body(ies).");
            }

            imported++;
        }

        context.Progress(1.0f);

        if (newUids.Count > 0)
        {
            // Its own step: the tile loop is over and the bar is full, so without this the window
            // shows the last tile's name for however long the ledger write takes.
            context.Step($"Writing the uid ledger ({newUids.Count} placement(s))");
            using (context.Timings.Measure("uid ledger"))
            {
                await WowExportEntityIds.UpsertAsync(storage, newUids).ConfigureAwait(false);
            }
        }

        context.Step("Reporting problems");
        context.Problems.Replace("wow.import.adt", assetProblems);
        context.Report($"Imported {imported} tile(s) into '{targetMap}'.");
    }

    /// <summary>Turns a tile's MDDF/MODF records into <see cref="SceneEntity"/> + <c>ModelRendererComponent</c>
    /// pairs, one per not-yet-seen uniqueId. The transform inverts <see cref="WowPlacementScan"/>'s
    /// model-origin offset (<see cref="PlacementMapper"/>), which needs each model's local-bounds
    /// centre. That comes from the file's own header — an M2's MD20 bounds or a WMO's MOHD bounds —
    /// read once per path and cached, never from a fully materialized <see cref="ModelAsset"/>: the
    /// import reads one <see cref="Aabb"/> and keeps nothing else a mesh/material/texture build would
    /// produce. Missing client data is reported as a <see cref="Problem"/> (not a failure).</summary>
    private static async Task<List<(SceneEntity Entity, long UniqueId)>> BuildPlacementEntitiesAsync(
        BatchContext context,
        AssetSystem assets,
        MapId map,
        ImportedTile parsed,
        HashSet<long> knownUids,
        Dictionary<string, Aabb?> m2BoundsCache,
        Dictionary<string, WmoRoot?> wmoRootCache,
        List<Problem> assetProblems,
        HashSet<string> reportedMissing)
    {
        var result = new List<(SceneEntity, long)>();

        void ReportMissing(string path)
        {
            if (reportedMissing.Add(path))
            {
                assetProblems.Add(new Problem
                {
                    Key = $"missing:{path}",
                    Severity = ProblemSeverity.Warning,
                    Category = "WoW Import",
                    Kind = "MissingAsset",
                    Message = $"Referenced model '{path}' is not in the client data; its placement keeps its stored position without a bounds re-centre.",
                });
            }
        }

        async Task<Aabb?> M2BoundsOf(string path)
        {
            if (m2BoundsCache.TryGetValue(path, out Aabb? cached))
            {
                return cached;
            }

            byte[]? bytes = await assets.ReadAssetBytesAsync(path).ConfigureAwait(false);
            Aabb? bounds = bytes != null ? M2Reader.Read(bytes, path)?.Bounds : null;
            m2BoundsCache[path] = bounds;
            return bounds;
        }

        async Task<WmoRoot?> WmoRootOf(string path)
        {
            if (wmoRootCache.TryGetValue(path, out WmoRoot? cached))
            {
                return cached;
            }

            byte[]? bytes = await assets.ReadAssetBytesAsync(path).ConfigureAwait(false);
            WmoRoot? root = bytes != null ? WmoRootReader.Read(bytes, path) : null;
            wmoRootCache[path] = root;
            return root;
        }

        string ModelPathOf(uint fdid) => WowFileDataIds.Shared.PathOf(fdid) ?? $"unknown/{fdid}";

        foreach (ImportedDoodad doodad in parsed.Doodads)
        {
            if (!knownUids.Add(doodad.UniqueId))
            {
                continue;
            }

            Aabb? bounds = await M2BoundsOf(ModelPathOf(doodad.ModelFdid)).ConfigureAwait(false);
            if (bounds == null)
            {
                ReportMissing(ModelPathOf(doodad.ModelFdid));
            }

            Transform3D transform = PlacementMapper.InvertTransform(
                doodad.Position, doodad.RotationDegrees, doodad.Scale, bounds?.GetCenter() ?? Vector3.Zero);

            var entity = new MapSceneEntity { Name = NameOf(ModelPathOf(doodad.ModelFdid)), Map = map, Transform = transform };
            entity.AddComponent(new ModelRendererComponent(assets, context.Editor.MeshMaterials) { ModelPath = ModelPathOf(doodad.ModelFdid) });
            result.Add((entity, doodad.UniqueId));
        }

        foreach (ImportedObject obj in parsed.Objects)
        {
            if (!knownUids.Add(obj.UniqueId))
            {
                continue;
            }

            WmoRoot? root = await WmoRootOf(ModelPathOf(obj.WmoFdid)).ConfigureAwait(false);
            if (root == null)
            {
                ReportMissing(ModelPathOf(obj.WmoFdid));
            }

            ushort objScale = (obj.Flags & ModfHasScaleFlag) != 0 ? obj.Scale : (ushort)1024;
            Transform3D transform = PlacementMapper.InvertTransform(
                obj.Position, obj.RotationDegrees, objScale, root?.Bounds.GetCenter() ?? Vector3.Zero);

            var entity = new MapSceneEntity { Name = NameOf(ModelPathOf(obj.WmoFdid)), Map = map, Transform = transform };
            var renderer = new ModelRendererComponent(assets, context.Editor.MeshMaterials) { ModelPath = ModelPathOf(obj.WmoFdid) };
            renderer.SelectedDoodadSet = DoodadSetName(root, obj.DoodadSet);
            entity.AddComponent(renderer);
            result.Add((entity, obj.UniqueId));
        }

        return result;
    }

    private static string DoodadSetName(WmoRoot? root, ushort index)
    {
        if (index == 0 || root == null || index >= root.DoodadSets.Length)
        {
            return "";
        }

        WmoDoodadSet set = root.DoodadSets[index];
        return set.Name.Length > 0 ? set.Name : $"Set {index}";
    }

    private static string NameOf(string path)
    {
        int slash = path.LastIndexOfAny(['/', '\\']);
        return slash >= 0 ? path[(slash + 1)..] : path;
    }

    /// <summary>
    /// Turns a tile's MH2O into one <c>wow.water.surface</c> <see cref="ProceduralModel"/> per merged
    /// body (<see cref="WaterMapper"/>) and adds each to the catalog, but does not commit — the caller
    /// folds these, plus the <see cref="ProceduralComponent"/> placements <see cref="BuildWaterEntities"/>
    /// builds from the result, into the same transaction as the tile entity and placements (see
    /// <see cref="RunAsync"/>). The body's world-space lattice vertices become the model's network in
    /// entity-local space, with the returned origin at the body's XZ-bounds centre so the placement
    /// drags as a unit.
    /// </summary>
    /// <remarks>Only building the <see cref="ProceduralModel"/> itself needs the main thread
    /// (<see cref="EditorContext.Catalog"/>, <see cref="ProceduralModel.LoadNetwork"/>), mirroring
    /// <see cref="AdtImportCatalog.EnsureSlotMaterialsAsync"/>. Everything else — mapping bodies and
    /// building each network — runs first, in the background. This hops to the main thread only around
    /// that step and switches back before returning, so the caller never has to. Returns empty/zero,
    /// with no hop at all, when the tile has no water.</remarks>
    private static async Task<(List<IEntity> Models, Vector3[] Origins, int BodyCount)> BuildWaterModelsAsync(
        BatchContext context, WorkContext work, LandscapeGrid grid, WowTileId tileId, ImportedTile parsed)
    {
        IReadOnlyList<ImportedWaterBody> bodies;
        using (context.Timings.Measure("water: map bodies"))
        {
            bodies = WaterMapper.Map(
                parsed.Chunks.Select(chunk => chunk.Liquid).ToList(), tileId, grid, context.Log);
        }

        if (bodies.Count == 0)
        {
            return ([], [], 0);
        }

        var origins = new Vector3[bodies.Count];
        var networks = new VertexNetwork[bodies.Count];
        for (int i = 0; i < bodies.Count; i++)
        {
            ImportedWaterBody body = bodies[i];
            Vector3 origin = BodyOrigin(body.Vertices);
            origins[i] = origin;

            var localVertices = new Vector3[body.Vertices.Count];
            for (int v = 0; v < body.Vertices.Count; v++)
            {
                localVertices[v] = body.Vertices[v] - origin;
            }

            var faces = new int[body.Faces.Count][];
            for (int f = 0; f < body.Faces.Count; f++)
            {
                WaterQuad quad = body.Faces[f];
                faces[f] = [quad.A, quad.B, quad.C, quad.D];
            }

            // A whole-tile ocean is one body with thousands of quads; AddFace in a loop is O(n²).
            using (context.Timings.Measure("water: build network"))
            {
                networks[i] = VertexNetwork.FromMesh(localVertices, faces);
            }
        }

        // Timed separately from the loop above: a main-thread hop waits for the frame pump, so a slow
        // frame rate and slow water work are two different problems with two different fixes.
        using (context.Timings.Measure("water: hop to main"))
        {
            await work.SwitchToMain();
        }

        EditorContext editor = context.Editor;
        var models = new List<IEntity>(bodies.Count);
        try
        {
            for (int i = 0; i < bodies.Count; i++)
            {
                ImportedWaterBody body = bodies[i];

                var values = new MeshParameterValues();
                values.Set(WowLiquidParameters.LiquidTypeIdLookup, body.LiquidTypeId);
                values.Set(WowLiquidParameters.FrameDuration, 0.15f);
                values.Set(WaterSurfaceMeshFunction.UvScale, 0.05f);
                values.Set(WaterAttributeParameters.Fishable, body.Fishable);
                values.Set(WaterAttributeParameters.Fatigue, body.Fatigue);

                var model = new ProceduralModel
                {
                    Name = WaterModelName(tileId, i),
                    FunctionId = WaterSurfaceMeshFunction.FunctionId,
                    Parameters = values.Serialize(),
                };
                using (context.Timings.Measure("water: attach network"))
                {
                    model.LoadNetwork(networks[i]);
                }

                // Adding bumps Context.Catalog.Version, and ProceduralSystem.Index() rebuilds from the
                // whole catalog the next time it is read — doing this for every model before any entity
                // resolves ProceduralComponent.Model (which SceneEntity.AddComponent does) keeps that a
                // one-off cost instead of O(bodies²).
                using (context.Timings.Measure("water: catalog add"))
                {
                    editor.Catalog.AssignId(model);
                    editor.Catalog.Add(model);
                }

                models.Add(model);
            }
        }
        finally
        {
            await work.SwitchToBackground();
        }

        return (models, origins, bodies.Count);
    }

    /// <summary>Builds the <see cref="SceneEntity"/> + <see cref="ProceduralComponent"/> placement for
    /// each model <see cref="BuildWaterModelsAsync"/> built. <see cref="ProceduralComponent.ModelId"/>
    /// only needs the model's client-assigned <see cref="ProceduralModel.RecordId"/>, so this can run
    /// before the models are committed — there is no FK to satisfy.</summary>
    private static List<IEntity> BuildWaterEntities(EditorContext editor, MapId map, IReadOnlyList<IEntity> models, IReadOnlyList<Vector3> origins)
    {
        var entities = new List<IEntity>(models.Count);
        for (int i = 0; i < models.Count; i++)
        {
            var model = (ProceduralModel)models[i];
            var entity = new MapSceneEntity
            {
                Name = model.Name,
                Map = map,
                Transform = new Transform3D(Basis.Identity, origins[i]),
            };
            entity.AddComponent(new ProceduralComponent(editor.Procedural) { ModelId = model.RecordId });
            entities.Add(entity);
        }

        return entities;
    }

    /// <summary>Drops a re-imported tile's previously imported water — its <see cref="ProceduralComponent"/>
    /// entities (matched by the deterministic <see cref="WaterModelName"/> prefix, so hand-authored water
    /// in the same area is left alone) and the models they bind.</summary>
    private static async Task DeleteExistingWaterAsync(
        BatchContext context, WorkContext work, MapId map, LandscapeGrid grid, WowTileId tileId)
    {
        Aabb tileBounds = WowTileAddressing.EnumerateChunks(tileId)
            .Select(entry => grid.BoundsOf(entry.Coord))
            .Aggregate((a, b) => a.Merge(b));

        IReadOnlyList<SceneEntity> hits = await context.ScanSceneAsync(map, tileBounds).ConfigureAwait(false);

        await work.SwitchToMain();
        try
        {
            EditorContext editor = context.Editor;
            EditorStorage storage = editor.Database.EditorStorage;
            string prefix = WaterModelName(tileId, -1)[..^2]; // "ADT Water X_Y #"
            var entities = new List<IEntity>();
            var models = new List<IEntity>();

            foreach (SceneEntity entity in hits)
            {
                if (entity.Component<ProceduralComponent>()?.Model is not { } model ||
                    !model.Name.StartsWith(prefix, StringComparison.Ordinal))
                {
                    continue;
                }

                if (entity.RecordId is { } entityId)
                {
                    entities.Add(new MapSceneEntity { RecordId = entityId, Map = map });
                }

                if (model.RecordId is not null)
                {
                    models.Add(model);
                    editor.Catalog.Remove(model);
                }
            }

            if (entities.Count > 0)
            {
                await storage.CommitAsync([], entities).ConfigureAwait(false);
            }

            if (models.Count > 0)
            {
                await storage.CommitAsync([], models).ConfigureAwait(false);
            }
        }
        finally
        {
            await work.SwitchToBackground();
        }
    }

    private static string WaterModelName(WowTileId tile, int index) => $"ADT Water {tile.FileX}_{tile.FileY} #{index}";

    /// <summary>The XZ-bounds centre of a body's vertices, at Y = 0 — the entity origin the body's
    /// network is stored relative to.</summary>
    private static Vector3 BodyOrigin(IReadOnlyList<Vector3> vertices)
    {
        float minX = vertices[0].X, maxX = vertices[0].X, minZ = vertices[0].Z, maxZ = vertices[0].Z;
        foreach (Vector3 v in vertices)
        {
            minX = Math.Min(minX, v.X);
            maxX = Math.Max(maxX, v.X);
            minZ = Math.Min(minZ, v.Z);
            maxZ = Math.Max(maxZ, v.Z);
        }

        return new Vector3((minX + maxX) * 0.5f, 0.0f, (minZ + maxZ) * 0.5f);
    }

    /// <summary>Drops a re-imported tile's previous <see cref="AdtTileComponent"/> entity (its chunk
    /// rows cascade). Image chunks and placements are keyed deterministically, so the rewrite covers
    /// them.</summary>
    private static async Task DeleteExistingTileEntityAsync(EditorStorage storage, MapId map, int fileX, int fileY)
    {
        int? entityId;
        using (IDisposable read = await storage.Lock.ReaderAsync().ConfigureAwait(false))
        await using (EditorDbContext db = storage.CreateContext())
        {
            entityId = await db.Set<SceneAdtTileComponentRecord>().AsNoTracking()
                .Where(component => component.FileX == fileX && component.FileY == fileY)
                .Join(db.MapEntities.AsNoTracking(), component => component.EntityId, sceneEntity => sceneEntity.Id,
                    (component, sceneEntity) => new { sceneEntity.Id, sceneEntity.MapId })
                .Where(row => row.MapId == map.Value)
                .Select(row => (int?)row.Id)
                .FirstOrDefaultAsync().ConfigureAwait(false);
        }

        if (entityId is { } id)
        {
            await storage.CommitAsync([], [new MapSceneEntity { RecordId = id, Map = map }]).ConfigureAwait(false);
        }
    }

    /// <summary>
    /// Reads and merges every one of a tile's images into the writes the caller should make, without
    /// making them — the caller folds the actual upsert into the same transaction as the tile's other
    /// writes (see <see cref="RunAsync"/>). <paramref name="merged"/> images (height, vertex colour,
    /// vertex light) are merged over what is already stored rather than replacing it — a chunk row is
    /// written whole, and a tile's apron reaches into its neighbours' image chunks, so an unmerged write
    /// would replace an already-imported neighbouring tile's entire chunk with this tile's apron and
    /// nothing else. <paramref name="direct"/> images (alpha, hole, area, MCNK flags) have no apron and
    /// are passed through as given. Reading the three merged images together, rather than one context
    /// per image, is what collapses the read from three round trips to one.
    /// </summary>
    private static async Task<IReadOnlyDictionary<PaintImage, IReadOnlyList<(ImageChunkCoord Coord, byte[] Pixels)>>> BuildMergedImageWritesAsync(
        EditorStorage storage,
        IReadOnlyDictionary<PaintImage, IReadOnlyList<ImportedImageChunk>> merged,
        IReadOnlyDictionary<PaintImage, IReadOnlyList<(ImageChunkCoord Coord, byte[] Pixels)>> direct)
    {
        var wanted = new Dictionary<PaintImage, IReadOnlyCollection<ImageChunkCoord>>();
        foreach ((PaintImage image, IReadOnlyList<ImportedImageChunk> chunks) in merged)
        {
            if (chunks.Count > 0)
            {
                wanted[image] = chunks.Select(chunk => chunk.Coord).ToList();
            }
        }

        IReadOnlyList<(PaintImage Image, ImageChunkCoord Coord, byte[] Pixels)> stored = wanted.Count > 0
            ? await storage.LoadImageChunksAsync(wanted).ConfigureAwait(false)
            : [];

        Dictionary<PaintImage, Dictionary<ImageChunkCoord, byte[]>> storedByImage = stored
            .GroupBy(row => row.Image)
            .ToDictionary(group => group.Key, group => group.ToDictionary(row => row.Coord, row => row.Pixels));

        var writes = new Dictionary<PaintImage, IReadOnlyList<(ImageChunkCoord Coord, byte[] Pixels)>>();
        foreach ((PaintImage image, IReadOnlyList<ImportedImageChunk> chunks) in merged)
        {
            if (chunks.Count == 0)
            {
                continue;
            }

            Dictionary<ImageChunkCoord, byte[]> previous = storedByImage.TryGetValue(image, out Dictionary<ImageChunkCoord, byte[]>? found)
                ? found
                : [];
            var result = new List<(ImageChunkCoord Coord, byte[] Pixels)>(chunks.Count);
            foreach (ImportedImageChunk chunk in chunks)
            {
                result.Add(previous.TryGetValue(chunk.Coord, out byte[]? already) && already.Length == chunk.Pixels.Length
                    ? (chunk.Coord, MergeOver(already, chunk, image.Stride))
                    : (chunk.Coord, chunk.Pixels));
            }

            writes[image] = result;
        }

        foreach ((PaintImage image, IReadOnlyList<(ImageChunkCoord Coord, byte[] Pixels)> chunks) in direct)
        {
            if (chunks.Count > 0)
            {
                writes[image] = chunks;
            }
        }

        return writes;
    }

    /// <summary>The tile's own texels over what is stored; an apron texel only where the stored pixel
    /// is still blank, so a neighbouring tile's real value survives whichever tile is imported first.</summary>
    private static byte[] MergeOver(byte[] stored, ImportedImageChunk chunk, int stride)
    {
        var result = (byte[])stored.Clone();
        for (int pixel = 0; pixel < chunk.Owned.Length; pixel++)
        {
            int at = pixel * stride;
            if (chunk.Owned[pixel] || IsBlank(result, at, stride))
            {
                Array.Copy(chunk.Pixels, at, result, at, stride);
            }
        }

        return result;
    }

    private static bool IsBlank(byte[] pixels, int at, int stride)
    {
        for (int i = 0; i < stride; i++)
        {
            if (pixels[at + i] != 0)
            {
                return false;
            }
        }

        return true;
    }

    /// <summary>Every one of the tile's 256 chunks' 32×32 height blocks, placed into the map-wide
    /// image's texel grid and split into image chunks, plus a one-texel apron replicating the imported
    /// region's outer edge so a boundary vertex does not blend against unwritten zero.</summary>
    internal static IReadOnlyList<ImportedImageChunk> BuildHeightChunks(WowTileId tile, ImportedTile parsed, int chunkSize)
    {
        var buffers = new Dictionary<ImageChunkCoord, float[]>();
        var owned = new Dictionary<ImageChunkCoord, bool[]>();

        void Stamp(int texelX, int texelY, float value)
        {
            if (texelX < 0 || texelY < 0)
            {
                return;
            }

            var coord = new ImageChunkCoord(texelX / chunkSize, texelY / chunkSize);
            if (!buffers.TryGetValue(coord, out float[]? buffer))
            {
                buffer = new float[chunkSize * chunkSize];
                buffers[coord] = buffer;
                owned[coord] = new bool[chunkSize * chunkSize];
            }

            int at = ((texelY % chunkSize) * chunkSize) + (texelX % chunkSize);
            buffer[at] = value;
            owned[coord][at] = true;
        }

        foreach ((int localCol, int localRow, ChunkCoord coord) in WowTileAddressing.EnumerateChunks(tile))
        {
            ImportedChunk chunk = parsed.Chunks[(localRow * WowTileAddressing.ChunksPerTileEdge) + localCol];
            float[] block = HeightMapper.ChunkBlock(chunk.OuterHeights, chunk.InnerHeights);

            int baseX = HeightMapper.TexelEdge * (coord.X - MapMinChunk.X);
            int baseY = HeightMapper.TexelEdge * (coord.Y - MapMinChunk.Y);

            for (int by = 0; by < HeightMapper.TexelEdge; by++)
            {
                for (int bx = 0; bx < HeightMapper.TexelEdge; bx++)
                {
                    Stamp(baseX + bx, baseY + by, block[(by * HeightMapper.TexelEdge) + bx]);
                }
            }
        }

        WriteApron(buffers, owned, chunkSize);

        return buffers
            .Select(pair => new ImportedImageChunk(
                pair.Key,
                ToBytes(pair.Value),
                owned.TryGetValue(pair.Key, out bool[]? mask) ? mask : new bool[chunkSize * chunkSize]))
            .ToList();
    }

    /// <summary>Grows the tile's own texel region by one texel, replicating the nearest written value —
    /// enough for a bilinear read at a boundary vertex not to reach unwritten zero. Apron texels are
    /// left out of <paramref name="owned"/>, so <see cref="UpsertMergedManyAsync"/> never lets them
    /// displace a neighbouring tile's real values.</summary>
    private static void WriteApron(
        Dictionary<ImageChunkCoord, float[]> buffers,
        Dictionary<ImageChunkCoord, bool[]> owned,
        int chunkSize)
    {
        // Point-in-region test against the per-chunk masks already built by the stamping loop, instead
        // of a HashSet<(int,int)> duplicating the same membership — a tuple hashset's Contains was the
        // measured cost, and owned already answers the same question in O(1) without it.
        bool IsWritten(int x, int y) =>
            owned.TryGetValue(new ImageChunkCoord(x / chunkSize, y / chunkSize), out bool[]? mask)
            && mask[((y % chunkSize) * chunkSize) + (x % chunkSize)];

        var apron = new Dictionary<(int X, int Y), float>();
        foreach ((ImageChunkCoord coord, bool[] mask) in owned)
        {
            for (int y = 0; y < chunkSize; y++)
            {
                for (int x = 0; x < chunkSize; x++)
                {
                    if (!mask[(y * chunkSize) + x])
                    {
                        continue;
                    }

                    int wx = (coord.X * chunkSize) + x;
                    int wy = (coord.Y * chunkSize) + y;
                    float value = SampleWritten(buffers, chunkSize, wx, wy);
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = -1; dx <= 1; dx++)
                        {
                            (int nx, int ny) = (wx + dx, wy + dy);
                            if (nx >= 0 && ny >= 0 && !IsWritten(nx, ny))
                            {
                                apron.TryAdd((nx, ny), value);
                            }
                        }
                    }
                }
            }
        }

        foreach (((int ax, int ay), float value) in apron)
        {
            var coord = new ImageChunkCoord(ax / chunkSize, ay / chunkSize);
            if (!buffers.TryGetValue(coord, out float[]? buffer))
            {
                buffer = new float[chunkSize * chunkSize];
                buffers[coord] = buffer;
            }

            buffer[((ay % chunkSize) * chunkSize) + (ax % chunkSize)] = value;
        }
    }

    private static float SampleWritten(Dictionary<ImageChunkCoord, float[]> buffers, int chunkSize, int x, int y) =>
        buffers.TryGetValue(new ImageChunkCoord(x / chunkSize, y / chunkSize), out float[]? buffer)
            ? buffer[((y % chunkSize) * chunkSize) + (x % chunkSize)]
            : 0.0f;

    private static byte[] ToBytes(float[] pixels)
    {
        var bytes = new byte[pixels.Length * 4];
        Buffer.BlockCopy(pixels, 0, bytes, 0, bytes.Length);
        return bytes;
    }

    /// <summary>The tile's 256 chunks' editor-oriented RGBA alpha blocks (<see cref="TextureMapper.AlphaBlock"/>),
    /// placed into the map-wide alpha image's texel grid. No apron — alpha is read at texel centres, so one image
    /// pixel per channel texel is already exact.</summary>
    internal static IReadOnlyList<(ImageChunkCoord Coord, byte[] Pixels)> BuildAlphaChunks(WowTileId tile, ImportedTile parsed, int chunkSize)
    {
        var buffers = new Dictionary<ImageChunkCoord, byte[]>();

        foreach ((int localCol, int localRow, ChunkCoord coord) in WowTileAddressing.EnumerateChunks(tile))
        {
            ImportedChunk chunk = parsed.Chunks[(localRow * WowTileAddressing.ChunksPerTileEdge) + localCol];
            byte[] block = TextureMapper.AlphaBlock(chunk);

            int baseX = TextureMapper.AlphaEdge * (coord.X - MapMinChunk.X);
            int baseY = TextureMapper.AlphaEdge * (coord.Y - MapMinChunk.Y);
            if (baseX < 0 || baseY < 0)
            {
                continue;
            }

            var imageChunk = new ImageChunkCoord(baseX / chunkSize, baseY / chunkSize);
            if (!buffers.TryGetValue(imageChunk, out byte[]? buffer))
            {
                buffer = new byte[chunkSize * chunkSize * 4];
                buffers[imageChunk] = buffer;
            }

            for (int by = 0; by < TextureMapper.AlphaEdge; by++)
            {
                int rowStart = ((((baseY + by) % chunkSize) * chunkSize) + (baseX % chunkSize)) * 4;
                Array.Copy(block, by * TextureMapper.AlphaEdge * 4, buffer, rowStart, TextureMapper.AlphaEdge * 4);
            }
        }

        return buffers.Select(pair => (pair.Key, pair.Value)).ToList();
    }

    /// <summary>The tile's 256 chunks' editor-oriented 4×4 hole byte blocks (<see cref="HoleMapper.HoleBlock"/>),
    /// placed into the map-wide hole image's texel grid. Read at texel centres, so no apron.</summary>
    internal static IReadOnlyList<(ImageChunkCoord Coord, byte[] Pixels)> BuildHoleChunks(WowTileId tile, ImportedTile parsed, int chunkSize)
    {
        var buffers = new Dictionary<ImageChunkCoord, byte[]>();

        foreach ((int localCol, int localRow, ChunkCoord coord) in WowTileAddressing.EnumerateChunks(tile))
        {
            ImportedChunk chunk = parsed.Chunks[(localRow * WowTileAddressing.ChunksPerTileEdge) + localCol];
            if (!HoleMapper.HasAny(chunk.Holes))
            {
                continue;
            }

            byte[] block = HoleMapper.HoleBlock(chunk.Holes);
            int baseX = HoleMapper.HoleEdge * (coord.X - MapMinChunk.X);
            int baseY = HoleMapper.HoleEdge * (coord.Y - MapMinChunk.Y);
            if (baseX < 0 || baseY < 0)
            {
                continue;
            }

            var imageChunk = new ImageChunkCoord(baseX / chunkSize, baseY / chunkSize);
            if (!buffers.TryGetValue(imageChunk, out byte[]? buffer))
            {
                buffer = new byte[chunkSize * chunkSize];
                buffers[imageChunk] = buffer;
            }

            for (int by = 0; by < HoleMapper.HoleEdge; by++)
            {
                int rowStart = (((baseY + by) % chunkSize) * chunkSize) + (baseX % chunkSize);
                Array.Copy(block, by * HoleMapper.HoleEdge, buffer, rowStart, HoleMapper.HoleEdge);
            }
        }

        return buffers.Select(pair => (pair.Key, pair.Value)).ToList();
    }

    /// <summary>The tile's 256 chunks' value of one MCNK header scalar — <paramref name="value"/> is
    /// <see cref="AreaMapper.AreaValue"/> or <see cref="AreaMapper.FlagsValue"/> — one float texel per
    /// MCNK, placed into a map-wide 1-component float image. Read at texel centres, so no apron.</summary>
    internal static IReadOnlyList<(ImageChunkCoord Coord, byte[] Pixels)> BuildScalarChunks(
        WowTileId tile, ImportedTile parsed, int chunkSize, System.Func<ImportedChunk, float> value)
    {
        var buffers = new Dictionary<ImageChunkCoord, float[]>();

        foreach ((int localCol, int localRow, ChunkCoord coord) in WowTileAddressing.EnumerateChunks(tile))
        {
            ImportedChunk chunk = parsed.Chunks[(localRow * WowTileAddressing.ChunksPerTileEdge) + localCol];

            int baseX = AdtImportCatalog.ChunkResolution * (coord.X - MapMinChunk.X);
            int baseY = AdtImportCatalog.ChunkResolution * (coord.Y - MapMinChunk.Y);
            if (baseX < 0 || baseY < 0)
            {
                continue;
            }

            var imageChunk = new ImageChunkCoord(baseX / chunkSize, baseY / chunkSize);
            if (!buffers.TryGetValue(imageChunk, out float[]? buffer))
            {
                buffer = new float[chunkSize * chunkSize];
                buffers[imageChunk] = buffer;
            }

            buffer[(((baseY % chunkSize) * chunkSize) + (baseX % chunkSize))] = value(chunk);
        }

        return buffers.Select(pair => (pair.Key, ToBytes(pair.Value))).ToList();
    }

    /// <summary>The tile's 256 chunks' 32×32 RGBA vertex-colour (or vertex-light) blocks, laid out
    /// like heights and placed into the map-wide image, plus the same one-texel apron heights
    /// use since these are vertex-sampled too. A chunk with an all-white MCCV (or all-black MCLV)
    /// contributes nothing.</summary>
    internal static IReadOnlyList<ImportedImageChunk> BuildVertexChunks(
        WowTileId tile, ImportedTile parsed, int chunkSize, bool vertexColour)
    {
        var buffers = new Dictionary<ImageChunkCoord, byte[]>();
        var owned = new Dictionary<ImageChunkCoord, bool[]>();

        void Put(int texelX, int texelY, ReadOnlySpan<byte> rgba)
        {
            var coord = new ImageChunkCoord(texelX / chunkSize, texelY / chunkSize);
            if (!buffers.TryGetValue(coord, out byte[]? buffer))
            {
                buffer = new byte[chunkSize * chunkSize * 4];
                buffers[coord] = buffer;
            }

            int at = (((texelY % chunkSize) * chunkSize) + (texelX % chunkSize)) * 4;
            rgba.CopyTo(buffer.AsSpan(at, 4));
        }

        foreach ((int localCol, int localRow, ChunkCoord coord) in WowTileAddressing.EnumerateChunks(tile))
        {
            ImportedChunk chunk = parsed.Chunks[(localRow * WowTileAddressing.ChunksPerTileEdge) + localCol];
            Color[]? source = vertexColour ? chunk.VertexColors : chunk.VertexLight;
            Color[]? inner = vertexColour ? chunk.InnerVertexColors : chunk.InnerVertexLight;
            bool present = vertexColour ? VertexColorMapper.HasVertexColor(source, inner) : VertexColorMapper.HasVertexLight(source, inner);
            if (!present)
            {
                continue;
            }

            byte[] block = vertexColour ? VertexColorMapper.ColorBlock(source!, inner) : VertexColorMapper.LightBlock(source!, inner);
            int baseX = VertexColorMapper.TexelEdge * (coord.X - MapMinChunk.X);
            int baseY = VertexColorMapper.TexelEdge * (coord.Y - MapMinChunk.Y);
            if (baseX < 0 || baseY < 0)
            {
                continue;
            }

            for (int by = 0; by < VertexColorMapper.TexelEdge; by++)
            {
                for (int bx = 0; bx < VertexColorMapper.TexelEdge; bx++)
                {
                    int texelX = baseX + bx;
                    int texelY = baseY + by;
                    Put(texelX, texelY, block.AsSpan((((by * VertexColorMapper.TexelEdge) + bx) * 4), 4));

                    var imageChunk = new ImageChunkCoord(texelX / chunkSize, texelY / chunkSize);
                    if (!owned.TryGetValue(imageChunk, out bool[]? mask))
                    {
                        mask = new bool[chunkSize * chunkSize];
                        owned[imageChunk] = mask;
                    }

                    mask[((texelY % chunkSize) * chunkSize) + (texelX % chunkSize)] = true;
                }
            }
        }

        // Point-in-region test against the per-chunk masks already built above, instead of a
        // HashSet<(int,int)> duplicating the same membership — a tuple hashset's Contains was the
        // measured cost, and owned already answers the same question in O(1) without it.
        bool IsWritten(int x, int y) =>
            owned.TryGetValue(new ImageChunkCoord(x / chunkSize, y / chunkSize), out bool[]? mask)
            && mask[((y % chunkSize) * chunkSize) + (x % chunkSize)];

        // Replicate the written region's edge one texel outward.
        var apron = new Dictionary<(int X, int Y), byte[]>();
        foreach ((ImageChunkCoord coord, bool[] mask) in owned)
        {
            for (int y = 0; y < chunkSize; y++)
            {
                for (int x = 0; x < chunkSize; x++)
                {
                    if (!mask[(y * chunkSize) + x])
                    {
                        continue;
                    }

                    int wx = (coord.X * chunkSize) + x;
                    int wy = (coord.Y * chunkSize) + y;
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = -1; dx <= 1; dx++)
                        {
                            (int nx, int ny) = (wx + dx, wy + dy);
                            if (nx >= 0 && ny >= 0 && !IsWritten(nx, ny) && !apron.ContainsKey((nx, ny)))
                            {
                                apron[(nx, ny)] = ReadTexel(buffers, chunkSize, wx, wy);
                            }
                        }
                    }
                }
            }
        }

        foreach (((int ax, int ay), byte[] rgba) in apron)
        {
            Put(ax, ay, rgba);
        }

        return buffers
            .Select(pair => new ImportedImageChunk(
                pair.Key,
                pair.Value,
                owned.TryGetValue(pair.Key, out bool[]? mask) ? mask : new bool[chunkSize * chunkSize]))
            .ToList();
    }

    private static byte[] ReadTexel(Dictionary<ImageChunkCoord, byte[]> buffers, int chunkSize, int x, int y)
    {
        var rgba = new byte[4];
        if (buffers.TryGetValue(new ImageChunkCoord(x / chunkSize, y / chunkSize), out byte[]? buffer))
        {
            int at = (((y % chunkSize) * chunkSize) + (x % chunkSize)) * 4;
            buffer.AsSpan(at, 4).CopyTo(rgba);
        }

        return rgba;
    }

    private static SceneEntity BuildTileEntity(
        AdtImportCatalog catalog,
        LandscapeGrid grid,
        WowTileId tile,
        ImportedTile parsed,
        IReadOnlyDictionary<(uint Texture, int Slot), int> materials)
    {
        Aabb bounds = WowTileAddressing.EnumerateChunks(tile)
            .Select(entry => grid.BoundsOf(entry.Coord))
            .Aggregate((a, b) => a.Merge(b));
        Vector3 centre = bounds.Position + (bounds.Size * 0.5f);

        var component = new AdtTileComponent
        {
            FileX = tile.FileX,
            FileY = tile.FileY,
            HeightLayerId = catalog.HeightLayerId,
            HeightMaterialId = catalog.HeightMaterialId,
            HoleLayerId = catalog.HoleLayerId,
            HoleMaterialId = catalog.HoleMaterialId,
            VertexColorLayerId = catalog.VertexColorLayerId,
            VertexColorMaterialId = catalog.VertexColorMaterialId,
            VertexLightLayerId = catalog.VertexLightLayerId,
            VertexLightMaterialId = catalog.VertexLightMaterialId,
            ChunkDataLayerId = catalog.ChunkLayerId,
            ChunkDataMaterialId = catalog.ChunkMaterialId,
            SlotLayerIds = catalog.SlotLayerIds.Select(id => (int?)id).ToArray(),
        };

        foreach ((int localCol, int localRow, _) in WowTileAddressing.EnumerateChunks(tile))
        {
            ImportedChunk chunk = parsed.Chunks[(localRow * WowTileAddressing.ChunksPerTileEdge) + localCol];
            component.SetChunk(localCol, localRow, new AdtChunkFacts(
                TextureMapper.SlotAssignments(chunk, parsed.Textures, materials),
                HoleMapper.HasAny(chunk.Holes),
                VertexColorMapper.HasVertexColor(chunk.VertexColors, chunk.InnerVertexColors),
                VertexColorMapper.HasVertexLight(chunk.VertexLight, chunk.InnerVertexLight)));
        }

        var entity = new MapSceneEntity { Name = $"ADT {tile.FileX},{tile.FileY}", Map = catalog.Map };
        entity.Transform = new Transform3D(Basis.Identity, new Vector3(centre.X, 0.0f, centre.Z));
        entity.AddComponent(component);
        return entity;
    }
}
