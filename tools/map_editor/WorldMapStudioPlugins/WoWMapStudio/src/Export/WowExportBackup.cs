using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;

namespace WorldMapStudio.WoW;

/// <summary>
/// Keeps the client's original copy of every file an export overwrites or creates, so the exports can be
/// undone. Backups live in <c>.wms-export-backup</c> next to the content root; a file is saved only the
/// first time it is touched, so the backup always holds the pre-export state.
/// </summary>
public sealed class WowExportBackup
{
    private const string ManifestName = "manifest.json";
    private const string Absent = "";

    /// <summary>Client-relative folder procedural models are exported into.</summary>
    public const string GeneratedDirectory = "world/generated";

    private readonly string _content;
    private readonly string _root;
    private readonly Dictionary<string, string> _manifest;

    public WowExportBackup(string content)
    {
        _content = Path.GetFullPath(content);
        _root = Path.Combine(Path.GetDirectoryName(_content.TrimEnd(Path.DirectorySeparatorChar))!, ".wms-export-backup");
        _manifest = File.Exists(ManifestPath)
            ? JsonSerializer.Deserialize<Dictionary<string, string>>(File.ReadAllText(ManifestPath)) ?? []
            : [];
    }

    private string ManifestPath => Path.Combine(_root, ManifestName);

    /// <summary>Client-relative (lowercase, '/') paths with a saved original; empty value means the file did not exist.</summary>
    public IReadOnlyDictionary<string, string> Entries => _manifest;

    /// <summary>
    /// Saves the originals under <paramref name="directories"/> and <paramref name="files"/> (client-relative),
    /// runs <paramref name="export"/>, then marks every file it created as absent in the original.
    /// </summary>
    public async System.Threading.Tasks.Task ProtectAsync(
        IEnumerable<string> directories, IEnumerable<string> files, Func<System.Threading.Tasks.Task> export)
    {
        string[] dirs = directories.Select(Normalize).ToArray();
        string[] single = files.Select(Normalize).ToArray();
        foreach (string file in Existing(dirs, single))
        {
            Save(file);
        }

        var before = new HashSet<string>(Existing(dirs, single));
        try
        {
            await export().ConfigureAwait(false);
        }
        finally
        {
            foreach (string file in Existing(dirs, single).Where(file => !before.Contains(file) && !_manifest.ContainsKey(file)))
            {
                _manifest[file] = Absent;
            }

            foreach (string file in single.Where(file => !before.Contains(file) && File.Exists(Absolute(file)) && !_manifest.ContainsKey(file)))
            {
                _manifest[file] = Absent;
            }

            WriteManifest();
        }
    }

    /// <summary>Puts every saved original back, deletes files the exports created, and clears the backup.</summary>
    public int Restore()
    {
        foreach ((string file, string saved) in _manifest)
        {
            string target = Absolute(file);
            if (saved.Length == 0)
            {
                File.Delete(target);
            }
            else
            {
                Directory.CreateDirectory(Path.GetDirectoryName(target)!);
                File.Copy(Path.Combine(_root, saved), target, overwrite: true);
            }
        }

        int count = _manifest.Count;
        _manifest.Clear();
        if (Directory.Exists(_root))
        {
            Directory.Delete(_root, recursive: true);
        }

        return count;
    }

    /// <summary>True when a WoW client process is running (it may hold a character in the world).</summary>
    public static bool ClientRunning()
    {
        if (!Directory.Exists("/proc"))
        {
            return System.Diagnostics.Process.GetProcessesByName("WowClassic").Length > 0;
        }

        foreach (string dir in Directory.EnumerateDirectories("/proc"))
        {
            try
            {
                if (File.ReadAllText(Path.Combine(dir, "cmdline")).Contains("WowClassic.exe", StringComparison.OrdinalIgnoreCase))
                {
                    return true;
                }
            }
            catch (IOException)
            {
            }
            catch (UnauthorizedAccessException)
            {
            }
        }

        return false;
    }

    private void Save(string file)
    {
        if (_manifest.ContainsKey(file))
        {
            return;
        }

        string saved = "files/" + file;
        string destination = Path.Combine(_root, saved);
        Directory.CreateDirectory(Path.GetDirectoryName(destination)!);
        File.Copy(Absolute(file), destination, overwrite: true);
        _manifest[file] = saved;
    }

    private IEnumerable<string> Existing(string[] directories, string[] files)
    {
        foreach (string directory in directories)
        {
            string absolute = Absolute(directory);
            if (!Directory.Exists(absolute))
            {
                continue;
            }

            foreach (string path in Directory.EnumerateFiles(absolute, "*", SearchOption.AllDirectories))
            {
                yield return Path.GetRelativePath(_content, path).Replace(Path.DirectorySeparatorChar, '/');
            }
        }

        foreach (string file in files.Where(file => File.Exists(Absolute(file))))
        {
            yield return file;
        }
    }

    private void WriteManifest()
    {
        Directory.CreateDirectory(_root);
        File.WriteAllText(ManifestPath, JsonSerializer.Serialize(_manifest));
    }

    private string Absolute(string relative) => WowClientPaths.ContentFile(_content, relative);

    private static string Normalize(string relative) => relative.ToLowerInvariant().Replace('\\', '/').Trim('/');
}
