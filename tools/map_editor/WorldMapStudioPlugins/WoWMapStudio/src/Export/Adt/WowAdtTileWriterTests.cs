using System.Linq;

namespace WorldMapStudio.WoW;

public static class WowAdtTileWriterTests
{
    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Edited_tile_round_trips_through_the_project_overlay()
    {
        WowTestContent.RequireFile("world/maps/azeroth/azeroth_32_48.adt");
        var fs = WowClientFiles.Shared;
        var wdt = WowWdt.Read(fs, "azeroth");
        var id = new ImportedTileId("azeroth", 32, 48);
        var tile = WowAdtTileReader.Read(fs, wdt, id);

        var chunks = tile.Chunks.ToList();
        var first = chunks[0];
        var raised = (float[])first.OuterHeights.Clone();
        raised[10] += 5f;
        chunks[0] = new ImportedChunk
        {
            LocalCol = first.LocalCol, LocalRow = first.LocalRow, OuterHeights = raised, InnerHeights = first.InnerHeights,
            Layers = first.Layers, Holes = first.Holes, AreaId = first.AreaId, Flags = first.Flags,
        };
        var edited = new ImportedTile { TileId = id, Textures = tile.Textures, HeightTextures = tile.HeightTextures, Doodads = tile.Doodads, Objects = tile.Objects, Chunks = chunks };

        WowAdtTileWriter.Write(fs, wdt, edited);
        var back = WowAdtTileReader.Read(fs, wdt, id);

        Assert.IsTrue(System.MathF.Abs(back.Chunks[0].OuterHeights[10] - raised[10]) < 0.01f, "edited height persists");
        Assert.IsTrue(System.MathF.Abs(back.Chunks[1].OuterHeights[3] - tile.Chunks[1].OuterHeights[3]) < 0.01f, "other chunks unchanged");
        Assert.AreEqual(tile.Textures.Count, back.Textures.Count);
        Assert.AreEqual(tile.Chunks[0].Layers.Count, back.Chunks[0].Layers.Count);
    }
}
