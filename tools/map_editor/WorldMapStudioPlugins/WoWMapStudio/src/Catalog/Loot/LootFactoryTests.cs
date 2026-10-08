using System.Collections.Generic;
using System.Linq;

namespace WorldMapStudio.WoW;

public static class LootFactoryTests
{
    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Loot_rows_round_trip_and_diff()
    {
        var factory = new CreatureLootFactory(null!);
        var existing = new List<CreatureLootTemplateRecord>
        {
            new() { Entry = 5, Item = 10, Groupid = 0, ChanceOrQuestChance = 50f, MincountOrRef = 1, Maxcount = 1 },
            new() { Entry = 5, Item = 11, Groupid = 1, ChanceOrQuestChance = 20f, MincountOrRef = -3, Maxcount = 1 },
        };
        LootTable table = factory.CallToEntity(5, existing);
        Assert.AreEqual(2, table.Rows.Count);
        Assert.AreEqual(-3, table.Rows[1].MinCountOrRef);

        table.SetRowField(0, "Chance", 75).Apply();
        table.RemoveRow(1).Apply();
        table.AddRow(12).Apply();

        var removed = new List<CreatureLootTemplateRecord>();
        var added = new List<CreatureLootTemplateRecord>();
        factory.CallDiff(5, existing, table.Rows, removed.Add, added.Add);
        Assert.AreEqual(75f, existing[0].ChanceOrQuestChance);
        Assert.AreEqual(1, removed.Count);
        Assert.AreEqual(11, removed[0].Item);
        Assert.AreEqual(1, added.Count);
        Assert.AreEqual(12, added[0].Item);
        Assert.AreEqual(5, added[0].Entry);
    }

    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Loot_diff_rejects_duplicate_items()
    {
        var factory = new SkinningLootFactory(null!);
        var table = new LootTable("Skinning") { ID = 1 };
        table.AddRow(7).Apply();
        table.AddRow(7).Apply();
        bool threw = false;
        try
        {
            factory.CallDiff(1, [], table.Rows, _ => { }, _ => { });
        }
        catch (System.InvalidOperationException)
        {
            threw = true;
        }

        Assert.IsTrue(threw);
    }
}
