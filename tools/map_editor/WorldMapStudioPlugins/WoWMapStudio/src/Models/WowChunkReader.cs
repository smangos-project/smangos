using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>
/// Walks the chunked container format shared by WMO (and ADT/WDT) files: repeating
/// (4-byte id, 4-byte payload size, payload) records. Unlike M2's leading "MD20" magic, chunk ids
/// are stored spelled backwards on disk — reading the 4 id bytes as a little-endian uint32
/// reproduces the chunk name written forwards as a 4-character code (e.g. "MVER").
/// </summary>
internal static class WowChunkReader
{
    public static uint ChunkId(string name) =>
        (uint)name[3] | ((uint)name[2] << 8) | ((uint)name[1] << 16) | ((uint)name[0] << 24);

    /// <summary>
    /// Reads one chunk header at <paramref name="offset"/> and advances it to the start of the
    /// chunk's payload. Returns false (leaving offset unchanged) if the header or its declared
    /// payload would run past the end of <paramref name="bytes"/>.
    /// </summary>
    public static bool TryReadChunkHeader(byte[] bytes, ref int offset, out uint id, out uint size)
    {
        if (offset < 0 || (long)offset + 8 > bytes.Length)
        {
            id = 0;
            size = 0;
            return false;
        }

        id = WowBinaryReader.ReadU32(bytes, offset);
        size = WowBinaryReader.ReadU32(bytes, offset + 4);
        if ((long)offset + 8 + size > bytes.Length)
        {
            id = 0;
            size = 0;
            return false;
        }

        offset += 8;
        return true;
    }

    /// <summary>Walks the records between <paramref name="start"/> and <paramref name="end"/>, yielding
    /// each one's id and the bounds of its payload. Used for both the top level of an ADT and the
    /// sub-chunks inside one MCNK; stops cleanly at the first malformed or out-of-bounds header.</summary>
    public static IEnumerable<(uint Id, int PayloadStart, int Size)> Chunks(byte[] bytes, int start, int end)
    {
        int offset = start;
        while (offset + 8 <= end && TryReadChunkHeader(bytes, ref offset, out uint id, out uint size))
        {
            if ((long)offset + size > end)
            {
                yield break;
            }

            yield return (id, offset, (int)size);
            offset += (int)size;
        }
    }
}
