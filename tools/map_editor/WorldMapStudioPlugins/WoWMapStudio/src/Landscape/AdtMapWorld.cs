using System;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>One imported chunk: the tile entity that owns it and its place in the tile.</summary>
public readonly record struct AdtChunkRef(
    ChunkCoord Coord,
    SceneEntity Entity,
    AdtTileComponent Tile,
    int Col,
    int Row);

/// <summary>
/// What the ADT editing tools share on one map: its grid, the loaded imported tiles, and the image
/// placements that carry the imported rasters. Chunk coordinates, tile slots and image pixels convert here,
/// so nothing else does that arithmetic.
/// </summary>
public sealed class AdtMapWorld
{
    private readonly Dictionary<(int FileX, int FileY), (SceneEntity Entity, AdtTileComponent Tile)> _tiles;
    private readonly Dictionary<string, ImageComponent> _placements = new(StringComparer.Ordinal);
    private readonly Action _changed;
    private readonly Func<ChunkCoord, LandscapeChunkOutput?>? _outputAt;

    public AdtMapWorld(
        LandscapeGrid grid,
        IEnumerable<(SceneEntity Entity, AdtTileComponent Tile)> tiles,
        IEnumerable<ImageComponent> placements,
        Action changed,
        Func<ChunkCoord, LandscapeChunkOutput?>? outputAt = null)
    {
        Grid = grid;
        _changed = changed;
        _outputAt = outputAt;
        _tiles = tiles.ToDictionary(pair => (pair.Tile.FileX, pair.Tile.FileY));
        foreach (ImageComponent placement in placements)
        {
            if (placement.Image != null)
            {
                _placements.TryAdd(placement.Channel, placement);
            }
        }
    }

    public LandscapeGrid Grid { get; }

    public int TileCount => _tiles.Count;

    /// <summary>The imported tiles loaded for this map.</summary>
    public IEnumerable<AdtTileComponent> Tiles => _tiles.Values.Select(pair => pair.Tile);

    /// <summary>Builds the world for the editor's current map. Null, with a reason, when the map has no
    /// landscape.</summary>
    public static AdtMapWorld? FromEditor(EditorContext editor, MapId map, out string? problem)
    {
        problem = null;
        if (editor.Landscape.Grid is not { } grid)
        {
            problem = "The landscape is not enabled.";
            return null;
        }

        var tiles = new List<(SceneEntity, AdtTileComponent)>();
        var placements = new List<ImageComponent>();
        foreach (SceneEntity entity in editor.Scene.Entities)
        {
            if (!entity.Map.Equals(map))
            {
                continue;
            }

            if (entity.Component<AdtTileComponent>() is { } tile)
            {
                tiles.Add((entity, tile));
            }
            else if (entity.Component<ImageComponent>() is { Image: not null } image)
            {
                placements.Add(image);
            }
        }

        return new AdtMapWorld(grid, tiles, placements, () => editor.Landscape.Rebuilder.NoticePaint(),
            coord => editor.Landscape.ChunkIndex.OutputAt(coord));
    }

    /// <summary>The placement of the imported raster that feeds <paramref name="channel"/>, or null when the
    /// map has none.</summary>
    public ImageComponent? PlacementFor(string channel) => _placements.GetValueOrDefault(channel);

    public bool TryChunk(ChunkCoord coord, out AdtChunkRef chunk)
    {
        (int fileX, int fileY, int col, int row) = WowTileAddressing.SlotOf(coord);
        if (_tiles.TryGetValue((fileX, fileY), out var tile) && tile.Tile.Chunks.ContainsKey((col, row)))
        {
            chunk = new AdtChunkRef(coord, tile.Entity, tile.Tile, col, row);
            return true;
        }

        chunk = default;
        return false;
    }

    public bool TryChunkAt(Vector3 world, out AdtChunkRef chunk) => TryChunk(Grid.CoordAt(world), out chunk);

    /// <summary>The top-left pixel of a chunk's block of <paramref name="edge"/> pixels in a placement's image.
    /// False when the block does not lie inside the image.</summary>
    public bool TryPixelBase(ImageComponent placement, ChunkCoord coord, int edge, out int pixelX, out int pixelY)
    {
        pixelX = pixelY = 0;
        if (placement.Image is not { } image)
        {
            return false;
        }

        Vector3 local = (placement.Owner?.Transform ?? Transform3D.Identity).AffineInverse() * Grid.OriginOf(coord);
        pixelX = Mathf.RoundToInt(((local.X / placement.WorldSizeX) + 0.5f) * image.Width);
        pixelY = Mathf.RoundToInt(((local.Z / placement.WorldSizeZ) + 0.5f) * image.Height);
        return pixelX >= 0 && pixelY >= 0 && pixelX + edge <= image.Width && pixelY + edge <= image.Height;
    }

    /// <summary>Whether a chunk's block of a placement's image can be edited: loaded, still loading, or not inside
    /// the image at all.</summary>
    public AdtBlockState BlockStateOf(ImageComponent placement, ChunkCoord coord, int edge)
    {
        if (!TryPixelBase(placement, coord, edge, out int x, out int y))
        {
            return AdtBlockState.Outside;
        }

        return placement.Image!.CanEditRegion(x, y, edge, edge) ? AdtBlockState.Ready : AdtBlockState.Loading;
    }

    /// <summary>Distance between neighbouring corners in world units: one cell.</summary>
    public float VertexStep => Grid.ChunkSize / AdtVertex.CellsPerChunk;

    /// <summary>The world position of a lattice vertex, at height zero.</summary>
    public Vector3 PositionOf(AdtVertex vertex)
    {
        (ChunkCoord chunk, int localX, int localZ) = vertex.Split();
        float half = VertexStep * 0.5f;
        return Grid.OriginOf(chunk) + new Vector3(localX * half, 0.0f, localZ * half);
    }

    /// <summary>The lattice vertex nearest a world position, corner or cell centre, whichever is closer.</summary>
    public AdtVertex NearestVertex(Vector3 world)
    {
        ChunkCoord chunk = Grid.CoordAt(world);
        Vector3 origin = Grid.OriginOf(chunk);
        float u = (world.X - origin.X) / VertexStep;
        float w = (world.Z - origin.Z) / VertexStep;

        int cornerX = Mathf.RoundToInt(u);
        int cornerZ = Mathf.RoundToInt(w);
        int cellX = Mathf.Clamp(Mathf.FloorToInt(u), 0, AdtVertex.CellsPerChunk - 1);
        int cellZ = Mathf.Clamp(Mathf.FloorToInt(w), 0, AdtVertex.CellsPerChunk - 1);

        float toCorner = ((u - cornerX) * (u - cornerX)) + ((w - cornerZ) * (w - cornerZ));
        float toCentre = ((u - (cellX + 0.5f)) * (u - (cellX + 0.5f))) + ((w - (cellZ + 0.5f)) * (w - (cellZ + 0.5f)));
        return toCentre < toCorner ? AdtVertex.Centre(chunk, cellX, cellZ) : AdtVertex.Corner(chunk, cornerX, cornerZ);
    }

    /// <summary>A chunk's 9x9 corners in world space, for <see cref="BrushRaster"/>: the grid is shifted half a cell
    /// so that each texel centre is a corner.</summary>
    public BrushGrid VertexGridOf(ChunkCoord coord)
    {
        Vector3 origin = Grid.OriginOf(coord);
        float step = VertexStep;
        int count = AdtVertex.CellsPerChunk + 1;
        return new BrushGrid(origin.X - (step * 0.5f), origin.Z - (step * 0.5f), step, step, count, count);
    }

    /// <summary>A chunk's 8x8 cell centres in world space, for <see cref="BrushRaster"/>: each texel is a cell, so
    /// its centre is the cell's centre vertex.</summary>
    public BrushGrid CentreGridOf(ChunkCoord coord)
    {
        Vector3 origin = Grid.OriginOf(coord);
        float step = VertexStep;
        int count = AdtVertex.CellsPerChunk;
        return new BrushGrid(origin.X, origin.Z, step, step, count, count);
    }

    /// <summary>Image pixels across one chunk of a placement's image, 0 when it has none.</summary>
    public int PixelsPerChunk(ImageComponent placement) =>
        placement.Image is { } image ? Mathf.RoundToInt(image.Width * Grid.ChunkSize / placement.WorldSizeX) : 0;

    /// <summary>Whether the landscape has built terrain for a chunk.</summary>
    public bool HasTerrain(ChunkCoord coord) => _outputAt?.Invoke(coord) != null;

    /// <summary>What the landscape built for a chunk, or null when nothing is built there yet.</summary>
    public LandscapeChunkOutput? OutputAt(ChunkCoord coord) => _outputAt?.Invoke(coord);

    /// <summary>Tells the landscape its content changed, so it rebuilds what was edited.</summary>
    public void NoticeChanged() => _changed();
}
