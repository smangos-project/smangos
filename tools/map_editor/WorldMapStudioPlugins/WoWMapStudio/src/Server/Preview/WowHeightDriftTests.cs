namespace WorldMapStudio.WoW;

public static class WowHeightDriftTests
{
    [EditorTest(Category = "WoW Server", Thread = TestThread.Background)]
    public static void Matching_terrain_has_zero_drift()
    {
        var drift = WowHeightDrift.Measure(new WowMapTile { GridHeight = 10.0f }, (_, _) => 10.0f, 0.01f);

        Assert.AreEqual(129 * 129, drift.Samples);
        Assert.IsTrue(drift.IsZero);
    }

    [EditorTest(Category = "WoW Server", Thread = TestThread.Background)]
    public static void Raised_vertex_is_reported()
    {
        var drift = WowHeightDrift.Measure(new WowMapTile { GridHeight = 10.0f }, (r, c) => r == 3 && c == 4 ? 12.0f : 10.0f, 0.01f);

        Assert.AreEqual(1, drift.Drifted);
        Assert.IsTrue(System.MathF.Abs(drift.MaxAbs - 2.0f) < 0.001f);
    }
}
