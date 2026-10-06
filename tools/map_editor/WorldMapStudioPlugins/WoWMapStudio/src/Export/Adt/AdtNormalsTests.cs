using System.Linq;
using WoWLib.Formats.ADT;
using WoWLib.Formats.Common;

namespace WorldMapStudio.WoW;

public static class AdtNormalsTests
{
    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Computed_normals_match_the_client_files_normals()
    {
        WowTestContent.RequireFile("world/maps/azeroth/azeroth_32_48.adt");
        string project = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "wms-normals-test-" + System.Guid.NewGuid().ToString("N"));
        var fs = WowClientFiles.OpenIsolated(project);
        var wdt = WowWdt.Read(fs, "azeroth");
        var id = new ImportedTileId("azeroth", 32, 48);
        var tile = WowAdtTileReader.Read(fs, wdt, id);
        using var adt = new ADTBfaPlus();
        adt.Read(fs, new WoWLib.FileKey("world/maps/azeroth/azeroth_32_48.adt"), wdt.BigAlpha ? AlphaFormat.Highres8Bit : AlphaFormat.Lowres4Bit);
        System.IO.Directory.Delete(project, true);

        int interiorBad = 0, edgeBad = 0, interior = 0, edge = 0;
        float worst = 0;
        for (int c = 0; c < 256; c++)
        {
            var computed = AdtNormals.Compute(tile.Chunks, tile.Chunks[c].LocalCol, tile.Chunks[c].LocalRow);
            var stored = adt.Chunks[c].Normals;
            Assert.AreEqual(145, stored.Count);
            for (int v = 0; v < 145; v++)
            {
                var n = stored[v].Normal;
                float err = System.MathF.Max(System.MathF.Abs(n[0] - computed[v * 3]), System.MathF.Max(System.MathF.Abs(n[1] - computed[(v * 3) + 1]), System.MathF.Abs(n[2] - computed[(v * 3) + 2])));
                bool onEdge = IsEdge(v);
                if (onEdge) { edge++; if (err > 6) edgeBad++; } else { interior++; if (err > 6) interiorBad++; }
                worst = System.MathF.Max(worst, err);
            }
        }

        Assert.IsTrue(interiorBad < interior / 15, $"interior mismatches {interiorBad}/{interior}, worst {worst}");
        Assert.IsTrue(edgeBad < edge / 5, $"edge mismatches {edgeBad}/{edge}");
    }

    private static bool IsEdge(int v)
    {
        int rowStride = 17;
        int row = v / rowStride, k = v % rowStride;
        if (k < 9)
        {
            return row == 0 || row == 8 || k == 0 || k == 8;
        }

        return false;
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Raised_vertex_changes_normals_of_its_chunk_and_neighbour_only()
    {
        WowTestContent.RequireFile("world/maps/azeroth/azeroth_32_48.adt");
        string project = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "wms-normals-test-" + System.Guid.NewGuid().ToString("N"));
        var fs = WowClientFiles.OpenIsolated(project);
        var wdt = WowWdt.Read(fs, "azeroth");
        var id = new ImportedTileId("azeroth", 32, 48);
        var tile = WowAdtTileReader.Read(fs, wdt, id);
        var alpha = wdt.BigAlpha ? AlphaFormat.Highres8Bit : AlphaFormat.Lowres4Bit;
        var key = new WoWLib.FileKey("world/maps/azeroth/azeroth_32_48.adt");

        // Chunk 17 = (col 1, row 1); raise its corner (0, 0), which is shared with chunk 0.
        var chunks = tile.Chunks.ToList();
        var src = chunks[17];
        var raised = (float[])src.OuterHeights.Clone();
        raised[0] += 20f;
        chunks[17] = new ImportedChunk
        {
            LocalCol = src.LocalCol, LocalRow = src.LocalRow, OuterHeights = raised, InnerHeights = src.InnerHeights,
            Layers = src.Layers, Holes = src.Holes, AreaId = src.AreaId, Flags = src.Flags,
        };
        var edited = new ImportedTile { TileId = id, Textures = tile.Textures, HeightTextures = tile.HeightTextures, Doodads = tile.Doodads, Objects = tile.Objects, Chunks = chunks };

        using var before = new ADTBfaPlus();
        before.Read(fs, key, alpha);
        var snapshot = new sbyte[256][];
        for (int c = 0; c < 256; c++)
        {
            snapshot[c] = Flatten(before.Chunks[c]);
        }

        WowAdtTileWriter.Write(fs, wdt, edited);
        using var after = new ADTBfaPlus();
        after.Read(fs, key, alpha);
        System.IO.Directory.Delete(project, true);

        Assert.IsFalse(Flatten(after.Chunks[17]).SequenceEqual(snapshot[17]), "edited chunk normals change");
        Assert.IsFalse(Flatten(after.Chunks[0]).SequenceEqual(snapshot[0]), "neighbour sharing the corner changes");
        Assert.IsTrue(Flatten(after.Chunks[200]).SequenceEqual(snapshot[200]), "distant chunk keeps its normals");
    }

    private static sbyte[] Flatten(WoWLib.Formats.ADT.MapChunkCataPlus chunk)
    {
        var result = new sbyte[chunk.Normals.Count * 3];
        for (int v = 0; v < chunk.Normals.Count; v++)
        {
            for (int k = 0; k < 3; k++)
            {
                result[(v * 3) + k] = chunk.Normals[v].Normal[k];
            }
        }

        return result;
    }
}
