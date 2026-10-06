using System;
namespace WorldMapStudio.WoW;

public static class HoleMapperTests
{
    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Has_any_reflects_whether_a_bit_is_set()
    {
        Assert.IsFalse(HoleMapper.HasAny(new bool[16]));

        var one = new bool[16];
        one[9] = true;
        Assert.IsTrue(HoleMapper.HasAny(one));
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Hole_block_is_255_where_cut_and_rotated_to_editor_axes()
    {
        const int edge = HoleMapper.HoleEdge;

        // Cut the on-disk cell (col=0, row=0) only.
        var holes = new bool[edge * edge];
        holes[0] = true;

        byte[] block = HoleMapper.HoleBlock(holes);

        // editor[y][x] = wow[edge-1-x][y]; the on-disk cell (0,0) is wow[row=0][col=0], which lands at
        // editor (x = edge-1, y = 0).
        Assert.AreEqual((byte)255, block[(0 * edge) + (edge - 1)]);
        Assert.AreEqual((byte)0, block[0]);

        int set = 0;
        foreach (byte b in block)
        {
            if (b != 0)
            {
                set++;
            }
        }

        Assert.AreEqual(1, set, "exactly one cell is cut");
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void FromBlock_inverts_HoleBlock()
    {
        var holes = new bool[HoleMapper.HoleEdge * HoleMapper.HoleEdge];
        holes[1] = holes[6] = holes[11] = true;
        Assert.IsTrue(holes.AsSpan().SequenceEqual(HoleMapper.FromBlock(HoleMapper.HoleBlock(holes))), "holes round trip");
    }
}
