using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using Microsoft.Data.Sqlite;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>Spell saves, edits and deletes against a throwaway copy of the world database.</summary>
public static class SpellCommitTests
{
    [EditorTest(Category = "WoW Commit", Thread = TestThread.Background)]
    public static async Task Spell_create_edit_and_delete_round_trip_through_the_database()
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

        string copy = Path.Combine(Path.GetTempPath(), $"__wms_spell_commit_test_{Guid.NewGuid():N}.doltlite");
        File.Copy(source, copy);
        try
        {
            var context = new EditorContext(new Node3D(), new Project { Name = "__wms_spell_commit_test__" });
            var world = (WorldStorage)context.Database.Storages.Single(s => s.Name == WorldStorage.StorageName);
            context.Database.EditorStorage.Location.DatabasePath = copy;
            context.Database.EditorStorage.EnsureSchema();
            var factory = new SpellFactory(world);

            const int Id = 2_000_000_123;
            var spell = (Spell)factory.Create(context, Id.ToString());
            spell.Name = "Commit Test";
            spell.ManaCost = 25;
            spell.AddEffect(0).Apply();
            spell.SetEffectField(0, nameof(SpellEffectEntry.EffectBasePoints), 41).Apply();
            await world.CommitAsync([spell], []).ConfigureAwait(false);

            (int effectId, int misc) = await RowIdsAsync(world, Id).ConfigureAwait(false);
            Assert.IsTrue(effectId > 0, "the effect row got a fresh id");

            spell.Name = "Commit Test 2";
            spell.SetEffectField(0, nameof(SpellEffectEntry.EffectBasePoints), 7).Apply();
            spell.AddEffect(1).Apply();
            await world.CommitAsync([spell], []).ConfigureAwait(false);
            Assert.AreEqual(effectId, (await RowIdsAsync(world, Id).ConfigureAwait(false)).Effect0, "edit keeps the row in place");
            Assert.AreEqual(misc, (await RowIdsAsync(world, Id).ConfigureAwait(false)).Misc);

            context.Catalog.Remove(spell);
            var reopened = (Spell)(await factory.OpenAsync(context, Id.ToString()).ConfigureAwait(false))!;
            Assert.AreEqual("Commit Test 2", reopened.Name);
            Assert.AreEqual(2, reopened.Effects.Count);
            Assert.AreEqual(7, reopened.Effects[0].EffectBasePoints);

            await world.CommitAsync([], [reopened]).ConfigureAwait(false);
            await using WorldDbContext db = world.CreateContext();
            Assert.IsFalse(await db.DbcSpelleffect.AnyAsync(r => r.SpellID == Id).ConfigureAwait(false));
            Assert.IsFalse(await db.DbcSpellname.AnyAsync(r => r.ID == Id).ConfigureAwait(false));
            Assert.IsFalse(await db.DbcSpellmisc.AnyAsync(r => r.SpellID == Id).ConfigureAwait(false));
        }
        finally
        {
            SqliteConnection.ClearAllPools();
            File.Delete(copy);
        }
    }

    private static async Task<(int Effect0, int Misc)> RowIdsAsync(WorldStorage world, int spellId)
    {
        await using WorldDbContext db = world.CreateContext();
        int effect = await db.DbcSpelleffect.AsNoTracking().Where(r => r.SpellID == spellId && r.EffectIndex == 0)
            .Select(r => r.ID).FirstOrDefaultAsync().ConfigureAwait(false);
        int misc = await db.DbcSpellmisc.AsNoTracking().Where(r => r.SpellID == spellId).Select(r => r.ID).FirstOrDefaultAsync().ConfigureAwait(false);
        return (effect, misc);
    }
}
