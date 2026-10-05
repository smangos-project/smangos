namespace WorldMapStudio.WoW;

public static class WmoReaderTests
{
    [EditorTest(Category = "WoW Models", Thread = TestThread.Background)]
    public static void Reads_a_client_wmo_root_and_group()
    {
        string file = WowTestContent.RequireFile("world/wmo/azeroth/buildings/goldshireinn/goldshireinn.wmo");
        WmoRoot? root = WmoRootReader.Read(System.IO.File.ReadAllBytes(file), file);

        Assert.IsNotNull(root);
        Assert.Greater(root!.GroupCount, 0);
        Assert.Greater(root.Materials.Length, 0);

        string groupFile = System.IO.Path.ChangeExtension(file, null) + "_000.wmo";
        WmoGroup? group = WmoGroupReader.Read(System.IO.File.ReadAllBytes(groupFile), root.Lighting);
        Assert.IsNotNull(group);
        Assert.Greater(group!.Vertices.Length, 0);
        Assert.Greater(group.Batches.Length, 0);
        foreach (ushort index in group.Indices)
        {
            Assert.IsTrue(index < group.Vertices.Length);
        }
    }
}
