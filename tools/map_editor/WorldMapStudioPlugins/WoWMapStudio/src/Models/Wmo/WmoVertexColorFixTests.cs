using Godot;

namespace WorldMapStudio.WoW;

public static class WmoVertexColorFixTests
{
    private static readonly WmoBatch[] NoBatches = [];

    [EditorTest(Category = "WoW", Thread = TestThread.Background)]
    public static void Exterior_group_rebases_rgb_and_forces_alpha_to_one()
    {
        var colors = new[] { new Color(0.05f, 0.05f, 0.05f, 0.5f) };
        var lighting = new WmoRootLighting(new Color(0.1f, 0.1f, 0.1f, 1f), SkipBaseColor: false, LightenInteriors: false);

        WmoVertexColorFix.Apply(colors, NoBatches, transBatchCount: 0, exterior: true, exteriorLit: false, lighting);

        // (r*a*255/64 + r - base) / 2 == (0.0996 + 0.05 - 0.1) / 2
        Assert.AreApproximatelyEqual(0.0248f, colors[0].R, 1e-3);
        Assert.AreEqual(1.0f, colors[0].A);
    }

    [EditorTest(Category = "WoW", Thread = TestThread.Background)]
    public static void Interior_group_forces_alpha_to_zero()
    {
        var colors = new[] { new Color(0.05f, 0.05f, 0.05f, 0.5f) };
        var lighting = new WmoRootLighting(new Color(0.1f, 0.1f, 0.1f, 1f), SkipBaseColor: false, LightenInteriors: false);

        WmoVertexColorFix.Apply(colors, NoBatches, transBatchCount: 0, exterior: false, exteriorLit: false, lighting);

        Assert.AreEqual(0.0f, colors[0].A);
    }

    [EditorTest(Category = "WoW", Thread = TestThread.Background)]
    public static void Rgb_never_goes_negative()
    {
        var colors = new[] { new Color(0.0f, 0.0f, 0.0f, 0.0f) };
        var lighting = new WmoRootLighting(new Color(0.3f, 0.3f, 0.3f, 1f), SkipBaseColor: false, LightenInteriors: false);

        WmoVertexColorFix.Apply(colors, NoBatches, transBatchCount: 0, exterior: true, exteriorLit: false, lighting);

        Assert.AreEqual(0.0f, colors[0].R);
        Assert.AreEqual(0.0f, colors[0].G);
        Assert.AreEqual(0.0f, colors[0].B);
    }

    [EditorTest(Category = "WoW", Thread = TestThread.Background)]
    public static void Lighten_interiors_keeps_raw_rgb_and_only_sets_alpha()
    {
        var colors = new[] { new Color(0.4f, 0.4f, 0.4f, 0.7f) };
        var lighting = new WmoRootLighting(new Color(0.1f, 0.1f, 0.1f, 1f), SkipBaseColor: false, LightenInteriors: true);

        WmoVertexColorFix.Apply(colors, NoBatches, transBatchCount: 0, exterior: true, exteriorLit: false, lighting);

        Assert.AreApproximatelyEqual(0.4f, colors[0].R, 1e-6);
        Assert.AreEqual(1.0f, colors[0].A);
    }

    [EditorTest(Category = "WoW", Thread = TestThread.Background)]
    public static void Skip_base_color_does_not_subtract_root_ambient()
    {
        var colors = new[] { new Color(0.05f, 0.05f, 0.05f, 0.5f) };
        var lighting = new WmoRootLighting(new Color(0.1f, 0.1f, 0.1f, 1f), SkipBaseColor: true, LightenInteriors: false);

        WmoVertexColorFix.Apply(colors, NoBatches, transBatchCount: 0, exterior: true, exteriorLit: false, lighting);

        // (0.05*0.5*255/64 + 0.05) / 2
        Assert.AreApproximatelyEqual(0.0748f, colors[0].R, 1e-3);
    }

    [EditorTest(Category = "WoW", Thread = TestThread.Background)]
    public static void Transition_vertices_use_the_first_block_formula_and_keep_their_alpha()
    {
        var colors = new[]
        {
            new Color(0.6f, 0.6f, 0.6f, 0.25f), // transition (batch 0 ends at vertex 1)
            new Color(0.6f, 0.6f, 0.6f, 0.25f),
            new Color(0.05f, 0.05f, 0.05f, 0.5f), // exterior
            new Color(0.05f, 0.05f, 0.05f, 0.5f),
        };
        var batches = new[] { new WmoBatch(0u, 6, 0, 1, 0) };
        var lighting = new WmoRootLighting(new Color(0.1f, 0.1f, 0.1f, 1f), SkipBaseColor: false, LightenInteriors: false);

        WmoVertexColorFix.Apply(colors, batches, transBatchCount: 1, exterior: true, exteriorLit: false, lighting);

        // transition: ((0.6-0.1) - 0.25*(0.6-0.1)) / 2 == 0.375 / 2
        Assert.AreApproximatelyEqual(0.1875f, colors[0].R, 1e-3);
        Assert.AreEqual(0.25f, colors[0].A, "transition alpha is preserved");

        // exterior tail: same as the exterior-group case
        Assert.AreApproximatelyEqual(0.0248f, colors[2].R, 1e-3);
        Assert.AreEqual(1.0f, colors[2].A);
    }
}
