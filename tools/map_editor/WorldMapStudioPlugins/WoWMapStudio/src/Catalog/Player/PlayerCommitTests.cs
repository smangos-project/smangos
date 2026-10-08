using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.Data.Sqlite;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Player start and level stat saves against a throwaway copy of the world database.</summary>
public static class PlayerCommitTests
{
    [EditorTest(Category = "WoW Commit", Thread = TestThread.Background)]
    public static async Task Player_rows_round_trip_through_the_database()
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

        string copy = Path.Combine(Path.GetTempPath(), $"__wms_player_commit_test_{Guid.NewGuid():N}.doltlite");
        File.Copy(source, copy);
        try
        {
            var context = new EditorContext(new Node3D(), new Project { Name = "__wms_player_commit_test__" });
            var world = (WorldStorage)context.Database.Storages.Single(s => s.Name == WorldStorage.StorageName);
            context.Database.EditorStorage.Location.DatabasePath = copy;
            context.Database.EditorStorage.EnsureSchema();
            var spells = new StartSpellFactory(world);
            var stats = new LevelStatsFactory(world);

            const int Entry = 99_007;
            var spellTable = (PlayerRowTable<StartSpellRow>)spells.Create(context, Entry.ToString());
            spellTable.AddRow(100).Apply();
            spellTable.AddRow(101).Apply();
            var statTable = (PlayerRowTable<LevelStatsRow>)stats.Create(context, Entry.ToString());
            statTable.AddRow(1).Apply();
            statTable.SetRowField(0, nameof(LevelStatsRow.Str), 12).Apply();
            await world.CommitAsync([spellTable, statTable], []).ConfigureAwait(false);

            await using (WorldDbContext db = world.CreateContext())
            {
                Assert.AreEqual(2, await db.PlayercreateinfoSpell.CountAsync(r => r.Race == 99 && r.Class == 7).ConfigureAwait(false));
            }

            context.Catalog.Remove(statTable);
            var reopened = (PlayerRowTable<LevelStatsRow>)(await stats.OpenAsync(context, Entry.ToString()).ConfigureAwait(false))!;
            Assert.AreEqual(12, reopened.Rows[0].Str);

            string existing = (await stats.SearchAsync(string.Empty).ConfigureAwait(false)).First(r => r.Key != Entry.ToString()).Key;
            var real = (PlayerRowTable<LevelStatsRow>)(await stats.OpenAsync(context, existing).ConfigureAwait(false))!;
            await world.CommitAsync([real], []).ConfigureAwait(false);

            await world.CommitAsync([], [spellTable, reopened]).ConfigureAwait(false);
            await using WorldDbContext after = world.CreateContext();
            Assert.IsFalse(await after.PlayercreateinfoSpell.AnyAsync(r => r.Race == 99).ConfigureAwait(false));
            Assert.IsFalse(await after.PlayerLevelstats.AnyAsync(r => r.Race == 99).ConfigureAwait(false));
        }
        finally
        {
            SqliteConnection.ClearAllPools();
            File.Delete(copy);
        }
    }
}
