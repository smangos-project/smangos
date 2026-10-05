using System;
using System.IO;

namespace WorldMapStudio.WoW;

/// <summary>Locates the extracted client content for tests that read real client files.</summary>
public static class WowTestContent
{
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
