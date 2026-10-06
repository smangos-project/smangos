using System.Linq;

namespace WorldMapStudio.WoW;

public static class WdlTileSamplerTests
{
    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Sampled_tile_heights_match_the_client_wdl()
    {
        WowTestContent.RequireFile("world/maps/azeroth/azeroth.wdl");
        WowTestContent.RequireFile("world/maps/azeroth/azeroth_32_48.adt");
        var fs = WowClientFiles.Shared;
        var wdt = WowWdt.Read(fs, "azeroth");
        var tile = WowAdtTileReader.Read(fs, wdt, new ImportedTileId("azeroth", 32, 48));
        using var wdl = WoWLib.Formats.WDL.WDL.ForVersion(WoWLib.Expansion.Bfa);
        wdl.Read(fs, new WoWLib.FileKey("world/maps/azeroth/azeroth.wdl"));
        int slot = Enumerable.Range(0, (48 * 64) + 32).Count(i => wdl.TileOffsets[i] != 0);
        var stock = wdl.Heightmaps[slot];
        var (outer, inner) = WdlTileSampler.Build(tile.Chunks);

        int bad = 0;
        for (int i = 0; i < outer.Length; i++) { if (System.Math.Abs(outer[i] - stock.Outer[i]) > 8) { bad++; } }
        for (int i = 0; i < inner.Length; i++) { if (System.Math.Abs(inner[i] - stock.Inner[i]) > 8) { bad++; } }
        Assert.IsTrue(bad < 20, $"{bad} of 545 heights differ from the stock WDL by more than 8");
    }
}
