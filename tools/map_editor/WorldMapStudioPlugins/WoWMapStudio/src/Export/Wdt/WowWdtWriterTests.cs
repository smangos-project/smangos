namespace WorldMapStudio.WoW;

public static class WowWdtWriterTests
{
    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Added_tile_appears_in_the_wdt_and_can_be_removed()
    {
        WowTestContent.RequireFile("world/maps/azeroth/azeroth.wdt");
        string project = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "wms-wdt-test-" + System.Guid.NewGuid().ToString("N"));
        var fs = WowClientFiles.OpenIsolated(project);
        Assert.IsFalse(WowWdt.Read(fs, "azeroth").Has(5, 5), "tile 5,5 is free");

        WowWdtWriter.AddTiles(fs, "azeroth", [((5, 5), new WowTileFiles(2_100_000_001, 2_100_000_002, 0, 2_100_000_003, 0, 0, 0, 0))]);
        var added = WowWdt.Read(fs, "azeroth");
        Assert.IsTrue(added.Has(5, 5), "tile added");
        Assert.IsTrue(added.Has(32, 48), "other tiles kept");
        Assert.AreEqual(2_100_000_001u, added.Files![(5 * 64) + 5].Root);

        WowWdtWriter.RemoveTiles(fs, "azeroth", [(5, 5)]);
        var removed = WowWdt.Read(fs, "azeroth");
        System.IO.Directory.Delete(project, true);
        Assert.IsFalse(removed.Has(5, 5), "tile removed");
        Assert.AreEqual(0u, removed.Files![(5 * 64) + 5].Root);
    }
}
