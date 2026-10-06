using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace WorldMapStudio.WoW;

public static class AdtObj1PatcherTests
{
    private static byte[] Chunk(string tag, byte[] data)
    {
        using var stream = new MemoryStream();
        using var writer = new BinaryWriter(stream);
        writer.Write((byte)tag[3]);
        writer.Write((byte)tag[2]);
        writer.Write((byte)tag[1]);
        writer.Write((byte)tag[0]);
        writer.Write(data.Length);
        writer.Write(data);
        return stream.ToArray();
    }

    private static byte[] Entries(int size, params uint[] uids)
    {
        var data = new byte[size * uids.Length];
        for (int i = 0; i < uids.Length; i++)
        {
            BitConverter.GetBytes(uids[i]).CopyTo(data, (i * size) + 4);
        }

        return data;
    }

    private static Dictionary<string, byte[]> Parse(byte[] file)
    {
        var result = new Dictionary<string, byte[]>();
        for (int p = 0; p < file.Length;)
        {
            string tag = new([(char)file[p + 3], (char)file[p + 2], (char)file[p + 1], (char)file[p]]);
            int size = BitConverter.ToInt32(file, p + 4);
            result[tag] = file.AsSpan(p + 8, size).ToArray();
            p += 8 + size;
        }

        return result;
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Removed_placements_leave_the_lod_lists_and_their_ranges_shrink()
    {
        uint[] lod = [0, 0, 0, 4, 0, 0, 0, 0, 1, 1, 0, 2];
        var lodBytes = new byte[48];
        Buffer.BlockCopy(lod, 0, lodBytes, 0, 48);
        byte[] file = Chunk("MLFD", lodBytes)
            .Concat(Chunk("MLDD", Entries(36, 10, 11, 12, 13)))
            .Concat(Chunk("MLDX", Entries(28, 0, 0, 0, 0)))
            .Concat(Chunk("MLDL", new byte[16]))
            .Concat(Chunk("MLMD", Entries(40, 20, 21, 22)))
            .Concat(Chunk("MLMX", Entries(28, 0, 0, 0)))
            .ToArray();

        byte[]? patched = AdtObj1Patcher.Patch(file, new HashSet<uint> { 10, 13 }, new HashSet<uint> { 21, 22 });
        Assert.IsNotNull(patched);
        var parts = Parse(patched!);
        Assert.AreEqual(2 * 36, parts["MLDD"].Length);
        Assert.AreEqual(2 * 28, parts["MLDX"].Length);
        Assert.AreEqual(2 * 4, parts["MLDL"].Length);
        Assert.AreEqual(2 * 40, parts["MLMD"].Length);
        Assert.AreEqual(2 * 28, parts["MLMX"].Length);
        Assert.AreEqual(10u, BitConverter.ToUInt32(parts["MLDD"], 4));
        Assert.AreEqual(21u, BitConverter.ToUInt32(parts["MLMD"], 4));

        var back = new uint[12];
        Buffer.BlockCopy(parts["MLFD"], 0, back, 0, 48);
        Assert.AreEqual(2u, back[3]);
        Assert.AreEqual(0u, back[8]);
        Assert.AreEqual(0u, back[9]);
        Assert.AreEqual(2u, back[11]);

        Assert.IsNull(AdtObj1Patcher.Patch(patched!, new HashSet<uint> { 10, 13 }, new HashSet<uint> { 21, 22 }), "nothing left to drop");
    }
}
