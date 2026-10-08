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

    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Quest_and_item_round_trip_through_their_records()
    {
        var quests = new QuestTemplateFactory(null!);
        QuestTemplate quest = quests.CallToEntity(new QuestTemplateRecord { Entry = 9, Title = "Wolves", NextQuestId = 10 });
        var questRow = new QuestTemplateRecord();
        quests.CallWrite(quest, questRow);
        Assert.AreEqual(9, questRow.Entry);
        Assert.AreEqual("Wolves", questRow.Title);
        Assert.AreEqual(10, questRow.NextQuestId);

        var items = new ItemFactory(null!);
        Item item = items.CallToEntity(new DbcItemsparseRecord { ID = 6948, DisplayLang = "Hearthstone", BuyPrice = 5 });
        var itemRow = new DbcItemsparseRecord();
        items.CallWrite(item, itemRow);
        Assert.AreEqual("Hearthstone", itemRow.DisplayLang);
        Assert.AreEqual(5, itemRow.BuyPrice);
    }

    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Npc_and_broadcast_text_round_trip_through_their_records()
    {
        var texts = new NpcTextFactory(null!);
        NpcText text = texts.CallToEntity(new NpcTextRecord { ID = 5, BroadcastTextID2 = 77, Probability2 = 0.5f });
        Assert.AreEqual(77, text.BroadcastTextIds[2]);
        text.BroadcastTextIds[7] = 9;
        var textRow = new NpcTextRecord();
        texts.CallWrite(text, textRow);
        Assert.AreEqual(77, textRow.BroadcastTextID2);
        Assert.AreEqual(0.5f, textRow.Probability2);
        Assert.AreEqual(9, textRow.BroadcastTextID7);

        var broadcasts = new BroadcastTextFactory(null!);
        BroadcastText broadcast = broadcasts.CallToEntity(new BroadcastTextRecord { Entry = 77, MaleText = "Hi", EmoteId1 = 1 });
        broadcast.FemaleText = "Hello";
        var row = new BroadcastTextRecord();
        broadcasts.CallWrite(broadcast, row);
        Assert.AreEqual("Hi", row.MaleText);
        Assert.AreEqual("Hello", row.FemaleText);
        Assert.AreEqual(1, row.EmoteId1);
    }
}
