using System.IO;

namespace WorldMapStudio.WoW;

public static class WowClientPathsTests
{
    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void ContentFile_lowercases_every_segment_of_the_relative_path_but_not_the_root()
    {
        string root = Path.Combine(Path.GetTempPath(), "Content");
        char separator = Path.DirectorySeparatorChar;

        Assert.AreEqual(
            Path.Combine(root, "world", "maps", "azeroth", "azeroth_32_49.adt"),
            WowClientPaths.ContentFile(root, "World\\Maps\\Azeroth\\Azeroth_32_49.adt"));
        Assert.AreEqual(
            $"{root}{separator}interface{separator}worldmap{separator}dun morogh.zmp",
            WowClientPaths.ContentFile(root, "Interface/WorldMap/Dun Morogh.zmp"));
    }
}
