using System;
using Godot;

namespace WorldMapStudio.WoW;

public static class SpellTests
{
    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Effect_edits_are_undoable_and_keyed_by_index()
    {
        var spell = new Spell { ID = 1 };
        IEditCommand add = spell.AddEffect(2);
        add.Apply();
        spell.SetEffectField(2, nameof(SpellEffectEntry.EffectBasePoints), 41).Apply();
        spell.SetEffectField(2, nameof(SpellEffectEntry.EffectAmplitude), 1.5).Apply();
        Assert.AreEqual(41, spell.Effects[0].EffectBasePoints);
        Assert.AreEqual(1.5f, spell.Effects[0].EffectAmplitude);

        Assert.IsTrue(Throws(() => spell.AddEffect(2)));
        Assert.IsTrue(Throws(() => spell.SetEffectField(9, nameof(SpellEffectEntry.Effect), 1)));
        Assert.IsTrue(Throws(() => spell.SetEffectField(2, "Nope", 1)));

        IEditCommand remove = spell.RemoveEffect(2);
        remove.Apply();
        Assert.AreEqual(0, spell.Effects.Count);
        remove.Revert();
        Assert.AreEqual(1, spell.Effects.Count);
        add.Revert();
        Assert.AreEqual(0, spell.Effects.Count);
    }

    [EditorTest(Category = "WoW Catalog", Thread = TestThread.Background)]
    public static void Effect_entry_round_trips_through_its_record()
    {
        var record = new DbcSpelleffectRecord
        {
            EffectIndex = 1, Effect = 6, EffectAura = 3, EffectBasePoints = 99, EffectMiscValue1 = 5_000_000_000,
            EffectRealPointsPerLevel = 0.5f, ImplicitTarget1 = 25, EffectTriggerSpell = 133,
        };
        SpellEffectEntry entry = SpellFactory.ToEffectEntry(record);
        var back = new DbcSpelleffectRecord { EffectSpellClassMask1 = 7 };
        SpellFactory.WriteEffect(entry, back);
        Assert.AreEqual(1, back.EffectIndex);
        Assert.AreEqual(99, back.EffectBasePoints);
        Assert.AreEqual(5_000_000_000, back.EffectMiscValue1);
        Assert.AreEqual(25, back.ImplicitTarget1);
        Assert.AreEqual(133, back.EffectTriggerSpell);
        Assert.AreEqual(7, back.EffectSpellClassMask1);
    }

    private static bool Throws(Action action)
    {
        try
        {
            action();
            return false;
        }
        catch (InvalidOperationException)
        {
            return true;
        }
    }
}
