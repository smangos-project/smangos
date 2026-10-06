using System;
using System.Collections.Generic;
using System.IO;

namespace WorldMapStudio.WoW;

/// <summary>
/// Drops placements that an export removed from a tile's <c>_obj1</c> LOD lists (<c>MLDD</c>/<c>MLDX</c>/<c>MLDL</c>
/// for doodads, <c>MLMD</c>/<c>MLMX</c> for WMOs, ranges in <c>MLFD</c>), so removed objects do not linger in the
/// distance. Added placements get no LOD entry. Everything else keeps its bytes.
/// </summary>
public static class AdtObj1Patcher
{
    private const int DoodadSize = 36;
    private const int DoodadBoundsSize = 28;
    private const int DoodadLodSize = 4;
    private const int ObjectSize = 40;
    private const int ObjectBoundsSize = 28;
    private const int LodLevels = 3;

    /// <summary>The patched file, or null when nothing in it referred to a removed placement.</summary>
    public static byte[]? Patch(byte[] file, ISet<uint> doodadUids, ISet<uint> objectUids)
    {
        var chunks = new List<(string Tag, byte[] Data)>();
        for (int p = 0; p + 8 <= file.Length;)
        {
            string tag = new([(char)file[p + 3], (char)file[p + 2], (char)file[p + 1], (char)file[p]]);
            int size = BitConverter.ToInt32(file, p + 4);
            chunks.Add((tag, file.AsSpan(p + 8, size).ToArray()));
            p += 8 + size;
        }

        int Find(string tag) => chunks.FindIndex(c => c.Tag == tag);
        int fd = Find("MLFD");
        if (fd < 0 || chunks[fd].Data.Length != 4 * 12)
        {
            return null;
        }

        uint[] lod = new uint[12];
        Buffer.BlockCopy(chunks[fd].Data, 0, lod, 0, 48);
        bool changed = false;
        changed |= Filter(chunks, Find("MLDD"), DoodadSize, doodadUids, [(Find("MLDX"), DoodadBoundsSize), (Find("MLDL"), DoodadLodSize)], lod, 0);
        changed |= Filter(chunks, Find("MLMD"), ObjectSize, objectUids, [(Find("MLMX"), ObjectBoundsSize)], lod, 6);
        if (!changed)
        {
            return null;
        }

        var data = new byte[48];
        Buffer.BlockCopy(lod, 0, data, 0, 48);
        chunks[fd] = ("MLFD", data);

        using var stream = new MemoryStream();
        using var writer = new BinaryWriter(stream);
        foreach ((string tag, byte[] bytes) in chunks)
        {
            writer.Write((byte)tag[3]);
            writer.Write((byte)tag[2]);
            writer.Write((byte)tag[1]);
            writer.Write((byte)tag[0]);
            writer.Write(bytes.Length);
            writer.Write(bytes);
        }

        return stream.ToArray();
    }

    // lod[first..first+2] are the level offsets, lod[first+3..first+5] the level lengths, in entries.
    private static bool Filter(
        List<(string Tag, byte[] Data)> chunks,
        int main,
        int size,
        ISet<uint> keep,
        (int Index, int Size)[] parallel,
        uint[] lod,
        int first)
    {
        if (main < 0)
        {
            return false;
        }

        byte[] data = chunks[main].Data;
        int count = data.Length / size;
        var kept = new bool[count];
        int removed = 0;
        for (int i = 0; i < count; i++)
        {
            kept[i] = keep.Contains(BitConverter.ToUInt32(data, (i * size) + 4));
            removed += kept[i] ? 0 : 1;
        }

        if (removed == 0)
        {
            return false;
        }

        // Ranges first, from the old indices.
        var before = new int[count + 1];
        for (int i = 0; i < count; i++)
        {
            before[i + 1] = before[i] + (kept[i] ? 1 : 0);
        }

        for (int level = 0; level < LodLevels; level++)
        {
            int start = Math.Min((int)lod[first + level], count);
            int end = Math.Min(start + (int)lod[first + LodLevels + level], count);
            lod[first + level] = (uint)before[start];
            lod[first + LodLevels + level] = (uint)(before[end] - before[start]);
        }

        chunks[main] = (chunks[main].Tag, Compact(data, size, kept));
        foreach ((int index, int entrySize) in parallel)
        {
            if (index >= 0 && chunks[index].Data.Length == count * entrySize)
            {
                chunks[index] = (chunks[index].Tag, Compact(chunks[index].Data, entrySize, kept));
            }
        }

        return true;
    }

    private static byte[] Compact(byte[] data, int size, bool[] kept)
    {
        var result = new List<byte>(data.Length);
        for (int i = 0; i < kept.Length; i++)
        {
            if (kept[i])
            {
                result.AddRange(new ArraySegment<byte>(data, i * size, size));
            }
        }

        return result.ToArray();
    }
}
