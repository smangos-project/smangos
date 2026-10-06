using System.Linq;
using Microsoft.EntityFrameworkCore;

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

    [EditorTest(Category = "WoW Export")]
    public static void Table_allocations_are_shared_between_instances_and_make_no_dolt_commit()
    {
        string dir = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "wms-fdid-" + System.Guid.NewGuid().ToString("N"));
        System.IO.Directory.CreateDirectory(dir);
        try
        {
            var project = new Project { Name = "__wms_fdid_table_test__" };
            project.StorageLocations["Editor"] = new StorageLocation { DatabasePath = System.IO.Path.Combine(dir, "world.doltlite") };
            var context = new EditorContext(new Godot.Node3D(), project);
            WorldStorage storage = context.Database.Storages.OfType<WorldStorage>().Single();
            using (WorldDbContext db = storage.CreateContext())
            {
                db.Database.ExecuteSqlRaw("CREATE TABLE IF NOT EXISTS file_data_id (id INTEGER PRIMARY KEY, path TEXT NOT NULL UNIQUE, source TEXT NOT NULL, retired INTEGER NOT NULL DEFAULT 0)");
                db.Database.ExecuteSqlRaw("INSERT INTO file_data_id (id, path, source) VALUES (10, 'world/a.adt', 'listfile')");
            }

            long Commits()
            {
                using WorldDbContext db = storage.CreateContext();
                return db.Database.SqlQuery<long>($"SELECT count(*) AS Value FROM dolt_log").AsEnumerable().Single();
            }

            long before = Commits();
            var a = new WowFileDataIds();
            var b = new WowFileDataIds();
            a.Attach(storage);
            b.Attach(storage);
            Assert.AreEqual(10u, a.Allocate("World\\A.adt"));
            uint first = a.Allocate("world/new1.adt");
            Assert.AreEqual(2_000_000_000u, first);
            Assert.AreEqual(first + 1, b.Allocate("world/new2.adt"));
            Assert.AreEqual(first, b.Allocate("World\\New1.adt"));
            Assert.AreEqual(first + 2, a.Allocate("world/new3.adt"));

            Assert.IsTrue(a.ExportIndex(dir));
            Assert.IsFalse(a.ExportIndex(dir));
            Assert.AreEqual(4, System.IO.File.ReadAllLines(System.IO.Path.Combine(dir, "fdid.index")).Length);

            Assert.AreEqual(before, Commits());
        }
        finally
        {
            System.IO.Directory.Delete(dir, true);
        }
    }
}
