namespace WorldMapStudio.WoW;

public static class ModelAssetLoadTests
{
    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Asset_system_loads_an_m2_and_a_wmo()
    {
        EditorContext context = WowTestContent.RequireAssetContext();

        ModelAsset? boar = context.Assets.LoadModelAsset("creature/boar/boar.m2");
        Assert.IsNotNull(boar);
        Assert.Greater(boar!.Parts.Count, 0);

        ModelAsset? inn = context.Assets.LoadModelAsset("world/wmo/azeroth/buildings/goldshireinn/goldshireinn.wmo");
        Assert.IsNotNull(inn);
        Assert.Greater(inn!.Parts.Count, 0);
    }

    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Doodad_m2_resolves_and_loads_its_textures()
    {
        EditorContext context = WowTestContent.RequireAssetContext();

        ModelAsset? tree = context.Assets.LoadModelAsset("world/azeroth/elwynn/passivedoodads/trees/canopylesstree01.m2");
        Assert.IsNotNull(tree);
        foreach (ModelSurface surface in tree!.Surfaces)
        {
            foreach (MeshMaterial material in surface.Materials)
            {
                string texture = material.Values.GetTexture(M2Material.Texture);
                Assert.IsTrue(texture.Length > 0, "texture path resolved");
                Assert.IsNotNull(context.Assets.LoadTextureAsset(texture), $"'{texture}' loads");
            }
        }
    }
}
