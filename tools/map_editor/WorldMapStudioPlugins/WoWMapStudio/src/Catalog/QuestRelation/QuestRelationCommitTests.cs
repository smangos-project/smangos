using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.Data.Sqlite;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Quest relation saves and deletes against a throwaway copy of the world database.</summary>
public static class QuestRelationCommitTests
{
    [EditorTest(Category = "WoW Commit", Thread = TestThread.Background)]
    public static async Task Quest_relations_round_trip_through_the_database()
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

        string copy = Path.Combine(Path.GetTempPath(), $"__wms_questrel_commit_test_{Guid.NewGuid():N}.doltlite");
        File.Copy(source, copy);
        try
        {
            var context = new EditorContext(new Node3D(), new Project { Name = "__wms_questrel_commit_test__" });
            var world = (WorldStorage)context.Database.Storages.Single(s => s.Name == WorldStorage.StorageName);
            context.Database.EditorStorage.Location.DatabasePath = copy;
            context.Database.EditorStorage.EnsureSchema();
            var starters = new CreatureQuestStarterFactory(world);
            var enders = new CreatureQuestEnderFactory(world);

            const int Entry = 2_000_000_456;
            var start = (QuestRelationTable)starters.Create(context, Entry.ToString());
            start.AddRow(10).Apply();
            start.AddRow(11).Apply();
            var end = (QuestRelationTable)enders.Create(context, Entry.ToString());
            end.AddRow(10).Apply();
            await world.CommitAsync([start, end], []).ConfigureAwait(false);

            start.RemoveRow(0).Apply();
            await world.CommitAsync([start], []).ConfigureAwait(false);
            context.Catalog.Remove(start);
            var reopened = (QuestRelationTable)(await starters.OpenAsync(context, Entry.ToString()).ConfigureAwait(false))!;
            Assert.AreEqual(1, reopened.Rows.Count);
            Assert.AreEqual(11, reopened.Rows[0].Quest);

            string existing = (await starters.SearchAsync(string.Empty).ConfigureAwait(false)).First(r => r.Key != Entry.ToString()).Key;
            var real = (QuestRelationTable)(await starters.OpenAsync(context, existing).ConfigureAwait(false))!;
            await world.CommitAsync([real], []).ConfigureAwait(false);

            await world.CommitAsync([], [reopened, end]).ConfigureAwait(false);
            await using WorldDbContext after = world.CreateContext();
            Assert.IsFalse(await after.CreatureQuestrelation.AnyAsync(r => r.Id == Entry).ConfigureAwait(false));
            Assert.IsFalse(await after.CreatureInvolvedrelation.AnyAsync(r => r.Id == Entry).ConfigureAwait(false));
        }
        finally
        {
            SqliteConnection.ClearAllPools();
            File.Delete(copy);
        }
    }
}
