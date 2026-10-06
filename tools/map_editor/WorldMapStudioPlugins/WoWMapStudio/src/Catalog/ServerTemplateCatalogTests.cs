using Godot;

namespace WorldMapStudio.WoW;

public static class ServerTemplateCatalogTests
{
    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Creature_template_round_trips_through_its_record()
    {
        var factory = new CreatureTemplateFactory(null!);
        var record = new CreatureTemplateRecord { Entry = 1234, Name = "Wolf", Subname = "Pet", LevelMin = 3, LevelMax = 5, DisplayId1 = 99 };
        CreatureTemplate entity = factory.CallToEntity(record);
        Assert.AreEqual(1234, entity.ID);
        entity.Name = "Dire Wolf";
        entity.Subname = string.Empty;
        var back = new CreatureTemplateRecord();
        factory.CallWrite(entity, back);
        Assert.AreEqual(1234, back.Entry);
        Assert.AreEqual("Dire Wolf", back.Name);
        Assert.IsTrue(back.Subname is null);
        Assert.AreEqual(99, back.DisplayId1);
    }

    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Gameobject_template_round_trips_through_its_record()
    {
        var factory = new GameObjectTemplateFactory(null!);
        var record = new GameobjectTemplateRecord { Entry = 55, Name = "Chest", Type = 3, DisplayId = 7, Size = 2f };
        GameObjectTemplate entity = factory.CallToEntity(record);
        var back = new GameobjectTemplateRecord();
        factory.CallWrite(entity, back);
        Assert.AreEqual(55, back.Entry);
        Assert.AreEqual("Chest", back.Name);
        Assert.AreEqual(2f, back.Size);
    }
}
