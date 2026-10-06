using System;
using System.IO;
using WoWLib;
using WoWLib.Filesystem;

namespace WorldMapStudio.WoW;

/// <summary>The shared wowlib view of the client install, opened on first use from the repo's
/// <c>.client</c> (CASC) and <c>.local/listfile.txt</c>.</summary>
public static class WowClientFiles
{
    private static FileSystem? _shared;
    private static readonly object Gate = new();

    public static FileSystem Shared
    {
        get
        {
            lock (Gate)
            {
                return _shared ??= Open();
            }
        }
    }

    /// <summary>A separate filesystem whose project overlay is <paramref name="projectDir"/>, so writes never leak into <see cref="Shared"/>.</summary>
    public static FileSystem OpenIsolated(string projectDir) => Open(projectDir);

    private static FileSystem Open(string? projectDir = null)
    {
        for (string? dir = AppContext.BaseDirectory; dir != null; dir = Path.GetDirectoryName(dir))
        {
            string client = Path.Combine(dir, ".client");
            string listfile = Path.Combine(dir, ".local", "listfile.txt");
            if (Directory.Exists(Path.Combine(client, "Data")) && File.Exists(listfile))
            {
                string project = projectDir ?? Path.Combine(Path.GetTempPath(), "wms-wowlib-project");
                Directory.CreateDirectory(project);
                var settings = FileSystemSettings.Detect(client, Locale.enUS, project, listfile, new FileDataId(1_000_000_000u));
                return FileSystem.Open(settings);
            }
        }

        throw new InvalidOperationException("client install (.client/Data) or .local/listfile.txt not found");
    }
}
