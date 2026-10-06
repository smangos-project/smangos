using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>One texture layer of a chunk as scripts see it. <c>Weight</c> is the layer's share at the queried
/// point, or its share of the whole chunk when no point was given.</summary>
public sealed class AdtLayerDescriptor
{
    public AdtLayerDescriptor(string texture, int slot, double weight, double coverage, int materialId, int effectId)
    {
        Texture = texture;
        Slot = slot;
        Weight = weight;
        Coverage = coverage;
        MaterialId = materialId;
        EffectId = effectId;
    }

    [ScriptProperty] public string Texture { get; }
    [ScriptProperty] public int Slot { get; }
    [ScriptProperty] public double Weight { get; }
    [ScriptProperty] public double Coverage { get; }
    [ScriptProperty] public int MaterialId { get; }
    [ScriptProperty] public int EffectId { get; }
}

/// <summary>Where an imported chunk is: its tile's file indices and its place in the tile.</summary>
public class AdtChunkAddressDescriptor
{
    public AdtChunkAddressDescriptor(AdtChunkRef chunk)
    {
        TileX = chunk.Tile.FileX;
        TileY = chunk.Tile.FileY;
        Col = chunk.Col;
        Row = chunk.Row;
    }

    [ScriptProperty] public int TileX { get; }
    [ScriptProperty] public int TileY { get; }
    [ScriptProperty] public int Col { get; }
    [ScriptProperty] public int Row { get; }
}

/// <summary>An imported chunk and its texture stack, bottom layer first.</summary>
public sealed class AdtChunkDescriptor : AdtChunkAddressDescriptor
{
    public AdtChunkDescriptor(AdtChunkRef chunk, AdtLayerDescriptor[] layers, string? overridden)
        : base(chunk)
    {
        Layers = layers;
        Overridden = overridden ?? "";
    }

    [ScriptProperty] public AdtLayerDescriptor[] Layers { get; }

    /// <summary>Why the chunk renders differently from its slots; empty when it does not.</summary>
    [ScriptProperty] public string Overridden { get; }
}

/// <summary>What a whole-chunk operation did.</summary>
public sealed class AdtEditResultDescriptor
{
    public AdtEditResultDescriptor(AdtTextureEditResult result)
    {
        Changed = result.Changed;
        Skipped = result.Skipped;
    }

    [ScriptProperty] public int Changed { get; }
    [ScriptProperty] public int Skipped { get; }
}

/// <summary>What a brush stroke did.</summary>
public sealed class AdtStrokeResultDescriptor
{
    public AdtStrokeResultDescriptor(bool changed, AdtChunkAddressDescriptor[] unpaintable, int loading)
    {
        Changed = changed;
        Unpaintable = unpaintable;
        Loading = loading;
    }

    [ScriptProperty] public bool Changed { get; }

    /// <summary>Chunks that could not take the texture: full, with nothing unused to free.</summary>
    [ScriptProperty] public AdtChunkAddressDescriptor[] Unpaintable { get; }

    /// <summary>Chunks skipped because their alpha data is not loaded yet.</summary>
    [ScriptProperty] public int Loading { get; }
}

/// <summary>A texture and how many chunks use it.</summary>
public sealed class AdtTextureUsageDescriptor
{
    public AdtTextureUsageDescriptor(string texture, int chunks)
    {
        Texture = texture;
        Chunks = chunks;
    }

    [ScriptProperty] public string Texture { get; }
    [ScriptProperty] public int Chunks { get; }
}

/// <summary>The ADT texture tool's settings as scripts see them.</summary>
public sealed class AdtBrushDescriptor : BrushDescriptor
{
    public AdtBrushDescriptor(Brush brush, AdtTexturePaintOptions options)
        : base(brush)
    {
        Texture = options.Texture;
        TargetWeight = options.TargetWeight;
        CleanupThreshold = options.CleanupThreshold;
    }

    [ScriptProperty] public string Texture { get; }
    [ScriptProperty] public double TargetWeight { get; }
    [ScriptProperty] public double CleanupThreshold { get; }
}

/// <summary>
/// The ADT texture tool as <c>wms.adtTextures</c>: everything the tool does, on imported chunks. Brush options
/// are the shared vocabulary (<c>Radius</c>, <c>Strength</c>, <c>Hardness</c>, ...) plus <c>TargetWeight</c> and
/// <c>CleanupThreshold</c>. A chunk is addressed as <c>{ TileX, TileY, Col, Row }</c> or <c>[tileX, tileY, col,
/// row]</c>; a region as <c>{ MinX, MinZ, MaxX, MaxZ }</c> in world units. Each edit is one undo step.
/// </summary>
[Subsystem(nameof(ScriptingSystem))]
public sealed class AdtTextureScriptApi : IScriptModule
{
    private readonly EditorContext _context;

    public AdtTextureScriptApi(ScriptingSystem system)
    {
        _context = system.Context;
    }

    public string Name => "adtTextures";

    /// <summary>Supplies the world in place of the current map's. Tests only.</summary>
    internal Func<AdtTextureWorld?>? WorldSource { get; set; }

    private AdtTextureToolFactory Factory => _context.Tools.Factories.OfType<AdtTextureToolFactory>().Single();

    /// <summary>The tool's brush and texture settings.</summary>
    [ScriptProperty]
    public AdtBrushDescriptor CurrentBrush => new(Factory.Brush, Factory.Options);

    /// <summary>Changes the tool's settings: any brush option, <c>Texture</c>, <c>TargetWeight</c>, <c>CleanupThreshold</c>.</summary>
    [ScriptFunction]
    public AdtBrushDescriptor SetBrush(object? options)
    {
        Apply(Factory.Brush, Factory.Options, options);
        if (Factory.Options.Texture.Length > 0)
        {
            Factory.Use(Factory.Options.Texture);
        }

        return CurrentBrush;
    }

    /// <summary>The chunk at a world point, with each layer's weight there.</summary>
    [ScriptFunction]
    public AdtChunkDescriptor At(double x, double z)
    {
        AdtTextureWorld world = RequireWorld();
        var point = new Vector3((float)x, 0.0f, (float)z);
        if (!world.TryChunkAt(point, out AdtChunkRef chunk))
        {
            throw new InvalidOperationException("There is no imported chunk at that point.");
        }

        return Describe(world, chunk, point);
    }

    /// <summary>A chunk addressed by its tile's file indices and its column and row in the tile.</summary>
    [ScriptFunction]
    public AdtChunkDescriptor Chunk(int tileX, int tileY, int col, int row)
    {
        AdtTextureWorld world = RequireWorld();
        return Describe(world, RequireChunk(world, AdtScriptArguments.ChunkAt(tileX, tileY, col, row)), null);
    }

    /// <summary>
    /// Paints <paramref name="texture"/> along <paramref name="points"/> ([x, z] in world space) as one stroke.
    /// <paramref name="options"/> overrides the tool's settings for this call only.
    /// </summary>
    [ScriptFunction]
    public AdtStrokeResultDescriptor Paint(string texture, object? points, object? options = null)
    {
        AdtTexturePaintOptions paint = Factory.Options.Clone();
        paint.Texture = texture;
        paint.Kind = AdtTextureStrokeKind.Paint;
        return Stroke(points, options, paint);
    }

    /// <summary>
    /// Swaps <paramref name="from"/> for <paramref name="to"/>. <paramref name="options"/> picks where:
    /// <c>Chunks</c> (whole chunks), <c>Tile</c> ([tileX, tileY], every imported chunk of it), or <c>Points</c>
    /// with a brush <c>Radius</c> (under the brush; <c>WholeChunk</c> swaps in every chunk it touches). For a
    /// brush swap <c>Changed</c> is 1 when anything changed and <c>Skipped</c> counts chunks not loaded yet.
    /// </summary>
    [ScriptFunction]
    public AdtEditResultDescriptor Replace(string from, string to, object? options)
    {
        IDictionary<string, object?> map = ScriptJson.AsMap(options) ?? throw new ArgumentException("Replace needs Chunks, Tile or Points.");
        AdtTextureWorld world = RequireWorld();

        if (AdtScriptArguments.Find(map, "points") is { } points)
        {
            AdtTexturePaintOptions paint = Factory.Options.Clone();
            paint.Texture = to;
            paint.ReplaceFrom = from;
            paint.Kind = AdtTextureStrokeKind.Replace;
            AdtStrokeResultDescriptor stroke = Stroke(points, options, paint);
            return new AdtEditResultDescriptor(new AdtTextureEditResult(stroke.Changed ? 1 : 0, stroke.Loading));
        }

        var edit = new AdtTextureEdit(world, _context.EditSessions);
        if (AdtScriptArguments.Find(map, "chunks") is { } chunks)
        {
            return new AdtEditResultDescriptor(edit.Replace(from, to, AdtScriptArguments.Chunks(chunks)));
        }

        if (AdtScriptArguments.Find(map, "tile") is { } tile)
        {
            return new AdtEditResultDescriptor(edit.Replace(from, to, ChunksOfTile(world, tile)));
        }

        throw new ArgumentException("Replace needs Chunks, Tile or Points.");
    }

    /// <summary>Removes <paramref name="texture"/> from the given chunks, its weight going to the others.</summary>
    [ScriptFunction]
    public AdtEditResultDescriptor Remove(string texture, object? chunks)
    {
        AdtTextureWorld world = RequireWorld();
        return new AdtEditResultDescriptor(new AdtTextureEdit(world, _context.EditSessions).Remove(texture, AdtScriptArguments.Chunks(chunks)));
    }

    /// <summary>Swaps the stacking order of two layers of one chunk.</summary>
    [ScriptFunction]
    public AdtEditResultDescriptor Swap(object? chunk, int slotA, int slotB)
    {
        AdtTextureWorld world = RequireWorld();
        return new AdtEditResultDescriptor(new AdtTextureEdit(world, _context.EditSessions)
            .Swap(AdtScriptArguments.Chunks(new[] { chunk }).Single(), slotA, slotB));
    }

    /// <summary>Drops layers nothing shows, in the given chunks or every loaded imported chunk.</summary>
    [ScriptFunction]
    public AdtEditResultDescriptor Cleanup(object? chunks = null, double? threshold = null)
    {
        AdtTextureWorld world = RequireWorld();
        return new AdtEditResultDescriptor(new AdtTextureEdit(world, _context.EditSessions).Cleanup(
            chunks == null ? AllChunks(world) : AdtScriptArguments.Chunks(chunks),
            threshold is { } value ? (float)value : AdtChunkTextureSet.VisibleThreshold));
    }

    /// <summary>Merges layers holding the same texture, in the given chunks or every loaded imported chunk.</summary>
    [ScriptFunction]
    public AdtEditResultDescriptor RemoveDuplicates(object? chunks = null)
    {
        AdtTextureWorld world = RequireWorld();
        return new AdtEditResultDescriptor(new AdtTextureEdit(world, _context.EditSessions)
            .RemoveDuplicates(chunks == null ? AllChunks(world) : AdtScriptArguments.Chunks(chunks)));
    }

    /// <summary>Chunks in <paramref name="region"/> that could not take <paramref name="texture"/>.</summary>
    [ScriptFunction]
    public AdtChunkAddressDescriptor[] Unpaintable(string texture, object? region)
    {
        AdtTextureWorld world = RequireWorld();
        var result = new List<AdtChunkAddressDescriptor>();
        foreach (ChunkCoord coord in ParseRegion(world, region))
        {
            if (AdtChunkEdit.TryLoad(world, coord, out AdtChunkEdit? edit) == AdtChunkLoadResult.Loaded && !edit!.Set.CanAdd(texture))
            {
                result.Add(new AdtChunkAddressDescriptor(edit.Chunk));
            }
        }

        return result.ToArray();
    }

    /// <summary>Every distinct texture in use, with how many chunks use it, in a region or everywhere loaded.</summary>
    [ScriptFunction]
    public AdtTextureUsageDescriptor[] Textures(object? region = null)
    {
        AdtTextureWorld world = RequireWorld();
        var counts = new Dictionary<string, (string Texture, int Chunks)>();
        foreach (ChunkCoord coord in region == null ? AllChunks(world) : ParseRegion(world, region))
        {
            if (!world.TryChunk(coord, out AdtChunkRef chunk) ||
                world.TexturesOf(chunk.Tile.Chunks[(chunk.Col, chunk.Row)]) is not { } textures)
            {
                continue;
            }

            foreach (string texture in textures.Distinct(StringComparer.OrdinalIgnoreCase))
            {
                string key = AdtTexturePath.Key(texture);
                counts[key] = counts.TryGetValue(key, out var known) ? (known.Texture, known.Chunks + 1) : (texture, 1);
            }
        }

        return counts.Values
            .OrderByDescending(entry => entry.Chunks)
            .ThenBy(entry => entry.Texture, StringComparer.OrdinalIgnoreCase)
            .Select(entry => new AdtTextureUsageDescriptor(entry.Texture, entry.Chunks))
            .ToArray();
    }

    private AdtStrokeResultDescriptor Stroke(object? points, object? options, AdtTexturePaintOptions paint)
    {
        AdtTextureWorld world = RequireWorld();
        Brush brush = Factory.Brush.Clone();
        Apply(brush, paint, options);

        var target = new AdtTexturePaintTarget(() => world, paint);
        var stroke = new BrushStroke(brush, target, _context.EditSessions);
        if (!stroke.Begin())
        {
            throw new InvalidOperationException("A stroke needs a texture and a map with imported ADT data.");
        }

        foreach (Vector3 point in AdtScriptArguments.Points(points))
        {
            stroke.MoveTo(point);
        }

        bool changed = stroke.Finish(record: true);
        return new AdtStrokeResultDescriptor(
            changed,
            target.Unpaintable.Where(coord => world.TryChunk(coord, out _)).Select(coord =>
            {
                world.TryChunk(coord, out AdtChunkRef chunk);
                return new AdtChunkAddressDescriptor(chunk);
            }).ToArray(),
            target.Loading.Count);
    }

    private AdtTextureWorld RequireWorld()
    {
        AdtTextureWorld? world = WorldSource != null
            ? WorldSource()
            : AdtTextureWorld.FromEditor(_context, _context.Maps.CurrentMap, out _);
        return world ?? throw new InvalidOperationException("The current map has no imported ADT data.");
    }

    private static AdtChunkRef RequireChunk(AdtTextureWorld world, ChunkCoord coord) =>
        world.TryChunk(coord, out AdtChunkRef chunk) ? chunk : throw new InvalidOperationException("That chunk is not imported.");

    private static AdtChunkDescriptor Describe(AdtTextureWorld world, AdtChunkRef chunk, Vector3? at)
    {
        if (AdtChunkEdit.TryLoad(world, chunk.Coord, out AdtChunkEdit? edit) != AdtChunkLoadResult.Loaded)
        {
            throw new InvalidOperationException("That chunk cannot be read yet: its alpha data is not loaded.");
        }

        AdtChunkTextureSet set = edit!.Set;
        AdtChunkFacts facts = chunk.Tile.Chunks[(chunk.Col, chunk.Row)];
        int x = 0;
        int y = 0;
        if (at is { } point)
        {
            BrushGrid grid = world.GridOf(chunk.Coord);
            x = Math.Clamp((int)((point.X - grid.OriginX) / grid.TexelWidth), 0, AdtChunkTextureSet.Edge - 1);
            y = Math.Clamp((int)((point.Z - grid.OriginZ) / grid.TexelHeight), 0, AdtChunkTextureSet.Edge - 1);
        }

        var layers = new AdtLayerDescriptor[set.Count];
        for (int slot = 0; slot < set.Count; slot++)
        {
            int? materialId = facts.Slots.ElementAtOrDefault(slot)?.MaterialId;
            layers[slot] = new AdtLayerDescriptor(
                set.Textures[slot],
                slot,
                at != null ? set.WeightAt(slot, x, y) : set.Coverage(slot),
                set.Coverage(slot),
                materialId ?? -1,
                world.Materials.EffectIdOf(materialId) ?? 0);
        }

        AdtChunkOverride? overridden = world.OverrideAt(chunk);
        return new AdtChunkDescriptor(chunk, layers, overridden == null ? null : AdtTextureTool.OverrideText(overridden));
    }

    private static IEnumerable<ChunkCoord> AllChunks(AdtTextureWorld world) =>
        world.Tiles.SelectMany(AdtTextureEdit.ChunksOfTile);

    private static IReadOnlyList<ChunkCoord> ChunksOfTile(AdtTextureWorld world, object tile)
    {
        double[] indices = AdtScriptArguments.Numbers(tile);
        if (indices.Length != 2)
        {
            throw new ArgumentException("Tile must be [tileX, tileY].");
        }

        return world.Tiles
            .Where(candidate => candidate.FileX == (int)indices[0] && candidate.FileY == (int)indices[1])
            .SelectMany(AdtTextureEdit.ChunksOfTile)
            .ToList();
    }

    private static IEnumerable<ChunkCoord> ParseRegion(AdtTextureWorld world, object? value)
    {
        double minX, minZ, maxX, maxZ;
        if (ScriptJson.AsMap(value) is { } map)
        {
            (minX, minZ, maxX, maxZ) = (AdtScriptArguments.Number(map, "minX"), AdtScriptArguments.Number(map, "minZ"), AdtScriptArguments.Number(map, "maxX"), AdtScriptArguments.Number(map, "maxZ"));
        }
        else if (AdtScriptArguments.Numbers(value) is { Length: 4 } parts)
        {
            (minX, minZ, maxX, maxZ) = (parts[0], parts[1], parts[2], parts[3]);
        }
        else
        {
            throw new ArgumentException("A region must be { MinX, MinZ, MaxX, MaxZ } or [minX, minZ, maxX, maxZ].");
        }

        ChunkCoord min = world.Grid.CoordAt(new Vector3((float)Math.Min(minX, maxX), 0.0f, (float)Math.Min(minZ, maxZ)));
        ChunkCoord max = world.Grid.CoordAt(new Vector3((float)Math.Max(minX, maxX), 0.0f, (float)Math.Max(minZ, maxZ)));
        return AdtTextureEdit.Range(min, max).Where(coord => world.TryChunk(coord, out _));
    }

    private static void Apply(Brush brush, AdtTexturePaintOptions paint, object? options)
    {
        BrushScriptOptions.Apply(brush, options, (key, value) =>
        {
            switch (key.ToLowerInvariant())
            {
                case "texture":
                    paint.Texture = Convert.ToString(value, System.Globalization.CultureInfo.InvariantCulture) ?? "";
                    return true;
                case "targetweight":
                    paint.TargetWeight = Math.Clamp((float)BrushScriptOptions.ToDouble(value), 0.0f, 1.0f);
                    return true;
                case "cleanupthreshold":
                    paint.CleanupThreshold = (float)BrushScriptOptions.ToDouble(value);
                    return true;
                case "wholechunk":
                    paint.ReplaceWholeChunk = BrushScriptOptions.ToBool(value);
                    return true;
                case "points":
                case "chunks":
                case "tile":
                    return true;
                default:
                    return false;
            }
        });
    }
}
