using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.Data.Sqlite;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Loot saves, edits and deletes against a throwaway copy of the world database.</summary>
public static class LootCommitTests
{
    [EditorTest(Category = "WoW Commit", Thread = TestThread.Background)]
    public static async Task Loot_create_edit_and_delete_round_trip_through_the_database()
    {
        string? source = null;
        for (string? dir = AppContext.BaseDirectory; dir != null && source is null; dir = Path.GetDirectoryName(dir))
        {
            string file = Path.Combine(dir, ".world", "world.doltlite");
            source = File.Exists(file) ? file : null;
        }

        if (source is null)
        {
            Assert.Skip("world database not available");
            return;
        }

        string copy = Path.Combine(Path.GetTempPath(), $"__wms_loot_commit_test_{Guid.NewGuid():N}.doltlite");
        File.Copy(source, copy);
        try
        {
            var context = new EditorContext(new Node3D(), new Project { Name = "__wms_loot_commit_test__" });
            var world = (WorldStorage)context.Database.Storages.Single(s => s.Name == WorldStorage.StorageName);
            context.Database.EditorStorage.Location.DatabasePath = copy;
            context.Database.EditorStorage.EnsureSchema();
            var factory = new CreatureLootFactory(world);

            const int Entry = 2_000_000_123;
            var table = (LootTable)factory.Create(context, Entry.ToString());
            table.AddRow(6948).Apply();
            table.AddRow(6949).Apply();
            await world.CommitAsync([table], []).ConfigureAwait(false);
            Assert.AreEqual(2, await CountAsync(world, Entry).ConfigureAwait(false));

            table.SetRowField(0, nameof(LootRow.Chance), 12).Apply();
            table.RemoveRow(1).Apply();
            table.AddRow(6950).Apply();
            await world.CommitAsync([table], []).ConfigureAwait(false);

            context.Catalog.Remove(table);
            var reopened = (LootTable)(await factory.OpenAsync(context, Entry.ToString()).ConfigureAwait(false))!;
            Assert.AreEqual(2, reopened.Rows.Count);
            Assert.AreEqual(12f, reopened.Rows[0].Chance);
            Assert.AreEqual(6950, reopened.Rows[1].Item);

            await world.CommitAsync([], [reopened]).ConfigureAwait(false);
            Assert.AreEqual(0, await CountAsync(world, Entry).ConfigureAwait(false));
        }
        finally
        {
            SqliteConnection.ClearAllPools();
            File.Delete(copy);
        }
    }

    private static async Task<int> CountAsync(WorldStorage world, int entry)
    {
        await using WorldDbContext db = world.CreateContext();
        return await db.CreatureLootTemplate.AsNoTracking().CountAsync(r => r.Entry == entry).ConfigureAwait(false);
    }
}
