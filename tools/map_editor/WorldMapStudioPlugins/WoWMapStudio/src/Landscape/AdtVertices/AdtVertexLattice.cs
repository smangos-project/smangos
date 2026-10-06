using System;
using System.Collections.Generic;
using System.Numerics;

namespace WorldMapStudio.WoW;

/// <summary>
/// A vertex of the map-wide vertex lattice, on a half-step lattice: chunk <c>c</c>'s <c>v</c>th cell boundary
/// along an axis is <c>2 * (c * 8 + v)</c> and the centre of its <c>v</c>th cell is <c>2 * (c * 8 + v) + 1</c>.
/// A vertex is a corner when <c>X</c> and <c>Z</c> are both even, and the centre of a cell when both are odd;
/// mixed parity is no vertex. A corner on a chunk edge is one lattice point shared by its chunks.
/// </summary>
public readonly record struct AdtVertex(int X, int Z)
{
    public const int CellsPerChunk = 8;

    /// <summary>Half-steps across one chunk.</summary>
    public const int StepsPerChunk = CellsPerChunk * 2;

    /// <summary>Whether this is the centre of a cell rather than a corner.</summary>
    public bool IsCentre => (X & 1) == 1;

    /// <summary>The chunk whose minimum corner region this vertex is in, and its half-step index within that
    /// chunk (0-15).</summary>
    public (ChunkCoord Chunk, int LocalX, int LocalZ) Split() => (
        new ChunkCoord(FloorDiv(X, StepsPerChunk), FloorDiv(Z, StepsPerChunk)),
        Mod(X, StepsPerChunk),
        Mod(Z, StepsPerChunk));

    /// <summary>Where this vertex sits in a chunk's grid: its corner index (0-8) for a corner, its cell index
    /// (0-7) for a cell centre. Corners on the far edge belong to the next chunk's grid unless
    /// <paramref name="chunk"/> is the chunk that holds them from before.</summary>
    public (int X, int Z) IndexIn(ChunkCoord chunk) =>
        ((X - (chunk.X * StepsPerChunk)) >> 1, (Z - (chunk.Y * StepsPerChunk)) >> 1);

    /// <summary>The vertex at half-step (<paramref name="localX"/>, <paramref name="localZ"/>) of a chunk.</summary>
    public static AdtVertex Of(ChunkCoord chunk, int localX, int localZ) =>
        new((chunk.X * StepsPerChunk) + localX, (chunk.Y * StepsPerChunk) + localZ);

    /// <summary>Corner (<paramref name="cornerX"/>, <paramref name="cornerZ"/>), 0-8, of a chunk.</summary>
    public static AdtVertex Corner(ChunkCoord chunk, int cornerX, int cornerZ) => Of(chunk, cornerX * 2, cornerZ * 2);

    /// <summary>The centre of cell (<paramref name="cellX"/>, <paramref name="cellZ"/>), 0-7, of a chunk.</summary>
    public static AdtVertex Centre(ChunkCoord chunk, int cellX, int cellZ) => Of(chunk, (cellX * 2) + 1, (cellZ * 2) + 1);

    internal static int FloorDiv(int value, int divisor) => value >= 0 ? value / divisor : ((value + 1) / divisor) - 1;

    internal static int Mod(int value, int divisor)
    {
        int remainder = value % divisor;
        return remainder < 0 ? remainder + divisor : remainder;
    }
}

/// <summary>Whether a chunk's block of an image can be edited.</summary>
public enum AdtBlockState
{
    Ready,

    /// <summary>The block is stored but not loaded yet.</summary>
    Loading,

    /// <summary>The block does not lie inside the image.</summary>
    Outside,
}

/// <summary>Whether a vertex can be edited: every chunk holding it has its block ready.</summary>
public enum AdtVertexAccess
{
    Editable,
    Loading,
    Frozen,
}

/// <summary>Where a <see cref="AdtVertexLattice"/> reads and writes: one 32x32 block of texels per chunk.</summary>
public interface IAdtVertexStore
{
    /// <summary>Opens a chunk's block, row-major, <see cref="AdtVertexCodec.Stride"/> bytes per texel.</summary>
    AdtBlockState Open(ChunkCoord chunk, out byte[]? block);

    /// <summary>Whether the chunk's own data is authoritative for the vertices it holds, as an imported chunk's is,
    /// where the block of a chunk that is not imported is only padding.</summary>
    bool IsSource(ChunkCoord chunk);

    void Write(ChunkCoord chunk, byte[] block);
}

/// <summary>
/// The vertex lattice of an imported map, over the 32x32 texel block each chunk stores (see
/// <see cref="CellCentreTexels"/>). A vertex sits on the corner shared by four texels, <c>2X-1</c> and <c>2X</c>
/// along each axis in half-steps <c>X</c>. A cell centre fills a 2x2 block of one chunk. A corner fills a 2x2
/// block too, but on a chunk edge that block is split between two chunks, or four at a chunk corner: the layout
/// the landscape build samples at, where the average of a vertex's texels is the vertex's value.
///
/// The texels between a corner and a cell centre are read by no vertex. Setting a corner keeps them at the mean
/// of the two corners along their edge, so previews of the image stay coherent.
///
/// A stroke reads and sets vertices here. A vertex that was set keeps its value as a float, so many small
/// changes do not stall on byte rounding, and only the texels of set vertices are ever rewritten: everything
/// else stays byte for byte as it was. Setting a vertex writes every chunk that holds it, which keeps shared
/// edges welded.
/// </summary>
public sealed class AdtVertexLattice : IAdtVertexReader
{
    public const int Edge = CellCentreTexels.PerCell * AdtVertex.CellsPerChunk;

    private sealed class Block(AdtBlockState state, byte[]? bytes, bool source)
    {
        public AdtBlockState State { get; } = state;

        public byte[]? Bytes { get; } = bytes;

        public bool Source { get; } = source;

        public bool Dirty { get; set; }
    }

    private readonly IAdtVertexStore _store;
    private readonly Dictionary<ChunkCoord, Block> _blocks = [];
    private readonly Dictionary<AdtVertex, Vector3> _values = [];
    private readonly List<ChunkCoord> _newlyChanged = [];
    private readonly HashSet<ChunkCoord> _changed = [];

    public AdtVertexLattice(IAdtVertexStore store, AdtVertexCodec codec)
    {
        _store = store;
        Codec = codec;
    }

    public AdtVertexCodec Codec { get; }

    /// <summary>The chunks that hold a vertex whose value was set.</summary>
    public IReadOnlyCollection<ChunkCoord> Changed => _changed;

    /// <summary>The chunks that hold a texel of <paramref name="vertex"/>: one, two or four. A cell centre is
    /// held by exactly one.</summary>
    public static IReadOnlyList<ChunkCoord> ChunksHolding(AdtVertex vertex)
    {
        var chunks = new List<ChunkCoord>(4);
        foreach ((ChunkCoord chunk, int _, int _) in Texels(vertex))
        {
            if (!chunks.Contains(chunk))
            {
                chunks.Add(chunk);
            }
        }

        return chunks;
    }

    /// <summary>The four texels of a vertex, as (chunk, texel x, texel z).</summary>
    public static IEnumerable<(ChunkCoord Chunk, int TexelX, int TexelZ)> Texels(AdtVertex vertex)
    {
        for (int dz = -1; dz <= 0; dz++)
        {
            for (int dx = -1; dx <= 0; dx++)
            {
                int gx = (vertex.X * 2) + dx;
                int gz = (vertex.Z * 2) + dz;
                yield return (
                    new ChunkCoord(AdtVertex.FloorDiv(gx, Edge), AdtVertex.FloorDiv(gz, Edge)),
                    AdtVertex.Mod(gx, Edge),
                    AdtVertex.Mod(gz, Edge));
            }
        }
    }

    public AdtVertexAccess AccessOf(AdtVertex vertex)
    {
        AdtVertexAccess access = AdtVertexAccess.Editable;
        foreach (ChunkCoord chunk in ChunksHolding(vertex))
        {
            switch (BlockOf(chunk).State)
            {
                case AdtBlockState.Outside:
                    return AdtVertexAccess.Frozen;
                case AdtBlockState.Loading:
                    access = AdtVertexAccess.Loading;
                    break;
            }
        }

        return access;
    }

    /// <summary>The value of a vertex: what an earlier set left, or the average of the texels that hold data,
    /// those of source chunks first. False when no chunk holding it can be read.</summary>
    public bool TryGet(AdtVertex vertex, out Vector3 value)
    {
        if (_values.TryGetValue(vertex, out value))
        {
            return true;
        }

        Vector3 sourceSum = Vector3.Zero;
        Vector3 otherSum = Vector3.Zero;
        int sourceCount = 0;
        int otherCount = 0;
        bool readable = false;

        foreach ((ChunkCoord chunk, int tx, int tz) in Texels(vertex))
        {
            Block block = BlockOf(chunk);
            if (block.State != AdtBlockState.Ready)
            {
                continue;
            }

            readable = true;
            int offset = ((tz * Edge) + tx) * Codec.Stride;
            if (!Codec.TryDecode(block.Bytes!.AsSpan(offset, Codec.Stride), out Vector3 texel))
            {
                continue;
            }

            if (block.Source)
            {
                sourceSum += texel;
                sourceCount++;
            }
            else
            {
                otherSum += texel;
                otherCount++;
            }
        }

        value = sourceCount > 0 ? sourceSum / sourceCount : otherCount > 0 ? otherSum / otherCount : Codec.Neutral;
        return readable;
    }

    public Vector3 Get(AdtVertex vertex) => TryGet(vertex, out Vector3 value) ? value : Codec.Neutral;

    /// <summary>Sets a vertex in every chunk that holds it. False, changing nothing, when it is not
    /// <see cref="AdtVertexAccess.Editable"/>.</summary>
    public bool Set(AdtVertex vertex, Vector3 value)
    {
        if (AccessOf(vertex) != AdtVertexAccess.Editable)
        {
            return false;
        }

        value = Codec.Clamp(value);
        _values[vertex] = value;
        foreach ((ChunkCoord chunk, int tx, int tz) in Texels(vertex))
        {
            Block block = BlockOf(chunk);
            Codec.Encode(value, block.Bytes!.AsSpan((((tz * Edge) + tx) * Codec.Stride), Codec.Stride));
            block.Dirty = true;
            if (_changed.Add(chunk))
            {
                _newlyChanged.Add(chunk);
            }
        }

        if (!vertex.IsCentre)
        {
            RefreshEdgeTexels(vertex, clear: false);
        }

        return true;
    }

    /// <summary>Zeroes every texel of a vertex, in every chunk that holds it: no data at all, which for colour and
    /// light means the layer is absent there. False, changing nothing, when it is not
    /// <see cref="AdtVertexAccess.Editable"/>.</summary>
    public bool Clear(AdtVertex vertex)
    {
        if (AccessOf(vertex) != AdtVertexAccess.Editable)
        {
            return false;
        }

        _values.Remove(vertex);
        foreach ((ChunkCoord chunk, int tx, int tz) in Texels(vertex))
        {
            Block block = BlockOf(chunk);
            block.Bytes!.AsSpan((((tz * Edge) + tx) * Codec.Stride), Codec.Stride).Clear();
            block.Dirty = true;
        }

        if (!vertex.IsCentre)
        {
            RefreshEdgeTexels(vertex, clear: true);
        }

        return true;
    }

    // The texels between a corner and its neighbouring corner along an axis are a corner along that axis and a
    // cell centre along the other. No vertex reads them; they hold the mean of the two corners so the image
    // still looks right, or nothing once a corner has been cleared.
    private void RefreshEdgeTexels(AdtVertex corner, bool clear)
    {
        Span<byte> texel = stackalloc byte[Codec.Stride];
        foreach (int side in (ReadOnlySpan<int>)[-1, 1])
        {
            // Texels 2Z+1, 2Z+2 lie beyond the corner, 2Z-3, 2Z-2 before it.
            int near = side > 0 ? 1 : -3;

            var alongZ = new AdtVertex(corner.X, corner.Z + (2 * side));
            var alongX = new AdtVertex(corner.X + (2 * side), corner.Z);
            Vector3 own = default;
            Vector3 valueZ = default;
            Vector3 valueX = default;
            bool haveZ = false;
            bool haveX = false;
            if (!clear && TryGet(corner, out own))
            {
                haveZ = TryGet(alongZ, out valueZ);
                haveX = TryGet(alongX, out valueX);
            }

            if (!clear && !haveZ && !haveX)
            {
                continue;
            }

            for (int step = 0; step < 2; step++)
            {
                for (int across = -1; across <= 0; across++)
                {
                    // Along Z: x runs over the corner's own two texels, z over the mixed pair.
                    WriteEdgeTexel((corner.X * 2) + across, (corner.Z * 2) + near + step, clear, haveZ, own, valueZ, texel);

                    // Along X: the same, transposed.
                    WriteEdgeTexel((corner.X * 2) + near + step, (corner.Z * 2) + across, clear, haveX, own, valueX, texel);
                }
            }
        }
    }

    private void WriteEdgeTexel(int gx, int gz, bool clear, bool have, Vector3 a, Vector3 b, Span<byte> scratch)
    {
        if (!clear && !have)
        {
            return;
        }

        var chunk = new ChunkCoord(AdtVertex.FloorDiv(gx, Edge), AdtVertex.FloorDiv(gz, Edge));
        Block block = BlockOf(chunk);
        if (block.State != AdtBlockState.Ready)
        {
            return;
        }

        int offset = ((AdtVertex.Mod(gz, Edge) * Edge) + AdtVertex.Mod(gx, Edge)) * Codec.Stride;
        Span<byte> target = block.Bytes!.AsSpan(offset, Codec.Stride);
        if (clear)
        {
            target.Clear();
        }
        else
        {
            Codec.Encode(Codec.Clamp((a + b) * 0.5f), scratch);
            scratch.CopyTo(target);
        }

        block.Dirty = true;
    }

    /// <summary>The chunks that first held a set vertex since the last call.</summary>
    public IReadOnlyList<ChunkCoord> TakeNewlyChanged()
    {
        ChunkCoord[] result = _newlyChanged.ToArray();
        _newlyChanged.Clear();
        return result;
    }

    /// <summary>Fills every texel of a chunk's block that carries no data with the neutral value. False when the
    /// block cannot be edited.</summary>
    public bool Seed(ChunkCoord chunk)
    {
        Block block = BlockOf(chunk);
        if (block.State != AdtBlockState.Ready)
        {
            return false;
        }

        int stride = Codec.Stride;
        for (int i = 0; i < Edge * Edge; i++)
        {
            if (Codec.Seed(block.Bytes!.AsSpan(i * stride, stride)))
            {
                block.Dirty = true;
            }
        }

        return true;
    }

    /// <summary>Writes back every block that changed. Returns the chunks written.</summary>
    public IReadOnlyList<ChunkCoord> Flush()
    {
        var written = new List<ChunkCoord>();
        foreach ((ChunkCoord chunk, Block block) in _blocks)
        {
            if (block.Dirty && block.Bytes != null)
            {
                _store.Write(chunk, block.Bytes);
                block.Dirty = false;
                written.Add(chunk);
            }
        }

        return written;
    }

    private Block BlockOf(ChunkCoord chunk)
    {
        if (!_blocks.TryGetValue(chunk, out Block? block))
        {
            AdtBlockState state = _store.Open(chunk, out byte[]? bytes);
            block = new Block(state, state == AdtBlockState.Ready ? bytes : null, _store.IsSource(chunk));
            _blocks[chunk] = block;
        }

        return block;
    }
}
