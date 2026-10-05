using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace WorldMapStudio.WoW;

/// <summary>
/// A source file's <c>(size, mtime)</c> as it was when a preview tile was built from it — the same
/// fingerprint vocabulary the extractor's manifests use. A missing file is size -1, so a tile built
/// while its data did not exist goes stale the moment the extractor writes it.
/// </summary>
public readonly record struct WowServerFileStamp(string Path, long Size, long MtimeTicks)
{
    public static WowServerFileStamp Of(string path)
    {
        var info = new FileInfo(path);
        return info.Exists ? new WowServerFileStamp(path, info.Length, info.LastWriteTimeUtc.Ticks) : new WowServerFileStamp(path, -1, 0);
    }

    public static WowServerFileStamp[] Capture(IEnumerable<string> paths) => paths.Select(Of).ToArray();

    /// <summary>Whether every file still looks exactly as it did when captured.</summary>
    public static bool AllCurrent(IReadOnlyList<WowServerFileStamp> stamps) => stamps.All(stamp => Of(stamp.Path) == stamp);
}
