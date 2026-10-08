using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.Data.Sqlite;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Trainer and vendor saves, edits and deletes against a throwaway copy of the world database.</summary>
public static class VendorTrainerCommitTests
{
    [EditorTest(Category = "WoW Commit", Thread = TestThread.Background)]
    public static async Task Trainer_and_vendor_rows_round_trip_through_the_database()
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

        string copy = Path.Combine(Path.GetTempPath(), $"__wms_trainer_commit_test_{Guid.NewGuid():N}.doltlite");
        File.Copy(source, copy);
        try
        {
            var context = new EditorContext(new Node3D(), new Project { Name = "__wms_trainer_commit_test__" });
            var world = (WorldStorage)context.Database.Storages.Single(s => s.Name == WorldStorage.StorageName);
            context.Database.EditorStorage.Location.DatabasePath = copy;
            context.Database.EditorStorage.EnsureSchema();
            var trainers = new NpcTrainerFactory(world);
            var vendors = new NpcVendorFactory(world);

            const int Entry = 2_000_000_123;
            var trainer = (TrainerTable)trainers.Create(context, Entry.ToString());
            trainer.AddRow(100).Apply();
            trainer.AddRow(101).Apply();
            trainer.SetRowField(0, nameof(TrainerRow.SpellCost), 55).Apply();
            var vendor = (VendorTable)vendors.Create(context, Entry.ToString());
            vendor.AddRow(6948).Apply();
            await world.CommitAsync([trainer, vendor], []).ConfigureAwait(false);

            trainer.RemoveRow(1).Apply();
            await world.CommitAsync([trainer], []).ConfigureAwait(false);
            context.Catalog.Remove(trainer);
            var reopened = (TrainerTable)(await trainers.OpenAsync(context, Entry.ToString()).ConfigureAwait(false))!;
            Assert.AreEqual(1, reopened.Rows.Count);
            Assert.AreEqual(55, reopened.Rows[0].SpellCost);

            await using (WorldDbContext db = world.CreateContext())
            {
                Assert.AreEqual(1, await db.NpcVendor.AsNoTracking().CountAsync(r => r.Entry == Entry).ConfigureAwait(false));
            }

            // An existing entry saves unchanged without key conflicts.
            IReadOnlyList<CatalogSearchResult> found = await trainers.SearchAsync(string.Empty).ConfigureAwait(false);
            string existing = found.First(r => r.Key != Entry.ToString()).Key;
            var real = (TrainerTable)(await trainers.OpenAsync(context, existing).ConfigureAwait(false))!;
            int before = real.Rows.Count;
            await world.CommitAsync([real], []).ConfigureAwait(false);
            context.Catalog.Remove(real);
            var again = (TrainerTable)(await trainers.OpenAsync(context, existing).ConfigureAwait(false))!;
            Assert.AreEqual(before, again.Rows.Count);

            await world.CommitAsync([], [reopened, vendor]).ConfigureAwait(false);
            await using WorldDbContext after = world.CreateContext();
            Assert.IsFalse(await after.NpcTrainer.AnyAsync(r => r.Entry == Entry).ConfigureAwait(false));
            Assert.IsFalse(await after.NpcVendor.AnyAsync(r => r.Entry == Entry).ConfigureAwait(false));
        }
        finally
        {
            SqliteConnection.ClearAllPools();
            File.Delete(copy);
        }
    }
}
