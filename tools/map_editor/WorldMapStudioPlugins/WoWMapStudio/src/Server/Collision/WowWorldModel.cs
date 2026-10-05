using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>One group of a world model: its collision mesh in model space.</summary>
public sealed record WowWorldModelGroup(Vector3 BoundLow, Vector3 BoundHigh, uint MogpFlags, uint GroupWmoId, float[] Vertices, int[] Triangles);

/// <summary>
/// A decoded <c>.vmo</c> — the collision model of one M2 or WMO: its groups' vertices and triangles. A group
/// without collision geometry carries no mesh. The BIH acceleration trees and the WMO liquid block are skipped
/// over; they serve queries, not drawing.
/// </summary>
public sealed class WowWorldModel
{
    public required uint RootWmoId { get; init; }

    public required IReadOnlyList<WowWorldModelGroup> Groups { get; init; }

    public int VertexCount => Groups.Sum(group => group.Vertices.Length / 3);

    private (float[] Vertices, int[] Edges)? _wireframe;

    /// <summary>
    /// Every group's vertices concatenated, and the model's unique triangle edges as vertex index pairs into
    /// them. Computed once: a model is placed hundreds of times, and its edges are the same every time.
    /// </summary>
    public (float[] Vertices, int[] Edges) Wireframe => _wireframe ??= BuildWireframe();

    private (float[] Vertices, int[] Edges) BuildWireframe()
    {
        var vertices = new float[VertexCount * 3];
        var edges = new HashSet<long>();
        var pairs = new List<int>();
        int vertexBase = 0;
        int written = 0;

        foreach (WowWorldModelGroup group in Groups)
        {
            group.Vertices.CopyTo(vertices, written);
            written += group.Vertices.Length;

            for (int triangle = 0; triangle + 2 < group.Triangles.Length; triangle += 3)
            {
                for (int corner = 0; corner < 3; corner++)
                {
                    int from = vertexBase + group.Triangles[triangle + corner];
                    int to = vertexBase + group.Triangles[triangle + ((corner + 1) % 3)];
                    long key = ((long)Math.Min(from, to) << 32) | (uint)Math.Max(from, to);
                    if (edges.Add(key))
                    {
                        pairs.Add(from);
                        pairs.Add(to);
                    }
                }
            }

            vertexBase += group.Vertices.Length / 3;
        }

        return (vertices, pairs.ToArray());
    }
}

/// <summary>Decodes <c>.vmo</c> bytes into a <see cref="WowWorldModel"/>.</summary>
public static class WowWorldModelReader
{
    public static WowWorldModel Read(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length < 8)
        {
            throw WowServerDataException.Truncated(".vmo header");
        }

        WowVmapTileReader.CheckMagic(bytes, ".vmo");
        int at = 8;

        Chunk(bytes, ref at, "WMOD");
        at += 4;
        uint rootId = U32(bytes, ref at);
        Chunk(bytes, ref at, "GMOD");
        uint groupCount = U32(bytes, ref at);

        var groups = new List<WowWorldModelGroup>((int)Math.Min(groupCount, 1024));
        for (uint i = 0; i < groupCount; i++)
        {
            Vector3 low = Vec3(bytes, ref at);
            Vector3 high = Vec3(bytes, ref at);
            uint mogpFlags = U32(bytes, ref at);
            uint groupId = U32(bytes, ref at);

            Chunk(bytes, ref at, "VERT");
            at += 4;
            uint vertexCount = U32(bytes, ref at);
            if (vertexCount == 0)
            {
                // A group with no collision geometry ends here, before its triangle and tree chunks.
                groups.Add(new WowWorldModelGroup(low, high, mogpFlags, groupId, [], []));
                continue;
            }

            float[] vertices = Floats(bytes, ref at, checked((int)vertexCount * 3));
            Chunk(bytes, ref at, "TRIM");
            at += 4;
            uint triangleCount = U32(bytes, ref at);
            int[] triangles = Ints(bytes, ref at, checked((int)triangleCount * 3));
            foreach (int index in triangles)
            {
                if ((uint)index >= vertexCount)
                {
                    throw WowServerDataException.Truncated(".vmo triangle index");
                }
            }

            // The mesh's BIH: bounds, then a tree and an object list, each length-prefixed.
            Chunk(bytes, ref at, "MBIH");
            at += 24;
            SkipCounted(bytes, ref at);
            SkipCounted(bytes, ref at);

            Chunk(bytes, ref at, "LIQU");
            uint liquidSize = U32(bytes, ref at);
            Need(bytes, at, liquidSize);
            at += (int)liquidSize;

            groups.Add(new WowWorldModelGroup(low, high, mogpFlags, groupId, vertices, triangles));
        }

        return new WowWorldModel { RootWmoId = rootId, Groups = groups };
    }

    private static void Chunk(ReadOnlySpan<byte> bytes, ref int at, string tag)
    {
        Need(bytes, at, 4);
        if (System.Text.Encoding.ASCII.GetString(bytes.Slice(at, 4)) != tag)
        {
            throw WowServerDataException.BadMagic($".vmo {tag} chunk");
        }

        at += 4;
    }

    private static void SkipCounted(ReadOnlySpan<byte> bytes, ref int at)
    {
        uint count = U32(bytes, ref at);
        Need(bytes, at, 4L * count);
        at += (int)(4 * count);
    }

    private static uint U32(ReadOnlySpan<byte> bytes, ref int at)
    {
        Need(bytes, at, 4);
        uint value = BinaryPrimitives.ReadUInt32LittleEndian(bytes[at..]);
        at += 4;
        return value;
    }

    private static Vector3 Vec3(ReadOnlySpan<byte> bytes, ref int at)
    {
        Need(bytes, at, 12);
        var value = new Vector3(
            BinaryPrimitives.ReadSingleLittleEndian(bytes[at..]),
            BinaryPrimitives.ReadSingleLittleEndian(bytes[(at + 4)..]),
            BinaryPrimitives.ReadSingleLittleEndian(bytes[(at + 8)..]));
        at += 12;
        return value;
    }

    private static float[] Floats(ReadOnlySpan<byte> bytes, ref int at, int count)
    {
        Need(bytes, at, 4L * count);
        var result = new float[count];
        for (int i = 0; i < count; i++)
        {
            result[i] = BinaryPrimitives.ReadSingleLittleEndian(bytes[(at + (i * 4))..]);
        }

        at += count * 4;
        return result;
    }

    private static int[] Ints(ReadOnlySpan<byte> bytes, ref int at, int count)
    {
        Need(bytes, at, 4L * count);
        var result = new int[count];
        for (int i = 0; i < count; i++)
        {
            result[i] = BinaryPrimitives.ReadInt32LittleEndian(bytes[(at + (i * 4))..]);
        }

        at += count * 4;
        return result;
    }

    private static void Need(ReadOnlySpan<byte> bytes, int at, long length)
    {
        if (at < 0 || length < 0 || at + length > bytes.Length)
        {
            throw WowServerDataException.Truncated(".vmo");
        }
    }
}
