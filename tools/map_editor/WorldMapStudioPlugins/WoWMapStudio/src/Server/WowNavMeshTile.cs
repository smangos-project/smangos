using System;
using System.Buffers.Binary;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Detour polygon areas as the generator assigns them (<c>NavMeshAreas</c> in <c>GridMapDefines.h</c>).</summary>
public static class WowNavArea
{
    public const byte None = 0;
    public const byte Ground = 1;
    public const byte GroundModel = 2;
    public const byte SteepSlope = 3;
    public const byte SteepSlopeModel = 4;
    public const byte WaterTransition = 5;
    public const byte Water = 6;
    public const byte Magma = 7;
    public const byte Slime = 8;
}

/// <summary>An off-mesh link between two positions, in Recast space.</summary>
public readonly record struct WowNavOffMeshConnection(Vector3 Start, Vector3 End, float Radius, ushort Poly, byte Flags, byte Side, uint UserId);

/// <summary>
/// One decoded <c>mmaps/MMMYYXX.mmtile</c>: an <c>MmapTileHeader</c> followed by a serialised Detour
/// tile. Only what is needed to draw the mesh is decoded: the header, vertices, polygons, the detail
/// mesh and the off-mesh connections. The link table and BV tree are skipped over — they exist to speed
/// up queries, and reimplementing Detour here is not the point.
///
/// All positions are in Recast space, <c>(serverY, serverZ, serverX)</c>; see <see cref="WowNavCoordinates"/>.
/// </summary>
public sealed class WowNavMeshTile
{
    public const int VertsPerPoly = 6;

    public bool UsesLiquids { get; init; }

    /// <summary>The tile's grid coordinates relative to the navmesh origin — not the file's tile indices.</summary>
    public int X { get; init; }

    public int Y { get; init; }

    public int Layer { get; init; }

    public Vector3 BoundsMin { get; init; }

    public Vector3 BoundsMax { get; init; }

    public float WalkableHeight { get; init; }

    public float WalkableRadius { get; init; }

    public float WalkableClimb { get; init; }

    /// <summary>Tile vertices, three floats each.</summary>
    public required float[] Vertices { get; init; }

    /// <summary>Six vertex indices per polygon, only the first <see cref="PolyVertexCount"/> of which are used.</summary>
    public required ushort[] PolyVertices { get; init; }

    public required byte[] PolyVertexCount { get; init; }

    public required byte[] PolyArea { get; init; }

    /// <summary>0 for a ground polygon, 1 for an off-mesh connection.</summary>
    public required byte[] PolyType { get; init; }

    public required ushort[] PolyFlags { get; init; }

    public required uint[] DetailVertexBase { get; init; }

    public required uint[] DetailTriangleBase { get; init; }

    public required byte[] DetailVertexCount { get; init; }

    public required byte[] DetailTriangleCount { get; init; }

    /// <summary>Extra detail vertices, three floats each.</summary>
    public required float[] DetailVertices { get; init; }

    /// <summary>Four bytes per triangle: three vertex indices and an edge-flag byte.</summary>
    public required byte[] DetailTriangles { get; init; }

    public required WowNavOffMeshConnection[] OffMeshConnections { get; init; }

    public int PolygonCount => PolyArea.Length;

    public int DetailTriangleTotal => DetailTriangles.Length / 4;

    /// <summary>The distinct <see cref="WowNavArea"/> values present, so a legend lists only what is really there.</summary>
    public byte[] AreasPresent()
    {
        var seen = new bool[64];
        for (int poly = 0; poly < PolygonCount; poly++)
        {
            if (PolyType[poly] == 0)
            {
                seen[PolyArea[poly] & 0x3f] = true;
            }
        }

        var result = new System.Collections.Generic.List<byte>();
        for (int area = 0; area < seen.Length; area++)
        {
            if (seen[area])
            {
                result.Add((byte)area);
            }
        }

        return result.ToArray();
    }

    /// <summary>
    /// The walkable surface as an indexed triangle list, resolved through the detail mesh the way Detour
    /// draws it. Indices address <c>Positions</c>, which is the tile vertices followed by the detail
    /// vertices; <c>Areas</c> holds each triangle's polygon area.
    /// </summary>
    public (float[] Positions, int[] Indices, byte[] Areas) BuildSurface()
    {
        int tileVertexCount = Vertices.Length / 3;
        var positions = new float[Vertices.Length + DetailVertices.Length];
        Vertices.CopyTo(positions, 0);
        DetailVertices.CopyTo(positions, Vertices.Length);

        var indices = new int[DetailTriangleTotal * 3];
        var areas = new byte[DetailTriangleTotal];
        int written = 0;

        for (int poly = 0; poly < DetailVertexBase.Length; poly++)
        {
            int polyVertexCount = PolyVertexCount[poly];
            for (int tri = 0; tri < DetailTriangleCount[poly]; tri++)
            {
                int source = (int)(DetailTriangleBase[poly] + tri) * 4;
                for (int corner = 0; corner < 3; corner++)
                {
                    int local = DetailTriangles[source + corner];
                    indices[(written * 3) + corner] = local < polyVertexCount
                        ? PolyVertices[(poly * VertsPerPoly) + local]
                        : tileVertexCount + (int)DetailVertexBase[poly] + local - polyVertexCount;
                }

                areas[written++] = (byte)(PolyArea[poly] & 0x3f);
            }
        }

        Array.Resize(ref indices, written * 3);
        Array.Resize(ref areas, written);
        return (positions, indices, areas);
    }
}

/// <summary>The navmesh's shared parameters from <c>mmaps/MMM.mmap</c> (<c>dtNavMeshParams</c>).</summary>
public readonly record struct WowNavMeshParams(Vector3 Origin, float TileWidth, float TileHeight, int MaxTiles, int MaxPolys)
{
    public const int Size = 28;

    public static WowNavMeshParams Read(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length < Size)
        {
            throw WowServerDataException.Truncated(".mmap");
        }

        return new WowNavMeshParams(
            new Vector3(F32(bytes, 0), F32(bytes, 4), F32(bytes, 8)),
            F32(bytes, 12),
            F32(bytes, 16),
            BinaryPrimitives.ReadInt32LittleEndian(bytes[20..]),
            BinaryPrimitives.ReadInt32LittleEndian(bytes[24..]));
    }

    private static float F32(ReadOnlySpan<byte> bytes, int offset) => BinaryPrimitives.ReadSingleLittleEndian(bytes[offset..]);
}

/// <summary>Decodes <c>.mmtile</c> bytes into a <see cref="WowNavMeshTile"/>.</summary>
public static class WowNavMeshTileReader
{
    private const int TileHeaderSize = 20;
    private const int MeshHeaderSize = 100;
    private const int PolySize = 32;
    private const int LinkSize = 16;
    private const int DetailMeshSize = 12;
    private const int BvNodeSize = 16;
    private const int OffMeshSize = 36;

    /// <summary>Throws <see cref="WowServerDataException"/> for a wrong version or magic, or a size that disagrees with the header.</summary>
    public static WowNavMeshTile Read(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length < TileHeaderSize + MeshHeaderSize)
        {
            throw WowServerDataException.Truncated(".mmtile header");
        }

        if (U32(bytes, 0) != WowServerFormats.MmapMagic)
        {
            throw WowServerDataException.BadMagic(".mmtile header");
        }

        uint dtVersion = U32(bytes, 4);
        if (dtVersion != WowServerFormats.DetourVersion)
        {
            throw WowServerDataException.VersionMismatch(".mmtile Detour", dtVersion, (uint)WowServerFormats.DetourVersion);
        }

        uint mmapVersion = U32(bytes, 8);
        if (mmapVersion != WowServerFormats.MmapVersion)
        {
            throw WowServerDataException.VersionMismatch(".mmtile", mmapVersion, WowServerFormats.MmapVersion);
        }

        uint declaredSize = U32(bytes, 12);
        bool usesLiquids = bytes[16] != 0;
        ReadOnlySpan<byte> data = bytes[TileHeaderSize..];
        if (declaredSize != data.Length)
        {
            throw WowServerDataException.Truncated(".mmtile data");
        }

        if (I32(data, 0) != WowServerFormats.DetourMagic)
        {
            throw WowServerDataException.BadMagic("Detour tile header");
        }

        int meshVersion = I32(data, 4);
        if (meshVersion != WowServerFormats.DetourVersion)
        {
            throw WowServerDataException.VersionMismatch("Detour tile", (uint)meshVersion, (uint)WowServerFormats.DetourVersion);
        }

        int polyCount = I32(data, 24);
        int vertCount = I32(data, 28);
        int maxLinkCount = I32(data, 32);
        int detailMeshCount = I32(data, 36);
        int detailVertCount = I32(data, 40);
        int detailTriCount = I32(data, 44);
        int bvNodeCount = I32(data, 48);
        int offMeshCount = I32(data, 52);

        long cursor = MeshHeaderSize;
        long vertsAt = cursor;
        cursor += Align4(12L * vertCount);
        long polysAt = cursor;
        cursor += Align4((long)PolySize * polyCount);
        cursor += Align4((long)LinkSize * maxLinkCount);
        long detailMeshesAt = cursor;
        cursor += Align4((long)DetailMeshSize * detailMeshCount);
        long detailVertsAt = cursor;
        cursor += Align4(12L * detailVertCount);
        long detailTrisAt = cursor;
        cursor += Align4(4L * detailTriCount);
        cursor += Align4((long)BvNodeSize * bvNodeCount);
        long offMeshAt = cursor;
        cursor += Align4((long)OffMeshSize * offMeshCount);

        // The declared size is the only integrity check the format has, and it has to match to the byte:
        // an off-by-one section would still parse into plausible-looking geometry.
        if (cursor != data.Length || polyCount < 0 || vertCount < 0 || detailMeshCount > polyCount)
        {
            throw WowServerDataException.Truncated(".mmtile sections");
        }

        var polyVertices = new ushort[polyCount * WowNavMeshTile.VertsPerPoly];
        var polyVertexCount = new byte[polyCount];
        var polyArea = new byte[polyCount];
        var polyType = new byte[polyCount];
        var polyFlags = new ushort[polyCount];
        for (int poly = 0; poly < polyCount; poly++)
        {
            ReadOnlySpan<byte> raw = data.Slice((int)polysAt + (poly * PolySize), PolySize);
            for (int i = 0; i < WowNavMeshTile.VertsPerPoly; i++)
            {
                polyVertices[(poly * WowNavMeshTile.VertsPerPoly) + i] = BinaryPrimitives.ReadUInt16LittleEndian(raw[(4 + (i * 2))..]);
            }

            polyFlags[poly] = BinaryPrimitives.ReadUInt16LittleEndian(raw[28..]);
            polyVertexCount[poly] = raw[30];
            polyArea[poly] = (byte)(raw[31] & 0x3f);
            polyType[poly] = (byte)(raw[31] >> 6);
        }

        var detailVertexBase = new uint[detailMeshCount];
        var detailTriangleBase = new uint[detailMeshCount];
        var detailVertexCountPerPoly = new byte[detailMeshCount];
        var detailTriangleCountPerPoly = new byte[detailMeshCount];
        for (int mesh = 0; mesh < detailMeshCount; mesh++)
        {
            ReadOnlySpan<byte> raw = data.Slice((int)detailMeshesAt + (mesh * DetailMeshSize), DetailMeshSize);
            detailVertexBase[mesh] = BinaryPrimitives.ReadUInt32LittleEndian(raw);
            detailTriangleBase[mesh] = BinaryPrimitives.ReadUInt32LittleEndian(raw[4..]);
            detailVertexCountPerPoly[mesh] = raw[8];
            detailTriangleCountPerPoly[mesh] = raw[9];
        }

        var offMesh = new WowNavOffMeshConnection[offMeshCount];
        for (int i = 0; i < offMeshCount; i++)
        {
            ReadOnlySpan<byte> raw = data.Slice((int)offMeshAt + (i * OffMeshSize), OffMeshSize);
            offMesh[i] = new WowNavOffMeshConnection(
                new Vector3(F32(raw, 0), F32(raw, 4), F32(raw, 8)),
                new Vector3(F32(raw, 12), F32(raw, 16), F32(raw, 20)),
                F32(raw, 24),
                BinaryPrimitives.ReadUInt16LittleEndian(raw[28..]),
                raw[30],
                raw[31],
                BinaryPrimitives.ReadUInt32LittleEndian(raw[32..]));
        }

        return new WowNavMeshTile
        {
            UsesLiquids = usesLiquids,
            X = I32(data, 8),
            Y = I32(data, 12),
            Layer = I32(data, 16),
            WalkableHeight = F32(data, 60),
            WalkableRadius = F32(data, 64),
            WalkableClimb = F32(data, 68),
            BoundsMin = new Vector3(F32(data, 72), F32(data, 76), F32(data, 80)),
            BoundsMax = new Vector3(F32(data, 84), F32(data, 88), F32(data, 92)),
            Vertices = Floats(data, vertsAt, vertCount * 3),
            PolyVertices = polyVertices,
            PolyVertexCount = polyVertexCount,
            PolyArea = polyArea,
            PolyType = polyType,
            PolyFlags = polyFlags,
            DetailVertexBase = detailVertexBase,
            DetailTriangleBase = detailTriangleBase,
            DetailVertexCount = detailVertexCountPerPoly,
            DetailTriangleCount = detailTriangleCountPerPoly,
            DetailVertices = Floats(data, detailVertsAt, detailVertCount * 3),
            DetailTriangles = data.Slice((int)detailTrisAt, detailTriCount * 4).ToArray(),
            OffMeshConnections = offMesh,
        };
    }

    private static long Align4(long size) => (size + 3) & ~3L;

    private static uint U32(ReadOnlySpan<byte> bytes, int offset) => BinaryPrimitives.ReadUInt32LittleEndian(bytes[offset..]);

    private static int I32(ReadOnlySpan<byte> bytes, int offset) => BinaryPrimitives.ReadInt32LittleEndian(bytes[offset..]);

    private static float F32(ReadOnlySpan<byte> bytes, int offset) => BinaryPrimitives.ReadSingleLittleEndian(bytes[offset..]);

    private static float[] Floats(ReadOnlySpan<byte> data, long offset, int count)
    {
        var result = new float[count];
        for (int i = 0; i < count; i++)
        {
            result[i] = BinaryPrimitives.ReadSingleLittleEndian(data[(int)(offset + (i * 4L))..]);
        }

        return result;
    }
}

/// <summary>
/// Recast space versus the world. The server hands Detour <c>(serverY, serverZ, serverX)</c> —
/// <c>PathGenerator</c> builds its query points as <c>{p.y, p.z, p.x}</c> — and the generator's tile
/// bounds sit in that space with no further shift (<c>MapBuilder::getTileBounds</c>).
/// </summary>
public static class WowNavCoordinates
{
    /// <summary>A Recast-space point in server-world coordinates.</summary>
    public static Vector3 ToServer(Vector3 recast) => new(recast.Z, recast.X, recast.Y);

    public static Vector3 FromServer(Vector3 server) => new(server.Y, server.Z, server.X);

    /// <summary>A Recast-space point in Godot space: <c>WowCoordinates.ToGodot</c> of <see cref="ToServer"/>.</summary>
    public static Vector3 ToGodot(Vector3 recast) => WowCoordinates.ToGodot(ToServer(recast));
}
