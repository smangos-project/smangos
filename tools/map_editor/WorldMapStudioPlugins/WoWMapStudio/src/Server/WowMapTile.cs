using System;
using System.Buffers.Binary;

namespace WorldMapStudio.WoW;

/// <summary>One cell's liquid, as the server would resolve it before any <c>LiquidType.dbc</c> override.</summary>
public readonly record struct WowMapLiquidCell(float Level, ushort Entry, byte TypeFlags);

/// <summary>
/// One decoded <c>maps/MMMYYXX.map</c>: the server's terrain for a single ADT tile. Grids are indexed
/// <c>[row * stride + col]</c> where <c>row</c> runs along server X <em>decreasing</em> and <c>col</c>
/// along server Y decreasing — see <see cref="WowMapGrid"/> for the mapping to world space.
///
/// Mirrors <c>GridMap::loadData</c> (<c>src/game/Maps/GridMap.cpp</c>) and <c>GridMapFileHeader</c>
/// (<c>src/game/Maps/GridMapDefines.h</c>). Integer-packed heights are decoded to floats on read;
/// the server interpolates them in integers, but the result is linear in the packed values so the two
/// agree up to float rounding.
/// </summary>
public sealed class WowMapTile
{
    /// <summary>Vertices along one edge of the outer height grid.</summary>
    public const int VertexEdge = 129;

    /// <summary>Cells along one edge; the inner grid holds one centre vertex per cell.</summary>
    public const int CellEdge = WowMapGrid.CellEdge;

    /// <summary>Area and hole entries along one edge — one per MCNK.</summary>
    public const int ChunkEdge = 16;

    /// <summary>The one area id of the tile, used wherever <see cref="Areas"/> is null.</summary>
    public ushort GridArea { get; init; }

    /// <summary>Per-chunk area ids, <c>[row * 16 + col]</c>; null when the whole tile is <see cref="GridArea"/>.</summary>
    public ushort[]? Areas { get; init; }

    /// <summary>The height used everywhere when <see cref="Corners"/> is null.</summary>
    public float GridHeight { get; init; }

    /// <summary>The 129 x 129 corner heights, <c>[row * 129 + col]</c>; null when the tile is flat.</summary>
    public float[]? Corners { get; init; }

    /// <summary>The 128 x 128 cell-centre heights, <c>[row * 128 + col]</c>; null when the tile is flat.</summary>
    public float[]? Centres { get; init; }

    /// <summary>Per-chunk 4x4 hole bitmasks, <c>[row * 16 + col]</c>; null when the tile has no holes.</summary>
    public ushort[]? Holes { get; init; }

    public bool HasLiquid { get; init; }

    public ushort LiquidGlobalEntry { get; init; }

    public byte LiquidGlobalFlags { get; init; }

    /// <summary>Column of the liquid level grid's first cell within the tile.</summary>
    public int LiquidOffsetCol { get; init; }

    /// <summary>Row of the liquid level grid's first cell within the tile.</summary>
    public int LiquidOffsetRow { get; init; }

    public int LiquidWidth { get; init; }

    public int LiquidHeight { get; init; }

    /// <summary>The single level used where <see cref="LiquidLevels"/> is null.</summary>
    public float LiquidLevel { get; init; }

    /// <summary>Per-chunk <c>LiquidType.dbc</c> ids, <c>[row * 16 + col]</c>; null when <see cref="LiquidGlobalEntry"/> applies.</summary>
    public ushort[]? LiquidEntries { get; init; }

    public byte[]? LiquidFlags { get; init; }

    /// <summary>Per-cell levels over the <see cref="LiquidHeight"/> x <see cref="LiquidWidth"/> sub-rectangle.</summary>
    public float[]? LiquidLevels { get; init; }

    public bool IsFlat => Corners == null;

    /// <summary>Corner height at a grid vertex (0..128 on both axes).</summary>
    public float CornerHeight(int row, int col) => Corners == null ? GridHeight : Corners[(row * VertexEdge) + col];

    /// <summary>Centre height of a cell (0..127 on both axes).</summary>
    public float CentreHeight(int row, int col) => Centres == null ? GridHeight : Centres[(row * CellEdge) + col];

    /// <summary>The area id covering a cell.</summary>
    public ushort AreaAt(int row, int col) =>
        Areas == null ? GridArea : Areas[((row >> 3) * ChunkEdge) + (col >> 3)];

    /// <summary>Whether the server treats a cell as a hole (no ground). Same bit layout as <c>GridMap::isHole</c>.</summary>
    public bool IsHole(int row, int col)
    {
        if (Holes == null)
        {
            return false;
        }

        int holeRow = (row % 8) / 2;
        int holeCol = (col % 8) / 2;
        ushort hole = Holes[((row / 8) * ChunkEdge) + (col / 8)];
        return (hole & (0x1111 << holeCol) & (0x000F << (holeRow * 4))) != 0;
    }

    /// <summary>
    /// Terrain height at a fractional grid position (<paramref name="row"/>/<paramref name="col"/> in cells,
    /// 0..128), using the server's four-triangle-per-cell split. Null over a hole.
    /// </summary>
    public float? HeightAt(float row, float col)
    {
        int rowInt = Math.Clamp((int)row, 0, CellEdge - 1);
        int colInt = Math.Clamp((int)col, 0, CellEdge - 1);
        float x = row - rowInt;
        float y = col - colInt;

        if (IsHole(rowInt, colInt))
        {
            return null;
        }

        if (Corners == null)
        {
            return GridHeight;
        }

        float h1 = CornerHeight(rowInt, colInt);
        float h2 = CornerHeight(rowInt + 1, colInt);
        float h3 = CornerHeight(rowInt, colInt + 1);
        float h4 = CornerHeight(rowInt + 1, colInt + 1);
        float h5 = 2 * CentreHeight(rowInt, colInt);

        float a;
        float b;
        float c;
        if (x + y < 1)
        {
            if (x > y)
            {
                a = h2 - h1;
                b = h5 - h1 - h2;
                c = h1;
            }
            else
            {
                a = h5 - h1 - h3;
                b = h3 - h1;
                c = h1;
            }
        }
        else if (x > y)
        {
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }

        return (a * x) + (b * y) + c;
    }

    /// <summary>The liquid in a cell, or null when the cell is dry or outside the liquid grid.</summary>
    public WowMapLiquidCell? LiquidAt(int row, int col)
    {
        if (!HasLiquid)
        {
            return null;
        }

        int chunk = ((row >> 3) * ChunkEdge) + (col >> 3);
        byte flags = LiquidFlags?[chunk] ?? LiquidGlobalFlags;
        ushort entry = LiquidEntries?[chunk] ?? LiquidGlobalEntry;
        if (flags == 0 && entry == 0)
        {
            return null;
        }

        int localRow = row - LiquidOffsetRow;
        int localCol = col - LiquidOffsetCol;
        if (localRow < 0 || localRow >= LiquidHeight || localCol < 0 || localCol >= LiquidWidth)
        {
            return null;
        }

        float level = LiquidLevels?[(localRow * LiquidWidth) + localCol] ?? LiquidLevel;
        return new WowMapLiquidCell(level, entry, flags);
    }
}

/// <summary>Decodes <c>.map</c> bytes into a <see cref="WowMapTile"/>.</summary>
public static class WowMapTileReader
{
    /// <summary><c>INVALID_HEIGHT</c> — what the server reports for a tile with no height block.</summary>
    private const float InvalidHeight = -100000f;

    private const int FileHeaderSize = 40;
    private const int AreaHeaderSize = 8;
    private const int HeightHeaderSize = 16;
    private const int LiquidHeaderSize = 16;

    private const uint AreaFlagNoArea = 0x1;
    private const uint HeightFlagNoHeight = 0x1;
    private const uint HeightFlagInt16 = 0x2;
    private const uint HeightFlagInt8 = 0x4;
    private const byte LiquidFlagNoType = 0x1;
    private const byte LiquidFlagNoHeight = 0x2;

    /// <summary>Throws <see cref="WowServerDataException"/> for a wrong version or magic, or a truncated file.</summary>
    public static WowMapTile Read(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length < FileHeaderSize)
        {
            throw WowServerDataException.Truncated(".map header");
        }

        if (!bytes[..4].SequenceEqual(WowServerFormats.MapMagic))
        {
            throw WowServerDataException.BadMagic(".map header");
        }

        if (!bytes.Slice(4, 4).SequenceEqual(WowServerFormats.MapVersionMagic))
        {
            throw WowServerDataException.MagicMismatch(
                ".map",
                System.Text.Encoding.ASCII.GetString(bytes.Slice(4, 4)),
                System.Text.Encoding.ASCII.GetString(WowServerFormats.MapVersionMagic));
        }

        uint areaOffset = U32(bytes, 8);
        uint heightOffset = U32(bytes, 16);
        uint liquidOffset = U32(bytes, 24);
        uint holesOffset = U32(bytes, 32);
        uint holesSize = U32(bytes, 36);

        ushort gridArea = 0;
        ushort[]? areas = null;
        if (areaOffset != 0)
        {
            ReadOnlySpan<byte> area = Section(bytes, areaOffset, AreaHeaderSize, WowServerFormats.MapAreaMagic, "AREA block");
            gridArea = U16(area, 6);
            if ((U16(area, 4) & AreaFlagNoArea) == 0)
            {
                areas = U16Array(bytes, areaOffset + AreaHeaderSize, WowMapTile.ChunkEdge * WowMapTile.ChunkEdge, "area map");
            }
        }

        float gridHeight = InvalidHeight;
        float[]? corners = null;
        float[]? centres = null;
        if (heightOffset != 0)
        {
            ReadOnlySpan<byte> height = Section(bytes, heightOffset, HeightHeaderSize, WowServerFormats.MapHeightMagic, "MHGT block");
            uint flags = U32(height, 4);
            gridHeight = F32(height, 8);
            float gridMaxHeight = F32(height, 12);

            long cursor = heightOffset + HeightHeaderSize;
            const int cornerCount = WowMapTile.VertexEdge * WowMapTile.VertexEdge;
            const int centreCount = WowMapTile.CellEdge * WowMapTile.CellEdge;
            if ((flags & HeightFlagNoHeight) == 0)
            {
                if ((flags & HeightFlagInt16) != 0)
                {
                    float scale = (gridMaxHeight - gridHeight) / 65535f;
                    corners = PackedHeights16(bytes, cursor, cornerCount, gridHeight, scale);
                    cursor += cornerCount * 2L;
                    centres = PackedHeights16(bytes, cursor, centreCount, gridHeight, scale);
                    cursor += centreCount * 2L;
                }
                else if ((flags & HeightFlagInt8) != 0)
                {
                    float scale = (gridMaxHeight - gridHeight) / 255f;
                    corners = PackedHeights8(bytes, cursor, cornerCount, gridHeight, scale);
                    cursor += cornerCount;
                    centres = PackedHeights8(bytes, cursor, centreCount, gridHeight, scale);
                    cursor += centreCount;
                }
                else
                {
                    corners = FloatArray(bytes, cursor, cornerCount);
                    cursor += cornerCount * 4L;
                    centres = FloatArray(bytes, cursor, centreCount);
                    cursor += centreCount * 4L;
                }
            }
        }

        ushort[]? holes = holesSize != 0
            ? U16Array(bytes, holesOffset, WowMapTile.ChunkEdge * WowMapTile.ChunkEdge, "holes")
            : null;

        LiquidBlock? liquid = ReadLiquid(bytes, liquidOffset);
        return new WowMapTile
        {
            GridArea = gridArea,
            Areas = areas,
            GridHeight = gridHeight,
            Corners = corners,
            Centres = centres,
            Holes = holes,
            HasLiquid = liquid != null,
            LiquidGlobalEntry = liquid?.GlobalEntry ?? 0,
            LiquidGlobalFlags = liquid?.GlobalFlags ?? 0,
            LiquidOffsetCol = liquid?.OffsetCol ?? 0,
            LiquidOffsetRow = liquid?.OffsetRow ?? 0,
            LiquidWidth = liquid?.Width ?? 0,
            LiquidHeight = liquid?.Height ?? 0,
            LiquidLevel = liquid?.Level ?? 0,
            LiquidEntries = liquid?.Entries,
            LiquidFlags = liquid?.Flags,
            LiquidLevels = liquid?.Levels,
        };
    }

    private sealed record LiquidBlock(
        ushort GlobalEntry, byte GlobalFlags, int OffsetCol, int OffsetRow, int Width, int Height, float Level,
        ushort[]? Entries, byte[]? Flags, float[]? Levels);

    private static LiquidBlock? ReadLiquid(ReadOnlySpan<byte> bytes, uint offset)
    {
        if (offset == 0)
        {
            return null;
        }

        ReadOnlySpan<byte> header = Section(bytes, offset, LiquidHeaderSize, WowServerFormats.MapLiquidMagic, "MLIQ block");
        byte flags = header[4];
        byte globalFlags = header[5];
        ushort globalEntry = U16(header, 6);
        int offsetCol = header[8];
        int offsetRow = header[9];
        int width = header[10];
        int height = header[11];
        float level = F32(header, 12);

        long cursor = offset + LiquidHeaderSize;
        ushort[]? entries = null;
        byte[]? typeFlags = null;
        if ((flags & LiquidFlagNoType) == 0)
        {
            const int count = WowMapTile.ChunkEdge * WowMapTile.ChunkEdge;
            entries = U16Array(bytes, cursor, count, "liquid entries");
            cursor += count * 2L;
            typeFlags = Slice(bytes, cursor, count, "liquid flags").ToArray();
            cursor += count;
        }

        float[]? levels = null;
        if ((flags & LiquidFlagNoHeight) == 0)
        {
            levels = FloatArray(bytes, cursor, width * height);
        }

        return new LiquidBlock(globalEntry, globalFlags, offsetCol, offsetRow, width, height, level, entries, typeFlags, levels);
    }

    private static ReadOnlySpan<byte> Section(ReadOnlySpan<byte> bytes, long offset, int headerSize, ReadOnlySpan<byte> magic, string what)
    {
        ReadOnlySpan<byte> header = Slice(bytes, offset, headerSize, what);
        if (!header[..4].SequenceEqual(magic))
        {
            throw WowServerDataException.BadMagic(what);
        }

        return header;
    }

    private static ReadOnlySpan<byte> Slice(ReadOnlySpan<byte> bytes, long offset, long length, string what)
    {
        if (offset < 0 || length < 0 || offset + length > bytes.Length)
        {
            throw WowServerDataException.Truncated(what);
        }

        return bytes.Slice((int)offset, (int)length);
    }

    private static uint U32(ReadOnlySpan<byte> bytes, int offset) => BinaryPrimitives.ReadUInt32LittleEndian(bytes[offset..]);

    private static ushort U16(ReadOnlySpan<byte> bytes, int offset) => BinaryPrimitives.ReadUInt16LittleEndian(bytes[offset..]);

    private static float F32(ReadOnlySpan<byte> bytes, int offset) => BinaryPrimitives.ReadSingleLittleEndian(bytes[offset..]);

    private static ushort[] U16Array(ReadOnlySpan<byte> bytes, long offset, int count, string what)
    {
        ReadOnlySpan<byte> raw = Slice(bytes, offset, count * 2L, what);
        var result = new ushort[count];
        for (int i = 0; i < count; i++)
        {
            result[i] = BinaryPrimitives.ReadUInt16LittleEndian(raw[(i * 2)..]);
        }

        return result;
    }

    private static float[] FloatArray(ReadOnlySpan<byte> bytes, long offset, int count)
    {
        ReadOnlySpan<byte> raw = Slice(bytes, offset, count * 4L, "float grid");
        var result = new float[count];
        for (int i = 0; i < count; i++)
        {
            result[i] = BinaryPrimitives.ReadSingleLittleEndian(raw[(i * 4)..]);
        }

        return result;
    }

    private static float[] PackedHeights16(ReadOnlySpan<byte> bytes, long offset, int count, float min, float scale)
    {
        ReadOnlySpan<byte> raw = Slice(bytes, offset, count * 2L, "height grid");
        var result = new float[count];
        for (int i = 0; i < count; i++)
        {
            result[i] = (BinaryPrimitives.ReadUInt16LittleEndian(raw[(i * 2)..]) * scale) + min;
        }

        return result;
    }

    private static float[] PackedHeights8(ReadOnlySpan<byte> bytes, long offset, int count, float min, float scale)
    {
        ReadOnlySpan<byte> raw = Slice(bytes, offset, count, "height grid");
        var result = new float[count];
        for (int i = 0; i < count; i++)
        {
            result[i] = (raw[i] * scale) + min;
        }

        return result;
    }
}
