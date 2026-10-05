using System;
using System.IO;

namespace WorldMapStudio.WoW;

/// <summary>Locates the extracted client content for tests that read real client files.</summary>
public static class WowTestContent
{
    /// <summary>A context whose only asset source is the extracted client content, or skips when absent.</summary>
    public static EditorContext RequireAssetContext()
    {
        for (string? dir = AppContext.BaseDirectory; dir != null; dir = Path.GetDirectoryName(dir))
        {
            string content = Path.Combine(dir, ".client", "Content");
            if (Directory.Exists(content))
            {
                var project = new Project { Name = "__wms_wow_assets_test__" };
                project.AssetSources.Add(new AssetSourceSettings { Id = "wow", Name = "WoW Client", RootPath = content });
                return new EditorContext(new Godot.Node3D(), project);
            }
        }

        Assert.Skip("client content not available");
        return null!;
    }

    /// <summary>Absolute path of a client file, or skips the test when the content is not extracted.</summary>
    public static string RequireFile(string relative)
    {
        for (string? dir = AppContext.BaseDirectory; dir != null; dir = Path.GetDirectoryName(dir))
        {
            string content = Path.Combine(dir, ".client", "Content");
            if (Directory.Exists(content))
            {
                string file = WowClientPaths.ContentFile(content, relative);
                if (File.Exists(file))
                {
                    return file;
                }
                break;
            }
        }

        Assert.Skip($"client file {relative} not available");
        return "";
    }
}
