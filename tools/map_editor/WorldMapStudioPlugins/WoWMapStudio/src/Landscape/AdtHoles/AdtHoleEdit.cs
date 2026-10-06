using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Cuts and fills the 4x4 hole cells of imported chunks on the map-wide hole raster. Cells are addressed in editor
/// axes, the row along +Z the slow index, the same order <see cref="HoleMapper.HoleBlock"/> stores them in. Each call
/// is one undo step.
/// </summary>
public sealed class AdtHoleEdit
{
    private const int Edge = HoleMapper.HoleEdge;

    private readonly AdtMapWorld _world;
    private readonly EditSessionManager _sessions;

    public AdtHoleEdit(AdtMapWorld world, EditSessionManager sessions)
    {
        _world = world;
        _sessions = sessions;
    }

    public static bool HasData(AdtMapWorld world) =>
        world.PlacementFor(AdtImportCatalog.HoleChannelName) is { Image: { Stride: 1 } } placement &&
        world.PixelsPerChunk(placement) == Edge;

    /// <summary>A chunk's 16 cells, true where cut. Null when the chunk is not imported or not loaded yet.</summary>
    public bool[]? Read(ChunkCoord coord)
    {
        if (!_world.TryChunk(coord, out _) || Placement() is not { } placement ||
            _world.BlockStateOf(placement, coord, Edge) != AdtBlockState.Ready)
        {
            return null;
        }

        _world.TryPixelBase(placement, coord, Edge, out int x, out int y);
        return placement.Image!.ReadPixels(x, y, Edge, Edge).Select(value => value >= 128).ToArray();
    }

    /// <summary>The cell of a chunk under a world point: its index from 0 to 15.</summary>
    public int CellAt(ChunkCoord coord, Vector3 world)
    {
        Vector3 origin = _world.Grid.OriginOf(coord);
        float size = _world.Grid.ChunkSize / Edge;
        int x = Mathf.Clamp(Mathf.FloorToInt((world.X - origin.X) / size), 0, Edge - 1);
        int z = Mathf.Clamp(Mathf.FloorToInt((world.Z - origin.Z) / size), 0, Edge - 1);
        return (z * Edge) + x;
    }

    /// <summary>Cuts or fills <paramref name="cells"/> (all 16 when null) of each chunk. Returns how many chunks
    /// changed.</summary>
    public int Set(IEnumerable<ChunkCoord> chunks, bool cut, IReadOnlyCollection<int>? cells = null) =>
        Apply(chunks.Distinct().ToDictionary(coord => coord, _ => cells), cut, cut ? "Cut holes" : "Fill holes");

    /// <summary>Cuts or fills the cell under each world point.</summary>
    public int SetAt(IEnumerable<Vector3> points, bool cut)
    {
        var byChunk = new Dictionary<ChunkCoord, HashSet<int>>();
        foreach (Vector3 point in points)
        {
            ChunkCoord coord = _world.Grid.CoordAt(point);
            if (!byChunk.TryGetValue(coord, out HashSet<int>? set))
            {
                byChunk[coord] = set = [];
            }

            set.Add(CellAt(coord, point));
        }

        return Apply(byChunk.ToDictionary(pair => pair.Key, pair => (IReadOnlyCollection<int>?)pair.Value), cut, cut ? "Cut holes" : "Fill holes");
    }

    private ImageComponent? Placement() => HasData(_world) ? _world.PlacementFor(AdtImportCatalog.HoleChannelName) : null;

    private int Apply(Dictionary<ChunkCoord, IReadOnlyCollection<int>?> requested, bool cut, string description)
    {
        if (Placement() is not { } placement)
        {
            return 0;
        }

        PaintImage image = placement.Image!;
        image.BeginStroke();
        int changed = 0;
        try
        {
            foreach ((ChunkCoord coord, IReadOnlyCollection<int>? cells) in requested)
            {
                if (!_world.TryChunk(coord, out _) ||
                    _world.BlockStateOf(placement, coord, Edge) != AdtBlockState.Ready)
                {
                    continue;
                }

                _world.TryPixelBase(placement, coord, Edge, out int x, out int y);
                byte[] block = image.ReadPixels(x, y, Edge, Edge);
                byte value = cut ? (byte)255 : (byte)0;
                bool any = false;
                foreach (int cell in cells ?? Enumerable.Range(0, Edge * Edge).ToArray())
                {
                    if (cell >= 0 && cell < block.Length && block[cell] != value)
                    {
                        block[cell] = value;
                        any = true;
                    }
                }

                if (any)
                {
                    image.WritePixels(x, y, Edge, Edge, block);
                    changed++;
                }
            }
        }
        catch
        {
            image.EndStroke();
            throw;
        }

        if (AdtEditCommit.Commit(image, placement, [], description) is { } command)
        {
            _sessions.Record(command);
            _world.NoticeChanged();
        }

        return changed;
    }
}
