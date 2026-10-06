using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Pins how the landscape build samples the imported vertex rasters, which is what makes a vertex on the
/// border of the imported area depend on the padding around it.</summary>
public static class AdtVertexSamplingTests
{
    private const string HeightChannel = "adt_height";
    private const int Edge = AdtVertexLattice.Edge;

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Background)]
    public static void A_vertex_on_the_border_of_the_imported_area_samples_half_its_height_unless_the_padding_holds_it_too()
    {
        LandscapeSettings settings = new WowAdtProfile(null!).CreateSettings();
        var channel = new LandscapeChannel { Name = HeightChannel, Resolution = Edge, BitDepth = 32, Components = 1 };
        var pool = new LandscapeChannelPool(settings, [channel]);
        ChunkCoord chunk = pool.Grid.CoordAt(new Vector3(1.0f, 0.0f, 1.0f));
        float step = pool.Grid.ChunkSize / AdtVertex.CellsPerChunk;
        Vector3 origin = pool.Grid.OriginOf(chunk);

        float[] own = pool.Buffer(HeightChannel, chunk)!;
        for (int i = 0; i < own.Length; i++)
        {
            own[i] = 40.0f;
        }

        Vector3 eastEdge = origin + new Vector3(AdtVertex.CellsPerChunk * step, 0.0f, 4 * step);
        Vector3 inside = origin + new Vector3(3 * step, 0.0f, 4 * step);
        Assert.AreEqual(40.0f, pool.Sample(HeightChannel, inside), "an interior vertex reads its own texels");
        Assert.AreApproximatelyEqual(20.0f, pool.Sample(HeightChannel, eastEdge), 1e-4, "the neighbour holds nothing, so half");

        float[] neighbour = pool.Buffer(HeightChannel, new ChunkCoord(chunk.X + 1, chunk.Y))!;
        for (int y = 0; y < Edge; y++)
        {
            neighbour[y * Edge] = 40.0f;
        }

        Assert.AreApproximatelyEqual(40.0f, pool.Sample(HeightChannel, eastEdge), 1e-4, "with the padding written it reads whole");
    }
}
