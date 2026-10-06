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

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Allocate_reuses_known_paths_and_counts_up()
    {
        var ids = WowFileDataIds.Parse(new[] { "10;World/A.adt" });
        Assert.AreEqual(10u, ids.Allocate("world/a.adt"));
        uint first = ids.Allocate("world/new1.adt");
        Assert.AreEqual(WowFileDataIds.CustomBase, first);
        Assert.AreEqual(first + 1, ids.Allocate("world/new2.adt"));
        Assert.AreEqual(first, ids.Allocate("World\\New1.adt"));
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Custom_allocations_survive_save_and_load()
    {
        string file = System.IO.Path.GetTempFileName();
        try
        {
            var a = WowFileDataIds.Parse(new[] { "10;World/A.adt" });
            uint id = a.Allocate("world/new1.adt");
            a.SaveCustom(file);

            var b = WowFileDataIds.Parse(new[] { "10;World/A.adt" });
            b.LoadCustom(file);
            Assert.AreEqual(id, b.IdOf("world/new1.adt"));
            Assert.AreEqual(id + 1, b.Allocate("world/new2.adt"));
        }
        finally
        {
            System.IO.File.Delete(file);
        }
    }
}
