using System;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

public static class WowCreatureSpawnFactoryTests
{
    private static WowCreatureSpawnFactory NewFactory(EditorContext context) =>
        new((WorldStorage)context.Database.Storages.Single(storage => storage.Name == WorldStorage.StorageName));

    [EditorTest(Category = "WoW Creature", Thread = TestThread.Background)]
    public static void Create_adds_an_undoable_spawn_at_the_given_transform()
    {
        var context = new EditorContext(new Node3D(), new Project { Name = "__wms_creature_spawn_factory_create_test__" });
        WowCreatureSpawnFactory factory = NewFactory(context);

        var transform = new Transform3D(Basis.Identity, new Vector3(10, 20, 30));
        var spawn = (WowCreatureSpawn)factory.Create(context, new MapId(0), transform, "68");

        Assert.AreEqual(68u, spawn.TemplateEntry);
        Assert.IsNull(spawn.Guid, "an uncommitted spawn has no row yet");
        Assert.IsTrue(context.Scene.Contains(spawn));
        Assert.IsTrue(spawn.Transform.Origin.IsEqualApprox(transform.Origin));

        context.EditSessions.Undo();
        Assert.IsFalse(context.Scene.Contains(spawn));
    }

    [EditorTest(Category = "WoW Creature", Thread = TestThread.Background)]
    public static void Create_rejects_a_key_that_is_not_a_valid_entry()
    {
        var context = new EditorContext(new Node3D(), new Project { Name = "__wms_creature_spawn_factory_bad_key_test__" });
        WowCreatureSpawnFactory factory = NewFactory(context);

        Assert.Throws<InvalidOperationException>(
            () => factory.Create(context, new MapId(0), Transform3D.Identity, "not-a-number"));
    }
}
