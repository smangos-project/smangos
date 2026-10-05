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

    [EditorTest(Category = "WoW")]
    public static void World_model_declares_every_generated_table_with_its_key()
    {
        var context = new EditorContext(new Node3D(), new Project { Name = "__wms_world_model_test__" });
        Schema expected = context.Database.Storages.Single(storage => storage.Name == WorldStorage.StorageName).ExpectedSchema()!;

        Assert.IsTrue(expected.Tables.Count > 900, "the model carries the world and db2 tables");
        Assert.AreEqual("ID", string.Join(",", expected.Tables["dbc_map"].PrimaryKey));
        Assert.IsNotNull(expected.Tables["dbc_map"].Column("MapName_lang"));
        Assert.IsTrue(expected.Tables["creature"].PrimaryKey.Count > 0);
    }
}
