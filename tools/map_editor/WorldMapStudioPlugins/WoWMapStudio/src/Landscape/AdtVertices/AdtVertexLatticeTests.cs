using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;

namespace WorldMapStudio.WoW;

/// <summary>Covers the vertex lattice, its codecs and the ops on arrays, with no image behind them.</summary>
public static class AdtVertexLatticeTests
{
    private const int Edge = AdtVertexLattice.Edge;

    // The corner or cell centre with these grid indices, on the half-step lattice.
    private static AdtVertex Corner(int x, int z) => new(x * 2, z * 2);

    private static AdtVertex Centre(int x, int z) => new((x * 2) + 1, (z * 2) + 1);

    private sealed class MemoryStore(AdtVertexCodec codec) : IAdtVertexStore
    {
        public Dictionary<ChunkCoord, byte[]> Blocks { get; } = [];

        public HashSet<ChunkCoord> Sources { get; } = [];

        public HashSet<ChunkCoord> Loading { get; } = [];

        public HashSet<ChunkCoord> Outside { get; } = [];

        public int Writes { get; private set; }

        public byte[] Block(ChunkCoord chunk)
        {
            if (!Blocks.TryGetValue(chunk, out byte[]? block))
            {
                Blocks[chunk] = block = new byte[Edge * Edge * codec.Stride];
            }

            return block;
        }

        public AdtBlockState Open(ChunkCoord chunk, out byte[]? block)
        {
            block = null;
            if (Outside.Contains(chunk))
            {
                return AdtBlockState.Outside;
            }

            if (Loading.Contains(chunk))
            {
                return AdtBlockState.Loading;
            }

            block = (byte[])Block(chunk).Clone();
            return AdtBlockState.Ready;
        }

        public bool IsSource(ChunkCoord chunk) => Sources.Contains(chunk);

        public void Write(ChunkCoord chunk, byte[] block)
        {
            Blocks[chunk] = (byte[])block.Clone();
            Writes++;
        }
    }

    // A chunk block the way the importer stores a height grid. Cell centres default to the mean of their corners.
    private static void FillNearest(
        MemoryStore store, ChunkCoord chunk, Func<int, int, float> cornerAt, Func<int, int, float>? centreAt = null)
    {
        centreAt ??= (x, z) => (cornerAt(x, z) + cornerAt(x + 1, z) + cornerAt(x, z + 1) + cornerAt(x + 1, z + 1)) * 0.25f;

        byte[] block = store.Block(chunk);
        for (int tz = 0; tz < Edge; tz++)
        {
            for (int tx = 0; tx < Edge; tx++)
            {
                float height = CellCentreTexels.Texel(tx, tz, cornerAt, centreAt, static (a, b) => (a + b) * 0.5f);
                BitConverter.TryWriteBytes(block.AsSpan(((tz * Edge) + tx) * sizeof(float)), height);
            }
        }
    }

    private static float HeightIn(MemoryStore store, ChunkCoord chunk, int tx, int tz) =>
        BitConverter.ToSingle(store.Block(chunk), ((tz * Edge) + tx) * sizeof(float));

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void An_interior_vertex_holds_a_two_by_two_block_of_one_chunk()
    {
        var texels = AdtVertexLattice.Texels(Corner(3, 3)).ToList();

        Assert.AreEqual(4, texels.Count);
        Assert.IsTrue(texels.All(texel => texel.Chunk == new ChunkCoord(0, 0)));
        Assert.IsTrue(texels.Select(texel => (texel.TexelX, texel.TexelZ)).OrderBy(pair => pair).SequenceEqual(
            [(11, 11), (11, 12), (12, 11), (12, 12)]));

        var centre = AdtVertexLattice.Texels(Centre(3, 3)).ToList();
        Assert.IsTrue(centre.Select(texel => (texel.TexelX, texel.TexelZ)).OrderBy(pair => pair).SequenceEqual(
            [(13, 13), (13, 14), (14, 13), (14, 14)]), "the two texels between a cell's corner texels, per axis");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void An_edge_vertex_spans_two_chunks_and_a_corner_vertex_four()
    {
        Assert.AreEqual(2, AdtVertexLattice.ChunksHolding(Corner(8, 3)).Count);
        Assert.AreEqual(2, AdtVertexLattice.ChunksHolding(Corner(3, 8)).Count);
        Assert.AreEqual(2, AdtVertexLattice.ChunksHolding(Corner(0, 3)).Count);
        Assert.AreEqual(4, AdtVertexLattice.ChunksHolding(Corner(8, 8)).Count);
        Assert.AreEqual(1, AdtVertexLattice.ChunksHolding(Corner(1, 7)).Count);
        Assert.AreEqual(1, AdtVertexLattice.ChunksHolding(Centre(7, 7)).Count, "a cell centre is never shared");
        Assert.AreEqual(1, AdtVertexLattice.ChunksHolding(Centre(0, 0)).Count);

        var edge = AdtVertexLattice.Texels(Corner(8, 3)).Where(texel => texel.Chunk == new ChunkCoord(0, 0)).ToList();
        Assert.IsTrue(edge.All(texel => texel.TexelX == Edge - 1), "the last column of the chunk to the west");
        var beyond = AdtVertexLattice.Texels(Corner(8, 3)).Where(texel => texel.Chunk == new ChunkCoord(1, 0)).ToList();
        Assert.IsTrue(beyond.All(texel => texel.TexelX == 0), "and the first column of the one to the east");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Negative_chunk_coordinates_address_the_right_texels()
    {
        var corner = AdtVertexLattice.Texels(Corner(0, 0)).ToList();
        Assert.AreEqual(4, corner.Select(texel => texel.Chunk).Distinct().Count());
        Assert.IsTrue(corner.Any(texel => texel.Chunk == new ChunkCoord(-1, -1) && texel.TexelX == Edge - 1 && texel.TexelZ == Edge - 1));

        (ChunkCoord chunk, int localX, int localZ) = Corner(-1, -9).Split();
        Assert.AreEqual(new ChunkCoord(-1, -2), chunk);
        Assert.AreEqual(14, localX, "half-steps within the chunk");
        Assert.AreEqual(14, localZ);
        Assert.AreEqual(Corner(-1, -9), AdtVertex.Of(chunk, localX, localZ));

        AdtVertex centre = Centre(-1, -9);
        Assert.IsTrue(centre.IsCentre);
        Assert.IsFalse(Corner(-1, -9).IsCentre);
        Assert.AreEqual((7, 7), centre.IndexIn(new ChunkCoord(-1, -2)));
        Assert.AreEqual((7, 7), Corner(-1, -9).IndexIn(new ChunkCoord(-1, -2)));
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void The_texels_a_vertex_holds_are_the_ones_the_importer_gives_it()
    {
        // Every vertex gets its own id; the texels the importer's layout gives that id must be the ones the
        // lattice says the vertex holds. The mean texels between two vertices take an id of neither.
        var chunk = new ChunkCoord(0, 0);
        const float mixed = -1.0f;
        var ids = new float[Edge * Edge];
        for (int tz = 0; tz < Edge; tz++)
        {
            for (int tx = 0; tx < Edge; tx++)
            {
                ids[(tz * Edge) + tx] = CellCentreTexels.Texel(
                    tx, tz, (x, z) => 1.0f + (x * 9) + z, (x, z) => 1000.0f + (x * 8) + z, static (_, _) => mixed);
            }
        }

        List<(int, int)> Theirs(float id) =>
        [
            .. Enumerable.Range(0, Edge * Edge)
                .Where(i => ids[i] == id)
                .Select(i => (i % Edge, i / Edge))
                .OrderBy(pair => pair),
        ];

        for (int vz = 0; vz <= 8; vz++)
        {
            for (int vx = 0; vx <= 8; vx++)
            {
                var mine = AdtVertexLattice.Texels(Corner(vx, vz))
                    .Where(texel => texel.Chunk == chunk)
                    .Select(texel => (texel.TexelX, texel.TexelZ))
                    .OrderBy(pair => pair)
                    .ToList();
                Assert.IsTrue(mine.SequenceEqual(Theirs(1 + (vx * 9) + vz)), $"corner {vx},{vz}");
            }
        }

        for (int cz = 0; cz < 8; cz++)
        {
            for (int cx = 0; cx < 8; cx++)
            {
                var mine = AdtVertexLattice.Texels(Centre(cx, cz))
                    .Select(texel => (texel.TexelX, texel.TexelZ))
                    .OrderBy(pair => pair)
                    .ToList();
                Assert.IsTrue(mine.SequenceEqual(Theirs(1000 + (cx * 8) + cz)), $"centre {cx},{cz}");
            }
        }
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Reading_a_vertex_returns_what_the_importer_stored_including_on_shared_edges()
    {
        var store = new MemoryStore(AdtVertexCodec.Height);
        var west = new ChunkCoord(0, 0);
        var east = new ChunkCoord(1, 0);
        store.Sources.Add(west);
        store.Sources.Add(east);
        FillNearest(store, west, (x, z) => (100 * x) + z, (x, z) => 5000 + (10 * x) + z);
        FillNearest(store, east, (x, z) => (100 * (x + 8)) + z, (x, z) => 6000 + (10 * x) + z);
        var lattice = new AdtVertexLattice(store, AdtVertexCodec.Height);

        Assert.AreEqual(305.0f, lattice.Get(Corner(3, 5)).X);
        Assert.AreEqual(805.0f, lattice.Get(Corner(8, 5)).X, "the shared vertex, from both chunks");
        Assert.AreEqual(1003.0f, lattice.Get(Corner(10, 3)).X);
        Assert.AreEqual(5000.0f + 32.0f, lattice.Get(Centre(3, 2)).X, "a cell centre reads its own value");
        Assert.AreEqual(6000.0f + 2.0f, lattice.Get(Centre(8, 2)).X, "the first cell of the chunk to the east");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void A_vertex_reads_from_source_chunks_and_ignores_padding()
    {
        var store = new MemoryStore(AdtVertexCodec.Height);
        var source = new ChunkCoord(0, 0);
        store.Sources.Add(source);
        FillNearest(store, source, (_, _) => 40.0f);
        var lattice = new AdtVertexLattice(store, AdtVertexCodec.Height);

        Assert.AreEqual(40.0f, lattice.Get(Corner(8, 4)).X, "not half of it, as the build would sample");
        Assert.AreEqual(40.0f, lattice.Get(Corner(8, 8)).X);
        Assert.AreEqual(40.0f, lattice.Get(Centre(7, 7)).X);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Setting_a_vertex_writes_every_chunk_that_holds_it_and_nothing_else()
    {
        var store = new MemoryStore(AdtVertexCodec.Height);
        var chunks = new[] { new ChunkCoord(0, 0), new ChunkCoord(1, 0), new ChunkCoord(0, 1), new ChunkCoord(1, 1) };
        foreach (ChunkCoord chunk in chunks)
        {
            store.Sources.Add(chunk);
            FillNearest(store, chunk, (x, z) => (x * 3) + z + (chunk.X * 50) + (chunk.Y * 70));
        }

        Dictionary<ChunkCoord, byte[]> before = chunks.ToDictionary(chunk => chunk, chunk => (byte[])store.Block(chunk).Clone());
        var lattice = new AdtVertexLattice(store, AdtVertexCodec.Height);

        Assert.IsTrue(lattice.Set(Corner(8, 8), new Vector3(999.0f, 0, 0)));
        lattice.Flush();

        // The vertex's own texels take the value. Beside them, up to two texels out along each axis, sit the texels
        // no vertex reads; those follow it. Nothing further away may change.
        var touched = AdtVertexLattice.Texels(Corner(8, 8)).ToList();
        foreach (ChunkCoord chunk in chunks)
        {
            for (int tz = 0; tz < Edge; tz++)
            {
                for (int tx = 0; tx < Edge; tx++)
                {
                    int offset = ((tz * Edge) + tx) * sizeof(float);
                    bool isTouched = touched.Any(texel => texel.Chunk == chunk && texel.TexelX == tx && texel.TexelZ == tz);
                    bool same = store.Block(chunk).AsSpan(offset, sizeof(float)).SequenceEqual(before[chunk].AsSpan(offset, sizeof(float)));
                    if (isTouched)
                    {
                        Assert.IsFalse(same, $"{chunk} texel {tx},{tz}");
                        continue;
                    }

                    // Global texel position, 32 to a chunk along each axis; the vertex owns 31 and 32.
                    int gx = (chunk.X * Edge) + tx;
                    int gz = (chunk.Y * Edge) + tz;
                    bool nearCorner = gx is >= 28 and <= 35 && gz is >= 28 and <= 35;
                    Assert.IsTrue(same || nearCorner, $"{chunk} texel {tx},{tz} changed away from the vertex");
                }
            }
        }

        Assert.AreEqual(999.0f, HeightIn(store, chunks[0], Edge - 1, Edge - 1));
        Assert.AreEqual(999.0f, HeightIn(store, chunks[1], 0, Edge - 1));
        Assert.AreEqual(999.0f, HeightIn(store, chunks[2], Edge - 1, 0));
        Assert.AreEqual(999.0f, HeightIn(store, chunks[3], 0, 0));
        Assert.AreEqual(4, lattice.Changed.Count);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void A_lattice_that_only_reads_writes_nothing()
    {
        var store = new MemoryStore(AdtVertexCodec.Color);
        var chunk = new ChunkCoord(0, 0);
        store.Sources.Add(chunk);
        new Random(4).NextBytes(store.Block(chunk));
        byte[] before = (byte[])store.Block(chunk).Clone();
        var lattice = new AdtVertexLattice(store, AdtVertexCodec.Color);

        for (int i = 0; i <= 8; i++)
        {
            lattice.Get(Corner(i, i));
        }

        lattice.Flush();
        Assert.AreEqual(0, store.Writes);
        Assert.IsTrue(store.Block(chunk).SequenceEqual(before));
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void A_vertex_that_cannot_be_written_everywhere_is_left_alone()
    {
        var store = new MemoryStore(AdtVertexCodec.Height);
        var west = new ChunkCoord(0, 0);
        var loading = new ChunkCoord(1, 0);
        var outside = new ChunkCoord(-1, 0);
        store.Sources.Add(west);
        store.Loading.Add(loading);
        store.Outside.Add(outside);
        var lattice = new AdtVertexLattice(store, AdtVertexCodec.Height);

        Assert.AreEqual(AdtVertexAccess.Editable, lattice.AccessOf(Corner(4, 4)));
        Assert.AreEqual(AdtVertexAccess.Editable, lattice.AccessOf(Centre(7, 3)), "a centre next to a loading chunk is still held by one");
        Assert.AreEqual(AdtVertexAccess.Loading, lattice.AccessOf(Corner(8, 4)));
        Assert.AreEqual(AdtVertexAccess.Frozen, lattice.AccessOf(Corner(0, 4)));

        Assert.IsFalse(lattice.Set(Corner(8, 4), new Vector3(5, 0, 0)));
        Assert.IsFalse(lattice.Set(Corner(0, 4), new Vector3(5, 0, 0)));
        Assert.AreEqual(0, lattice.Changed.Count);
        lattice.Flush();
        Assert.AreEqual(0, store.Writes);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Colour_bytes_read_as_client_multipliers_with_seven_f_neutral()
    {
        Span<byte> texel = stackalloc byte[4];
        AdtVertexCodec.Color.Encode(Vector3.One, texel);

        Assert.AreEqual(AdtVertexCodec.NeutralColorByte, texel[0]);
        Assert.AreEqual(255, texel[3]);

        AdtVertexCodec.Color.Encode(new Vector3(0.0f, 2.0f, 5.0f), texel);
        Assert.AreEqual(0, texel[0]);
        Assert.AreEqual(254, texel[1]);
        Assert.AreEqual(255, texel[2], "clamped to the byte range");

        texel[0] = 0xBF;
        AdtVertexCodec.Color.TryDecode(texel, out Vector3 value);
        Assert.AreApproximatelyEqual(191.0 / 127.0, value.X, 1e-5);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void A_colour_texel_with_no_alpha_holds_no_data_and_seeding_fills_only_those()
    {
        Span<byte> empty = stackalloc byte[4];
        Assert.IsFalse(AdtVertexCodec.Color.TryDecode(empty, out _));
        Assert.IsTrue(AdtVertexCodec.Color.Seed(empty));
        Assert.AreEqual(AdtVertexCodec.NeutralColorByte, empty[1]);
        Assert.AreEqual(255, empty[3]);

        Span<byte> painted = stackalloc byte[4] { 10, 20, 30, 255 };
        Assert.IsFalse(AdtVertexCodec.Color.Seed(painted));
        Assert.AreEqual(20, painted[1]);

        var store = new MemoryStore(AdtVertexCodec.Color);
        var chunk = new ChunkCoord(0, 0);
        store.Sources.Add(chunk);
        var lattice = new AdtVertexLattice(store, AdtVertexCodec.Color);
        Assert.AreEqual(Vector3.One, lattice.Get(Corner(2, 2)), "a vertex with no data is neutral");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Light_reads_as_added_light_and_writes_no_alpha()
    {
        Span<byte> texel = stackalloc byte[4] { 0, 0, 0, 200 };
        AdtVertexCodec.Light.Encode(new Vector3(1.0f, 0.5f, 0.0f), texel);

        Assert.AreEqual(255, texel[0]);
        Assert.AreEqual(128, texel[1]);
        Assert.AreEqual(0, texel[3]);
        Assert.IsTrue(AdtVertexCodec.Light.TryDecode(texel, out Vector3 value));
        Assert.AreApproximatelyEqual(0.5, value.Y, 0.01);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Many_tiny_colour_dabs_converge_where_per_dab_rounding_would_stall()
    {
        var store = new MemoryStore(AdtVertexCodec.Color);
        var chunk = new ChunkCoord(0, 0);
        store.Sources.Add(chunk);
        var lattice = new AdtVertexLattice(store, AdtVertexCodec.Color);
        lattice.Set(Corner(4, 4), Vector3.One);
        var op = new AdtPaintOp(new Vector3(1.4f, 1.0f, 0.5f), Vector3.One);
        var vertex = Corner(4, 4);

        for (int i = 0; i < 300; i++)
        {
            Vector3 next = op.Apply(lattice.Get(vertex), new AdtVertexStep(vertex, 0, 0, 0.02f, false), lattice);
            lattice.Set(vertex, next);
        }

        lattice.Flush();
        Assert.AreApproximatelyEqual(1.4, lattice.Get(vertex).X, 0.01);
        Assert.AreEqual((byte)Math.Round(1.4f * 127.0f), store.Block(chunk)[((16 * Edge) + 16) * 4]);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Clearing_a_vertex_zeroes_its_texels_in_every_chunk()
    {
        var store = new MemoryStore(AdtVertexCodec.Color);
        var west = new ChunkCoord(0, 0);
        var east = new ChunkCoord(1, 0);
        store.Sources.Add(west);
        store.Sources.Add(east);
        var lattice = new AdtVertexLattice(store, AdtVertexCodec.Color);
        lattice.Set(Corner(8, 2), new Vector3(1.2f, 1.2f, 1.2f));
        lattice.Flush();

        Assert.IsTrue(lattice.Clear(Corner(8, 2)));
        lattice.Flush();

        foreach ((ChunkCoord chunk, int tx, int tz) in AdtVertexLattice.Texels(Corner(8, 2)))
        {
            Assert.IsTrue(store.Block(chunk).AsSpan(((tz * Edge) + tx) * 4, 4).ToArray().All(value => value == 0));
        }
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Raise_lowers_when_inverted_and_flatten_pulls_towards_its_target()
    {
        var raise = new AdtRaiseOp { Rate = 2.0f };
        var step = new AdtVertexStep(Corner(0, 0), 0, 0, 0.5f, false);
        Assert.AreEqual(11.0f, raise.Apply(new Vector3(10, 0, 0), step, new NoNeighbours()).X);
        Assert.AreEqual(9.0f, raise.Apply(new Vector3(10, 0, 0), step with { Invert = true }, new NoNeighbours()).X);

        var flatten = new AdtFlattenOp { Target = 20.0f };
        flatten.Begin(Corner(0, 0), new NoNeighbours());
        Assert.AreEqual(15.0f, flatten.Apply(new Vector3(10, 0, 0), step, new NoNeighbours()).X);

        var raiseOnly = new AdtFlattenOp { Target = 20.0f, Side = AdtFlattenSide.RaiseOnly };
        raiseOnly.Begin(Corner(0, 0), new NoNeighbours());
        Assert.AreEqual(30.0f, raiseOnly.Apply(new Vector3(30, 0, 0), step, new NoNeighbours()).X, "already above the target");
        Assert.AreEqual(15.0f, raiseOnly.Apply(new Vector3(10, 0, 0), step, new NoNeighbours()).X);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Flatten_without_a_target_takes_the_height_under_the_first_dab()
    {
        var store = new MemoryStore(AdtVertexCodec.Height);
        var chunk = new ChunkCoord(0, 0);
        store.Sources.Add(chunk);
        FillNearest(store, chunk, (x, _) => x * 10.0f);
        var lattice = new AdtVertexLattice(store, AdtVertexCodec.Height);

        var flatten = new AdtFlattenOp();
        flatten.Begin(Corner(3, 3), lattice);

        Assert.AreEqual(30.0f, flatten.ResolvedTarget);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Smooth_averages_the_neighbourhood_the_same_whichever_vertex_is_visited_first()
    {
        var store = new MemoryStore(AdtVertexCodec.Height);
        var chunk = new ChunkCoord(0, 0);
        store.Sources.Add(chunk);
        FillNearest(store, chunk, (x, z) => (x == 4 && z == 4) ? 90.0f : 0.0f, (_, _) => 0.0f);
        var lattice = new AdtVertexLattice(store, AdtVertexCodec.Height);
        var smooth = new AdtSmoothOp();

        Vector3 middle = smooth.Apply(lattice.Get(Corner(4, 4)), new AdtVertexStep(Corner(4, 4), 0, 0, 1.0f, false), lattice);
        Vector3 side = smooth.Apply(lattice.Get(Corner(5, 4)), new AdtVertexStep(Corner(5, 4), 0, 0, 1.0f, false), lattice);

        Assert.AreApproximatelyEqual(90.0 / 13.0, middle.X, 1e-4, "itself and twelve neighbours");
        Assert.AreApproximatelyEqual(90.0 / 13.0, side.X, 1e-4, "a same-kind neighbour one cell away sees the spike too");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Smooth_treats_the_corners_around_a_cell_centre_as_neighbours()
    {
        var store = new MemoryStore(AdtVertexCodec.Height);
        var chunk = new ChunkCoord(0, 0);
        store.Sources.Add(chunk);
        FillNearest(store, chunk, (x, z) => (x == 4 && z == 4) ? 90.0f : 0.0f, (_, _) => 0.0f);
        var lattice = new AdtVertexLattice(store, AdtVertexCodec.Height);
        var smooth = new AdtSmoothOp();

        AdtVertex touching = Centre(3, 3);
        Vector3 next = smooth.Apply(lattice.Get(touching), new AdtVertexStep(touching, 0, 0, 1.0f, false), lattice);
        Assert.AreApproximatelyEqual(90.0 / 13.0, next.X, 1e-4, "the raised corner is one of its four diagonal neighbours");

        AdtVertex away = Centre(1, 1);
        Assert.AreEqual(0.0f, smooth.Apply(lattice.Get(away), new AdtVertexStep(away, 0, 0, 1.0f, false), lattice).X);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Setting_a_cell_centre_writes_four_texels_of_one_chunk_and_leaves_the_edge_texels_alone()
    {
        var store = new MemoryStore(AdtVertexCodec.Height);
        var chunk = new ChunkCoord(0, 0);
        store.Sources.Add(chunk);
        FillNearest(store, chunk, (x, z) => (x * 3) + z);
        byte[] before = (byte[])store.Block(chunk).Clone();
        var lattice = new AdtVertexLattice(store, AdtVertexCodec.Height);

        Assert.IsTrue(lattice.Set(Centre(2, 5), new Vector3(777.0f, 0, 0)));
        lattice.Flush();

        var written = AdtVertexLattice.Texels(Centre(2, 5)).ToList();
        for (int tz = 0; tz < Edge; tz++)
        {
            for (int tx = 0; tx < Edge; tx++)
            {
                int offset = ((tz * Edge) + tx) * sizeof(float);
                bool isWritten = written.Any(texel => texel.TexelX == tx && texel.TexelZ == tz);
                bool same = store.Block(chunk).AsSpan(offset, sizeof(float)).SequenceEqual(before.AsSpan(offset, sizeof(float)));
                Assert.AreEqual(!isWritten, same, $"texel {tx},{tz}");
            }
        }

        Assert.AreEqual(1, lattice.Changed.Count);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Setting_a_corner_keeps_the_edge_texels_at_the_mean_of_the_corners_along_them()
    {
        var store = new MemoryStore(AdtVertexCodec.Height);
        var chunk = new ChunkCoord(0, 0);
        store.Sources.Add(chunk);
        FillNearest(store, chunk, (x, z) => 10.0f * (x + z));
        var lattice = new AdtVertexLattice(store, AdtVertexCodec.Height);

        Assert.IsTrue(lattice.Set(Corner(3, 3), new Vector3(500.0f, 0, 0)));
        lattice.Flush();

        // Corner 3 owns texels 11 and 12 per axis. Between it and corner 4 along an axis are texels 13 and 14,
        // between it and corner 2 texels 9 and 10.
        Assert.AreEqual((500.0f + 70.0f) * 0.5f, HeightIn(store, chunk, 11, 13), "along +z");
        Assert.AreEqual((500.0f + 50.0f) * 0.5f, HeightIn(store, chunk, 12, 10), "along -z");
        Assert.AreEqual((500.0f + 70.0f) * 0.5f, HeightIn(store, chunk, 14, 12), "along +x");
        Assert.AreEqual((500.0f + 50.0f) * 0.5f, HeightIn(store, chunk, 9, 11), "along -x");
        Assert.AreEqual(500.0f, HeightIn(store, chunk, 12, 12));

        Assert.AreEqual(500.0f, lattice.Get(Corner(3, 3)).X);
        Assert.AreEqual(70.0f, lattice.Get(Centre(3, 3)).X, "a centre keeps its own value");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Paint_moves_towards_its_target_and_inverting_moves_towards_neutral()
    {
        var op = new AdtPaintOp(new Vector3(2.0f, 0.0f, 1.0f), Vector3.One);
        var step = new AdtVertexStep(Corner(0, 0), 0, 0, 0.5f, false);

        Assert.AreEqual(new Vector3(1.5f, 0.5f, 1.0f), op.Apply(Vector3.One, step, new NoNeighbours()));
        Assert.AreEqual(new Vector3(1.5f, 0.75f, 1.0f), op.Apply(new Vector3(2.0f, 0.5f, 1.0f), step with { Invert = true }, new NoNeighbours()));
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void Brighten_keeps_the_hue_and_tint_keeps_the_brightness()
    {
        var brighten = new AdtBrightnessOp { Rate = 1.0f };
        var step = new AdtVertexStep(Corner(0, 0), 0, 0, 0.5f, false);
        Vector3 lit = brighten.Apply(new Vector3(1.0f, 0.5f, 0.25f), step, new NoNeighbours());
        Assert.AreEqual(new Vector3(1.5f, 0.75f, 0.375f), lit);
        Vector3 dark = brighten.Apply(new Vector3(1.0f, 0.5f, 0.25f), step with { Invert = true }, new NoNeighbours());
        Assert.AreEqual(new Vector3(0.5f, 0.25f, 0.125f), dark);

        var tint = new AdtTintOp(new Vector3(2.0f, 1.0f, 0.5f));
        Vector3 tinted = tint.Apply(new Vector3(0.5f, 0.5f, 0.5f), step with { Amount = 1.0f }, new NoNeighbours());
        Vector3 luma = new(0.299f, 0.587f, 0.114f);
        Assert.AreApproximatelyEqual(0.5, Vector3.Dot(tinted, luma), 1e-4);
        Assert.IsTrue(tinted.X > tinted.Z, "it took the tint's hue");
    }

    private sealed class NoNeighbours : IAdtVertexReader
    {
        public bool TryGet(AdtVertex vertex, out Vector3 value)
        {
            value = default;
            return false;
        }
    }
}
