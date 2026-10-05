using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;

namespace WorldMapStudio.WoW;

public static class AdtReimportGuardTests
{
    private const int TestMap = 987_651;

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void A_tiles_chunk_range_bounds_all_its_chunks_exactly()
    {
        (ChunkCoord min, ChunkCoord max) = AdtReimportGuard.ChunkRange(32, 32);
        List<ChunkCoord> chunks = WowTileAddressing.EnumerateChunks(new WowTileId("", 32, 32)).Select(entry => entry.Coord).ToList();

        Assert.AreEqual(chunks.Min(coord => coord.X), min.X);
        Assert.AreEqual(chunks.Min(coord => coord.Y), min.Y);
        Assert.AreEqual(chunks.Max(coord => coord.X), max.X);
        Assert.AreEqual(chunks.Max(coord => coord.Y), max.Y);
        Assert.AreEqual(WowTileAddressing.ChunksPerTileEdge - 1, max.X - min.X);
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Main)]
    public static async Task Only_a_tile_with_an_edit_newer_than_its_import_is_reported()
    {
        var context = new EditorContext(new Node3D(), new Project { Name = "__wms_adt_reimport_test__" });
        EditorStorage storage = context.Database.EditorStorage;
        string databasePath = System.IO.Path.Combine(System.IO.Path.GetTempPath(), $"__wms_adt_reimport_{Guid.NewGuid():N}.doltlite");
        storage.Location.DatabasePath = databasePath;
        storage.EnsureSchema();
        (ChunkCoord min, _) = AdtReimportGuard.ChunkRange(10, 10);

        await storage.RemoveChunkChangesAsync([(TestMap, min.X, min.Y)]);
        await storage.UpsertChunkChangesAsync([(TestMap, min.X, min.Y)]);
        try
        {
            var imported = new AdtImportTileRecord { ProjectMap = TestMap, FileX = 10, FileY = 10, ImportedAtUtc = DateTime.UtcNow.AddHours(-1) };
            var untouched = new AdtImportTileRecord { ProjectMap = TestMap, FileX = 11, FileY = 11, ImportedAtUtc = DateTime.UtcNow.AddHours(-1) };
            var editedBeforeImport = new AdtImportTileRecord { ProjectMap = TestMap, FileX = 10, FileY = 10, ImportedAtUtc = DateTime.UtcNow.AddHours(1) };

            var edited = await AdtReimportGuard.EditedTilesAsync(storage, new MapId(TestMap), [imported, untouched]);
            var stale = await AdtReimportGuard.EditedTilesAsync(storage, new MapId(TestMap), [editedBeforeImport]);

            Assert.AreEqual(1, edited.Count);
            Assert.AreEqual((10, 10), edited[0]);
            Assert.AreEqual(0, stale.Count, "an edit older than the import was overwritten by it already");
        }
        finally
        {
            await storage.RemoveChunkChangesAsync([(TestMap, min.X, min.Y)]);
            Microsoft.Data.Sqlite.SqliteConnection.ClearAllPools();
            System.IO.File.Delete(databasePath);
        }
    }
}
