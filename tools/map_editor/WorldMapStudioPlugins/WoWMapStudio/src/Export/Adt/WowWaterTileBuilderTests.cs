using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

public static class WowWaterTileBuilderTests
{
    private static readonly WowTileId Tile = new("TestMap", 32, 32);

    [EditorTest(Category = "WoW Water", Thread = TestThread.Background)]
    public static void Imported_water_rasterizes_back_to_an_equivalent_cell()
    {
        LandscapeGrid grid = Grid();
        var template = new ImportedChunkLiquid?[256];
        template[18] = new ImportedChunkLiquid
        {
            HasAttributes = false,
            Fishable = ulong.MaxValue,
            Fatigue = 0,
            Instances = [Instance(xOffset: 1, yOffset: 2, width: 3, height: 2, exists: [true, true, false, true, true, true])],
        };

        IReadOnlyList<WaterSurfacePatch> patches = WaterMapper.Map(template, Tile, grid)
            .Select(body => new WaterSurfacePatch(
                body.LiquidTypeId,
                LiquidBasicType.Water,
                body.Vertices,
                body.Faces.SelectMany(q => new[] { q.A, q.B, q.C, q.A, q.C, q.D }).ToList()))
            .ToList();

        var rebuilt = WowWaterTileBuilder.Build(patches, Tile, grid, Terrain(), new ImportedChunkLiquid?[256]);

        Assert.IsTrue(WowWaterTileBuilder.Equivalent(template[18], rebuilt[18]), "the rebuilt cell covers the same subchunks at the same heights");
        for (int i = 0; i < rebuilt.Count; i++)
        {
            Assert.IsTrue(i == 18 || rebuilt[i] == null, $"cell {i} stays dry");
        }
    }

    [EditorTest(Category = "WoW Water", Thread = TestThread.Background)]
    public static void An_equivalent_cell_keeps_the_templates_liquid()
    {
        LandscapeGrid grid = Grid();
        var template = new ImportedChunkLiquid?[256];
        template[18] = new ImportedChunkLiquid
        {
            HasAttributes = false,
            Fishable = ulong.MaxValue,
            Fatigue = 0,
            Instances = [Instance(xOffset: 0, yOffset: 0, width: 8, height: 8, exists: null)],
        };

        IReadOnlyList<WaterSurfacePatch> patches = WaterMapper.Map(template, Tile, grid)
            .Select(body => new WaterSurfacePatch(
                body.LiquidTypeId,
                LiquidBasicType.Water,
                body.Vertices,
                body.Faces.SelectMany(q => new[] { q.A, q.B, q.C, q.A, q.C, q.D }).ToList()))
            .ToList();

        var result = WowWaterTileBuilder.Build(patches, Tile, grid, Terrain(), template);

        Assert.IsTrue(ReferenceEquals(template[18], result[18]), "an unedited cell is not replaced, so its depth bytes survive");
    }

    [EditorTest(Category = "WoW Water", Thread = TestThread.Background)]
    public static void No_patches_clear_the_water()
    {
        var template = new ImportedChunkLiquid?[256];
        template[18] = new ImportedChunkLiquid
        {
            HasAttributes = false,
            Fishable = ulong.MaxValue,
            Fatigue = 0,
            Instances = [Instance(xOffset: 0, yOffset: 0, width: 2, height: 2, exists: null)],
        };

        var result = WowWaterTileBuilder.Build([], Tile, Grid(), Terrain(), template);

        Assert.IsTrue(result.All(cell => cell == null));
    }

    private static ImportedLiquidInstance Instance(int xOffset, int yOffset, int width, int height, bool[]? exists) => new()
    {
        LiquidType = 1,
        LiquidObjectOrFormat = 0,
        MinHeight = 5.0f,
        MaxHeight = 5.0f,
        XOffset = xOffset,
        YOffset = yOffset,
        Width = width,
        Height = height,
        Exists = exists == null ? null : Pack(exists),
        Heights = Enumerable.Repeat(5.0f, (width + 1) * (height + 1)).ToArray(),
        Depths = Enumerable.Repeat((byte)77, (width + 1) * (height + 1)).ToArray(),
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

    private static ImportedTile Terrain() => new()
    {
        TileId = new ImportedTileId(Tile.Map, Tile.FileX, Tile.FileY),
        Textures = [],
        HeightTextures = [],
        Doodads = [],
        Objects = [],
        Chunks = Enumerable.Range(0, 256).Select(i => new ImportedChunk
        {
            LocalCol = i % 16,
            LocalRow = i / 16,
            OuterHeights = new float[81],
            InnerHeights = new float[64],
            Layers = [],
            Holes = new bool[16],
            AreaId = 0,
            Flags = 0,
        }).ToList(),
    };

    private static LandscapeGrid Grid() => new(new LandscapeSettings
    {
        ChunkWorldSize = 33.33333f,
        OriginChunkX = 0,
        OriginChunkY = 0,
    });
}
