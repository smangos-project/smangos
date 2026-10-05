namespace WorldMapStudio.WoW;

public static class WowFileDataIdsTests
{
    [EditorTest(Category = "WoW Assets", Thread = TestThread.Background)]
    public static void Resolves_both_directions_ignoring_case_and_separators()
    {
        var ids = WowFileDataIds.Parse(["53183;sound/music/Darnassus Intro.mp3", "# junk", "bad;x", "7;"]);

        Assert.AreEqual(1, ids.Count);
        Assert.AreEqual("sound/music/darnassus intro.mp3", ids.PathOf(53183));
        Assert.AreEqual(53183u, ids.IdOf("Sound\\Music\\DARNASSUS INTRO.mp3"));
        Assert.IsNull(ids.PathOf(1));
        Assert.IsNull(ids.IdOf("nope"));
    }

    [EditorTest(Category = "WoW Assets", Thread = TestThread.Background)]
    public static void Listfile_resolves_a_known_client_file()
    {
        string file = WowTestContent.RequireFile("interface/icons/inv_misc_questionmark.blp");
        string listfile = System.IO.Path.GetFullPath(System.IO.Path.Combine(file, "..", "..", "..", "..", "..", ".local", "listfile.txt"));
        if (!System.IO.File.Exists(listfile))
        {
            Assert.Skip("no listfile");
        }

        var ids = WowFileDataIds.Load(listfile);
        uint id = ids.IdOf("interface/icons/inv_misc_questionmark.blp")!.Value;
        Assert.AreEqual("interface/icons/inv_misc_questionmark.blp", ids.PathOf(id));
    }
}
