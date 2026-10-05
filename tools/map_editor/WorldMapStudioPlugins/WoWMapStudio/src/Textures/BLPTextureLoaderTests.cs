using System.IO;

namespace WorldMapStudio.WoW;

public static class BLPTextureLoaderTests
{
    [EditorTest(Category = "WoW Textures", Thread = TestThread.Background)]
    public static void Decode_reads_a_client_blp_to_rgba()
    {
        string file = WowTestContent.RequireFile("interface/icons/inv_misc_questionmark.blp");

        var image = BLPTextureLoader.Decode(File.ReadAllBytes(file));
        Assert.Greater(image.GetWidth(), 0);
        Assert.Greater(image.GetHeight(), 0);
        Assert.AreEqual(image.GetWidth() * image.GetHeight() * 4, image.GetData().Length);
    }
}
