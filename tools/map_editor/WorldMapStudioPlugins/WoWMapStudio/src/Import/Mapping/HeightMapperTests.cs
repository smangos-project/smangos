using System;

namespace WorldMapStudio.WoW;

public static class HeightMapperTests
{
    private const int Outer = HeightMapper.VertexEdge;
    private const int Inner = HeightMapper.CentreEdge;

    // Distinct, irregular values, so nothing a smoothing read could reproduce by accident survives.
    private static float[] OuterGrid()
    {
        var mcnk = new float[Outer * Outer];
        for (int row = 0; row < Outer; row++)
        {
            for (int col = 0; col < Outer; col++)
            {
                mcnk[(row * Outer) + col] = 100.0f + (row * 13.5f) - (col * col * 2.0f) + ((row * col) * 0.25f);
            }
        }

        return mcnk;
    }

    private static float[] InnerGrid()
    {
        var mcnk = new float[Inner * Inner];
        for (int row = 0; row < Inner; row++)
        {
            for (int col = 0; col < Inner; col++)
            {
                mcnk[(row * Inner) + col] = 500.0f - (row * 7.25f) + (col * col * 1.5f) + ((row * col) * 0.5f);
            }
        }

        return mcnk;
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Texels_belong_to_a_corner_a_centre_or_the_mean_between_two_corners()
    {
        // Along one axis: 4V is corner V, 4V+1 and 4V+2 the centre of cell V, 4V+3 corner V+1.
        float Corner(int x, int y) => (100 * x) + y;
        float Centre(int x, int y) => 10000 + (100 * x) + y;
        float Mean(float a, float b) => (a + b) * 0.5f;
        float At(int tx, int ty) => CellCentreTexels.Texel(tx, ty, Corner, Centre, Mean);

        Assert.AreEqual(Corner(0, 0), At(0, 0));
        Assert.AreEqual(Corner(1, 0), At(3, 0), "texel 3 is corner 1");
        Assert.AreEqual(Corner(1, 0), At(4, 0), "and so is texel 4");
        Assert.AreEqual(Corner(8, 8), At(31, 31));
        Assert.AreEqual(Centre(2, 3), At(9, 13));
        Assert.AreEqual(Centre(2, 3), At(10, 14));
        Assert.AreEqual(Mean(Corner(2, 3), Corner(2, 4)), At(8, 13), "a corner along x, a centre along y");
        Assert.AreEqual(Mean(Corner(2, 3), Corner(3, 3)), At(9, 12), "a centre along x, a corner along y");
    }

    /// <summary>
    /// The channel's bilinear read at every corner and every centre returns the original vertex height
    /// exactly, not a smoothed neighbour average — so a re-export of an untouched imported
    /// tile reproduces MCVT byte for byte.
    /// </summary>
    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Block_reads_back_every_corner_and_centre_height_exactly()
    {
        float[] outer = OuterGrid();
        float[] inner = InnerGrid();
        float[] block = HeightMapper.ChunkBlock(outer, inner);

        for (int y = 0; y < Outer; y++)
        {
            for (int x = 0; x < Outer; x++)
            {
                // Editor-grid corner (x, y) is the MCNK-order value at [(edge-1-x)*edge + y].
                float expected = outer[((Outer - 1 - x) * Outer) + y];
                Assert.AreApproximatelyEqual(expected, HeightMapper.SampleAtVertex(block, x, y), 1e-3, $"corner ({x},{y})");
            }
        }

        for (int y = 0; y < Inner; y++)
        {
            for (int x = 0; x < Inner; x++)
            {
                float expected = inner[((Inner - 1 - x) * Inner) + y];
                Assert.AreApproximatelyEqual(expected, HeightMapper.SampleAtCentre(block, x, y), 1e-3, $"centre ({x},{y})");
            }
        }
    }

    /// <summary>
    /// A raised centre in an otherwise symmetric chunk: the quarter-turn of the 8x8 grid is invisible on flat
    /// terrain and on any grid that is its own rotation, so this one raised cell has to land where the outer
    /// grid's quarter-turn says it does.
    /// </summary>
    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void The_inner_grid_turns_the_same_quarter_as_the_outer_grid()
    {
        var outer = new float[Outer * Outer];
        var inner = new float[Inner * Inner];
        const int row = 1;
        const int col = 5;
        inner[(row * Inner) + col] = 42.0f;
        outer[(row * Outer) + col] = 17.0f;

        float[] block = HeightMapper.ChunkBlock(outer, inner);

        // editor[y][x] = mcnk[edge-1-x][y]: the MCNK entry (row, col) is editor x = edge-1-row, y = col.
        Assert.AreEqual(42.0f, HeightMapper.SampleAtCentre(block, Inner - 1 - row, col));
        Assert.AreEqual(17.0f, HeightMapper.SampleAtVertex(block, Outer - 1 - row, col));

        for (int y = 0; y < Inner; y++)
        {
            for (int x = 0; x < Inner; x++)
            {
                if (x != Inner - 1 - row || y != col)
                {
                    Assert.AreEqual(0.0f, HeightMapper.SampleAtCentre(block, x, y), $"centre ({x},{y}) is untouched");
                }
            }
        }
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Block_is_32x32_and_a_flat_grid_stays_flat()
    {
        var outer = new float[Outer * Outer];
        var inner = new float[Inner * Inner];
        Array.Fill(outer, -37.5f);
        Array.Fill(inner, -37.5f);

        float[] block = HeightMapper.ChunkBlock(outer, inner);

        Assert.AreEqual(32, HeightMapper.TexelEdge);
        Assert.AreEqual(HeightMapper.TexelEdge * HeightMapper.TexelEdge, block.Length);
        foreach (float texel in block)
        {
            Assert.AreApproximatelyEqual(-37.5, texel, 1e-4, "a flat chunk encodes to a flat block, negatives included");
        }
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void A_grid_of_the_wrong_size_is_refused()
    {
        Assert.Throws<ArgumentException>(() => HeightMapper.ChunkBlock(new float[80], new float[Inner * Inner]));
        Assert.Throws<ArgumentException>(() => HeightMapper.ChunkBlock(new float[Outer * Outer], new float[63]));
    }
}
