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
}
