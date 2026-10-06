using Godot;

namespace WorldMapStudio.WoW;

public static class RiverSurfaceMeshFunctionTests
{
    [EditorTest(Category = "WoW Water", Thread = TestThread.Main)]
    public static void Straight_river_has_constant_width_and_length()
    {
        var network = new VertexNetwork();
        int a = network.AddVertex(new Vector3(0.0f, 0.0f, 0.0f));
        int b = network.AddVertex(new Vector3(10.0f, 0.0f, 0.0f));
        network.AddEdge(a, b);

        var values = new MeshParameterValues();
        values.Set(RiverSurfaceMeshFunction.Width, 4.0f);
        var output = new ProceduralOutputBuilder();
        new RiverSurfaceMeshFunction(null!).Build(new ProceduralBuildContext(network, values, null!), output);

        ProceduralBuildResult result = output.Build([RiverSurfaceMeshFunction.Output], _ => MeshModelFormat.FormatId);
        Assert.AreEqual(1, result.Models.Count);
        ModelAsset built = result.Models[0].Asset;
        Assert.AreApproximatelyEqual(10.0, built.LocalBounds.Size.X, 0.1, "the ribbon should span the centreline's length");
        Assert.AreApproximatelyEqual(4.0, built.LocalBounds.Size.Z, 0.1, "a straight river's banks should sit exactly width/2 either side");
        Assert.AreApproximatelyEqual(0.0, built.LocalBounds.Size.Y, 1e-4, "a flat river should have no vertical extent");
    }

    [EditorTest(Category = "WoW Water", Thread = TestThread.Main)]
    public static void Sloped_river_keeps_its_authored_height_instead_of_flattening_it()
    {
        var network = new VertexNetwork();
        int a = network.AddVertex(new Vector3(0.0f, 5.0f, 0.0f));
        int b = network.AddVertex(new Vector3(10.0f, 0.0f, 0.0f));
        network.AddEdge(a, b);

        var values = new MeshParameterValues();
        var output = new ProceduralOutputBuilder();
        new RiverSurfaceMeshFunction(null!).Build(new ProceduralBuildContext(network, values, null!), output);

        ProceduralBuildResult result = output.Build([RiverSurfaceMeshFunction.Output], _ => MeshModelFormat.FormatId);
        ModelAsset built = result.Models[0].Asset;
        Assert.AreApproximatelyEqual(5.0, built.LocalBounds.Size.Y, 0.1,
            "a road-style flatten-to-zero would report 0 here; a river must keep its authored slope");
    }

    [EditorTest(Category = "WoW Water", Thread = TestThread.Main)]
    public static void Closed_loop_river_wraps_without_a_gap()
    {
        var network = new VertexNetwork();
        int a = network.AddVertex(new Vector3(0.0f, 0.0f, 0.0f));
        int b = network.AddVertex(new Vector3(10.0f, 0.0f, 0.0f));
        int c = network.AddVertex(new Vector3(5.0f, 0.0f, 8.0f));
        network.AddEdge(a, b);
        network.AddEdge(b, c);
        network.AddEdge(c, a);

        var values = new MeshParameterValues();
        values.Set(RiverSurfaceMeshFunction.Width, 2.0f);
        var output = new ProceduralOutputBuilder();
        new RiverSurfaceMeshFunction(null!).Build(new ProceduralBuildContext(network, values, null!), output);

        ProceduralBuildResult result = output.Build([RiverSurfaceMeshFunction.Output], _ => MeshModelFormat.FormatId);
        Assert.AreEqual(1, result.Models.Count);
        ModelAsset built = result.Models[0].Asset;
        Assert.AreEqual(1, built.Surfaces[0].Mesh.GetSurfaceCount());
        Assert.Greater(built.LocalBounds.Size.X, 0.0);
        Assert.Greater(built.LocalBounds.Size.Z, 0.0);
    }
}
