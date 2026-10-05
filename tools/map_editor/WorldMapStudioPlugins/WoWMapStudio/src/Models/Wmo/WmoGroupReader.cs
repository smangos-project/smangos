using Godot;
using static WorldMapStudio.WoW.WowBinaryReader;
using static WorldMapStudio.WoW.WowChunkReader;

namespace WorldMapStudio.WoW;

internal readonly record struct WmoBatch(uint IndexStart, ushort IndexCount, ushort VertexStart, ushort VertexEnd, byte MaterialIndex);

internal sealed class WmoGroup
{
    public required Vector3[] Vertices { get; init; }

    public required Vector3[] Normals { get; init; }

    public required Vector2[] Uvs { get; init; }

    /// <summary>Null when the group has no MOCV chunk (no baked vertex lighting).</summary>
    public required Color[]? VertexColors { get; init; }

    public required ushort[] Indices { get; init; }

    public required WmoBatch[] Batches { get; init; }
}

/// <summary>
/// Reads a group <c>_NNN.wmo</c> file: the fixed 68-byte MOGP header followed by its nested
/// sub-chunks (MOVI/MOVT/MONR/MOTV/MOBA and, if present, MOCV). Sub-chunks are walked generically
/// by id rather than assuming a fixed conditional order, since several are flag-gated and this
/// pass only needs a handful of them.
/// </summary>
internal static class WmoGroupReader
{
    private const int MogpHeaderSize = 68;
    private const int MobaStride = 24;
    private const int MogpTransBatchCountOffset = 40;
    private const uint HasVertexColorFlag = 0x4;
    private const uint ExteriorFlag = 0x8;
    private const uint ExteriorLitFlag = 0x40;

    public static WmoGroup? Read(byte[] bytes, WmoRootLighting? lighting = null)
    {
        int offset = 0;
        if (!TryReadChunkHeader(bytes, ref offset, out uint id, out uint size) || id != ChunkId("MVER"))
        {
            return null;
        }

        offset += (int)size;
        if (!TryReadChunkHeader(bytes, ref offset, out id, out size) || id != ChunkId("MOGP") || size < MogpHeaderSize)
        {
            return null;
        }

        int mogpPayloadEnd = offset + (int)size;
        uint flagBits = ReadU32(bytes, offset + 8);
        bool hasVertexColor = (flagBits & HasVertexColorFlag) != 0;
        bool exterior = (flagBits & ExteriorFlag) != 0;
        bool exteriorLit = (flagBits & ExteriorLitFlag) != 0;
        ushort transBatchCount = ReadU16(bytes, offset + MogpTransBatchCountOffset);
        offset += MogpHeaderSize;

        Vector3[] vertices = [];
        Vector3[] normals = [];
        Vector2[] uvs = [];
        Color[]? vertexColors = null;
        ushort[] indices = [];
        WmoBatch[] batches = [];

        while (offset < mogpPayloadEnd && TryReadChunkHeader(bytes, ref offset, out id, out size))
        {
            if (id == ChunkId("MOVI"))
            {
                indices = ReadUInt16Array(bytes, (uint)offset, size / 2);
            }
            else if (id == ChunkId("MOVT"))
            {
                vertices = ReadPositions(bytes, offset, size / 12);
            }
            else if (id == ChunkId("MONR"))
            {
                normals = ReadPositions(bytes, offset, size / 12);
            }
            else if (id == ChunkId("MOTV"))
            {
                uvs = ReadUvs(bytes, offset, size / 8);
            }
            else if (id == ChunkId("MOBA"))
            {
                batches = ReadBatches(bytes, offset, size / MobaStride);
            }
            else if (hasVertexColor && vertexColors == null && id == ChunkId("MOCV"))
            {
                // First MOCV only: a Cata two-MOCV (CVERTS2) group's second chunk is a separate
                // material-blend layer, out of scope here.
                vertexColors = ReadVertexColors(bytes, offset, size / 4);
            }

            offset += (int)size;
        }

        if (vertexColors != null && lighting is { } rootLighting)
        {
            WmoVertexColorFix.Apply(vertexColors, batches, transBatchCount, exterior, exteriorLit, rootLighting);
        }

        return new WmoGroup
        {
            Vertices = vertices,
            Normals = normals,
            Uvs = uvs,
            VertexColors = vertexColors,
            Indices = indices,
            Batches = batches,
        };
    }

    private static Vector3[] ReadPositions(byte[] bytes, int offset, uint count)
    {
        var result = new Vector3[count];
        for (int i = 0; i < count; i++)
        {
            result[i] = WowCoordinates.ToGodot(ReadVector3(bytes, offset + (i * 12)));
        }

        return result;
    }

    private static Vector2[] ReadUvs(byte[] bytes, int offset, uint count)
    {
        var result = new Vector2[count];
        for (int i = 0; i < count; i++)
        {
            result[i] = ReadVector2(bytes, offset + (i * 8));
        }

        return result;
    }

    private static WmoBatch[] ReadBatches(byte[] bytes, int offset, uint count)
    {
        var result = new WmoBatch[count];
        for (int i = 0; i < count; i++)
        {
            int at = offset + (i * MobaStride);
            uint indexStart = ReadU32(bytes, at + 12);
            ushort indexCount = ReadU16(bytes, at + 16);
            ushort vertexStart = ReadU16(bytes, at + 18);
            ushort vertexEnd = ReadU16(bytes, at + 20);
            byte materialIndex = bytes[at + 23];
            result[i] = new WmoBatch(indexStart, indexCount, vertexStart, vertexEnd, materialIndex);
        }

        return result;
    }

    private static Color[] ReadVertexColors(byte[] bytes, int offset, uint count)
    {
        // On-disk byte order is B, G, R, A (CImVector), verified against a working renderer's decode.
        var result = new Color[count];
        for (int i = 0; i < count; i++)
        {
            int at = offset + (i * 4);
            result[i] = new Color(bytes[at + 2] / 255f, bytes[at + 1] / 255f, bytes[at] / 255f, bytes[at + 3] / 255f);
        }

        return result;
    }
}
