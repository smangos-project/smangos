using System.Linq;

namespace WorldMapStudio.WoW;

public static class ImportedTileComparerTests
{
    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Comparer_detects_a_height_edit_only()
    {
        WowTestContent.RequireFile("world/maps/azeroth/azeroth_32_48.adt");
        string project = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "wms-cmp-test-" + System.Guid.NewGuid().ToString("N"));
        var fs = WowClientFiles.OpenIsolated(project);
        var wdt = WowWdt.Read(fs, "azeroth");
        var tile = WowAdtTileReader.Read(fs, wdt, new ImportedTileId("azeroth", 32, 48));
        System.IO.Directory.Delete(project, true);

        Assert.IsTrue(ImportedTileComparer.IsUnchanged(tile, tile), "identical tiles match");

        var chunks = tile.Chunks.ToList();
        var first = chunks[0];
        var raised = (float[])first.OuterHeights.Clone();
        raised[0] += 1f;
        chunks[0] = new ImportedChunk
        {
            LocalCol = first.LocalCol, LocalRow = first.LocalRow, OuterHeights = raised, InnerHeights = first.InnerHeights,
            Layers = first.Layers, Holes = first.Holes, AreaId = first.AreaId, Flags = first.Flags,
            VertexColors = first.VertexColors, InnerVertexColors = first.InnerVertexColors,
            VertexLight = first.VertexLight, InnerVertexLight = first.InnerVertexLight,
        };
        var edited = new ImportedTile { TileId = tile.TileId, Textures = tile.Textures, HeightTextures = tile.HeightTextures, Doodads = tile.Doodads, Objects = tile.Objects, Chunks = chunks };
        Assert.IsTrue(!ImportedTileComparer.IsUnchanged(tile, edited), "edited height detected");
    }
}
