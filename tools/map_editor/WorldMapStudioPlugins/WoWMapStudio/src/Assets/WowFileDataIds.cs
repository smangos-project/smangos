using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>
/// FileDataID to path mapping backed by the world database's <c>file_data_id</c> table. Paths are
/// compared lowercase with forward slashes, matching the client's loose content tree.
/// </summary>
public sealed class WowFileDataIds
{
    private readonly Dictionary<uint, string> _paths = new();
    private readonly Dictionary<string, uint> _ids = new(StringComparer.Ordinal);
    private readonly object _gate = new();
    private WorldStorage? _storage;
    private bool _loaded;

    private static WowFileDataIds? _shared;

    /// <summary>The process-wide map. It reads the table of the attached <see cref="WorldStorage"/> on first use;
    /// until one is attached it reads <c>.client/Content/fdid.index</c> and allocates in memory only.</summary>
    public static WowFileDataIds Shared => _shared ??= new WowFileDataIds();

    private static readonly Dictionary<string, WowFileDataIds> Detached = new();

    /// <summary>The map for <paramref name="editor"/>: <see cref="Shared"/> when its database is the one backing it, otherwise
    /// an in-memory map of its own, so scratch projects never write ids into the real table.</summary>
    public static WowFileDataIds For(EditorContext editor)
    {
        WowFileDataIds shared = Shared;
        string? path = editor.Database.EditorStorage.Location.DatabasePath;
        lock (shared._gate)
        {
            if (shared._storage == null || Equals(shared._storage.Location.DatabasePath, path))
            {
                return shared;
            }
        }

        lock (Detached)
        {
            return Detached.TryGetValue(path ?? "", out WowFileDataIds? ids) ? ids : Detached[path ?? ""] = Parse([]);
        }
    }

    public int Count
    {
        get
        {
            EnsureLoaded();
            return _paths.Count;
        }
    }

    /// <summary>Makes <paramref name="storage"/> the backing table (null: the index file); the next lookup reloads.</summary>
    public void Attach(WorldStorage? storage)
    {
        lock (_gate)
        {
            _storage = storage;
            _loaded = false;
        }
    }

    /// <summary>Attaches <paramref name="storage"/> unless one is already attached: the first world storage of the process
    /// (the open project's) stays the backing table, scratch projects do not take it over.</summary>
    public void AttachFirst(WorldStorage storage)
    {
        lock (_gate)
        {
            if (_storage == null)
            {
                Attach(storage);
            }
        }
    }

    private void EnsureLoaded()
    {
        lock (_gate)
        {
            if (_loaded)
            {
                return;
            }

            _paths.Clear();
            _ids.Clear();
            if (_storage != null && !HasTable(_storage))
            {
                // A database without the table (a scratch project) gets the index file and in-memory ids.
                _storage = null;
            }

            if (_storage != null)
            {
                using WorldDbContext db = _storage.CreateContext();
                foreach (var row in db.FileDataId.AsNoTracking().Where(r => r.Retired == 0).Select(r => new { r.Id, r.Path }))
                {
                    Add((uint)row.Id, row.Path);
                }
            }
            else if (FindIndexFile() is { } file)
            {
                foreach (string line in File.ReadLines(file))
                {
                    AddLine(line);
                }
            }

            _loaded = true;
        }
    }

    private static bool HasTable(WorldStorage storage)
    {
        using WorldDbContext db = storage.CreateContext();
        return db.Database.SqlQueryRaw<int>("SELECT count(*) AS Value FROM sqlite_master WHERE type = 'table' AND name = 'file_data_id'")
            .AsEnumerable().Single() > 0;
    }

    private static string? FindIndexFile()
    {
        for (string? dir = AppContext.BaseDirectory; dir != null; dir = Path.GetDirectoryName(dir))
        {
            string candidate = Path.Combine(dir, ".client", "Content", "fdid.index");
            if (File.Exists(candidate))
            {
                return candidate;
            }
        }

        return null;
    }

    public static string Normalize(string path) => path.Replace('\\', '/').ToLowerInvariant();

    public static WowFileDataIds Parse(IEnumerable<string> lines)
    {
        var result = new WowFileDataIds { _loaded = true };
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

    /// <summary>The existing id of <paramref name="path"/>, or a new one from the custom range. With a storage attached the
    /// id is allocated by one atomic statement on the table, so other tools never get the same id.</summary>
    public uint Allocate(string path)
    {
        EnsureLoaded();
        lock (_gate)
        {
            if (IdOf(path) is { } existing)
            {
                return existing;
            }

            uint next = _storage != null ? AllocateInTable(Normalize(path)) : NextLocal();
            Add(next, path);
            return next;
        }
    }

    private uint NextLocal()
    {
        uint next = CustomBase;
        foreach (uint id in _paths.Keys)
        {
            if (id >= next)
            {
                next = id + 1;
            }
        }
        return next;
    }

    private uint AllocateInTable(string path)
    {
        using WorldDbContext db = _storage!.CreateContext();
        db.Database.ExecuteSql(
            $"""
            INSERT OR IGNORE INTO file_data_id (id, path, source)
            SELECT max(coalesce(max(id), 0) + 1, {(int)CustomBase}), {path}, 'map_editor'
            FROM file_data_id WHERE id >= {(int)CustomBase}
            """);
        db.Database.ExecuteSql($"UPDATE file_data_id SET retired = 0 WHERE path = {path} AND retired = 1");
        return (uint)db.FileDataId.AsNoTracking().Where(r => r.Path == path).Select(r => r.Id).Single();
    }

    /// <summary>Rewrites <c>fdid.index</c> in <paramref name="contentDirectory"/> from the table (the format of
    /// <c>database_tool fdid export</c>); true when the file changed.</summary>
    public bool ExportIndex(string contentDirectory)
    {
        EnsureLoaded();
        if (_storage == null)
        {
            return false;
        }

        using WorldDbContext db = _storage.CreateContext();
        string text = string.Concat(db.FileDataId.AsNoTracking().Where(r => r.Retired == 0).OrderBy(r => r.Id)
            .Select(r => new { r.Id, r.Path }).AsEnumerable().Select(r => $"{r.Id};{r.Path}\n"));
        string target = Path.Combine(contentDirectory, "fdid.index");
        if (File.Exists(target) && File.ReadAllText(target) == text)
        {
            return false;
        }

        string temp = target + ".tmp";
        File.WriteAllText(temp, text);
        File.Move(temp, target, true);
        File.Delete(Path.Combine(contentDirectory, "custom.index"));
        return true;
    }

    /// <summary>Writes every known pair as an <c>id;path</c> listfile for wowlib.</summary>
    public void WriteListfile(string file)
    {
        EnsureLoaded();
        lock (_gate)
        {
            File.WriteAllLines(file, _paths.OrderBy(p => p.Key).Select(p => $"{p.Key};{p.Value}"));
        }
    }

    public string? PathOf(uint id)
    {
        EnsureLoaded();
        return _paths.GetValueOrDefault(id);
    }

    /// <summary>The listed path, or a stable stand-in for ids the listfile does not know.</summary>
    public string PathOrPlaceholder(uint id) => PathOf(id) ?? $"unknown/{id}.blp";

    public uint? IdOf(string path)
    {
        EnsureLoaded();
        return _ids.TryGetValue(Normalize(path), out uint id) ? id : null;
    }

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
