using System;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Covers <see cref="AdtBlendSpace"/> and <see cref="AdtChunkTextureSet"/>.</summary>
public static class AdtChunkTextureSetTests
{
    private const int Edge = AdtChunkTextureSet.Edge;
    private const float Tolerance = 3.0f / 255.0f;

    private static readonly BrushGrid Grid = new(0.0f, 0.0f, 0.5f, 0.5f, Edge, Edge);

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void Weights_of_any_alphas_sum_to_one_and_convert_back()
    {
        var random = new Random(11);
        Span<float> weights = stackalloc float[4];
        Span<float> again = stackalloc float[4];
        for (int i = 0; i < 2000; i++)
        {
            byte a1 = (byte)random.Next(256);
            byte a2 = (byte)random.Next(256);
            byte a3 = (byte)random.Next(256);

            AdtBlendSpace.ToWeights(a1, a2, a3, weights);
            Assert.AreApproximatelyEqual(1.0, weights[0] + weights[1] + weights[2] + weights[3], 1e-5);

            AdtBlendSpace.ToAlphas(weights, out byte b1, out byte b2, out byte b3);
            AdtBlendSpace.ToWeights(b1, b2, b3, again);
            for (int layer = 0; layer < 4; layer++)
            {
                Assert.IsTrue(MathF.Abs(weights[layer] - again[layer]) <= Tolerance,
                    $"alphas {a1},{a2},{a3} layer {layer}: {weights[layer]} vs {again[layer]}");
            }
        }
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void A_top_layer_that_is_fully_opaque_hides_the_alphas_below_it()
    {
        Span<float> weights = stackalloc float[4];
        AdtBlendSpace.ToWeights(200, 90, 255, weights);
        AdtBlendSpace.ToAlphas(weights, out byte a1, out byte a2, out byte a3);

        Assert.AreEqual((byte)255, a3);
        Assert.AreEqual((byte)0, a2);
        Assert.AreEqual((byte)0, a1);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void An_untouched_chunk_stores_back_byte_for_byte()
    {
        foreach (int count in new[] { 1, 2, 3, 4 })
        {
            byte[] block = RandomBlock(count, seed: 5 + count);
            AdtStoredChunk stored = AdtChunkTextureSet.FromStored(Names(count), block).ToStored();

            Assert.IsTrue(stored.Rgba.SequenceEqual(block), $"{count} layers");
            Assert.IsFalse(stored.SlotsChanged);
            Assert.AreEqual(count, stored.Textures.Count);
        }
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void Painting_only_changes_the_texels_under_the_brush()
    {
        byte[] block = RandomBlock(3, seed: 21);
        AdtChunkTextureSet set = AdtChunkTextureSet.FromStored(Names(3), block);

        bool changed = set.Paint(2, Grid, Dab(new Vector3(8, 0, 8), 4.0f, strength: 1.0f), 1.0f);
        AdtStoredChunk stored = set.ToStored();

        Assert.IsTrue(changed);
        Assert.IsFalse(stored.SlotsChanged);
        int same = 0;
        for (int i = 0; i < Edge * Edge; i++)
        {
            int x = i % Edge;
            int y = i / Edge;
            float distance = MathF.Sqrt((((x + 0.5f) * 0.5f) - 8) * (((x + 0.5f) * 0.5f) - 8) + (((y + 0.5f) * 0.5f) - 8) * (((y + 0.5f) * 0.5f) - 8));
            if (distance > 4.5f)
            {
                Assert.IsTrue(stored.Rgba.AsSpan(i * 4, 4).SequenceEqual(block.AsSpan(i * 4, 4)), $"texel {x},{y} is outside the brush");
                same++;
            }
        }

        Assert.IsTrue(same > 3000);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void Painted_texels_keep_summing_to_one_and_reach_the_target()
    {
        AdtChunkTextureSet set = AdtChunkTextureSet.FromStored(Names(3), RandomBlock(3, seed: 33));

        Assert.IsTrue(set.Paint(1, Grid, Dab(new Vector3(16.25f, 0, 16.25f), 6.0f, strength: 1.0f), 0.6f));

        for (int y = 0; y < Edge; y++)
        {
            for (int x = 0; x < Edge; x++)
            {
                Assert.AreApproximatelyEqual(1.0, Sum(set, x, y), 1e-4, $"texel {x},{y}");
            }
        }

        Assert.AreApproximatelyEqual(0.6, set.WeightAt(1, 32, 32), 1e-4);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void Repeated_dabs_converge_on_the_target_without_overshooting()
    {
        AdtChunkTextureSet set = AdtChunkTextureSet.FromStored(Names(2), new byte[Edge * Edge * 4]);
        BrushDab dab = Dab(new Vector3(16.25f, 0, 16.25f), 6.0f, strength: 0.4f);

        float previous = 0.0f;
        for (int i = 0; i < 30; i++)
        {
            set.Paint(1, Grid, dab, 0.5f);
            float now = set.WeightAt(1, 32, 32);
            Assert.IsTrue(now >= previous - 1e-6f && now <= 0.5f + 1e-4f, $"dab {i}: {now}");
            previous = now;
        }

        Assert.AreApproximatelyEqual(0.5, previous, 0.01);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void An_inverted_dab_paints_towards_zero_and_gives_the_weight_to_the_others()
    {
        var block = new byte[Edge * Edge * 4];
        for (int i = 0; i < Edge * Edge; i++)
        {
            block[i * 4] = 255;
        }

        AdtChunkTextureSet set = AdtChunkTextureSet.FromStored(Names(2), block);
        var dab = new BrushDab(new Vector3(16.25f, 0, 16.25f), 6.0f, 1.0f, Invert: true, 0.0f);

        set.Paint(1, Grid, dab, 1.0f);

        Assert.AreApproximatelyEqual(0.0, set.WeightAt(1, 32, 32), 1e-4);
        Assert.AreApproximatelyEqual(1.0, set.WeightAt(0, 32, 32), 1e-4);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void A_new_texture_is_appended_and_a_zero_strength_stroke_never_adds_one()
    {
        AdtChunkTextureSet set = AdtChunkTextureSet.FromStored(Names(2), RandomBlock(2, seed: 3));

        Assert.AreEqual(-1, set.GetOrAdd("new", 0.0f));
        Assert.AreEqual(2, set.Count);

        Assert.AreEqual(2, set.GetOrAdd("new", 0.5f));
        Assert.AreEqual(3, set.Count);
        Assert.IsTrue(set.SlotsChanged);
        Assert.AreEqual(2, set.GetOrAdd("NEW", 0.5f), "texture keys compare case-insensitively");
        Assert.AreEqual(0.0f, set.MaxWeight(2));
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void A_full_chunk_frees_a_slot_from_an_unused_layer_or_refuses()
    {
        var block = new byte[Edge * Edge * 4];
        for (int i = 0; i < Edge * Edge; i++)
        {
            block[(i * 4) + 0] = 128;
            block[(i * 4) + 1] = 0;
            block[(i * 4) + 2] = 128;
        }

        AdtChunkTextureSet withUnused = AdtChunkTextureSet.FromStored(Names(4), block);
        Assert.AreEqual(0.0f, withUnused.MaxWeight(2), "slot 2 has no alpha of its own here");
        Assert.IsTrue(withUnused.CanAdd("fifth"));
        int slot = withUnused.GetOrAdd("fifth", 1.0f);
        Assert.AreEqual(3, slot);
        Assert.AreEqual("fifth", withUnused.Textures[slot]);

        AdtChunkTextureSet full = AdtChunkTextureSet.FromStored(Names(4), RandomBlock(4, seed: 9));
        Assert.IsFalse(full.HasUnused(AdtChunkTextureSet.VisibleThreshold));
        Assert.IsFalse(full.CanAdd("fifth"));
        Assert.AreEqual(-1, full.GetOrAdd("fifth", 1.0f));
        Assert.AreEqual(4, full.Count);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void Erasing_unused_layers_shifts_the_ones_above_down()
    {
        var block = new byte[Edge * Edge * 4];
        for (int i = 0; i < Edge * Edge; i++)
        {
            block[(i * 4) + 0] = 0;
            block[(i * 4) + 1] = 100;
        }

        AdtChunkTextureSet set = AdtChunkTextureSet.FromStored(Names(3), block);

        Assert.IsTrue(set.EraseUnused(AdtChunkTextureSet.VisibleThreshold));

        Assert.AreEqual(2, set.Count);
        Assert.AreEqual("t0", set.Textures[0]);
        Assert.AreEqual("t2", set.Textures[1]);
        Assert.AreApproximatelyEqual(1.0, Sum(set, 5, 5), 1e-4);
        Assert.IsTrue(set.SlotsChanged);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void Removing_the_base_promotes_the_layer_above_and_keeps_its_share()
    {
        AdtChunkTextureSet set = AdtChunkTextureSet.FromStored(Names(3), RandomBlock(3, seed: 40));
        float layer1 = set.WeightAt(1, 10, 10);
        float layer2 = set.WeightAt(2, 10, 10);
        float removed = set.WeightAt(0, 10, 10);

        Assert.IsTrue(set.Remove(0));

        Assert.AreEqual(2, set.Count);
        Assert.AreEqual("t1", set.Textures[0]);
        float others = layer1 + layer2;
        Assert.AreApproximatelyEqual(layer1 + (removed * layer1 / others), set.WeightAt(0, 10, 10), 1e-4);
        Assert.AreApproximatelyEqual(1.0, Sum(set, 10, 10), 1e-4);

        AdtStoredChunk stored = set.ToStored();
        Assert.AreEqual((byte)0, stored.Rgba[(((10 * Edge) + 10) * 4) + 1], "the freed channel is cleared");
        Assert.IsTrue(stored.SlotsChanged);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void The_last_layer_cannot_be_removed()
    {
        AdtChunkTextureSet set = AdtChunkTextureSet.FromStored(Names(1), new byte[Edge * Edge * 4]);

        Assert.IsFalse(set.Remove(0));
        Assert.AreEqual(1, set.Count);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void Swapping_layers_changes_the_stacking_but_not_how_each_texture_looks()
    {
        AdtChunkTextureSet set = AdtChunkTextureSet.FromStored(Names(4), RandomBlock(4, seed: 50));
        float[] before = Enumerable.Range(0, 4).Select(slot => set.WeightAt(slot, 20, 7)).ToArray();

        Assert.IsTrue(set.Swap(1, 3));
        Assert.AreEqual("t3", set.Textures[1]);
        Assert.AreEqual("t1", set.Textures[3]);

        AdtStoredChunk stored = set.ToStored();
        AdtChunkTextureSet reloaded = AdtChunkTextureSet.FromStored(stored.Textures, stored.Rgba);
        for (int texture = 0; texture < 4; texture++)
        {
            int slot = reloaded.IndexOf($"t{texture}");
            Assert.IsTrue(MathF.Abs(reloaded.WeightAt(slot, 20, 7) - before[texture]) <= Tolerance,
                $"t{texture}: {before[texture]} vs {reloaded.WeightAt(slot, 20, 7)}");
        }
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void Replacing_a_texture_renames_the_layer_or_merges_into_an_existing_one()
    {
        AdtChunkTextureSet renamed = AdtChunkTextureSet.FromStored(Names(3), RandomBlock(3, seed: 60));
        Assert.IsTrue(renamed.Replace("t1", "grass"));
        Assert.AreEqual("grass", renamed.Textures[1]);
        Assert.AreEqual(3, renamed.Count);

        AdtChunkTextureSet merged = AdtChunkTextureSet.FromStored(Names(3), RandomBlock(3, seed: 60));
        float expected = merged.WeightAt(0, 4, 4) + merged.WeightAt(2, 4, 4);
        Assert.IsTrue(merged.Replace("t2", "t0"));
        Assert.AreEqual(2, merged.Count);
        Assert.AreApproximatelyEqual(expected, merged.WeightAt(0, 4, 4), 1e-4);

        Assert.IsFalse(merged.Replace("missing", "t0"));
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void A_masked_replace_moves_only_the_masked_texels()
    {
        AdtChunkTextureSet set = AdtChunkTextureSet.FromStored(Names(2), RandomBlock(2, seed: 70));
        var mask = new bool[Edge * Edge];
        for (int x = 0; x < 32; x++)
        {
            mask[(10 * Edge) + x] = true;
        }

        float outside = set.WeightAt(1, 50, 10);
        float inside = set.WeightAt(1, 5, 10);

        Assert.IsTrue(set.Replace("t1", "rock", mask));

        int rock = set.IndexOf("rock");
        Assert.AreEqual(2, rock);
        Assert.AreApproximatelyEqual(inside, set.WeightAt(rock, 5, 10), 1e-5);
        Assert.AreEqual(0.0f, set.WeightAt(1, 5, 10));
        Assert.AreEqual(outside, set.WeightAt(1, 50, 10));
        Assert.AreEqual(0.0f, set.WeightAt(rock, 50, 10));
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void Duplicate_layers_are_merged()
    {
        AdtChunkTextureSet set = AdtChunkTextureSet.FromStored(["a", "b", "A"], RandomBlock(3, seed: 80));
        float expected = set.WeightAt(0, 9, 9) + set.WeightAt(2, 9, 9);

        Assert.AreEqual(1, set.RemoveDuplicates());

        Assert.AreEqual(2, set.Count);
        Assert.AreApproximatelyEqual(expected, set.WeightAt(0, 9, 9), 1e-4);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void A_slot_change_re_encodes_every_texel_and_reloads_to_the_same_look()
    {
        AdtChunkTextureSet set = AdtChunkTextureSet.FromStored(Names(4), RandomBlock(4, seed: 90));
        float[,] before = new float[4, 3];
        (int X, int Y)[] probes = [(0, 0), (31, 40), (63, 63)];
        for (int p = 0; p < probes.Length; p++)
        {
            for (int slot = 0; slot < 4; slot++)
            {
                before[slot, p] = set.WeightAt(slot, probes[p].X, probes[p].Y);
            }
        }

        set.Remove(2);
        AdtStoredChunk stored = set.ToStored();
        AdtChunkTextureSet reloaded = AdtChunkTextureSet.FromStored(stored.Textures, stored.Rgba);

        Assert.AreEqual(3, reloaded.Count);
        Assert.AreEqual((byte)0, stored.Rgba[2], "the top channel is unused now");
        for (int p = 0; p < probes.Length; p++)
        {
            for (int slot = 0; slot < 3; slot++)
            {
                Assert.IsTrue(MathF.Abs(reloaded.WeightAt(slot, probes[p].X, probes[p].Y) - set.WeightAt(slot, probes[p].X, probes[p].Y)) <= Tolerance,
                    $"probe {p} slot {slot}");
            }
        }
    }

    private static BrushDab Dab(Vector3 centre, float radius, float strength) =>
        new(centre, radius, strength, Invert: false, Hardness: 0.0f);

    private static string[] Names(int count) => Enumerable.Range(0, count).Select(i => $"t{i}").ToArray();

    private static float Sum(AdtChunkTextureSet set, int x, int y)
    {
        float sum = 0.0f;
        for (int slot = 0; slot < set.Count; slot++)
        {
            sum += set.WeightAt(slot, x, y);
        }

        return sum;
    }

    // Random alphas for slots 1..count-1 only, like a real chunk: unused channels are zero.
    private static byte[] RandomBlock(int count, int seed)
    {
        var random = new Random(seed);
        var block = new byte[Edge * Edge * 4];
        for (int i = 0; i < Edge * Edge; i++)
        {
            for (int channel = 0; channel < count - 1; channel++)
            {
                block[(i * 4) + channel] = (byte)random.Next(256);
            }
        }

        return block;
    }
}
