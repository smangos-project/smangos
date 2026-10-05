using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.Text;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>One placed model in a <c>.vmtile</c>, as <c>ModelSpawn::readFromFile</c> reads it.</summary>
public sealed record WowVmapSpawn(
    uint Flags,
    ushort AdtId,
    uint Id,
    Vector3 Position,
    Vector3 Rotation,
    float Scale,
    (Vector3 Low, Vector3 High)? Bound,
    string Name,
    uint TreeIndex)
{
    public const uint FlagM2 = 1;
    public const uint FlagWorldSpawn = 2;
    public const uint FlagHasBound = 4;

    /// <summary>A doodad model rather than a world model object.</summary>
    public bool IsM2 => (Flags & FlagM2) != 0;
}

/// <summary>Decodes a <c>vmaps/MMM_XX_YY.vmtile</c>: a spawn count, then each spawn with the name of its model and its index in the map's tree.</summary>
public static class WowVmapTileReader
{
    private const uint MaxNameLength = 500;

    public static IReadOnlyList<WowVmapSpawn> Read(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length < 8)
        {
            throw WowServerDataException.Truncated(".vmtile header");
        }

        CheckMagic(bytes, ".vmtile");
        int at = 8;

        uint count = U32(bytes, ref at, ".vmtile spawn count");
        var spawns = new List<WowVmapSpawn>((int)Math.Min(count, 65536));
        for (uint i = 0; i < count; i++)
        {
            uint flags = U32(bytes, ref at, ".vmtile spawn");
            ushort adtId = U16(bytes, ref at);
            uint id = U32(bytes, ref at, ".vmtile spawn");
            Vector3 position = Vec3(bytes, ref at);
            Vector3 rotation = Vec3(bytes, ref at);
            float scale = F32(bytes, ref at);
            (Vector3, Vector3)? bound = (flags & WowVmapSpawn.FlagHasBound) != 0 ? (Vec3(bytes, ref at), Vec3(bytes, ref at)) : null;
            uint nameLength = U32(bytes, ref at, ".vmtile spawn");
            if (nameLength > MaxNameLength)
            {
                throw WowServerDataException.Truncated(".vmtile spawn name");
            }

            Need(bytes, at, (int)nameLength);
            string name = Encoding.UTF8.GetString(bytes.Slice(at, (int)nameLength));
            at += (int)nameLength;
            uint treeIndex = U32(bytes, ref at, ".vmtile spawn");
            spawns.Add(new WowVmapSpawn(flags, adtId, id, position, rotation, scale, bound, name, treeIndex));
        }

        return spawns;
    }

    internal static void CheckMagic(ReadOnlySpan<byte> bytes, string what)
    {
        ReadOnlySpan<byte> expected = WowServerFormats.VmapMagic;
        if (!bytes[..expected.Length].SequenceEqual(expected))
        {
            string found = Encoding.ASCII.GetString(bytes[..expected.Length]);
            if (found.StartsWith("VMAP", StringComparison.Ordinal))
            {
                throw WowServerDataException.MagicMismatch(what, found, Encoding.ASCII.GetString(expected));
            }

            throw WowServerDataException.BadMagic(what);
        }
    }

    private static ushort U16(ReadOnlySpan<byte> bytes, ref int at)
    {
        Need(bytes, at, 2);
        ushort value = BinaryPrimitives.ReadUInt16LittleEndian(bytes[at..]);
        at += 2;
        return value;
    }

    private static uint U32(ReadOnlySpan<byte> bytes, ref int at, string what)
    {
        Need(bytes, at, 4, what);
        uint value = BinaryPrimitives.ReadUInt32LittleEndian(bytes[at..]);
        at += 4;
        return value;
    }

    private static float F32(ReadOnlySpan<byte> bytes, ref int at)
    {
        Need(bytes, at, 4);
        float value = BinaryPrimitives.ReadSingleLittleEndian(bytes[at..]);
        at += 4;
        return value;
    }

    private static Vector3 Vec3(ReadOnlySpan<byte> bytes, ref int at) => new(F32(bytes, ref at), F32(bytes, ref at), F32(bytes, ref at));

    private static void Need(ReadOnlySpan<byte> bytes, int at, int length, string what = ".vmtile spawn")
    {
        if (at + length > bytes.Length)
        {
            throw WowServerDataException.Truncated(what);
        }
    }
}
