using System;
using System.Collections.Concurrent;
using System.IO;

namespace WorldMapStudio.WoW;

/// <summary>
/// Parsed <c>.vmo</c> models shared by every tile, keyed by path and validated against the file's fingerprint
/// on each use, so a re-extracted model is read again while an unchanged one is read once however many tiles
/// place it. Thread safe; bounded by clearing outright when it grows past a fixed number of models.
/// </summary>
public static class WowServerModelCache
{
    private const int MaxModels = 3000;

    private sealed record Entry(WowServerFileStamp Stamp, WowWorldModel? Model);

    private static readonly ConcurrentDictionary<string, Entry> Models = new(StringComparer.Ordinal);

    public static string PathOf(string vmapsRoot, string modelName) => Path.Combine(vmapsRoot, modelName + ".vmo");

    /// <summary>The model, or null when its file does not exist. Throws <see cref="WowServerDataException"/> naming the model when it cannot be read.</summary>
    public static WowWorldModel? Get(string vmapsRoot, string modelName)
    {
        string path = PathOf(vmapsRoot, modelName);
        WowServerFileStamp stamp = WowServerFileStamp.Of(path);
        if (Models.TryGetValue(path, out Entry? cached) && cached.Stamp == stamp)
        {
            return cached.Model;
        }

        WowWorldModel? model = null;
        if (stamp.Size >= 0)
        {
            try
            {
                model = WowWorldModelReader.Read(File.ReadAllBytes(path));
            }
            catch (WowServerDataException e)
            {
                throw new WowServerDataException($"{modelName}: {e.Message}");
            }
        }

        if (Models.Count >= MaxModels)
        {
            Models.Clear();
        }

        Models[path] = new Entry(stamp, model);
        return model;
    }

    /// <summary>Forgets every model, for tests.</summary>
    public static void Clear() => Models.Clear();
}
