namespace WorldMapStudio.WoW;

/// <summary>
/// Pins <see cref="WowTileAddressing"/> against six real MCNK records read out of the local WotLK
/// client's <c>World\Maps\Azeroth\Azeroth_32_49.adt</c> (<c>ix</c>/<c>iy</c>/<c>zpos</c>/<c>xpos</c>
/// taken straight from the file's bytes, not computed) — the addressing math is unchanged from
/// Vanilla through Cata+.
/// </summary>
public static class WowTileAddressingTests
{
    private const string Map = "Azeroth";

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Real_chunks_resolve_to_the_tile_and_slot_the_client_wrote()
    {
        // The editor ChunkCoord whose terrain belongs in each slot is a step south of the raw
        // -coord.X formula: a WoW MCNK grid is laid south from its header (north) edge, so the slot
        // the client draws over [X-chunk, X] is the editor chunk one step south. See WowTileAddressing.
        AssertAddress(new ChunkCoord(-273, 0), Map, 32, 49, localCol: 0, localRow: 0);
        AssertAddress(new ChunkCoord(-273, 1), Map, 32, 49, localCol: 1, localRow: 0);
        AssertAddress(new ChunkCoord(-273, 2), Map, 32, 49, localCol: 2, localRow: 0);
        AssertAddress(new ChunkCoord(-274, 0), Map, 32, 49, localCol: 0, localRow: 1);
        AssertAddress(new ChunkCoord(-274, 1), Map, 32, 49, localCol: 1, localRow: 1);
        AssertAddress(new ChunkCoord(-288, 15), Map, 32, 49, localCol: 15, localRow: 15);
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Mcnk_slot_index_matches_the_files_own_order()
    {
        // The real file's linear MCNK order for these six records was 0, 1, 2, 16, 17, 255.
        Assert.AreEqual(0, Index(-273, 0));
        Assert.AreEqual(1, Index(-273, 1));
        Assert.AreEqual(2, Index(-273, 2));
        Assert.AreEqual(16, Index(-274, 0));
        Assert.AreEqual(17, Index(-274, 1));
        Assert.AreEqual(255, Index(-288, 15));
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Chunk_at_inverts_address_of()
    {
        foreach (ChunkCoord coord in new[]
                 {
                     new ChunkCoord(-273, 0), new ChunkCoord(-273, 1), new ChunkCoord(-274, 0),
                     new ChunkCoord(-288, 15), new ChunkCoord(0, 0), new ChunkCoord(300, -40),
                 })
        {
            WowMcnkAddress address = WowTileAddressing.AddressOf(Map, coord);
            ChunkCoord roundTripped = WowTileAddressing.ChunkAt(address.Tile, address.LocalCol, address.LocalRow);
            Assert.AreEqual(coord, roundTripped, $"{coord} did not round-trip through its address {address}");
        }
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Enumerate_chunks_yields_all_256_in_on_disk_order()
    {
        var tile = new WowTileId(Map, 32, 49);
        var seen = new System.Collections.Generic.List<(int LocalCol, int LocalRow, ChunkCoord Coord)>(
            WowTileAddressing.EnumerateChunks(tile));

        Assert.AreEqual(256, seen.Count);
        Assert.AreEqual((0, 0, new ChunkCoord(-273, 0)), seen[0]);
        Assert.AreEqual((1, 0, new ChunkCoord(-273, 1)), seen[1]);
        Assert.AreEqual((0, 1, new ChunkCoord(-274, 0)), seen[16]);
        Assert.AreEqual((15, 15, new ChunkCoord(-288, 15)), seen[255]);
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Header_position_matches_the_real_files_mcnk_headers()
    {
        LandscapeGrid grid = AdtGrid();

        // (chunk, expected zpos, expected xpos) — the raw floats read from the real file's MCNK
        // headers at ix/iy (0,0), (1,0), (0,1), (1,1) and (15,15).
        AssertHeaderPosition(grid, new ChunkCoord(-273, 0), -9066.666f, 0.000f);
        AssertHeaderPosition(grid, new ChunkCoord(-273, 1), -9066.666f, -33.334f);
        AssertHeaderPosition(grid, new ChunkCoord(-274, 0), -9100.000f, 0.000f);
        AssertHeaderPosition(grid, new ChunkCoord(-274, 1), -9100.000f, -33.334f);
        AssertHeaderPosition(grid, new ChunkCoord(-288, 15), -9566.666f, -500.000f);
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Tile_centre_and_lookup_match_the_real_tile()
    {
        // Azeroth_32_49's own MCNK headers span server X [-9600, -9066.67], Y [-533.33, 0] (see
        // Header_position_matches...), so its centre is (-9333.33, -266.67).
        (float x, float y) = WowTileAddressing.TileCentreServer(32, 49);
        Assert.AreApproximatelyEqual(-9333.333, x, 0.01, "centre X");
        Assert.AreApproximatelyEqual(-266.667, y, 0.01, "centre Y");

        (int fileX, int fileY) = WowTileAddressing.TileAtServer(x, y);
        Assert.AreEqual(32, fileX, "lookup FileX");
        Assert.AreEqual(49, fileY, "lookup FileY");
    }

    private static int Index(int x, int y) => WowTileAddressing.AddressOf(Map, new ChunkCoord(x, y)).Index;

    private static void AssertAddress(ChunkCoord coord, string map, int fileX, int fileY, int localCol, int localRow)
    {
        WowMcnkAddress address = WowTileAddressing.AddressOf(map, coord);
        Assert.AreEqual(fileX, address.Tile.FileX, $"{coord} FileX");
        Assert.AreEqual(fileY, address.Tile.FileY, $"{coord} FileY");
        Assert.AreEqual(localCol, address.LocalCol, $"{coord} LocalCol");
        Assert.AreEqual(localRow, address.LocalRow, $"{coord} LocalRow");
        Assert.AreEqual($"World\\Maps\\{map}\\{map}_{fileX}_{fileY}", address.Tile.Stem);
    }

    private static void AssertHeaderPosition(LandscapeGrid grid, ChunkCoord coord, float expectedZpos, float expectedXpos)
    {
        (float zpos, float xpos) = WowTileAddressing.HeaderPositionOf(grid, coord);
        Assert.AreApproximatelyEqual(expectedZpos, zpos, 0.01, $"{coord} zpos");
        Assert.AreApproximatelyEqual(expectedXpos, xpos, 0.01, $"{coord} xpos");
    }

    private static LandscapeGrid AdtGrid() => new(new LandscapeSettings
    {
        ChunkWorldSize = WowTileAddressing.CellSize,
        OriginChunkX = 0,
        OriginChunkY = 0,
    });
}
