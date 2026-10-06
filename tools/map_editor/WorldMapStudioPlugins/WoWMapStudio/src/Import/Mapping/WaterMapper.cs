using System;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>One quad face of a water body: four indices into <see cref="ImportedWaterBody.Vertices"/>,
/// one covered subchunk.</summary>
public readonly record struct WaterQuad(int A, int B, int C, int D);

/// <summary>
/// One editable body of water: a single liquid type, its lattice-aligned corner vertices in world
/// space, one quad per covered subchunk, and the two per-body attribute flags. Fed straight into a
/// <c>wow.water.surface</c> <see cref="ProceduralModel"/> on import.
/// </summary>
public readonly record struct ImportedWaterBody(
    int LiquidTypeId,
    IReadOnlyList<Vector3> Vertices,
    IReadOnlyList<WaterQuad> Faces,
    bool Fishable,
    bool Fatigue);

/// <summary>
/// Turns a tile's per-cell <see cref="ImportedChunkLiquid"/> into merged, editable
/// <see cref="ImportedWaterBody"/>s: per liquid type, the 4-connected components of covered subchunks
/// across cell boundaries become one body each. Pure — no Godot scene types, no catalog.
///
/// On-disk cell index resolves to a <see cref="ChunkCoord"/> through
/// <see cref="WowTileAddressing.ChunkAt"/>, the same path the other mappers use. Inside a cell the
/// 8x8 subchunk grid needs <see cref="WowChunkGridOrientation"/>'s quarter-turn as well, exactly like
/// MCVT/MCAL: MH2O's fast index runs East and its slow index South, the editor's run +X and +Z. The
/// subchunk lattice is <c>ChunkSize / 8</c> apart and aligned to world zero (<see cref="LandscapeGrid.OriginOf"/>),
/// which is exactly where <c>WaterSurfaceMeshFunction.SnapVertex</c> puts authored vertices, so an
/// imported body re-exports onto the same lattice it came from.
///
/// Bodies stay tile-local (not merged across tile boundaries), so re-import and partial export stay
/// tile-local too. Two lossy cases are logged, not silently flattened: a shared boundary corner whose
/// two cells disagree on height, and a body whose fishable/fatigue bits vary within it.
/// </summary>
public static class WaterMapper
{
    private const int SubchunksPerCell = WowLiquidLattice.SubchunksPerCell;

    /// <param name="cells">The tile's 256 cells in MCNK slot order (<c>localRow * 16 + localCol</c>);
    /// null for a dry cell.</param>
    public static IReadOnlyList<ImportedWaterBody> Map(
        IReadOnlyList<ImportedChunkLiquid?> cells,
        WowTileId tile,
        LandscapeGrid grid,
        Action<string>? log = null)
    {
        if (cells.Count != WowTileAddressing.ChunksPerTileEdge * WowTileAddressing.ChunksPerTileEdge)
        {
            throw new ArgumentException($"Expected {WowTileAddressing.ChunksPerTileEdge * WowTileAddressing.ChunksPerTileEdge} cells, got {cells.Count}.", nameof(cells));
        }

        float subStep = grid.ChunkSize / SubchunksPerCell;

        // Every covered subchunk on the tile, keyed by (liquid type, global lattice X, global lattice Z).
        var subchunks = new Dictionary<(int Type, int Gx, int Gz), Subchunk>();

        for (int i = 0; i < cells.Count; i++)
        {
            if (cells[i] is not { } cell)
            {
                continue;
            }

            int localRow = i / WowTileAddressing.ChunksPerTileEdge;
            int localCol = i % WowTileAddressing.ChunksPerTileEdge;
            Vector3 origin = grid.OriginOf(WowTileAddressing.ChunkAt(tile, localCol, localRow));
            int cellGx = (int)MathF.Round(origin.X / subStep);
            int cellGz = (int)MathF.Round(origin.Z / subStep);

            ulong fishableMask = cell.HasAttributes ? cell.Fishable : ulong.MaxValue;
            ulong fatigueMask = cell.HasAttributes ? cell.Fatigue : 0UL;

            foreach (ImportedLiquidInstance instance in cell.Instances)
            {
                int w = instance.Width;
                int h = instance.Height;
                int cornersX = w + 1;

                for (int sz = 0; sz < h; sz++)
                {
                    for (int sx = 0; sx < w; sx++)
                    {
                        if (instance.Exists is { } exists && (exists[((sz * w) + sx) >> 3] & (1 << (((sz * w) + sx) & 7))) == 0)
                        {
                            continue;
                        }

                        int diskX = instance.XOffset + sx;
                        int diskZ = instance.YOffset + sz;
                        (int cellSubX, int cellSubZ) = WowChunkGridOrientation.ToEditorGrid(diskX, diskZ, SubchunksPerCell);
                        int gx = cellGx + cellSubX;
                        int gz = cellGz + cellSubZ;
                        int bit = (diskZ * SubchunksPerCell) + diskX;

                        float HeightAt(int vx, int vz) =>
                            instance.Heights is { } map ? map[(vz * cornersX) + vx] : instance.MinHeight;

                        // The same quarter-turn on the corner grid (edge 9, not 8): editor corner
                        // (x, z) is on-disk corner (z, 8 - x), so the subchunk's four corners come off
                        // the height grid rotated rather than in place.
                        var subchunk = new Subchunk(
                            Corner(cellGx, cellGz, origin, subStep, cellSubX, cellSubZ, HeightAt(sx, sz + 1)),
                            Corner(cellGx, cellGz, origin, subStep, cellSubX + 1, cellSubZ, HeightAt(sx, sz)),
                            Corner(cellGx, cellGz, origin, subStep, cellSubX + 1, cellSubZ + 1, HeightAt(sx + 1, sz)),
                            Corner(cellGx, cellGz, origin, subStep, cellSubX, cellSubZ + 1, HeightAt(sx + 1, sz + 1)),
                            (fishableMask & (1UL << bit)) != 0,
                            (fatigueMask & (1UL << bit)) != 0);

                        subchunks[(instance.LiquidType, gx, gz)] = subchunk;
                    }
                }
            }
        }

        return BuildBodies(subchunks, log ?? (_ => { }));
    }

    private readonly record struct CornerRef(int Gx, int Gz, Vector3 World);

    private readonly record struct Subchunk(CornerRef C00, CornerRef C10, CornerRef C11, CornerRef C01, bool Fishable, bool Fatigue);

    private static CornerRef Corner(int cellGx, int cellGz, Vector3 origin, float subStep, int cellSubX, int cellSubZ, float height) =>
        new(cellGx + cellSubX, cellGz + cellSubZ,
            new Vector3(origin.X + (cellSubX * subStep), float.IsFinite(height) ? height : 0.0f, origin.Z + (cellSubZ * subStep)));

    /// <summary>A component bigger than this is split into <see cref="BlockSize"/>-aligned blocks: a
    /// whole-tile ocean is one connected component of 16384 subchunks, and one editable network that
    /// large is unusable to edit and quadratic to build. Small lakes stay a single body.</summary>
    private const int MaxSubchunksPerBody = 1024;

    private const int BlockSize = 32;

    private static IReadOnlyList<ImportedWaterBody> BuildBodies(
        Dictionary<(int Type, int Gx, int Gz), Subchunk> subchunks,
        Action<string> log)
    {
        var bodies = new List<ImportedWaterBody>();
        var visited = new HashSet<(int, int, int)>();

        foreach (var key in subchunks.Keys)
        {
            if (!visited.Add(key))
            {
                continue;
            }

            // 4-connected flood fill within one liquid type.
            var component = new List<(int Type, int Gx, int Gz)> { key };
            var queue = new Queue<(int Type, int Gx, int Gz)>();
            queue.Enqueue(key);
            while (queue.Count > 0)
            {
                (int type, int gx, int gz) = queue.Dequeue();
                foreach ((int nx, int nz) in new[] { (gx - 1, gz), (gx + 1, gz), (gx, gz - 1), (gx, gz + 1) })
                {
                    var neighbour = (type, nx, nz);
                    if (subchunks.ContainsKey(neighbour) && visited.Add(neighbour))
                    {
                        component.Add(neighbour);
                        queue.Enqueue(neighbour);
                    }
                }
            }

            if (component.Count <= MaxSubchunksPerBody)
            {
                bodies.Add(BuildBody(key.Type, component, subchunks, log));
                continue;
            }

            foreach (List<(int Type, int Gx, int Gz)> block in component
                .GroupBy(sub => (FloorDiv(sub.Gx, BlockSize), FloorDiv(sub.Gz, BlockSize)))
                .Select(g => g.ToList()))
            {
                bodies.Add(BuildBody(key.Type, block, subchunks, log));
            }
        }

        return bodies;
    }

    private static int FloorDiv(int a, int b) => (int)Math.Floor(a / (double)b);

    private static ImportedWaterBody BuildBody(
        int liquidTypeId,
        List<(int Type, int Gx, int Gz)> component,
        Dictionary<(int Type, int Gx, int Gz), Subchunk> subchunks,
        Action<string> log)
    {
        var vertices = new List<Vector3>();
        var vertexIndex = new Dictionary<(int Gx, int Gz), int>();
        var faces = new List<WaterQuad>();
        int fishableCount = 0;
        int fatigueCount = 0;
        int heightConflicts = 0;

        int Intern(CornerRef corner)
        {
            if (vertexIndex.TryGetValue((corner.Gx, corner.Gz), out int existing))
            {
                if (Mathf.Abs(vertices[existing].Y - corner.World.Y) > 1e-3f)
                {
                    heightConflicts++;
                }

                return existing;
            }

            int index = vertices.Count;
            vertices.Add(corner.World);
            vertexIndex[(corner.Gx, corner.Gz)] = index;
            return index;
        }

        foreach ((int Type, int Gx, int Gz) cell in component)
        {
            Subchunk s = subchunks[cell];
            faces.Add(new WaterQuad(Intern(s.C00), Intern(s.C10), Intern(s.C11), Intern(s.C01)));
            if (s.Fishable)
            {
                fishableCount++;
            }

            if (s.Fatigue)
            {
                fatigueCount++;
            }
        }

        bool fishable = fishableCount == component.Count;      // fishable only where every subchunk is
        bool fatigue = fatigueCount > 0;                       // fatigue where any subchunk is

        if (fishableCount != 0 && fishableCount != component.Count)
        {
            log($"Water body (liquid {liquidTypeId}): fishable varies within the body ({fishableCount}/{component.Count} subchunks); exporting as not fishable.");
        }

        if (fatigueCount != 0 && fatigueCount != component.Count)
        {
            log($"Water body (liquid {liquidTypeId}): fatigue varies within the body ({fatigueCount}/{component.Count} subchunks); exporting as fatigue.");
        }

        if (heightConflicts > 0)
        {
            log($"Water body (liquid {liquidTypeId}): {heightConflicts} shared corner(s) had different heights in adjacent cells; kept the first.");
        }

        return new ImportedWaterBody(liquidTypeId, vertices, faces, fishable, fatigue);
    }
}
