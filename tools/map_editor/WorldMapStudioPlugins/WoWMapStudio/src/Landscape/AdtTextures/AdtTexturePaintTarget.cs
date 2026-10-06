using System;
using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>What a stroke does to the chunks under it.</summary>
public enum AdtTextureStrokeKind
{
    /// <summary>Paints the texture in, adding it to chunks that lack it.</summary>
    Paint,

    /// <summary>Swaps <see cref="AdtTexturePaintOptions.ReplaceFrom"/> for the texture.</summary>
    Replace,
}

/// <summary>What the texture paint tool paints with, on top of the shared <see cref="Brush"/>.</summary>
public sealed class AdtTexturePaintOptions
{
    /// <summary>The texture being painted, or swapped in.</summary>
    public string Texture { get; set; } = "";

    public AdtTextureStrokeKind Kind { get; set; }

    /// <summary>The texture a replace stroke swaps out.</summary>
    public string ReplaceFrom { get; set; } = "";

    /// <summary>Whether a replace stroke swaps the texture in every chunk it touches, not just under the brush.</summary>
    public bool ReplaceWholeChunk { get; set; }

    /// <summary>The weight the texture is painted towards, 0 to 1.</summary>
    public float TargetWeight { get; set; } = 1.0f;

    /// <summary>A layer whose strongest texel falls below this at the end of a stroke is dropped.</summary>
    public float CleanupThreshold { get; set; } = AdtChunkTextureSet.VisibleThreshold;

    public AdtTexturePaintOptions Clone() => new()
    {
        Texture = Texture,
        Kind = Kind,
        ReplaceFrom = ReplaceFrom,
        ReplaceWholeChunk = ReplaceWholeChunk,
        TargetWeight = TargetWeight,
        CleanupThreshold = CleanupThreshold,
    };
}

/// <summary>
/// An <see cref="IStrokeTarget"/> over the imported chunks of a map. A dab finds the chunks under it, loads
/// each chunk's texture stack once per stroke, paints the chosen texture into it (adding it to the chunk when
/// it is not there), and writes the result back straight away so the viewport stays live. One stroke is one
/// undo step: the alpha image's chunk edits plus the tiles' slot changes.
/// </summary>
public sealed class AdtTexturePaintTarget : IStrokeTarget
{
    private const double WorldRefreshSeconds = 0.5;

    private readonly Func<AdtTextureWorld?> _worldFactory;
    private readonly AdtTexturePaintOptions _options;
    private readonly HashSet<ChunkCoord> _unpaintable = [];
    private readonly HashSet<ChunkCoord> _loading = [];
    private readonly HashSet<ChunkCoord> _skipped = [];

    private AdtTextureWorld? _world;
    private double _worldTime = double.NegativeInfinity;
    private Dictionary<ChunkCoord, AdtChunkEdit>? _edits;

    public AdtTexturePaintTarget(Func<AdtTextureWorld?> worldFactory, AdtTexturePaintOptions options)
    {
        _worldFactory = worldFactory;
        _options = options;
    }

    /// <summary>Chunks the last stroke could not put the texture into: full, with nothing unused to free.</summary>
    public IReadOnlyCollection<ChunkCoord> Unpaintable => _unpaintable;

    /// <summary>Chunks the last stroke skipped because their alpha data is not loaded yet.</summary>
    public IReadOnlyCollection<ChunkCoord> Loading => _loading;

    public AdtTextureWorld? World => _world;

    public bool Covers(Vector3 world)
    {
        if (_edits == null && Time.GetTicksMsec() / 1000.0 - _worldTime >= WorldRefreshSeconds)
        {
            _world = _worldFactory();
            _worldTime = Time.GetTicksMsec() / 1000.0;
        }

        return _world != null && _world.TryChunkAt(world, out _);
    }

    public bool Begin()
    {
        _world = _worldFactory();
        _worldTime = Time.GetTicksMsec() / 1000.0;
        if (_world == null || _options.Texture.Length == 0)
        {
            return false;
        }

        _unpaintable.Clear();
        _loading.Clear();
        _skipped.Clear();
        _world.Materials.EnsureTexture(_options.Texture);
        _edits = [];
        _world.Image.BeginStroke();
        return true;
    }

    public bool Dab(in BrushDab dab)
    {
        if (_world is not { } world || _edits == null)
        {
            return false;
        }

        ChunkCoord min = world.Grid.CoordAt(new Vector3(dab.Center.X - dab.Radius, 0.0f, dab.Center.Z - dab.Radius));
        ChunkCoord max = world.Grid.CoordAt(new Vector3(dab.Center.X + dab.Radius, 0.0f, dab.Center.Z + dab.Radius));

        bool changed = false;
        for (int x = min.X; x <= max.X; x++)
        {
            for (int y = min.Y; y <= max.Y; y++)
            {
                var coord = new ChunkCoord(x, y);
                BrushGrid grid = world.GridOf(coord);
                if (BrushRaster.Overlaps(grid, dab) && Edit(world, coord) is { } edit)
                {
                    changed |= DabChunk(edit, grid, dab);
                }
            }
        }

        if (changed)
        {
            world.NoticeChanged();
        }

        return changed;
    }

    public IEditCommand? Finish()
    {
        if (_world is not { } world || _edits == null)
        {
            return null;
        }

        Dictionary<ChunkCoord, AdtChunkEdit> edits = _edits;
        _edits = null;

        foreach (AdtChunkEdit edit in edits.Values)
        {
            if (edit.Set.EraseUnused(_options.CleanupThreshold))
            {
                edit.Store();
            }
        }

        return AdtChunkEdit.Commit(world, edits.Values, _options.Kind == AdtTextureStrokeKind.Replace ? "Replace texture" : "Paint texture");
    }

    public void Cancel()
    {
        _edits = null;
        _world?.Image.EndStroke();
    }

    private bool DabChunk(AdtChunkEdit edit, in BrushGrid grid, in BrushDab dab) =>
        _options.Kind == AdtTextureStrokeKind.Replace ? ReplaceInChunk(edit, grid, dab) : PaintChunk(edit, grid, dab);

    private bool ReplaceInChunk(AdtChunkEdit edit, in BrushGrid grid, in BrushDab dab)
    {
        string from = _options.ReplaceFrom;
        if (from.Length == 0 || AdtTexturePath.Same(from, _options.Texture) || edit.Set.IndexOf(from) < 0)
        {
            return false;
        }

        bool replaced;
        if (_options.ReplaceWholeChunk)
        {
            replaced = edit.Set.Replace(from, _options.Texture);
        }
        else
        {
            var mask = new bool[AdtChunkTextureSet.TexelCount];
            BrushRaster.Walk(grid, dab, (x, y, _) => mask[(y * AdtChunkTextureSet.Edge) + x] = true);
            replaced = edit.Set.Replace(from, _options.Texture, mask);
            if (!replaced && !edit.Set.CanAdd(_options.Texture))
            {
                _unpaintable.Add(edit.Chunk.Coord);
            }
        }

        if (replaced)
        {
            edit.Store();
        }

        return replaced;
    }

    private bool PaintChunk(AdtChunkEdit edit, in BrushGrid grid, in BrushDab dab)
    {
        int slot = edit.Set.GetOrAdd(_options.Texture, dab.Invert ? 0.0f : 1.0f);
        if (slot < 0)
        {
            if (!dab.Invert)
            {
                _unpaintable.Add(edit.Chunk.Coord);
            }

            return false;
        }

        _unpaintable.Remove(edit.Chunk.Coord);

        // A texture just added to a chunk changed its slot list even if no texel has moved yet.
        if (!edit.Set.Paint(slot, grid, dab, _options.TargetWeight) && !edit.Set.SlotsChanged)
        {
            return false;
        }

        edit.Store();
        return true;
    }

    // Loads a chunk's stack the first time a stroke reaches it; a chunk that cannot be edited is
    // remembered so later dabs skip it without looking again.
    private AdtChunkEdit? Edit(AdtTextureWorld world, ChunkCoord coord)
    {
        if (_edits!.TryGetValue(coord, out AdtChunkEdit? existing))
        {
            return existing;
        }

        if (_skipped.Contains(coord))
        {
            return null;
        }

        AdtChunkLoadResult result = AdtChunkEdit.TryLoad(world, coord, out AdtChunkEdit? edit);
        if (edit == null)
        {
            _skipped.Add(coord);
            if (result == AdtChunkLoadResult.Loading)
            {
                _loading.Add(coord);
            }

            return null;
        }

        _edits[coord] = edit;
        return edit;
    }
}
