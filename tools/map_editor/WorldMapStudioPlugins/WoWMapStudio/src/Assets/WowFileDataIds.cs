using System;
using System.Collections.Generic;
using System.IO;

namespace WorldMapStudio.WoW;

/// <summary>
/// FileDataID to path mapping from a community listfile (<c>id;path</c> per line). Paths are
/// compared lowercase with forward slashes, matching the client's loose content tree.
/// </summary>
public sealed class WowFileDataIds
{
    private readonly Dictionary<uint, string> _paths = new();
    private readonly Dictionary<string, uint> _ids = new(StringComparer.Ordinal);

    public int Count => _paths.Count;

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

    public string? PathOf(uint id) => _paths.GetValueOrDefault(id);

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
