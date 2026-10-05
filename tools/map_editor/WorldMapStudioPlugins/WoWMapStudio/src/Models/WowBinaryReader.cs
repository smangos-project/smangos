using System;
using System.Buffers.Binary;
using System.Text;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Shared little-endian primitives for hand-parsed WoW binary formats (M2, WMO).</summary>
internal static class WowBinaryReader
{
    public static bool InBounds(byte[] bytes, uint offset, uint count, int stride) =>
        count == 0 || ((long)offset + (long)count * stride <= bytes.Length && offset <= bytes.Length);

    public static uint ReadU32(byte[] bytes, long offset) =>
        BinaryPrimitives.ReadUInt32LittleEndian(bytes.AsSpan((int)offset, 4));

    public static ushort ReadU16(byte[] bytes, long offset) =>
        BinaryPrimitives.ReadUInt16LittleEndian(bytes.AsSpan((int)offset, 2));

    public static float ReadSingle(byte[] bytes, long offset) =>
        BinaryPrimitives.ReadSingleLittleEndian(bytes.AsSpan((int)offset, 4));

    public static Vector3 ReadVector3(byte[] bytes, long offset) => new(
        BinaryPrimitives.ReadSingleLittleEndian(bytes.AsSpan((int)offset, 4)),
        BinaryPrimitives.ReadSingleLittleEndian(bytes.AsSpan((int)offset + 4, 4)),
        BinaryPrimitives.ReadSingleLittleEndian(bytes.AsSpan((int)offset + 8, 4)));

    public static Vector2 ReadVector2(byte[] bytes, long offset) => new(
        BinaryPrimitives.ReadSingleLittleEndian(bytes.AsSpan((int)offset, 4)),
        BinaryPrimitives.ReadSingleLittleEndian(bytes.AsSpan((int)offset + 4, 4)));

    public static ushort[] ReadUInt16Array(byte[] bytes, uint offset, uint count)
    {
        if (!InBounds(bytes, offset, count, 2))
        {
            return [];
        }

        var result = new ushort[count];
        for (int i = 0; i < count; i++)
        {
            result[i] = ReadU16(bytes, offset + (i * 2));
        }

        return result;
    }

    /// <summary>Reads a byte-offset string, trimming a single trailing NUL if present (some exporters omit it).</summary>
    public static string ReadFixedString(byte[] bytes, uint offset, uint length)
    {
        if (length == 0 || (long)offset + length > bytes.Length)
        {
            return "";
        }

        int actualLength = (int)length;
        if (actualLength > 0 && bytes[offset + actualLength - 1] == 0)
        {
            actualLength--;
        }

        return AssetPath.Normalize(Encoding.UTF8.GetString(bytes, (int)offset, actualLength));
    }

    /// <summary>Reads a C string embedded in a fixed-size buffer (NUL can appear anywhere within it, or not at all).</summary>
    public static string ReadNameString(byte[] bytes, uint offset, uint maxLength)
    {
        if ((long)offset + maxLength > bytes.Length)
        {
            return "";
        }

        int start = (int)offset;
        int limit = start + (int)maxLength;
        int end = start;
        while (end < limit && bytes[end] != 0)
        {
            end++;
        }

        return end > start ? Encoding.UTF8.GetString(bytes, start, end - start) : "";
    }
}
