using System;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Rasterizes one tile's authored water into the 256 per-chunk <see cref="ImportedChunkLiquid"/>s the
/// ADT writer applies, in MCNK slot order (<c>localRow * 16 + localCol</c>) and on-disk subchunk
/// orientation. A chunk whose result is equivalent to the template's keeps the template's liquid
/// (depth bytes, UVs and unauthored detail included), so unedited water is never rewritten.
/// </summary>
public static class WowWaterTileBuilder
{
    private const int Edge = WowTileAddressing.ChunksPerTileEdge;
    private const float HeightTolerance = 0.01f;

    public static IReadOnlyList<ImportedChunkLiquid?> Build(
        IReadOnlyList<WaterSurfacePatch> patches,
        WowTileId tile,
        LandscapeGrid grid,
        ImportedTile terrain,
        IReadOnlyList<ImportedChunkLiquid?> template)
    {
        RasterCell[] cells = Rasterize(patches, tile, grid, terrain);
        var result = new ImportedChunkLiquid?[cells.Length];
        for (int i = 0; i < cells.Length; i++)
        {
            ImportedChunkLiquid? built = Convert(cells[i]);
            result[i] = Equivalent(template[i], built) ? template[i] : built;
        }

        return result;
    }

    internal static RasterCell[] Rasterize(IReadOnlyList<WaterSurfacePatch> patches, WowTileId tile, LandscapeGrid grid, ImportedTile terrain)
    {
        if (patches.Count == 0)
        {
            return EmptyCells(Edge * Edge);
        }

        Vector3 minCorner = grid.OriginOf(WowTileAddressing.ChunkAt(tile, localCol: 0, localRow: Edge - 1));
        float[][] heights = EditorHeights(terrain);
        float HeightAt(float x, float z) => SampleHeight(heights, minCorner, grid.ChunkSize, x, z);
        return RotateCellGrids(RemapToMcnkOrder(WowLiquidRasterizer.BuildTile(minCorner, patches, HeightAt)));
    }

    /// <summary>Each chunk's 9x9 outer heights in editor axes (<c>z * 9 + x</c>), indexed by
    /// <c>editorX * 16 + editorZ</c> where editorX = 15 - localRow and editorZ = localCol.</summary>
    private static float[][] EditorHeights(ImportedTile terrain)
    {
        const int outer = ImportedChunk.OuterEdge;
        var result = new float[Edge * Edge][];
        foreach (ImportedChunk chunk in terrain.Chunks)
        {
            var grid = new float[outer * outer];
            for (int slow = 0; slow < outer; slow++)
            {
                for (int fast = 0; fast < outer; fast++)
                {
                    (int x, int z) = WowChunkGridOrientation.ToEditorGrid(fast, slow, outer);
                    grid[(z * outer) + x] = chunk.OuterHeights[(slow * outer) + fast];
                }
            }

            result[((Edge - 1 - chunk.LocalRow) * Edge) + chunk.LocalCol] = grid;
        }

        return result;
    }

    private static float SampleHeight(float[][] heights, Vector3 minCorner, float chunkSize, float x, float z)
    {
        const int outer = ImportedChunk.OuterEdge;
        int cx = Mathf.Clamp((int)Mathf.Floor((x - minCorner.X) / chunkSize), 0, Edge - 1);
        int cz = Mathf.Clamp((int)Mathf.Floor((z - minCorner.Z) / chunkSize), 0, Edge - 1);
        float fx = Mathf.Clamp((x - minCorner.X) / chunkSize - cx, 0.0f, 1.0f) * (outer - 1);
        float fz = Mathf.Clamp((z - minCorner.Z) / chunkSize - cz, 0.0f, 1.0f) * (outer - 1);
        int x0 = Mathf.Clamp((int)fx, 0, outer - 2);
        int z0 = Mathf.Clamp((int)fz, 0, outer - 2);
        float tx = fx - x0;
        float tz = fz - z0;
        float[] h = heights[(cx * Edge) + cz];
        float h00 = h[(z0 * outer) + x0];
        float h10 = h[(z0 * outer) + x0 + 1];
        float h01 = h[((z0 + 1) * outer) + x0];
        float h11 = h[((z0 + 1) * outer) + x0 + 1];
        return Mathf.Lerp(Mathf.Lerp(h00, h10, tx), Mathf.Lerp(h01, h11, tx), tz);
    }

    /// <summary>
    /// <see cref="WowLiquidRasterizer.BuildTile"/> indexes its output <c>cellY*16+cellX</c>, walking
    /// positive world X as <c>cellX</c> and positive world Z as <c>cellY</c> from <c>tileOrigin</c> —
    /// so calling it with <c>tileOrigin</c> at the tile's minimum-X/minimum-Z corner (as
    /// <see cref="Build"/> does) makes <c>cellX = ChunksPerTileEdge-1-localRow</c> (world X
    /// <em>decreases</em> as <c>localRow</c> increases — see <see cref="WowTileAddressing"/>'s ground
    /// truth) and <c>cellY = localCol</c> (world Z increases with <c>localCol</c> directly).
    /// </summary>
    internal static RasterCell[] RemapToMcnkOrder(RasterCell[] rasterized)
    {
        const int edge = WowTileAddressing.ChunksPerTileEdge;
        var remapped = new RasterCell[edge * edge];
        for (int localRow = 0; localRow < edge; localRow++)
        {
            int cellX = (edge - 1) - localRow;
            for (int localCol = 0; localCol < edge; localCol++)
            {
                int cellY = localCol;
                remapped[(localRow * edge) + localCol] = rasterized[(cellY * edge) + cellX];
            }
        }

        return remapped;
    }

    /// <summary>
    /// Turns each cell's own 8x8 subchunk grid from editor axes into MCNK axes — the within-cell half
    /// of the same quarter-turn <see cref="WowChunkGridOrientation"/> applies to MCVT and MCAL, which
    /// <see cref="RemapToMcnkOrder"/> only does at the cell-slot level.
    /// <see cref="WowLiquidRasterizer"/> walks its grids in world +X/+Z, so without this a cell's water
    /// lands in the right cell but a quarter-turn off inside it — invisible on a fully covered cell,
    /// wrong on every partially covered one, which is every waterbody edge.
    /// </summary>
    internal static RasterCell[] RotateCellGrids(RasterCell[] cells)
    {
        foreach (RasterCell cell in cells)
        {
            (cell.Fishable, cell.Fatigue) = (RotateBits(cell.Fishable), RotateBits(cell.Fatigue));
            for (int i = 0; i < cell.Instances.Count; i++)
            {
                cell.Instances[i] = RotateInstance(cell.Instances[i]);
            }
        }

        return cells;
    }

    /// <summary>An <c>MH2O_Attributes</c> word, one bit per subchunk at <c>slow * 8 + fast</c>.</summary>
    private static ulong RotateBits(ulong editorBits)
    {
        const int edge = WowLiquidRasterizer.SubchunksPerCell;
        ulong wow = 0;
        for (int slow = 0; slow < edge; slow++)
        {
            for (int fast = 0; fast < edge; fast++)
            {
                (int x, int z) = WowChunkGridOrientation.ToEditorGrid(fast, slow, edge);
                if ((editorBits & (1UL << ((z * edge) + x))) != 0)
                {
                    wow |= 1UL << ((slow * edge) + fast);
                }
            }
        }

        return wow;
    }

    private static RasterLayer RotateInstance(RasterLayer editor)
    {
        const int edge = WowLiquidRasterizer.SubchunksPerCell;

        // The turn maps editor (x, z) to on-disk (fast z, slow edge-1-x), so the rect's axes swap and
        // its X extent becomes the on-disk Y extent measured from the far edge.
        int xOffset = editor.YOffset;
        int yOffset = edge - editor.XOffset - editor.Width;
        int width = editor.Height;
        int height = editor.Width;

        bool[]? exists = null;
        if (editor.ExistsBitmap is { } editorExists)
        {
            exists = new bool[width * height];
            for (int slow = 0; slow < height; slow++)
            {
                for (int fast = 0; fast < width; fast++)
                {
                    (int x, int z) = WowChunkGridOrientation.ToEditorGrid(xOffset + fast, yOffset + slow, edge);
                    exists[(slow * width) + fast] = editorExists[((z - editor.YOffset) * editor.Width) + (x - editor.XOffset)];
                }
            }
        }

        // Corner grids are one entry wider in each axis, so they take the same turn at edge + 1.
        T[]? RotateCorners<T>(T[]? editorGrid)
        {
            if (editorGrid is null)
            {
                return null;
            }

            var rotated = new T[(width + 1) * (height + 1)];
            for (int slow = 0; slow <= height; slow++)
            {
                for (int fast = 0; fast <= width; fast++)
                {
                    (int x, int z) = WowChunkGridOrientation.ToEditorGrid(xOffset + fast, yOffset + slow, edge + 1);
                    rotated[(slow * (width + 1)) + fast] = editorGrid[((z - editor.YOffset) * (editor.Width + 1)) + (x - editor.XOffset)];
                }
            }

            return rotated;
        }

        return new RasterLayer
        {
            LiquidTypeId = editor.LiquidTypeId,
            VertexFormat = editor.VertexFormat,
            MinHeight = editor.MinHeight,
            MaxHeight = editor.MaxHeight,
            XOffset = xOffset,
            YOffset = yOffset,
            Width = width,
            Height = height,
            ExistsBitmap = exists,
            Heightmap = RotateCorners(editor.Heightmap),
            Depthmap = RotateCorners(editor.Depthmap),
        };
    }

    private static RasterCell[] EmptyCells(int count)
    {
        var cells = new RasterCell[count];
        for (int i = 0; i < count; i++)
        {
            cells[i] = new RasterCell();
        }

        return cells;
    }

    private static ImportedChunkLiquid? Convert(RasterCell cell)
    {
        if (cell.Instances.Count == 0)
        {
            return null;
        }

        var instances = new List<ImportedLiquidInstance>(cell.Instances.Count);
        foreach (RasterLayer layer in cell.Instances)
        {
            byte[]? exists = null;
            if (layer.ExistsBitmap is { } bitmap && bitmap.Any(bit => !bit))
            {
                exists = new byte[(bitmap.Length + 7) / 8];
                for (int i = 0; i < bitmap.Length; i++)
                {
                    if (bitmap[i])
                    {
                        exists[i >> 3] |= (byte)(1 << (i & 7));
                    }
                }
            }

            instances.Add(new ImportedLiquidInstance
            {
                LiquidType = (ushort)layer.LiquidTypeId,
                LiquidObjectOrFormat = (ushort)layer.VertexFormat,
                MinHeight = layer.MinHeight,
                MaxHeight = layer.MaxHeight,
                XOffset = layer.XOffset,
                YOffset = layer.YOffset,
                Width = layer.Width,
                Height = layer.Height,
                Exists = exists,
                Heights = layer.Heightmap,
                Depths = layer.Depthmap,
            });
        }

        return new ImportedChunkLiquid { HasAttributes = cell.HasAttributes, Fishable = cell.Fishable, Fatigue = cell.Fatigue, Instances = instances };
    }

    /// <summary>Whether two chunks' liquid covers the same subchunks with the same liquid types, corner
    /// heights (within tolerance) and fishable/fatigue flags. Depth bytes and layer packing are ignored.</summary>
    internal static bool Equivalent(ImportedChunkLiquid? a, ImportedChunkLiquid? b)
    {
        Dictionary<(int Type, int X, int Z), float[]> left = Coverage(a);
        Dictionary<(int Type, int X, int Z), float[]> right = Coverage(b);
        if (left.Count != right.Count)
        {
            return false;
        }

        foreach (((int Type, int X, int Z) key, float[] corners) in left)
        {
            if (!right.TryGetValue(key, out float[]? other) || !corners.Zip(other).All(p => MathF.Abs(p.First - p.Second) <= HeightTolerance))
            {
                return false;
            }

            ulong bit = 1UL << ((key.Z * WowLiquidRasterizer.SubchunksPerCell) + key.X);
            if (Flag(a!, bit, fishable: true) != Flag(b!, bit, fishable: true) || Flag(a!, bit, fishable: false) != Flag(b!, bit, fishable: false))
            {
                return false;
            }
        }

        return true;
    }

    private static bool Flag(ImportedChunkLiquid cell, ulong bit, bool fishable)
    {
        ulong mask = fishable
            ? (cell.HasAttributes ? cell.Fishable : ulong.MaxValue)
            : (cell.HasAttributes ? cell.Fatigue : 0UL);
        return (mask & bit) != 0;
    }

    /// <summary>Corner heights (00, 10, 11, 01 of the on-disk subchunk) of every covered subchunk, per liquid type.</summary>
    private static Dictionary<(int Type, int X, int Z), float[]> Coverage(ImportedChunkLiquid? cell)
    {
        var result = new Dictionary<(int, int, int), float[]>();
        if (cell == null)
        {
            return result;
        }

        foreach (ImportedLiquidInstance instance in cell.Instances)
        {
            int corners = instance.Width + 1;
            float At(int x, int z) => instance.Heights is { } map ? map[(z * corners) + x] : instance.MinHeight;
            for (int z = 0; z < instance.Height; z++)
            {
                for (int x = 0; x < instance.Width; x++)
                {
                    int index = (z * instance.Width) + x;
                    if (instance.Exists is { } exists && (exists[index >> 3] & (1 << (index & 7))) == 0)
                    {
                        continue;
                    }

                    result[(instance.LiquidType, instance.XOffset + x, instance.YOffset + z)] = [At(x, z), At(x + 1, z), At(x + 1, z + 1), At(x, z + 1)];
                }
            }
        }

        return result;
    }
}
