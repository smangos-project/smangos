using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

public static class WorldStorageTests
{
    [EditorTest(Category = "WoW")]
    public static void World_storage_shares_the_editor_location()
    {
        var context = new EditorContext(new Node3D(), new Project { Name = "__wms_world_storage_test__" });
        Storage world = context.Database.Storages.Single(storage => storage.Name == WorldStorage.StorageName);

        Assert.IsFalse(world.OwnsLocation);
        Assert.IsFalse(world.OwnsTable("dbc_map"));
        Assert.IsTrue(ReferenceEquals(world.Location, context.Database.EditorStorage.Location));
    }
}
