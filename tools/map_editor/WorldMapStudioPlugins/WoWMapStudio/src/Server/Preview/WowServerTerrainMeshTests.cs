namespace WorldMapStudio.WoW;

public static class WowServerTerrainMeshTests
{
    [EditorTest(Category = "WoW Server", Thread = TestThread.Background)]
    public static void Flat_tile_builds_a_line_grid_at_its_height()
    {
        var mesh = WowServerTerrainMesh.Build(new WowMapTile { GridHeight = 10.0f }, 32, 48);

        Assert.AreEqual(33 * 33, mesh.Vertices.Length);
        Assert.AreEqual(2 * 32 * 33 * 2, mesh.Indices!.Length);
    }
}
