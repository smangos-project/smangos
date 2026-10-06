using System;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Turns authored water triangles into per-cell MH2O-shaped data, in editor axes (cell and subchunk
/// grids walk +X/+Z). Pure: plain arrays and delegates only. <see cref="WowWaterTileBuilder"/> turns the
/// result into on-disk orientation.
/// </summary>
public static class WowLiquidRasterizer
{
    public const float TileSize = 533.33333f;
    public const int CellsPerTile = 16;
    public const float CellSize = TileSize / CellsPerTile;
    public const int SubchunksPerCell = 8;
    public const float SubchunkSize = CellSize / SubchunksPerCell;

    /// <summary>Depth is normalized against this before quantizing to the on-disk byte. A documented
    /// approximation of the client's opacity formula: fog + depth tint, not byte-exact.</summary>
    public const float MaxDepthMeters = 20.0f;

    /// <summary>Builds every one of a tile's 256 cells (16x16, row-major <c>y * 16 + x</c>) from
    /// <paramref name="patches"/> and a terrain height sampler. <paramref name="terrainHeightAt"/> takes
    /// world (x, z) and returns terrain height — deliberately a plain delegate rather than any landscape-
    /// chunk-grid type, so this stays decoupled from where the caller's heights actually came from.</summary>
    public static RasterCell[] BuildTile(Vector3 tileOrigin, IReadOnlyList<WaterSurfacePatch> patches, Func<float, float, float> terrainHeightAt)
    {
        var cells = new RasterCell[CellsPerTile * CellsPerTile];
        for (int cellY = 0; cellY < CellsPerTile; cellY++)
        {
            for (int cellX = 0; cellX < CellsPerTile; cellX++)
            {
                Vector3 cellOrigin = tileOrigin + new Vector3(cellX * CellSize, 0.0f, cellY * CellSize);
                cells[(cellY * CellsPerTile) + cellX] = BuildCell(cellOrigin, patches, terrainHeightAt);
            }
        }

        return cells;
    }

    private static RasterCell BuildCell(Vector3 cellOrigin, IReadOnlyList<WaterSurfacePatch> patches, Func<float, float, float> terrainHeightAt)
    {
        var cell = new RasterCell();
        var cellBounds = new Rect2(cellOrigin.X, cellOrigin.Z, CellSize, CellSize);

        List<WaterSurfacePatch> relevant = patches.Where(patch => patch.BoundsXZ.Intersects(cellBounds)).ToList();
        if (relevant.Count == 0)
        {
            return cell;
        }

        // One MH2O layer per distinct liquid type present, not per authored entity — matching the
        // format's own model (ChunkWater::_layers stacks liquid *types*; painting more of the same type
        // extends one layer's mask, it does not add a second layer). Two overlapping entities of the
        // same liquid type merge into one instance; two different types produce two stacked instances.
        foreach (IGrouping<int, WaterSurfacePatch> group in relevant.GroupBy(patch => patch.LiquidTypeId).OrderBy(group => group.Key))
        {
            RasterLayer? instance = BuildLayer(cell, cellOrigin, group.Key, group.ToList(), terrainHeightAt);
            if (instance != null)
            {
                cell.Instances.Add(instance);
            }
        }

        return cell;
    }

    private static RasterLayer? BuildLayer(RasterCell cell, Vector3 cellOrigin, int liquidTypeId, List<WaterSurfacePatch> layerPatches, Func<float, float, float> terrainHeightAt)
    {
        var mask = new bool[SubchunksPerCell * SubchunksPerCell];
        bool any = false;

        // MH2O_Attributes are per subchunk and per cell (not per layer): fishable defaults all-set, so
        // a non-fishable body clears its subchunks' bits; fatigue defaults all-clear, so a fatigue body
        // sets them. HasAttributes flips only when a body actually departs from those defaults, matching
        // real tiles that omit the block entirely for ordinary water.
        for (int sz = 0; sz < SubchunksPerCell; sz++)
        {
            for (int sx = 0; sx < SubchunksPerCell; sx++)
            {
                float centerX = cellOrigin.X + ((sx + 0.5f) * SubchunkSize);
                float centerZ = cellOrigin.Z + ((sz + 0.5f) * SubchunkSize);
                if (FindCoveringPatch(layerPatches, centerX, centerZ) is not { } covering)
                {
                    continue;
                }

                mask[(sz * SubchunksPerCell) + sx] = true;
                any = true;

                ulong bit = 1UL << ((sz * SubchunksPerCell) + sx);
                if (!covering.Fishable)
                {
                    cell.Fishable &= ~bit;
                    cell.HasAttributes = true;
                }

                if (covering.Fatigue)
                {
                    cell.Fatigue |= bit;
                    cell.HasAttributes = true;
                }
            }
        }

        if (!any)
        {
            return null;
        }

        int minX = SubchunksPerCell;
        int minZ = SubchunksPerCell;
        int maxX = 0;
        int maxZ = 0;
        for (int sz = 0; sz < SubchunksPerCell; sz++)
        {
            for (int sx = 0; sx < SubchunksPerCell; sx++)
            {
                if (!mask[(sz * SubchunksPerCell) + sx])
                {
                    continue;
                }

                minX = Math.Min(minX, sx);
                minZ = Math.Min(minZ, sz);
                maxX = Math.Max(maxX, sx + 1);
                maxZ = Math.Max(maxZ, sz + 1);
            }
        }

        int width = maxX - minX;
        int height = maxZ - minZ;
        var existsBitmap = new bool[width * height];
        for (int z = 0; z < height; z++)
        {
            for (int x = 0; x < width; x++)
            {
                existsBitmap[(z * width) + x] = mask[((minZ + z) * SubchunksPerCell) + (minX + x)];
            }
        }

        int vertsX = width + 1;
        int vertsZ = height + 1;
        var heights = new float[vertsX * vertsZ];
        var depths = new byte[vertsX * vertsZ];
        float minHeight = float.PositiveInfinity;
        float maxHeight = float.NegativeInfinity;

        for (int vz = 0; vz < vertsZ; vz++)
        {
            for (int vx = 0; vx < vertsX; vx++)
            {
                float worldX = cellOrigin.X + ((minX + vx) * SubchunkSize);
                float worldZ = cellOrigin.Z + ((minZ + vz) * SubchunkSize);
                float sampledHeight = SampleHeight(layerPatches, worldX, worldZ);
                float terrainHeight = terrainHeightAt(worldX, worldZ);
                float depthMeters = Mathf.Max(0.0f, terrainHeight - sampledHeight);
                byte depthByte = (byte)Mathf.Clamp(Mathf.RoundToInt((depthMeters / MaxDepthMeters) * 255.0f), 0, 255);

                int index = (vz * vertsX) + vx;
                heights[index] = sampledHeight;
                depths[index] = depthByte;
                minHeight = Mathf.Min(minHeight, sampledHeight);
                maxHeight = Mathf.Max(maxHeight, sampledHeight);
            }
        }

        LiquidBasicType basicType = layerPatches[0].BasicType;
        WaterVertexFormat format = SelectVertexFormat(basicType, minHeight, maxHeight);

        return new RasterLayer
        {
            LiquidTypeId = liquidTypeId,
            VertexFormat = format,
            MinHeight = minHeight,
            MaxHeight = maxHeight,
            XOffset = minX,
            YOffset = minZ,
            Width = width,
            Height = height,
            ExistsBitmap = existsBitmap,
            Heightmap = format == WaterVertexFormat.DepthOnly ? null : heights,
            Depthmap = format == WaterVertexFormat.HeightUv ? null : depths,
        };
    }

    /// <summary>Switches on the liquid's basic type: magma/slime always animate via UV rather than
    /// depth; a perfectly flat ocean (min == max == 0) drops height entirely (the format's own "it's
    /// sea level" optimization); everything else (river/generic water, and a non-flat ocean) carries
    /// height + depth.</summary>
    private static WaterVertexFormat SelectVertexFormat(LiquidBasicType basicType, float minHeight, float maxHeight) =>
        basicType switch
        {
            LiquidBasicType.Magma or LiquidBasicType.Slime => WaterVertexFormat.HeightUv,
            LiquidBasicType.Ocean when minHeight == 0.0f && maxHeight == 0.0f => WaterVertexFormat.DepthOnly,
            _ => WaterVertexFormat.HeightDepth,
        };

    private static bool TryFindHeight(List<WaterSurfacePatch> patches, float x, float z, out float height)
    {
        var point = new Vector2(x, z);
        foreach (WaterSurfacePatch patch in patches)
        {
            if (!patch.BoundsXZ.HasPoint(point))
            {
                continue;
            }

            if (TryHeightInPatch(patch, x, z, out height))
            {
                return true;
            }
        }

        height = 0.0f;
        return false;
    }

    /// <summary>The layer patch whose surface actually covers world (x, z) — the one a subchunk centre
    /// falls inside, so its per-body attribute flags apply to that subchunk. Null when no patch covers
    /// the point.</summary>
    private static WaterSurfacePatch? FindCoveringPatch(List<WaterSurfacePatch> patches, float x, float z)
    {
        var point = new Vector2(x, z);
        foreach (WaterSurfacePatch patch in patches)
        {
            if (patch.BoundsXZ.HasPoint(point) && TryHeightInPatch(patch, x, z, out _))
            {
                return patch;
            }
        }

        return null;
    }

    private static bool TryHeightInPatch(WaterSurfacePatch patch, float x, float z, out float height)
    {
        IReadOnlyList<Vector3> vertices = patch.Vertices;
        IReadOnlyList<int> indices = patch.Indices;
        for (int i = 0; i + 2 < indices.Count; i += 3)
        {
            Vector3 a = vertices[indices[i]];
            Vector3 b = vertices[indices[i + 1]];
            Vector3 c = vertices[indices[i + 2]];
            if (TryBarycentricXZ(a, b, c, x, z, out float u, out float v, out float w))
            {
                height = (u * a.Y) + (v * b.Y) + (w * c.Y);
                return true;
            }
        }

        height = 0.0f;
        return false;
    }

    private static bool TryBarycentricXZ(Vector3 a, Vector3 b, Vector3 c, float px, float pz, out float u, out float v, out float w)
    {
        float denom = ((b.Z - c.Z) * (a.X - c.X)) + ((c.X - b.X) * (a.Z - c.Z));
        if (Mathf.Abs(denom) < 1e-8f)
        {
            u = v = w = 0.0f;
            return false;
        }

        u = (((b.Z - c.Z) * (px - c.X)) + ((c.X - b.X) * (pz - c.Z))) / denom;
        v = (((c.Z - a.Z) * (px - c.X)) + ((a.X - c.X) * (pz - c.Z))) / denom;
        w = 1.0f - u - v;

        const float epsilon = -1e-4f;
        return u >= epsilon && v >= epsilon && w >= epsilon;
    }

    /// <summary>Height at an arbitrary rect vertex, which — unlike a subchunk centre — is not
    /// guaranteed to land inside a covering triangle (the coverage mask samples subchunk centres, not
    /// every corner). Falls back to the nearest authored vertex among the layer's patches, a reasonable
    /// degrade right at a coverage boundary rather than a hard failure.</summary>
    private static float SampleHeight(List<WaterSurfacePatch> patches, float x, float z)
    {
        if (TryFindHeight(patches, x, z, out float height))
        {
            return height;
        }

        float best = float.PositiveInfinity;
        float bestHeight = 0.0f;
        foreach (WaterSurfacePatch patch in patches)
        {
            foreach (Vector3 vertex in patch.Vertices)
            {
                float dx = vertex.X - x;
                float dz = vertex.Z - z;
                float distance = (dx * dx) + (dz * dz);
                if (distance < best)
                {
                    best = distance;
                    bestHeight = vertex.Y;
                }
            }
        }

        return bestHeight;
    }
}
