using System;
using System.Collections.Generic;
using System.IO;

namespace WorldMapStudio.WoW;

/// <summary>
/// Drops placements that an export removed from a tile's <c>_obj1</c> LOD lists (<c>MLDD</c>/<c>MLDX</c>/<c>MLDL</c>
/// for doodads, <c>MLMD</c>/<c>MLMX</c> for WMOs, ranges in <c>MLFD</c>), so removed objects do not linger in the
/// distance. Everything else keeps its bytes.
/// </summary>
public static class AdtObj1Patcher
{
    private const int DoodadSize = 36;
    private const int DoodadBoundsSize = 28;
    private const int DoodadLodSize = 4;
    private const int ObjectSize = 40;
    private const int ObjectBoundsSize = 28;
    private const int LodLevels = 3;

    private const float TileOrigin = 17066.666667f;
    private const int MddfSize = 36;
    private const int ModfSize = 64;
    private const byte StockDoodadFlags = 0x20;
    private const byte StockObjectFlags = 0x04;

    /// <summary>
    /// Appends an LOD entry for every <c>MDDF</c>/<c>MODF</c> placement of <paramref name="obj0"/> the file lacks (the
    /// client draws doodads from <c>MLDD</c>, not from the <c>_obj0</c> list). Null when nothing was missing.
    /// </summary>
    public static byte[]? AddMissing(byte[] file, byte[] obj0)
    {
        var chunks = Parse(file);
        var source = Parse(obj0);
        int Find(string tag) => chunks.FindIndex(c => c.Tag == tag);
        int fd = Find("MLFD"), dd = Find("MLDD"), dx = Find("MLDX"), dl = Find("MLDL"), md = Find("MLMD"), mx = Find("MLMX");
        int mddf = source.FindIndex(c => c.Tag == "MDDF"), modf = source.FindIndex(c => c.Tag == "MODF");
        if (fd < 0 || chunks[fd].Data.Length != 48 || dd < 0 || dx < 0 || dl < 0 || md < 0 || mx < 0)
        {
            return null;
        }

        uint[] lod = new uint[12];
        Buffer.BlockCopy(chunks[fd].Data, 0, lod, 0, 48);
        bool changed = false;
        int added = 0;

        var known = Uids(chunks[dd].Data, MddfSize);
        var doodads = new List<byte>();
        var bounds = new List<byte>();
        var lods = new List<byte>();
        byte[] lodValue = chunks[dl].Data.Length >= DoodadLodSize ? chunks[dl].Data[..DoodadLodSize] : new byte[DoodadLodSize];
        for (int p = 0; mddf >= 0 && p + MddfSize <= source[mddf].Data.Length; p += MddfSize)
        {
            byte[] entry = source[mddf].Data[p..(p + MddfSize)];
            if (!known.Add(BitConverter.ToUInt32(entry, 4)))
            {
                continue;
            }

            float x = BitConverter.ToSingle(entry, 8), y = BitConverter.ToSingle(entry, 12), z = BitConverter.ToSingle(entry, 16);
            entry[34] |= StockDoodadFlags;
            doodads.AddRange(entry);
            bounds.AddRange(Bounds(TileOrigin - z - 3, TileOrigin - x - 3, y - 3, TileOrigin - z + 3, TileOrigin - x + 3, y + 3));
            lods.AddRange(lodValue);
            added++;
            changed = true;
        }

        if (added > 0)
        {
            // The client ignores doodad entries appended after the last one, so they go in front of level 0.
            int at = (int)lod[0];
            chunks[dd] = ("MLDD", Splice(chunks[dd].Data, at * DoodadSize, doodads));
            chunks[dx] = ("MLDX", Splice(chunks[dx].Data, at * DoodadBoundsSize, bounds));
            chunks[dl] = ("MLDL", Splice(chunks[dl].Data, at * DoodadLodSize, lods));
            for (int level = 1; level < LodLevels; level++)
            {
                if (lod[LodLevels + level] > 0 && lod[level] >= at)
                {
                    lod[level] += (uint)added;
                }
            }

            lod[LodLevels] += (uint)added;
        }

        known = Uids(chunks[md].Data, ObjectSize);
        var objects = new List<byte>(chunks[md].Data);
        var objectBounds = new List<byte>(chunks[mx].Data);
        for (int p = 0; modf >= 0 && p + ModfSize <= source[modf].Data.Length; p += ModfSize)
        {
            byte[] entry = source[modf].Data[p..(p + ModfSize)];
            if (!known.Add(BitConverter.ToUInt32(entry, 4)))
            {
                continue;
            }

            float ax = BitConverter.ToSingle(entry, 32), ay = BitConverter.ToSingle(entry, 36), az = BitConverter.ToSingle(entry, 40);
            float bx = BitConverter.ToSingle(entry, 44), by = BitConverter.ToSingle(entry, 48), bz = BitConverter.ToSingle(entry, 52);
            objects.AddRange(entry[..32]);
            byte[] tail = entry[56..64];
            tail[0] |= StockObjectFlags;
            objects.AddRange(tail);
            objectBounds.AddRange(Bounds(
                TileOrigin - MathF.Max(az, bz), TileOrigin - MathF.Max(ax, bx), MathF.Min(ay, by),
                TileOrigin - MathF.Min(az, bz), TileOrigin - MathF.Min(ax, bx), MathF.Max(ay, by)));
            lod[6 + LodLevels + LastLevel(lod, 6)]++;
            changed = true;
        }

        if (!changed)
        {
            return null;
        }

        var lodBytes = new byte[48];
        Buffer.BlockCopy(lod, 0, lodBytes, 0, 48);
        chunks[fd] = ("MLFD", lodBytes);
        chunks[md] = ("MLMD", objects.ToArray());
        chunks[mx] = ("MLMX", objectBounds.ToArray());
        return Serialize(chunks);
    }

    // Appended WMO entries extend the last occupied level so the ranges of the earlier ones stay valid.
    private static int LastLevel(uint[] lod, int first)
    {
        for (int level = LodLevels - 1; level > 0; level--)
        {
            if (lod[first + LodLevels + level] > 0)
            {
                return level;
            }
        }

        return 0;
    }

    private static byte[] Splice(byte[] data, int at, List<byte> inserted)
    {
        var result = new List<byte>(data);
        result.InsertRange(Math.Min(at, result.Count), inserted);
        return result.ToArray();
    }

    private static byte[] Bounds(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
    {
        float radius = MathF.Sqrt(((maxX - minX) * (maxX - minX)) + ((maxY - minY) * (maxY - minY)) + ((maxZ - minZ) * (maxZ - minZ))) * 0.5f;
        var bytes = new byte[DoodadBoundsSize];
        float[] values = [minX, minY, minZ, maxX, maxY, maxZ, radius];
        Buffer.BlockCopy(values, 0, bytes, 0, bytes.Length);
        return bytes;
    }

    private static HashSet<uint> Uids(byte[] data, int size)
    {
        var uids = new HashSet<uint>();
        for (int p = 0; p + size <= data.Length; p += size)
        {
            uids.Add(BitConverter.ToUInt32(data, p + 4));
        }

        return uids;
    }

    private static List<(string Tag, byte[] Data)> Parse(byte[] file)
    {
        var chunks = new List<(string Tag, byte[] Data)>();
        for (int p = 0; p + 8 <= file.Length;)
        {
            string tag = new([(char)file[p + 3], (char)file[p + 2], (char)file[p + 1], (char)file[p]]);
            int size = BitConverter.ToInt32(file, p + 4);
            chunks.Add((tag, file.AsSpan(p + 8, size).ToArray()));
            p += 8 + size;
        }

        return chunks;
    }

    private static byte[] Serialize(List<(string Tag, byte[] Data)> chunks)
    {
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
