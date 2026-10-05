using System.Linq;

namespace WorldMapStudio.WoW;

public static class WowAdtTileReaderTests
{
    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Elwynn_tile_decodes_through_wowlib()
    {
        WowTestContent.RequireFile("world/maps/azeroth/azeroth_32_48.adt");
        var fs = WowClientFiles.Shared;
        var wdt = WowWdt.Read(fs, "azeroth");
        Assert.IsTrue(wdt.Has(32, 48), "Northshire tile is present");
        Assert.IsTrue(wdt.Files != null && wdt.Files[(48 * 64) + 32].Root != 0, "MAID lists the tile");

        var tile = WowAdtTileReader.Read(fs, wdt, new ImportedTileId("azeroth", 32, 48));
        Assert.AreEqual(ImportedTile.ChunkCount, tile.Chunks.Count);
        Assert.IsTrue(tile.Textures.Count > 0 && tile.Textures.All(t => t != 0), "diffuse FDIDs");
        Assert.IsTrue(tile.Doodads.Count > 0 && tile.Doodads.All(d => d.ModelFdid != 0), "doodads carry FDIDs");
        Assert.IsTrue(tile.Objects.Count > 0 && tile.Objects.All(o => o.WmoFdid != 0), "WMOs carry FDIDs");

        var c = tile.Chunks[0];
        Assert.AreEqual(81, c.OuterHeights.Length);
        Assert.IsTrue(c.OuterHeights.All(h => h > -1000 && h < 3000), "heights are absolute world heights");
        Assert.IsTrue(c.Layers.Count > 0 && c.Layers[0].Alpha == null, "base layer has no alpha");
        Assert.IsTrue(c.Layers.Skip(1).All(l => l.Alpha is { Length: 4096 }), "overlay alpha is 64x64");
        Assert.IsTrue(c.Layers.All(l => l.TextureIndex < tile.Textures.Count), "layer texture indices in range");
    }
}
