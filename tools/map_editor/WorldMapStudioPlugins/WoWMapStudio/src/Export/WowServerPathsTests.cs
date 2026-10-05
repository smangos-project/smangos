using System.IO;
using Godot;

namespace WorldMapStudio.WoW;

public static class WowServerPathsTests
{
    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Subfolders_hang_off_the_configured_data_root()
    {
        string root = Path.Combine(Path.GetTempPath(), "data");
        var project = new Project { Name = "__wms_wow_server_paths_test__" };
        project.Paths[WowServerPaths.DataKey] = root;
        var context = new EditorContext(new Node3D(), project);
        WowServerPaths paths = context.WowServerPaths;

        Assert.AreEqual(root, paths.Data);
        Assert.AreEqual(Path.Combine(root, "maps"), paths.Maps);
        Assert.AreEqual(Path.Combine(root, "vmaps"), paths.Vmaps);
        Assert.AreEqual(Path.Combine(root, "mmaps"), paths.Mmaps);
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Unset_data_root_leaves_every_path_null()
    {
        var context = new EditorContext(new Node3D(), new Project { Name = "__wms_wow_server_paths_unset_test__" });
        WowServerPaths paths = context.WowServerPaths;

        Assert.IsNull(paths.Data);
        Assert.IsNull(paths.Maps);
        Assert.IsNull(paths.Vmaps);
        Assert.IsNull(paths.Mmaps);
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Extractor_path_setting_overrides_the_default()
    {
        string exe = Path.GetTempFileName();
        var project = new Project { Name = "__wms_wow_server_paths_exe_test__" };
        project.Paths[WowServerPaths.ExtractorExeKey] = exe;
        var context = new EditorContext(new Node3D(), project);

        Assert.AreEqual(exe, context.WowServerPaths.ExtractorExe);

        project.Paths[WowServerPaths.ExtractorExeKey] = exe + ".missing";
        Assert.IsNull(context.WowServerPaths.ExtractorExe, "a configured path that does not exist is not replaced by the default");
        File.Delete(exe);
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Default_extractor_is_the_checkouts_build_output()
    {
        string? plugin = PluginDirectories.Find("WoWMapStudio");
        string expected = Path.GetFullPath(Path.Combine(plugin!, "..", "..", "..", "..", "build", "tools", "map_extractor", "map_extractor"));
        var context = new EditorContext(new Node3D(), new Project { Name = "__wms_wow_server_paths_default_test__" });

        Assert.AreEqual(File.Exists(expected) ? expected : null, context.WowServerPaths.ExtractorExe);
    }
}
