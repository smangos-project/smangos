using Godot;

namespace WorldMapStudio.WoW;

public static class VertexColorMapperTests
{
    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Has_flags_ignore_the_neutral_start_colour()
    {
        // MCCV's on-disk neutral is mid-grey (0x7F/255), not white — the reader no longer decodes it.
        const float neutralByte = 0x7F / 255.0f;
        var neutral = new Color[81];
        System.Array.Fill(neutral, new Color(neutralByte, neutralByte, neutralByte, 1.0f));
        Assert.IsFalse(VertexColorMapper.HasVertexColor(neutral));

        var black = new Color[81];
        System.Array.Fill(black, Colors.Black);
        Assert.IsFalse(VertexColorMapper.HasVertexLight(black));

        neutral[40] = new Color(0.5f, 0.5f, 0.5f);
        Assert.IsTrue(VertexColorMapper.HasVertexColor(neutral));
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Colour_block_forces_opaque_and_reads_back_every_corner_and_centre()
    {
        const int edge = 9;
        const int cells = 8;
        var outer = new Color[edge * edge];
        for (int row = 0; row < edge; row++)
        {
            for (int col = 0; col < edge; col++)
            {
                outer[(row * edge) + col] = new Color(row / 8.0f, col / 8.0f, 0.25f, 0.3f);
            }
        }

        var inner = new Color[cells * cells];
        for (int row = 0; row < cells; row++)
        {
            for (int col = 0; col < cells; col++)
            {
                inner[(row * cells) + col] = new Color(0.9f - (row / 20.0f), 0.1f + (col / 12.0f), (row * col) / 64.0f, 0.3f);
            }
        }

        byte[] block = VertexColorMapper.ColorBlock(outer, inner);

        Assert.AreEqual(32 * 32 * 4, block.Length);

        // Editor corner (x, y) reads MCNK order [(edge-1-x)*edge + y]; a cell centre the same over 8.
        for (int y = 0; y < edge; y++)
        {
            for (int x = 0; x < edge; x++)
            {
                Color expected = outer[((edge - 1 - x) * edge) + y];
                Assert.AreEqual((byte)System.Math.Round(expected.R * 255.0f), VertexColorMapper.SampleAtVertex(block, 0, x, y), $"R at corner {x},{y}");
                Assert.AreEqual((byte)System.Math.Round(expected.G * 255.0f), VertexColorMapper.SampleAtVertex(block, 1, x, y), $"G at corner {x},{y}");
                Assert.AreEqual(255.0f, VertexColorMapper.SampleAtVertex(block, 3, x, y), "alpha forced opaque so the paint weight is 1");
            }
        }

        for (int y = 0; y < cells; y++)
        {
            for (int x = 0; x < cells; x++)
            {
                Color expected = inner[((cells - 1 - x) * cells) + y];
                Assert.AreEqual((byte)System.Math.Round(expected.R * 255.0f), VertexColorMapper.SampleAtCentre(block, 0, x, y), $"R at centre {x},{y}");
                Assert.AreEqual((byte)System.Math.Round(expected.B * 255.0f), VertexColorMapper.SampleAtCentre(block, 2, x, y), $"B at centre {x},{y}");
            }
        }
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void A_missing_inner_grid_takes_the_mean_of_each_cells_corners()
    {
        var outer = new Color[81];
        for (int i = 0; i < outer.Length; i++)
        {
            outer[i] = new Color(0.2f + (i % 9) / 40.0f, 0.5f, 0.5f);
        }

        byte[] block = VertexColorMapper.ColorBlock(outer);

        // Editor centre (2, 3): its four corners are MCNK entries [(8-x)*9 + y] over x in 2..3, y in 3..4.
        float sum = 0.0f;
        foreach ((int x, int y) in new[] { (2, 3), (3, 3), (2, 4), (3, 4) })
        {
            sum += outer[((8 - x) * 9) + y].R;
        }

        Assert.AreEqual((byte)System.Math.Round(sum * 0.25f * 255.0f), VertexColorMapper.SampleAtCentre(block, 0, 2, 3));
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void The_has_flags_see_a_centre_that_differs_when_every_corner_is_neutral()
    {
        const float neutralByte = 0x7F / 255.0f;
        var neutral = new Color[81];
        System.Array.Fill(neutral, new Color(neutralByte, neutralByte, neutralByte, 1.0f));
        var inner = new Color[64];
        System.Array.Fill(inner, new Color(neutralByte, neutralByte, neutralByte, 1.0f));
        Assert.IsFalse(VertexColorMapper.HasVertexColor(neutral, inner));

        inner[10] = new Color(0.9f, neutralByte, neutralByte, 1.0f);
        Assert.IsTrue(VertexColorMapper.HasVertexColor(neutral, inner));

        var light = new Color[64];
        Assert.IsFalse(VertexColorMapper.HasVertexLight(new Color[81], light));
        light[3] = new Color(0.1f, 0.0f, 0.0f);
        Assert.IsTrue(VertexColorMapper.HasVertexLight(new Color[81], light));
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Light_block_leaves_alpha_zero()
    {
        var outer = new Color[81];
        System.Array.Fill(outer, new Color(0.4f, 0.6f, 0.8f));

        byte[] block = VertexColorMapper.LightBlock(outer);

        Assert.AreEqual(32 * 32 * 4, block.Length);
        for (int i = 0; i < block.Length; i += 4)
        {
            Assert.AreEqual((byte)0, block[i + 3]);
        }
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void From_block_inverts_the_colour_block()
    {
        var outer = new Color[81];
        var inner = new Color[64];
        for (int i = 0; i < outer.Length; i++)
        {
            outer[i] = new Color((i * 3 % 256) / 255.0f, (i * 7 % 256) / 255.0f, (i * 11 % 256) / 255.0f, 1.0f);
        }

        for (int i = 0; i < inner.Length; i++)
        {
            inner[i] = new Color((i * 5 % 256) / 255.0f, (i * 13 % 256) / 255.0f, (i * 17 % 256) / 255.0f, 1.0f);
        }

        (Color[] o, Color[] n) = VertexColorMapper.FromBlock(VertexColorMapper.ColorBlock(outer, inner));

        for (int i = 0; i < outer.Length; i++)
        {
            Assert.AreEqual(outer[i], o[i], $"outer {i}");
        }

        for (int i = 0; i < inner.Length; i++)
        {
            Assert.AreEqual(inner[i], n[i], $"inner {i}");
        }
    }
}
