using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Covers <see cref="WaterSurfaceMeshFunction"/>'s two departures from the core
/// <c>PanelNetworkMeshFunction</c> it is modeled on: authored height is preserved rather than
/// flattened, and UV comes from a shared world-space XZ projection rather than each face's own basis.</summary>
public static class WaterSurfaceMeshFunctionTests
{
    [EditorTest(Category = "WoW Water", Thread = TestThread.Main)]
    public static void Flat_face_builds_a_single_surface_with_no_height()
    {
        var network = new VertexNetwork();
        int a = network.AddVertex(new Vector3(0.0f, 0.0f, 0.0f));
        int b = network.AddVertex(new Vector3(2.0f, 0.0f, 0.0f));
        int c = network.AddVertex(new Vector3(2.0f, 0.0f, 2.0f));
        int d = network.AddVertex(new Vector3(0.0f, 0.0f, 2.0f));
        network.AddFace([a, b, c, d]);

        var values = new MeshParameterValues();
        var output = new ProceduralOutputBuilder();
        new WaterSurfaceMeshFunction(null!).Build(new ProceduralBuildContext(network, values, null!), output);

        ProceduralBuildResult result = output.Build([WaterSurfaceMeshFunction.Output], _ => MeshModelFormat.FormatId);
        Assert.AreEqual(1, result.Models.Count);
        ModelAsset built = result.Models[0].Asset;
        Assert.AreEqual(1, built.Surfaces.Count);
        Assert.AreApproximatelyEqual(2.0, built.LocalBounds.Size.X, 1e-4);
        Assert.AreApproximatelyEqual(2.0, built.LocalBounds.Size.Z, 1e-4);
        Assert.AreApproximatelyEqual(0.0, built.LocalBounds.Size.Y, 1e-4);
    }

    [EditorTest(Category = "WoW Water", Thread = TestThread.Main)]
    public static void Sloped_face_keeps_its_authored_height_instead_of_flattening_it()
    {
        var network = new VertexNetwork();
        int a = network.AddVertex(new Vector3(0.0f, 0.0f, 0.0f));
        int b = network.AddVertex(new Vector3(2.0f, 1.0f, 0.0f));
        int c = network.AddVertex(new Vector3(2.0f, 1.0f, 2.0f));
        int d = network.AddVertex(new Vector3(0.0f, 0.0f, 2.0f));
        network.AddFace([a, b, c, d]);

        var values = new MeshParameterValues();
        var output = new ProceduralOutputBuilder();
        new WaterSurfaceMeshFunction(null!).Build(new ProceduralBuildContext(network, values, null!), output);

        ProceduralBuildResult result = output.Build([WaterSurfaceMeshFunction.Output], _ => MeshModelFormat.FormatId);
        ModelAsset built = result.Models[0].Asset;
        Assert.AreApproximatelyEqual(1.0, built.LocalBounds.Size.Y, 1e-4,
            "a road/panel-style flatten-to-zero would report 0 here; water must keep the authored slope");
    }

    [EditorTest(Category = "WoW Water", Thread = TestThread.Main)]
    public static void Uv_is_a_shared_world_space_projection_not_a_per_face_basis()
    {
        // Two coplanar faces meeting at a shared edge, at different local orientations relative to
        // their own vertex winding. A per-face UV basis (like PanelNetworkMeshFunction's) could pick a
        // different rotation for each; the shared XZ projection must not.
        var network = new VertexNetwork();
        int a = network.AddVertex(new Vector3(0.0f, 0.0f, 0.0f));
        int b = network.AddVertex(new Vector3(1.0f, 0.0f, 0.0f));
        int c = network.AddVertex(new Vector3(1.0f, 0.0f, 1.0f));
        int d = network.AddVertex(new Vector3(0.0f, 0.0f, 1.0f));
        int e = network.AddVertex(new Vector3(2.0f, 0.0f, 0.0f));
        int f = network.AddVertex(new Vector3(2.0f, 0.0f, 1.0f));
        network.AddFace([a, b, c, d]);
        network.AddFace([b, e, f, c]);

        var values = new MeshParameterValues();
        values.Set(WaterSurfaceMeshFunction.UvScale, 1.0f);
        var output = new ProceduralOutputBuilder();
        new WaterSurfaceMeshFunction(null!).Build(new ProceduralBuildContext(network, values, null!), output);

        ProceduralBuildResult result = output.Build([WaterSurfaceMeshFunction.Output], _ => MeshModelFormat.FormatId);
        ModelSurface surface = result.Models[0].Asset.Surfaces[0];
        Vector2[] uvs = surface.Mesh.SurfaceGetArrays(0)[(int)Mesh.ArrayType.TexUV].AsVector2Array();

        // Vertex b (1, 0, 0) is shared by both faces; its UV must equal its world XZ position regardless
        // of which face's fan it was emitted by.
        foreach (Vector2 uv in uvs)
        {
            if (uv.IsEqualApprox(new Vector2(1.0f, 0.0f)))
            {
                return;
            }
        }

        throw new TestAssertException("Expected at least one vertex UV to equal its world-space XZ position (1, 0).");
    }
}
