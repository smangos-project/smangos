using System.IO;
using System.Linq;
using System.Threading.Tasks;

namespace WorldMapStudio.WoW;

public static class WowExportBackupTests
{
    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static async Task Restore_brings_back_overwritten_files_and_removes_created_ones()
    {
        string root = Path.Combine(Path.GetTempPath(), "wms_backup_test_" + Path.GetRandomFileName());
        string content = Path.Combine(root, "Content");
        string maps = Path.Combine(content, "world", "maps", "m");
        Directory.CreateDirectory(maps);
        File.WriteAllText(Path.Combine(maps, "m.wdt"), "stock");
        File.WriteAllText(Path.Combine(content, "fdid.index"), "index");
        try
        {
            async Task Export(string text)
            {
                await new WowExportBackup(content).ProtectAsync(["world/maps/m"], ["fdid.index"], () =>
                {
                    File.WriteAllText(Path.Combine(maps, "m.wdt"), text);
                    File.WriteAllText(Path.Combine(maps, "m_1_1.adt"), text);
                    File.WriteAllText(Path.Combine(content, "fdid.index"), text);
                    return Task.CompletedTask;
                });
            }

            await Export("one");
            await Export("two");
            Assert.AreEqual(3, new WowExportBackup(content).Entries.Count);

            Assert.AreEqual(3, new WowExportBackup(content).Restore());
            Assert.AreEqual("stock", File.ReadAllText(Path.Combine(maps, "m.wdt")));
            Assert.AreEqual("index", File.ReadAllText(Path.Combine(content, "fdid.index")));
            Assert.IsFalse(File.Exists(Path.Combine(maps, "m_1_1.adt")));
            Assert.AreEqual(0, new WowExportBackup(content).Entries.Count);
            Assert.IsFalse(Directory.EnumerateFileSystemEntries(root).Any(path => path.EndsWith(".wms-export-backup")));
        }
        finally
        {
            Directory.Delete(root, recursive: true);
        }
    }
}
