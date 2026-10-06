using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Turns a <see cref="VertexNetwork"/> into a Catmull-Rom river ribbon — the river authoring mode. Reuses
/// <see cref="NetworkSplineChains"/>'s graph-to-chains decomposition and centripetal Catmull-Rom flattening (the same
/// maths <c>RoadPath</c> uses for a road's centreline), but unlike a road the network's authored height is kept — a
/// river's centerline needs a real elevation profile to flow downhill smoothly, not a flat plane later stamped onto
/// whatever terrain happens to be underneath.
///
/// The ribbon's width is measured horizontally (in the XZ plane), not perpendicular to the 3D tangent:
/// a river's banks stay level with its centre at every cross-section, the way a real one does, rather
/// than tilting to stay perpendicular to a sloped flow direction. UV is the same shared world-space XZ
/// projection <see cref="WaterSurfaceMeshFunction"/> uses, so a river and a hand-authored lake it flows
/// into tile consistently.
///
/// Like <see cref="WaterSurfaceMeshFunction"/>, the liquid type is one of this function's own
/// <see cref="Parameters"/> (<see cref="WowLiquidParameters"/>), built directly into a
/// <see cref="MeshMaterial"/> rather than through a material slot/preset.
/// </summary>
[Subsystem(nameof(ProceduralSystem))]
public sealed class RiverSurfaceMeshFunction : IProceduralFunction
{
    public static readonly MeshParameter Width =
        MeshParameter.Float("width", "Width", 6.0f, 0.1f, 256.0f, "The river's width, measured horizontally.");

    public static readonly MeshParameter UvScale =
        MeshParameter.Float("uv_scale", "UV scale", 0.05f, 0.0001f, 16.0f, "Texture repeats per world unit.");

    // No material slots, bound to Mh2oFormat — see WaterSurfaceMeshFunction.Output's comment.
    public static readonly ProceduralOutputSlot Output = new("mesh", "Mesh", [Mh2oFormat.FormatId], []);

    public const string FunctionId = "wow.water.river";

    public string Id => FunctionId;

    public string DisplayName => "River";

    public string Description => "A Catmull-Rom ribbon of water following the network, at its own authored height.";

    public int Version => 1;

    /// <summary>Snaps authored control points onto the MH2O subchunk lattice, the same lattice
    /// <see cref="WaterSurfaceMeshFunction"/> uses, so a river and a lake it flows into stay aligned.</summary>
    public Vector3 SnapVertex(Vector3 worldPosition) => WaterLatticeSnap.ToSubchunkCorner(_system.Context, worldPosition);

    public IReadOnlyList<MeshParameter> Parameters =>
        MeshParameter.List(
            Width,
            UvScale,
            WowLiquidParameters.LiquidTypeIdChoice(_system.Context),
            WowLiquidParameters.FrameDuration,
            WaterAttributeParameters.Fishable,
            WaterAttributeParameters.Fatigue);

    public IReadOnlyList<ProceduralOutputSlot> Outputs { get; } = [Output];

    private readonly ProceduralSystem _system;

    public RiverSurfaceMeshFunction(ProceduralSystem system)
    {
        _system = system;
    }

    public void Build(in ProceduralBuildContext context, ProceduralOutputBuilder output)
    {
        float half = Mathf.Max(0.005f, context.Float(Width) * 0.5f);
        float uvScale = Mathf.Max(0.0001f, context.Float(UvScale));

        var vertices = new List<Vector3>();
        var normals = new List<Vector3>();
        var uvs = new List<Vector2>();
        var indices = new List<int>();

        foreach (NetworkSplinePolyline polyline in NetworkSplineChains.FlattenChains(context.Network))
        {
            AddRibbon(polyline, half, uvScale, vertices, normals, uvs, indices);
        }

        int liquidTypeId = context.Int(WowLiquidParameters.LiquidTypeIdLookup);
        float frameDuration = context.Float(WowLiquidParameters.FrameDuration);
        output.AddSurface(Output, "River", vertices, indices, WowLiquidParameters.BuildMaterial(liquidTypeId, frameDuration), normals, uvs);
    }

    private static void AddRibbon(
        NetworkSplinePolyline polyline,
        float half,
        float uvScale,
        List<Vector3> vertices,
        List<Vector3> normals,
        List<Vector2> uvs,
        List<int> indices)
    {
        IReadOnlyList<Vector3> points = polyline.Points;
        int n = points.Count;
        if (n < 2)
        {
            return;
        }

        int start = vertices.Count;
        for (int i = 0; i < n; i++)
        {
            int previousIndex = polyline.IsClosed ? ((i - 1) + n) % n : System.Math.Max(i - 1, 0);
            int nextIndex = polyline.IsClosed ? (i + 1) % n : System.Math.Min(i + 1, n - 1);
            Vector3 point = points[i];
            Vector3 previous = points[previousIndex];
            Vector3 next = points[nextIndex];

            // Horizontal tangent only — see the class comment on why width is a plan-view measurement.
            float tangentX = next.X - previous.X;
            float tangentZ = next.Z - previous.Z;
            float tangentLength = Mathf.Sqrt((tangentX * tangentX) + (tangentZ * tangentZ));
            float acrossX = tangentLength > 1e-4f ? -tangentZ / tangentLength : 1.0f;
            float acrossZ = tangentLength > 1e-4f ? tangentX / tangentLength : 0.0f;

            var offset = new Vector3(acrossX, 0.0f, acrossZ) * half;
            Vector3 left = point + offset;
            Vector3 right = point - offset;

            vertices.Add(left);
            vertices.Add(right);
            // A constant up normal rather than one derived from the ribbon's own (possibly sloped)
            // geometry — a river's surface is locally flat across its width at every cross-section (the
            // same reasoning behind measuring width horizontally above), so this is exact, not a
            // simplification.
            normals.Add(Vector3.Up);
            normals.Add(Vector3.Up);
            uvs.Add(new Vector2(left.X * uvScale, left.Z * uvScale));
            uvs.Add(new Vector2(right.X * uvScale, right.Z * uvScale));
        }

        int spanCount = polyline.IsClosed ? n : n - 1;
        for (int i = 0; i < spanCount; i++)
        {
            int a = start + (i * 2);
            int b = start + (((i + 1) % n) * 2);

            indices.Add(a);
            indices.Add(a + 1);
            indices.Add(b + 1);

            indices.Add(a);
            indices.Add(b + 1);
            indices.Add(b);
        }
    }
}
