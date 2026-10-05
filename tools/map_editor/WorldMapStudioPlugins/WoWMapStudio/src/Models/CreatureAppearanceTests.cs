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
}
