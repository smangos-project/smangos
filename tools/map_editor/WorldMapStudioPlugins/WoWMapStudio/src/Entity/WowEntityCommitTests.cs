using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.Data.Sqlite;

namespace WorldMapStudio.WoW;

/// <summary>Commits created entities into a throwaway copy of the world database.</summary>
public static class WowEntityCommitTests
{
    private static string? FindWorldDatabase()
    {
        for (string? dir = AppContext.BaseDirectory; dir != null; dir = Path.GetDirectoryName(dir))
        {
            string file = Path.Combine(dir, ".world", "world.doltlite");
            if (File.Exists(file))
            {
                return file;
            }
        }

        return null;
    }

    [EditorTest(Category = "WoW Commit", Thread = TestThread.Background)]
    public static async Task Point_entities_commit_into_a_scratch_world_database()
    {
        string? source = FindWorldDatabase();
        if (source is null)
        {
            Assert.Skip("world database not available");
            return;
        }

        string copy = Path.Combine(Path.GetTempPath(), $"__wms_wow_commit_test_{Guid.NewGuid():N}.doltlite");
        File.Copy(source, copy);
        try
        {
            var context = new EditorContext(new Node3D(), new Project { Name = "__wms_wow_commit_test__" });
            var world = (WorldStorage)context.Database.Storages.Single(s => s.Name == WorldStorage.StorageName);
            context.Database.EditorStorage.Location.DatabasePath = copy;
            context.Database.EditorStorage.EnsureSchema();

            var map = new MapId(1);
            await WowMapBindings.For(context).SetAsync(map, 0).ConfigureAwait(false);
            var transform = new Transform3D(Basis.Identity, new Vector3(10, 20, 30));

            var node = (WowTaxiNode)new WowTaxiNodeFactory(world).Create(context, map, transform, "Commit Test");
            var trigger = (WowAreaTrigger)new WowAreaTriggerFactory(world).Create(context, map, transform, "");
            var light = (WowLight)new WowLightFactory(world).Create(context, map, transform, "");
            var emitter = (WowSoundEmitter)new WowSoundEmitterFactory(world).Create(context, map, transform, "");

            await world.CommitAsync([node, trigger, light, emitter], []).ConfigureAwait(false);

            Assert.IsNotNull(node.NodeId);
            Assert.IsNotNull(trigger.TriggerId);
            Assert.IsNotNull(light.LightId);
            Assert.IsNotNull(emitter.EmitterId);

            await world.CommitAsync([], [node, trigger, light, emitter]).ConfigureAwait(false);
        }
        finally
        {
            SqliteConnection.ClearAllPools();
            File.Delete(copy);
        }
    }
}
