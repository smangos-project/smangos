using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

public static class CreatureAppearanceTests
{
    private static WowFileDataIds Ids() => WowFileDataIds.Parse(["10;creature/boar/boar.m2", "11;creature/boar/boarskin.blp"]);

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Build_maps_variation_ids_to_creature_skin_slots()
    {
        CreatureAppearance? a = CreatureAppearanceResolver.Build(Ids(), 10, 0f, [11, 0, 99]);

        Assert.IsNotNull(a);
        Assert.AreEqual("creature/boar/boar.m2", a!.ModelPath);
        Assert.AreEqual(1f, a.Scale);
        Assert.AreEqual(1, a.SlotTextures.Count);
        Assert.AreEqual("creature/boar/boarskin.blp", a.SlotTextures[M2TextureSlot.CreatureSkin1]);
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Build_binds_a_baked_npc_skin_to_the_skin_slot()
    {
        CreatureAppearance? a = CreatureAppearanceResolver.Build(Ids(), 10, 1f, [], 11);

        Assert.AreEqual("creature/boar/boarskin.blp", a!.SlotTextures[M2TextureSlot.Skin]);
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Build_without_a_known_model_resolves_nothing()
    {
        Assert.IsNull(CreatureAppearanceResolver.Build(Ids(), 0, 1f, []));
        Assert.IsNull(CreatureAppearanceResolver.Build(Ids(), 77, 1f, []));
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Override_rewrites_only_the_matching_replaceable_slot()
    {
        CreatureAppearance a = CreatureAppearanceResolver.Build(Ids(), 10, 1f, [11])!;
        var options = a.ToInstantiateOptions();

        MeshMaterial skin = M2Material.Describe(textureSlot: M2TextureSlot.CreatureSkin1);
        MeshMaterial fixedTex = M2Material.Describe(texture: "a.blp");

        Assert.AreEqual("creature/boar/boarskin.blp", options.MaterialOverride!(null!, null!, skin).Values.GetTexture(M2Material.Texture));
        Assert.AreEqual("a.blp", options.MaterialOverride!(null!, null!, fixedTex).Values.GetTexture(M2Material.Texture));
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static async Task Resolve_reads_display_rows_from_the_world_database()
    {
        string? db = null;
        for (string? dir = AppContext.BaseDirectory; dir != null && db == null; dir = Path.GetDirectoryName(dir))
        {
            string candidate = Path.Combine(dir, ".world", "world.doltlite");
            db = File.Exists(candidate) ? candidate : null;
        }
        if (db == null)
        {
            Assert.Skip("world.doltlite not available");
        }

        var project = new Project { Name = "__wms_creature_resolve_test__" };
        project.StorageLocations["Editor"] = new StorageLocation { DatabasePath = db! };
        var context = new EditorContext(new Godot.Node3D(), project);
        WorldStorage storage = context.Database.Storages.OfType<WorldStorage>().Single();

        int[] displayIds;
        await using (WorldDbContext world = storage.CreateContext())
        {
            displayIds = await world.DbcCreaturedisplayinfo.AsNoTracking().Select(row => row.ID).Take(200).ToArrayAsync();
        }

        CreatureAppearance? found = null;
        foreach (int id in displayIds)
        {
            found = await CreatureAppearanceResolver.ResolveAsync(storage, id);
            if (found != null)
            {
                break;
            }
        }

        Assert.IsNotNull(found, "some display resolves to a model path");
        Assert.IsTrue(found!.ModelPath.EndsWith(".m2", StringComparison.OrdinalIgnoreCase));
    }
}
