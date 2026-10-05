using System;
using System.Buffers.Binary;
using System.IO;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Golden tests over real extractor outputs in <c>Fixtures/</c> (cut from <c>.server/data/maps</c>), with
/// expected values decoded independently of this reader. When the extractor bumps the <c>.map</c>
/// version these fail on the version check, which is the signal to re-extract the fixtures and update
/// the reader.
/// </summary>
public static class WowMapTileTests
{
    private const double Tolerance = 0.01;

    /// <summary>Int16 heights, per-chunk areas, holes, per-cell liquid. Map 0, ADT (32, 48): Northshire.</summary>
    private const string NorthshireTile = "0004832.map";

    /// <summary>Int8 heights, one global area, tile-wide ocean with no per-chunk type or level grids. Map 0, ADT (35, 20).</summary>
    private const string Int8Tile = "0002035.map";

    /// <summary>Int8 heights with a per-chunk area map. Map 0, ADT (36, 20).</summary>
    private const string Int8AreaTile = "0002036.map";

    /// <summary>No height block content at all. Map 0, ADT (25, 26).</summary>
    private const string FlatTile = "0002625.map";

    /// <summary>Per-chunk liquid types with one global level. Map 0, ADT (28, 25).</summary>
    private const string ChunkTypeLiquidTile = "0002528.map";

    /// <summary>A per-cell level grid with no per-chunk types. Map 0, ADT (25, 36).</summary>
    private const string LevelGridLiquidTile = "0003625.map";

    /// <summary>Per-chunk types and a small level rectangle. Map 0, ADT (27, 28).</summary>
    private const string RectLiquidTile = "0002827.map";

    private static byte[] Fixture(string name)
    {
        string plugin = PluginDirectories.Find("WoWMapStudio") ?? throw new InvalidOperationException("WoWMapStudio plugin directory not found");
        return File.ReadAllBytes(Path.Combine(plugin, "src", "Server", "Fixtures", name));
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Int16_tile_decodes_heights_and_per_chunk_areas()
    {
        WowMapTile tile = WowMapTileReader.Read(Fixture(NorthshireTile));

        Assert.IsFalse(tile.IsFlat);
        Assert.AreApproximatelyEqual(236.70396, tile.CornerHeight(0, 0), Tolerance);
        Assert.AreApproximatelyEqual(82.17773, tile.CornerHeight(64, 64), Tolerance);
        Assert.AreApproximatelyEqual(146.80527, tile.CornerHeight(128, 128), Tolerance);
        Assert.AreApproximatelyEqual(135.46139, tile.CentreHeight(10, 10), Tolerance);
        Assert.IsNotNull(tile.Areas);
        Assert.AreEqual((ushort)12, tile.AreaAt(0, 0));
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Holes_follow_the_servers_bit_layout()
    {
        WowMapTile tile = WowMapTileReader.Read(Fixture(NorthshireTile));

        int holeCells = 0;
        for (int row = 0; row < WowMapTile.CellEdge; row++)
        {
            for (int col = 0; col < WowMapTile.CellEdge; col++)
            {
                holeCells += tile.IsHole(row, col) ? 1 : 0;
            }
        }

        Assert.AreEqual(16, holeCells);
        Assert.IsTrue(tile.IsHole(30, 28));
        Assert.IsTrue(tile.IsHole(31, 31));
        Assert.IsFalse(tile.IsHole(0, 0));
        Assert.IsNull(tile.HeightAt(30.5f, 28.5f), "the server reports no ground over a hole");
        Assert.IsNotNull(tile.HeightAt(0.5f, 0.5f));
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Liquid_rectangle_is_offset_within_the_tile()
    {
        WowMapTile tile = WowMapTileReader.Read(Fixture(NorthshireTile));

        Assert.IsTrue(tile.HasLiquid);
        Assert.AreEqual(47, tile.LiquidOffsetCol);
        Assert.AreEqual(0, tile.LiquidOffsetRow);
        Assert.AreEqual(79, tile.LiquidWidth);
        Assert.AreEqual(129, tile.LiquidHeight);

        WowMapLiquidCell? wet = tile.LiquidAt(0, 109);
        Assert.IsTrue(wet.HasValue);
        Assert.AreApproximatelyEqual(143.98892, wet.Value.Level, Tolerance);
        Assert.AreEqual((ushort)5, wet.Value.Entry);
        Assert.AreEqual((byte)8, wet.Value.TypeFlags);

        Assert.IsNull(tile.LiquidAt(0, 92), "a chunk with no liquid type is dry");
        Assert.IsNull(tile.LiquidAt(0, 0), "outside the liquid rectangle");
        Assert.IsNull(tile.LiquidAt(0, 126), "one column past the rectangle");
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Northshire_spawn_lands_in_the_expected_tile_cell_and_height()
    {
        const float serverX = -8949.95f;
        const float serverY = -132.49f;

        Assert.AreEqual((32, 48), WowTileAddressing.TileAtServer(serverX, serverY));
        Assert.AreEqual("0004832.map", WowMapGrid.FileName(0, 32, 48));

        (float row, float col) = WowMapGrid.ToGrid(serverX, serverY, 32, 48);
        Assert.AreApproximatelyEqual(99.988, row, 0.01);
        Assert.AreApproximatelyEqual(31.798, col, 0.01);

        WowMapTile tile = WowMapTileReader.Read(Fixture(NorthshireTile));
        float height = tile.HeightAt(row, col)!.Value;
        Assert.AreApproximatelyEqual(83.53174, height, Tolerance);

        Vector3 point = WowMapGrid.ToGodot(row, col, height, 32, 48);
        Assert.AreApproximatelyEqual(serverX, point.X, 0.05);
        Assert.AreApproximatelyEqual(height, point.Y, Tolerance);
        Assert.AreApproximatelyEqual(-serverY, point.Z, 0.05);

        (float backX, float backY) = WowMapGrid.ToServer(row, col, 32, 48);
        Assert.AreApproximatelyEqual(serverX, backX, 0.01);
        Assert.AreApproximatelyEqual(serverY, backY, 0.01);
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Int8_tile_decodes_and_uses_the_global_area_and_ocean()
    {
        WowMapTile tile = WowMapTileReader.Read(Fixture(Int8Tile));

        Assert.AreApproximatelyEqual(-515.55942, tile.CornerHeight(0, 0), Tolerance);
        Assert.AreApproximatelyEqual(-515.56384, tile.CornerHeight(64, 64), Tolerance);
        Assert.AreApproximatelyEqual(-515.21412, tile.CentreHeight(10, 10), Tolerance);
        Assert.IsNull(tile.Areas);
        Assert.AreEqual((ushort)2399, tile.AreaAt(100, 100));

        Assert.AreEqual(2, (int)tile.LiquidGlobalEntry);
        Assert.AreEqual((byte)18, tile.LiquidGlobalFlags);
        WowMapLiquidCell? cell = tile.LiquidAt(64, 64);
        Assert.IsTrue(cell.HasValue);
        Assert.AreEqual(0f, cell.Value.Level);
        Assert.AreEqual((byte)18, cell.Value.TypeFlags);
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Int8_tile_reads_its_area_map()
    {
        WowMapTile tile = WowMapTileReader.Read(Fixture(Int8AreaTile));

        Assert.IsNotNull(tile.Areas);
        Assert.AreEqual((ushort)2399, tile.AreaAt(0, 0));
        Assert.AreApproximatelyEqual(-515.18756, tile.CornerHeight(64, 64), Tolerance);
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Flat_tile_has_no_grids_and_answers_with_its_grid_height()
    {
        WowMapTile tile = WowMapTileReader.Read(Fixture(FlatTile));

        Assert.IsTrue(tile.IsFlat);
        Assert.IsNull(tile.Centres);
        Assert.AreApproximatelyEqual(-501.50668, tile.CornerHeight(40, 40), Tolerance);
        Assert.AreApproximatelyEqual(-501.50668, tile.HeightAt(17.3f, 99.9f)!.Value, Tolerance);
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Liquid_blocks_decode_each_flag_combination()
    {
        WowMapTile chunkTypes = WowMapTileReader.Read(Fixture(ChunkTypeLiquidTile));
        WowMapLiquidCell? ocean = chunkTypes.LiquidAt(0, 0);
        Assert.IsTrue(ocean.HasValue);
        Assert.AreEqual((ushort)2, ocean.Value.Entry);
        Assert.AreEqual((byte)18, ocean.Value.TypeFlags);
        Assert.AreEqual(0f, ocean.Value.Level);

        WowMapTile levelGrid = WowMapTileReader.Read(Fixture(LevelGridLiquidTile));
        Assert.AreEqual(129 * 129, levelGrid.LiquidLevels!.Length);
        Assert.AreEqual((ushort)2, levelGrid.LiquidAt(5, 5)!.Value.Entry);

        WowMapTile rect = WowMapTileReader.Read(Fixture(RectLiquidTile));
        Assert.AreEqual(0, rect.LiquidOffsetCol);
        Assert.AreEqual(119, rect.LiquidOffsetRow);
        Assert.AreEqual(9, rect.LiquidWidth);
        Assert.AreEqual(10, rect.LiquidHeight);
        Assert.AreApproximatelyEqual(-2000, rect.LiquidAt(119, 0)!.Value.Level, Tolerance, "a cell with no water carries the sentinel level");
        Assert.AreEqual((byte)2, rect.LiquidAt(119, 0)!.Value.TypeFlags);
        Assert.IsNull(rect.LiquidAt(118, 0), "one row before the rectangle");
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Float_heights_and_the_four_triangle_split_reproduce_a_plane()
    {
        // Corners and centres sampled from h = 2 * row + 3 * col, so every triangle of a cell must
        // interpolate back onto the plane. The corner grid is offset by one cell in each direction from
        // the centre grid, so a wrong index in any of the four branches lands off the plane.
        byte[] bytes = BuildFloatTile((row, col) => (2 * row) + (3 * col));
        WowMapTile tile = WowMapTileReader.Read(bytes);

        foreach ((float row, float col) in new[] { (10.6f, 20.2f), (10.2f, 20.6f), (10.9f, 20.5f), (10.5f, 20.9f) })
        {
            Assert.AreApproximatelyEqual((2 * row) + (3 * col), tile.HeightAt(row, col)!.Value, 0.001);
        }
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Wrong_version_is_refused_with_a_re_extract_message()
    {
        byte[] bytes = Fixture(FlatTile);
        "z1.5"u8.CopyTo(bytes.AsSpan(4));

        WowServerDataException error = Assert.Throws<WowServerDataException>(() => WowMapTileReader.Read(bytes));
        Assert.IsTrue(error.Message.Contains("Re-extract"), error.Message);
        Assert.IsTrue(error.Message.Contains("z1.5"), error.Message);
    }

    [EditorTest(Category = "WoW Server Data", Thread = TestThread.Background)]
    public static void Wrong_magic_and_truncation_are_refused()
    {
        byte[] wrongMagic = Fixture(FlatTile);
        wrongMagic[0] = (byte)'X';
        Assert.Throws<WowServerDataException>(() => WowMapTileReader.Read(wrongMagic));

        byte[] cut = Fixture(NorthshireTile).AsSpan(0, 30_000).ToArray();
        Assert.Throws<WowServerDataException>(() => WowMapTileReader.Read(cut));
        Assert.Throws<WowServerDataException>(() => WowMapTileReader.Read(new byte[10]));
    }

    /// <summary>A minimal float-height <c>.map</c>: header, then a height block, nothing else.</summary>
    private static byte[] BuildFloatTile(Func<float, float, float> surface)
    {
        const int cornerCount = WowMapTile.VertexEdge * WowMapTile.VertexEdge;
        const int centreCount = WowMapTile.CellEdge * WowMapTile.CellEdge;
        const int heightOffset = 40;
        int size = heightOffset + 16 + ((cornerCount + centreCount) * 4);
        byte[] bytes = new byte[size];
        Span<byte> span = bytes;

        "MAPS"u8.CopyTo(span);
        WowServerFormats.MapVersionMagic.CopyTo(span[4..]);
        BinaryPrimitives.WriteUInt32LittleEndian(span[16..], heightOffset);
        BinaryPrimitives.WriteUInt32LittleEndian(span[20..], (uint)(size - heightOffset));

        "MHGT"u8.CopyTo(span[heightOffset..]);
        int cursor = heightOffset + 16;
        for (int row = 0; row < WowMapTile.VertexEdge; row++)
        {
            for (int col = 0; col < WowMapTile.VertexEdge; col++)
            {
                BinaryPrimitives.WriteSingleLittleEndian(span[cursor..], surface(row, col));
                cursor += 4;
            }
        }

        for (int row = 0; row < WowMapTile.CellEdge; row++)
        {
            for (int col = 0; col < WowMapTile.CellEdge; col++)
            {
                BinaryPrimitives.WriteSingleLittleEndian(span[cursor..], surface(row + 0.5f, col + 0.5f));
                cursor += 4;
            }
        }

        return bytes;
    }
}
