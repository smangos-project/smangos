using System.Linq;

namespace WorldMapStudio.WoW;

public static class WowAdtTileWriterTests
{
    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Edited_tile_round_trips_through_the_project_overlay()
    {
        WowTestContent.RequireFile("world/maps/azeroth/azeroth_32_48.adt");
        string project = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "wms-writer-test-" + System.Guid.NewGuid().ToString("N"));
        var fs = WowClientFiles.OpenIsolated(project);
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
        System.IO.Directory.Delete(project, true);

        Assert.IsTrue(System.MathF.Abs(back.Chunks[0].OuterHeights[10] - raised[10]) < 0.01f, "edited height persists");
        Assert.IsTrue(System.MathF.Abs(back.Chunks[1].OuterHeights[3] - tile.Chunks[1].OuterHeights[3]) < 0.01f, "other chunks unchanged");
        Assert.AreEqual(tile.Textures.Count, back.Textures.Count);
        Assert.AreEqual(tile.Chunks[0].Layers.Count, back.Chunks[0].Layers.Count);
        Assert.IsTrue(back.Chunks[0].Layers.Skip(1).All(l => l.Alpha is { Length: 4096 }), "overlay alpha survives");
    }
    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Unedited_tile_round_trip_has_no_height_drift()
    {
        WowTestContent.RequireFile("world/maps/azeroth/azeroth_32_48.adt");
        string project = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "wms-writer-test-" + System.Guid.NewGuid().ToString("N"));
        var fs = WowClientFiles.OpenIsolated(project);
        var wdt = WowWdt.Read(fs, "azeroth");
        var id = new ImportedTileId("azeroth", 32, 48);
        var tile = WowAdtTileReader.Read(fs, wdt, id);
        WowAdtTileWriter.Write(fs, wdt, tile);
        var back = WowAdtTileReader.Read(fs, wdt, id);
        System.IO.Directory.Delete(project, true);

        Assert.AreEqual(tile.Chunks.Count, back.Chunks.Count);
        float max = 0;
        for (int c = 0; c < tile.Chunks.Count; c++)
        {
            for (int i = 0; i < tile.Chunks[c].OuterHeights.Length; i++)
                max = System.MathF.Max(max, System.MathF.Abs(tile.Chunks[c].OuterHeights[i] - back.Chunks[c].OuterHeights[i]));
            for (int i = 0; i < tile.Chunks[c].InnerHeights.Length; i++)
                max = System.MathF.Max(max, System.MathF.Abs(tile.Chunks[c].InnerHeights[i] - back.Chunks[c].InnerHeights[i]));
        }
        Assert.IsTrue(max < 0.01f, "max height drift " + max);
        for (int c = 0; c < tile.Chunks.Count; c++)
        {
            Assert.IsTrue((tile.Chunks[c].VertexColors == null) == (back.Chunks[c].VertexColors == null), "vertex colour presence");
            if (tile.Chunks[c].VertexColors is { } a)
            {
                for (int i = 0; i < a.Length; i++)
                {
                    Assert.IsTrue(a[i].IsEqualApprox(back.Chunks[c].VertexColors![i]), "vertex colour " + c + "/" + i);
                }
            }
            for (int l = 0; l < tile.Chunks[c].Layers.Count; l++)
            {
                Assert.AreEqual(tile.Chunks[c].Layers[l].EffectId, back.Chunks[c].Layers[l].EffectId);
            }
        }
    }
    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Moved_doodad_is_written_back()
    {
        WowTestContent.RequireFile("world/maps/azeroth/azeroth_32_48.adt");
        string project = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "wms-writer-test-" + System.Guid.NewGuid().ToString("N"));
        var fs = WowClientFiles.OpenIsolated(project);
        var wdt = WowWdt.Read(fs, "azeroth");
        var id = new ImportedTileId("azeroth", 32, 48);
        var tile = WowAdtTileReader.Read(fs, wdt, id);
        Assert.IsTrue(tile.Doodads.Count > 0, "tile has doodads");

        var doodads = tile.Doodads.ToList();
        var moved = doodads[0] with { Position = doodads[0].Position + new Godot.Vector3(3, 0, 0) };
        doodads[0] = moved;
        var edited = new ImportedTile { TileId = id, Textures = tile.Textures, HeightTextures = tile.HeightTextures, Doodads = doodads, Objects = tile.Objects, Chunks = tile.Chunks };
        WowAdtTileWriter.Write(fs, wdt, edited);
        var back = WowAdtTileReader.Read(fs, wdt, id);
        System.IO.Directory.Delete(project, true);

        var found = back.Doodads.First(d => d.UniqueId == moved.UniqueId);
        Assert.IsTrue(found.Position.DistanceTo(moved.Position) < 0.01f, "doodad position persists");
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Added_and_removed_placements_are_written_with_refs()
    {
        WowTestContent.RequireFile("world/maps/azeroth/azeroth_32_48.adt");
        string project = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "wms-writer-test-" + System.Guid.NewGuid().ToString("N"));
        var fs = WowClientFiles.OpenIsolated(project);
        var wdt = WowWdt.Read(fs, "azeroth");
        var id = new ImportedTileId("azeroth", 32, 48);
        var tile = WowAdtTileReader.Read(fs, wdt, id);

        var doodads = tile.Doodads.ToList();
        var removed = doodads[0];
        doodads.RemoveAt(0);
        var added = doodads[0] with { UniqueId = 0x7fff0001, Position = new Godot.Vector3(32.5f * 533.33333f + 10, 50, 48.5f * 533.33333f + 10) };
        doodads.Add(added);
        var edited = new ImportedTile { TileId = id, Textures = tile.Textures, HeightTextures = tile.HeightTextures, Doodads = doodads, Objects = tile.Objects, Chunks = tile.Chunks };
        WowAdtTileWriter.Write(fs, wdt, edited);
        var back = WowAdtTileReader.Read(fs, wdt, id);
        System.IO.Directory.Delete(project, true);

        Assert.AreEqual(doodads.Count, back.Doodads.Count);
        Assert.IsFalse(back.Doodads.Any(d => d.UniqueId == removed.UniqueId), "removed doodad is gone");
        var found = back.Doodads.First(d => d.UniqueId == added.UniqueId);
        Assert.AreEqual(added.ModelFdid, found.ModelFdid);
    }
}
