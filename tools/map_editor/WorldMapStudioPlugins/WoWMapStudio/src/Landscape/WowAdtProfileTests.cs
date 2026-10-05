namespace WorldMapStudio.WoW;

/// <summary>Covers <see cref="WowAdtProfile"/>'s produced settings and its self-registration.</summary>
public static class WowAdtProfileTests
{
    [EditorTest(Category = "WoW Landscape")]
    public static void Settings_validate_with_no_problems()
    {
        LandscapeSettings settings = new WowAdtProfile(null!).CreateSettings();

        Assert.AreEqual(0, settings.Validate().Count);
    }

    [EditorTest(Category = "WoW Landscape")]
    public static void Chunk_world_size_matches_the_water_rasterizers_cell_size()
    {
        LandscapeSettings settings = new WowAdtProfile(null!).CreateSettings();

        Assert.AreApproximatelyEqual(WowTileAddressing.CellSize, settings.ChunkWorldSize, tolerance: 1e-4);
    }
}
