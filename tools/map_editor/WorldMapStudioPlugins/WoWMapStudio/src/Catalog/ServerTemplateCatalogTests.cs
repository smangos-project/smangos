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

    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Pool_and_creature_addon_round_trip_through_their_records()
    {
        var pools = new PoolTemplateFactory(null!);
        PoolTemplate pool = pools.CallToEntity(new PoolTemplateRecord { Entry = 4, MaxLimit = 2, Description = "Ore" });
        pool.MaxLimit = 3;
        var poolRow = new PoolTemplateRecord();
        pools.CallWrite(pool, poolRow);
        Assert.AreEqual(4, poolRow.Entry);
        Assert.AreEqual(3, poolRow.MaxLimit);

        var addons = new CreatureAddonFactory(null!);
        CreatureAddon addon = addons.CallToEntity(new CreatureAddonRecord { Guid = 8, Auras = "1 2", EmoteState = 3 });
        addon.Auras = string.Empty;
        var addonRow = new CreatureAddonRecord();
        addons.CallWrite(addon, addonRow);
        Assert.AreEqual(8, addonRow.Guid);
        Assert.IsTrue(addonRow.Auras is null);
        Assert.AreEqual(3, addonRow.EmoteState);
    }

    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Game_event_group_and_link_round_trip_through_their_records()
    {
        var events = new GameEventFactory(null!);
        GameEvent ev = events.CallToEntity(new GameEventRecord { Entry = 5, Description = "Fair", Length = 60 });
        ev.Description = string.Empty;
        var evRow = new GameEventRecord();
        events.CallWrite(ev, evRow);
        Assert.AreEqual(5, evRow.Entry);
        Assert.IsTrue(evRow.Description is null);
        Assert.AreEqual(60, evRow.Length);

        var groups = new CreatureGroupMemberFactory(null!);
        CreatureGroupMember member = groups.CallToEntity(new CreatureGroupsRecord { MemberGuid = 9, LeaderGuid = 4, Dist = 2.5f });
        member.Angle = 1.5f;
        var groupRow = new CreatureGroupsRecord();
        groups.CallWrite(member, groupRow);
        Assert.AreEqual(9, groupRow.MemberGuid);
        Assert.AreEqual(4, groupRow.LeaderGuid);
        Assert.AreEqual(1.5f, groupRow.Angle);

        var links = new CreatureLinkFactory(null!);
        CreatureLink link = links.CallToEntity(new CreatureLinkingRecord { Guid = 7, MasterGuid = 3, Flag = 2 });
        var linkRow = new CreatureLinkingRecord();
        links.CallWrite(link, linkRow);
        Assert.AreEqual(7, linkRow.Guid);
        Assert.AreEqual(3, linkRow.MasterGuid);
    }

    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Pool_members_round_trip_through_their_records()
    {
        var creatures = new PoolCreatureMemberFactory(null!);
        PoolCreatureMember c = creatures.CallToEntity(new PoolCreatureRecord { Guid = 6, PoolEntry = 2, Chance = 10 });
        c.PoolEntry = 3;
        var cRow = new PoolCreatureRecord();
        creatures.CallWrite(c, cRow);
        Assert.AreEqual(6, cRow.Guid);
        Assert.AreEqual(3, cRow.PoolEntry);

        var pools = new PoolPoolMemberFactory(null!);
        PoolPoolMember p = pools.CallToEntity(new PoolPoolRecord { PoolId = 8, MotherPool = 1 });
        var pRow = new PoolPoolRecord();
        pools.CallWrite(p, pRow);
        Assert.AreEqual(8, pRow.PoolId);
        Assert.AreEqual(1, pRow.MotherPool);
    }

    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Game_event_spawn_rows_round_trip()
    {
        var factory = new GameEventCreatureFactory(null!);
        PlayerRowTable<EventSpawnRow> table = factory.CallToEntity(12,
            [new GameEventCreatureRecord { Event = 12, Guid = 40 }, new GameEventCreatureRecord { Event = 12, Guid = 30 }]);
        Assert.AreEqual(2, table.Rows.Count);
        Assert.AreEqual(30, table.Rows[0].Guid);
        var added = new System.Collections.Generic.List<GameEventCreatureRecord>();
        factory.CallDiff(12, [new GameEventCreatureRecord { Event = 12, Guid = 40 }],
            [new EventSpawnRow { Guid = 40 }, new EventSpawnRow { Guid = 50 }], _ => { }, added.Add);
        Assert.AreEqual(1, added.Count);
        Assert.AreEqual(12, added[0].Event);
        Assert.AreEqual(50, added[0].Guid);

        var quests = new GameEventQuestFactory(null!);
        PlayerRowTable<EventQuestRow> qTable = quests.CallToEntity(3, [new GameEventQuestRecord { Event = 3, Quest = 77 }]);
        Assert.AreEqual(77, qTable.Rows[0].Quest);
    }

    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Talent_tab_round_trips_through_its_record()
    {
        var tabs = new TalentTabFactory(null!);
        RecordEntity<DbcTalenttabRecord> tab = tabs.CallToEntity(new DbcTalenttabRecord { ID = 4, NameLang = "Arms", ClassMask = 1 });
        tab.SetNumber("OrderIndex", 2);
        var row = new DbcTalenttabRecord();
        tabs.CallWrite(tab, row);
        Assert.AreEqual(4, row.ID);
        Assert.AreEqual("Arms", row.NameLang);
        Assert.AreEqual(2, row.OrderIndex);
    }

    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Generic_script_rows_round_trip()
    {
        var factory = new GenericScriptFactory(null!);
        ScriptTable table = factory.CallToEntity(9,
            [new GenericScriptsRecord { Id = 9, Delay = 5, Command = 1 }, new GenericScriptsRecord { Id = 9, Delay = 2, Command = 15, X = 1.5f }]);
        Assert.AreEqual(2, table.Rows[0].Delay);
        Assert.AreEqual(1.5f, table.Rows[0].X);
        var added = new System.Collections.Generic.List<GenericScriptsRecord>();
        var removed = new System.Collections.Generic.List<GenericScriptsRecord>();
        factory.CallDiff(9, [new GenericScriptsRecord { Id = 9, Delay = 5, Command = 1 }],
            [new ScriptRow { Delay = 5, Command = 2 }], removed.Add, added.Add);
        Assert.AreEqual(1, removed.Count);
        Assert.AreEqual(1, added.Count);
        Assert.AreEqual(9, added[0].Id);
        Assert.AreEqual(2, added[0].Command);
    }

    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Record_backed_dbc_catalogs_round_trip()
    {
        var sets = new ItemSetFactory(null!);
        RecordEntity<DbcItemsetRecord> set = sets.CallToEntity(new DbcItemsetRecord { ID = 3, NameLang = "Dungeon Set", ItemID2 = 55 });
        Assert.AreEqual(55, set.Record.ItemID2);
        set.SetNumber("ItemID17", 9);
        var row = new DbcItemsetRecord();
        sets.CallWrite(set, row);
        Assert.AreEqual(3, row.ID);
        Assert.AreEqual("Dungeon Set", row.NameLang);
        Assert.AreEqual(55, row.ItemID2);
        Assert.AreEqual(9, row.ItemID17);
    }

    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Creature_spells_and_ai_events_round_trip()
    {
        var spells = new CreatureSpellsFactory(null!);
        RecordEntity<CreatureSpellsRecord> list = spells.CallToEntity(new CreatureSpellsRecord { Entry = 12, SpellId3 = 100 });
        Assert.AreEqual(12, list.ID);
        list.SetNumber("SpellId8", 7);
        var row = new CreatureSpellsRecord();
        spells.CallWrite(list, row);
        Assert.AreEqual(12, row.Entry);
        Assert.AreEqual(100, row.SpellId3);
        Assert.AreEqual(7, row.SpellId8);

        var events = new CreatureAiEventFactory(null!);
        RecordEntity<CreatureAiEventsRecord> ev = events.CallToEntity(new CreatureAiEventsRecord { Id = 5, CreatureId = 99, EventType = 4 });
        var evRow = new CreatureAiEventsRecord();
        events.CallWrite(ev, evRow);
        Assert.AreEqual(5, evRow.Id);
        Assert.AreEqual(99, evRow.CreatureId);
    }
}
