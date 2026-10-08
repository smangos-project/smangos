using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.Data.Sqlite;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Gossip menu saves, edits and deletes against a throwaway copy of the world database.</summary>
public static class GossipCommitTests
{
    [EditorTest(Category = "WoW Commit", Thread = TestThread.Background)]
    public static async Task Gossip_rows_round_trip_through_the_database()
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

        string copy = Path.Combine(Path.GetTempPath(), $"__wms_gossip_commit_test_{Guid.NewGuid():N}.doltlite");
        File.Copy(source, copy);
        try
        {
            var context = new EditorContext(new Node3D(), new Project { Name = "__wms_gossip_commit_test__" });
            var world = (WorldStorage)context.Database.Storages.Single(s => s.Name == WorldStorage.StorageName);
            context.Database.EditorStorage.Location.DatabasePath = copy;
            context.Database.EditorStorage.EnsureSchema();
            var menus = new GossipMenuFactory(world);
            var options = new GossipMenuOptionFactory(world);

            const int Entry = 2_000_000_321;
            var menu = (GossipTextTable)menus.Create(context, Entry.ToString());
            menu.AddRow(7).Apply();
            var option = (GossipOptionTable)options.Create(context, Entry.ToString());
            option.AddRow(0).Apply();
            option.AddRow(1).Apply();
            option.SetRowText(0, nameof(GossipOptionRow.OptionText), "Hello").Apply();
            option.SetRowField(0, nameof(GossipOptionRow.OptionId), 1).Apply();
            await world.CommitAsync([menu, option], []).ConfigureAwait(false);

            option.RemoveRow(1).Apply();
            await world.CommitAsync([option], []).ConfigureAwait(false);
            context.Catalog.Remove(option);
            var reopened = (GossipOptionTable)(await options.OpenAsync(context, Entry.ToString()).ConfigureAwait(false))!;
            Assert.AreEqual(1, reopened.Rows.Count);
            Assert.AreEqual("Hello", reopened.Rows[0].OptionText);
            Assert.AreEqual(1, reopened.Rows[0].OptionId);

            var real = (GossipOptionTable)(await options.OpenAsync(context, (await options.SearchAsync(string.Empty).ConfigureAwait(false))
                .First(r => r.Key != Entry.ToString()).Key).ConfigureAwait(false))!;
            await world.CommitAsync([real], []).ConfigureAwait(false);

            await world.CommitAsync([], [reopened, menu]).ConfigureAwait(false);
            await using WorldDbContext after = world.CreateContext();
            Assert.IsFalse(await after.GossipMenu.AnyAsync(r => r.Entry == Entry).ConfigureAwait(false));
            Assert.IsFalse(await after.GossipMenuOption.AnyAsync(r => r.MenuId == Entry).ConfigureAwait(false));
        }
        finally
        {
            SqliteConnection.ClearAllPools();
            File.Delete(copy);
        }
    }
}
