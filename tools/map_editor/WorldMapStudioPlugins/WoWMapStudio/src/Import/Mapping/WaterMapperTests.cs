using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

public static class WaterMapperTests
{
    private const float ChunkWorldSize = 33.33333f;
    private static readonly WowTileId Tile = new("TestMap", 32, 32);

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void One_rectangular_instance_becomes_one_body_with_a_quad_per_subchunk()
    {
        var cells = EmptyCells();
        cells[18] = Cell(Instance(width: 2, height: 2));

        IReadOnlyList<ImportedWaterBody> bodies = WaterMapper.Map(cells, Tile, Grid());

        Assert.AreEqual(1, bodies.Count);
        ImportedWaterBody body = bodies[0];
        Assert.AreEqual(1, body.LiquidTypeId);
        Assert.AreEqual(4, body.Faces.Count);
        Assert.AreEqual(9, body.Vertices.Count, "a 2x2 subchunk rect shares a 3x3 corner grid");
        Assert.IsTrue(body.Fishable, "fishable is the default when no attributes block was authored");
        Assert.IsFalse(body.Fatigue);
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void A_body_spanning_two_cells_merges_and_dedupes_the_shared_edge()
    {
        var cells = EmptyCells();
        // Cells 18 and 19 are adjacent along the localCol axis (world Z); both full 8x8.
        cells[18] = Cell(Instance(width: 8, height: 8));
        cells[19] = Cell(Instance(width: 8, height: 8));

        IReadOnlyList<ImportedWaterBody> bodies = WaterMapper.Map(cells, Tile, Grid());

        Assert.AreEqual(1, bodies.Count, "the two cells' subchunks are 4-connected across the boundary");
        Assert.AreEqual(128, bodies[0].Faces.Count);
        Assert.AreEqual(153, bodies[0].Vertices.Count, "9*9 + 8*9 — the shared row of 9 corners is not duplicated");
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Two_liquid_types_in_one_cell_produce_two_bodies()
    {
        var cells = EmptyCells();
        cells[18] = Cell(Instance(width: 2, height: 2, liquidTypeId: 1), Instance(width: 2, height: 2, liquidTypeId: 2));

        IReadOnlyList<ImportedWaterBody> bodies = WaterMapper.Map(cells, Tile, Grid());

        Assert.AreEqual(2, bodies.Count);
        Assert.IsTrue(bodies.Any(b => b.LiquidTypeId == 1));
        Assert.IsTrue(bodies.Any(b => b.LiquidTypeId == 2));
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void A_gap_in_the_mask_splits_a_body_into_components()
    {
        var cells = EmptyCells();
        cells[18] = Cell(Instance(width: 3, height: 1, exists: [true, false, true]));

        IReadOnlyList<ImportedWaterBody> bodies = WaterMapper.Map(cells, Tile, Grid());

        Assert.AreEqual(2, bodies.Count, "the middle subchunk is a gap, so the two ends are separate bodies");
        Assert.IsTrue(bodies.All(b => b.Faces.Count == 1));
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void A_huge_component_splits_into_capped_blocks()
    {
        // A 5x5 block of fully-wet MCNK cells = 40x40 = 1600 covered subchunks, one connected component,
        // over the per-body cap.
        var cells = EmptyCells();
        for (int lr = 0; lr < 5; lr++)
        {
            for (int lc = 0; lc < 5; lc++)
            {
                cells[(lr * 16) + lc] = Cell(Instance(width: 8, height: 8));
            }
        }

        IReadOnlyList<ImportedWaterBody> bodies = WaterMapper.Map(cells, Tile, Grid());

        Assert.Greater(bodies.Count, 1, "a 1600-subchunk component must be split");
        Assert.IsTrue(bodies.All(b => b.Faces.Count <= 1024), "no block exceeds the cap");
        Assert.AreEqual(1600, bodies.Sum(b => b.Faces.Count), "every covered subchunk still becomes exactly one quad");
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Mixed_fishable_within_a_body_is_logged_and_flattened()
    {
        var cells = EmptyCells();
        // subchunk bit 0 not fishable, the other three are; fatigue on bits 0,1,8,9 — every covered subchunk
        cells[18] = WithAttributes(Cell(Instance(width: 2, height: 2)), ulong.MaxValue & ~1UL, 0b11_0000_0011UL);

        var logs = new List<string>();
        IReadOnlyList<ImportedWaterBody> bodies = WaterMapper.Map(cells, Tile, Grid(), logs.Add);

        Assert.AreEqual(1, bodies.Count);
        Assert.IsFalse(bodies[0].Fishable, "a body with any non-fishable subchunk exports as not fishable");
        Assert.IsTrue(bodies[0].Fatigue, "every covered subchunk had fatigue set");
        Assert.IsTrue(logs.Any(line => line.Contains("fishable varies")), "the lossy flatten must be logged");
        Assert.IsFalse(logs.Any(line => line.Contains("fatigue varies")), "fatigue was uniform, nothing to log");
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void A_cells_subchunk_grid_takes_the_same_quarter_turn_as_its_terrain_grid()
    {
        var cells = EmptyCells();
        cells[18] = Cell(Instance(width: 1, height: 1));

        LandscapeGrid grid = Grid();
        IReadOnlyList<ImportedWaterBody> bodies = WaterMapper.Map(cells, Tile, grid);

        // On-disk subchunk (fast 0, slow 0) is the editor grid's (x 7, z 0) corner of the cell — the
        // turn WowChunkGridOrientation already applies to MCVT and MCAL.
        Vector3 origin = grid.OriginOf(WowTileAddressing.ChunkAt(Tile, localCol: 2, localRow: 1));
        float step = ChunkWorldSize / 8.0f;
        Assert.AreEqual(1, bodies[0].Faces.Count);
        Assert.AreApproximatelyEqual(origin.X + (7.0f * step), bodies[0].Vertices.Min(v => v.X), 0.01);
        Assert.AreApproximatelyEqual(origin.Z, bodies[0].Vertices.Min(v => v.Z), 0.01);
    }

    private static ImportedChunkLiquid?[] EmptyCells() => new ImportedChunkLiquid?[256];

    private static ImportedChunkLiquid Cell(params ImportedLiquidInstance[] instances) => new()
    {
        HasAttributes = false,
        Fishable = ulong.MaxValue,
        Fatigue = 0,
        Instances = instances,
    };

    private static ImportedChunkLiquid WithAttributes(ImportedChunkLiquid cell, ulong fishable, ulong fatigue) => new()
    {
        HasAttributes = true,
        Fishable = fishable,
        Fatigue = fatigue,
        Instances = cell.Instances,
    };

    private static ImportedLiquidInstance Instance(int width, int height, int liquidTypeId = 1, bool[]? exists = null) => new()
    {
        LiquidType = (ushort)liquidTypeId,
        LiquidObjectOrFormat = 0,
        MinHeight = 5.0f,
        MaxHeight = 5.0f,
        XOffset = 0,
        YOffset = 0,
        Width = width,
        Height = height,
        Exists = exists is null ? null : Pack(exists),
        Heights = Enumerable.Repeat(5.0f, (width + 1) * (height + 1)).ToArray(),
    };

    private static byte[] Pack(bool[] bits)
    {
        var bytes = new byte[(bits.Length + 7) / 8];
        for (int i = 0; i < bits.Length; i++)
        {
            if (bits[i])
            {
                bytes[i >> 3] |= (byte)(1 << (i & 7));
            }
        }

        return bytes;
    }

    private static LandscapeGrid Grid() => new(new LandscapeSettings
    {
        ChunkWorldSize = ChunkWorldSize,
        OriginChunkX = 0,
        OriginChunkY = 0,
    });
}
