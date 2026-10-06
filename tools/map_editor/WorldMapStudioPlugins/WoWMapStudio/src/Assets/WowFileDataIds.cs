using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;

namespace WorldMapStudio.WoW;

/// <summary>
/// FileDataID to path mapping from a community listfile (<c>id;path</c> per line). Paths are
/// compared lowercase with forward slashes, matching the client's loose content tree.
/// </summary>
public sealed class WowFileDataIds
{
    private readonly Dictionary<uint, string> _paths = new();
    private readonly Dictionary<string, uint> _ids = new(StringComparer.Ordinal);

    private static WowFileDataIds? _shared;

    /// <summary>The project listfile (<c>.local/listfile.txt</c> found above the editor binary), or an
    /// empty map when none exists.</summary>
    public static WowFileDataIds Shared => _shared ??= LoadShared();

    public int Count => _paths.Count;

    private static WowFileDataIds LoadShared()
    {
        for (string? dir = AppContext.BaseDirectory; dir != null; dir = Path.GetDirectoryName(dir))
        {
            string candidate = Path.Combine(dir, ".local", "listfile.txt");
            if (File.Exists(candidate))
            {
                WowFileDataIds ids = Load(candidate);
                string content = Path.Combine(dir, ".client", "Content");
                if (Directory.Exists(content))
                {
                    ids.AttachRegistry(content);
                }
                return ids;
            }
        }

        return new WowFileDataIds();
    }

    public static string Normalize(string path) => path.Replace('\\', '/').ToLowerInvariant();

    public static WowFileDataIds Load(string listfilePath)
    {
        var result = new WowFileDataIds();
        foreach (string line in File.ReadLines(listfilePath))
        {
            result.AddLine(line);
        }
        return result;
    }

    public static WowFileDataIds Parse(IEnumerable<string> lines)
    {
        var result = new WowFileDataIds();
        foreach (string line in lines)
        {
            result.AddLine(line);
        }
        return result;
    }

    /// <summary>Registers an id/path pair; an id keeps its first path, a path its first id.</summary>
    public void Add(uint id, string path)
    {
        string normalized = Normalize(path);
        _paths.TryAdd(id, normalized);
        _ids.TryAdd(normalized, id);
    }

    /// <summary>First id handed out for project-created files, well above anything the stock client uses.</summary>
    public const uint CustomBase = 2_000_000_000;

    /// <summary>File name of the shared custom id registry (<c>id;path</c> lines) in the Content directory; the client bypass reads it as a name table.</summary>
    public const string RegistryFile = "custom.index";

    private string? _registryPath;

    /// <summary>The existing id of <paramref name="path"/>, or a new one from the custom range. With a registry attached
    /// the id is allocated under the registry lock from the file's current contents, so other tools never get the same id.</summary>
    public uint Allocate(string path)
    {
        if (_registryPath != null)
        {
            using var _ = CustomIndexLock.Acquire(_registryPath);
            LoadRegistry();
        }

        if (IdOf(path) is { } existing)
        {
            return existing;
        }

        uint next = CustomBase;
        foreach (uint id in _paths.Keys)
        {
            if (id >= next)
            {
                next = id + 1;
            }
        }

        Add(next, path);
        if (_registryPath != null)
        {
            AppendRegistry(next, Normalize(path));
        }
        return next;
    }

    /// <summary>Uses <c>custom.index</c> in <paramref name="contentDirectory"/> as the allocation file. The older allocation
    /// files (<c>.local/listfile.custom.txt</c>, <c>wms.index</c>, <c>wms-custom-fdids.txt</c>) are merged into it once and deleted.</summary>
    public void AttachRegistry(string contentDirectory)
    {
        string registry = Path.Combine(contentDirectory, RegistryFile);
        if (_registryPath == registry)
        {
            return;
        }

        _registryPath = registry;
        string[] legacy =
        [
            Path.Combine(contentDirectory, "..", "..", ".local", "listfile.custom.txt"),
            Path.Combine(contentDirectory, "wms.index"),
            Path.Combine(contentDirectory, "wms-custom-fdids.txt"),
        ];
        if (legacy.Any(File.Exists))
        {
            using var _ = CustomIndexLock.Acquire(registry);
            MergeLegacy(registry, legacy);
        }
        LoadRegistry();
    }

    private void LoadRegistry()
    {
        if (File.Exists(_registryPath!))
        {
            foreach (string line in File.ReadLines(_registryPath!))
            {
                AddLine(line);
            }
        }
    }

    private static void MergeLegacy(string registry, string[] legacy)
    {
        var entries = new SortedDictionary<uint, string>();
        var byPath = new Dictionary<string, uint>(StringComparer.Ordinal);
        void Read(string file, bool strict)
        {
            foreach (string line in File.ReadLines(file))
            {
                int sep = line.IndexOf(';');
                if (sep <= 0 || sep == line.Length - 1 || !uint.TryParse(line.AsSpan(0, sep), out uint id))
                {
                    continue;
                }

                string path = Normalize(line[(sep + 1)..]);
                if (strict && ((entries.TryGetValue(id, out string? have) && have != path) || (byPath.TryGetValue(path, out uint other) && other != id)))
                {
                    Godot.GD.PushWarning($"fdid registry conflict in {Path.GetFileName(file)}: {id};{path} (kept the registry entry)");
                    continue;
                }

                entries[id] = path;
                byPath[path] = id;
            }
        }

        if (File.Exists(registry))
        {
            Read(registry, false);
        }
        foreach (string file in legacy.Where(File.Exists))
        {
            Read(file, true);
        }

        File.WriteAllLines(registry, entries.Select(e => $"{e.Key};{e.Value}"));
        foreach (string file in legacy.Where(File.Exists))
        {
            File.Delete(file);
        }
    }

    private void AppendRegistry(uint id, string path)
    {
        var entries = new SortedDictionary<uint, string>();
        if (File.Exists(_registryPath!))
        {
            foreach (string line in File.ReadLines(_registryPath!))
            {
                int sep = line.IndexOf(';');
                if (sep > 0 && uint.TryParse(line.AsSpan(0, sep), out uint existing))
                {
                    entries[existing] = line[(sep + 1)..];
                }
            }
        }

        entries[id] = path;
        string temp = _registryPath + ".tmp";
        File.WriteAllLines(temp, entries.Select(e => $"{e.Key};{e.Value}"));
        File.Move(temp, _registryPath!, true);
    }

    public string? PathOf(uint id) => _paths.GetValueOrDefault(id);

    /// <summary>The listed path, or a stable stand-in for ids the listfile does not know.</summary>
    public string PathOrPlaceholder(uint id) => PathOf(id) ?? $"unknown/{id}.blp";

    public uint? IdOf(string path) => _ids.TryGetValue(Normalize(path), out uint id) ? id : null;

    private void AddLine(string line)
    {
        int separator = line.IndexOf(';');
        if (separator <= 0 || separator == line.Length - 1 || !uint.TryParse(line.AsSpan(0, separator), out uint id))
        {
            return;
        }
        Add(id, line[(separator + 1)..]);
    }
}

/// <summary>Exclusive <c>&lt;registry&gt;.lock</c> file shared with the Blender addon; locks older than a minute are taken over.</summary>
internal sealed class CustomIndexLock : IDisposable
{
    private readonly string _lockPath;

    private CustomIndexLock(string lockPath) => _lockPath = lockPath;

    public static CustomIndexLock Acquire(string registryPath, TimeSpan? timeout = null)
    {
        string lockPath = registryPath + ".lock";
        Directory.CreateDirectory(Path.GetDirectoryName(Path.GetFullPath(registryPath))!);
        DateTime end = DateTime.UtcNow + (timeout ?? TimeSpan.FromSeconds(30));
        while (true)
        {
            try
            {
                using (new FileStream(lockPath, FileMode.CreateNew, FileAccess.Write, FileShare.None))
                {
                }
                return new CustomIndexLock(lockPath);
            }
            catch (IOException) when (File.Exists(lockPath))
            {
                if (DateTime.UtcNow - File.GetLastWriteTimeUtc(lockPath) > TimeSpan.FromSeconds(60))
                {
                    File.Delete(lockPath);
                    continue;
                }
                if (DateTime.UtcNow > end)
                {
                    throw new TimeoutException($"{lockPath} is held by another process");
                }
                Thread.Sleep(20);
            }
        }
    }

    public void Dispose() => File.Delete(_lockPath);
}
