using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Turns every authored network face directly into a water face — the manual authoring mode. Modeled on the core
/// <c>PanelNetworkMeshFunction</c> (fan- triangulate each face via <see cref="PolygonTriangulator"/>, Newell normal),
/// with two differences: height is fully authored per vertex rather than flattened (a water surface can be flat,
/// sloped, or hand-contoured — there is no terrain-follow mode here, unlike a road), and UV is a single flat
/// world-space XZ projection shared by every face rather than each face's own in-plane basis, so adjacent faces of
/// one lake tile seamlessly instead of each picking an independent rotation.
///
/// The liquid type is one of this function's own <see cref="Parameters"/> (<see cref="WowLiquidParameters"/>),
/// not a material slot the user binds a preset to: the function builds its own <c>wow.liquid</c>
/// <see cref="MeshMaterial"/> from that parameter directly. The output slot is bound to
/// <see cref="Mh2oFormat"/> — a one-candidate combo, so nothing for the user to choose — which is what
/// makes <see cref="ProceduralBuildContext.MaterialType"/> report <c>wow.liquid</c> for this slot.
///
/// Depth (needed for the eventual MH2O export, and for depth-based tinting once it exists — see
/// <see cref="WowLiquidMaterialType"/>) is deliberately not authored here: it is terrain height below
/// each vertex, computed once by the rasterizer, not a second value the user has to keep consistent
/// by hand.
/// </summary>
[Subsystem(nameof(ProceduralSystem))]
public sealed class WaterSurfaceMeshFunction : IProceduralFunction
{
    public static readonly MeshParameter UvScale =
        MeshParameter.Float("uv_scale", "UV scale", 0.05f, 0.0001f, 16.0f, "Texture repeats per world unit.");

    // No material slots: this function builds its own MeshMaterial directly from LiquidTypeId/
    // FrameDuration rather than through the slot/preset system. Bound to Mh2oFormat (its one output
    // channel), which also collapses the inspector's Format combo since there is only one candidate.
    public static readonly ProceduralOutputSlot Output = new("mesh", "Mesh", [Mh2oFormat.FormatId], []);

    public const string FunctionId = "wow.water.surface";

    public string Id => FunctionId;

    public string DisplayName => "Water Surface";

    public string Description => "Turns every network face into a water face, at its own authored height.";

    public int Version => 1;

    public NetworkCapabilities Capabilities => new(AllowsFaces: true);

    public bool OutputWithinNetwork => true;

    /// <summary>Snaps authored vertices onto the MH2O subchunk lattice so a hand-drawn lake round-trips
    /// through <see cref="WowLiquidRasterizer"/> losslessly.</summary>
    public Vector3 SnapVertex(Vector3 worldPosition) => WaterLatticeSnap.ToSubchunkCorner(_system.Context, worldPosition);

    public IReadOnlyList<MeshParameter> Parameters =>
        MeshParameter.List(
            UvScale,
            WowLiquidParameters.LiquidTypeIdChoice(_system.Context),
            WowLiquidParameters.FrameDuration,
            WaterAttributeParameters.Fishable,
            WaterAttributeParameters.Fatigue);

    public IReadOnlyList<ProceduralOutputSlot> Outputs { get; } = [Output];

    private readonly ProceduralSystem _system;

    public WaterSurfaceMeshFunction(ProceduralSystem system)
    {
        _system = system;
    }

    public void Build(in ProceduralBuildContext context, ProceduralOutputBuilder output)
    {
        float uvScale = Mathf.Max(0.0001f, context.Float(UvScale));

        var vertices = new List<Vector3>();
        var normals = new List<Vector3>();
        var uvs = new List<Vector2>();
        var indices = new List<int>();

        // network.Vertex(id) is a linear scan; an imported whole-tile water body has tens of thousands
        // of vertices, so index them once here instead of scanning per face corner.
        var positionById = new Dictionary<int, Vector3>(context.Network.Vertices.Count);
        foreach (NetworkVertex vertex in context.Network.Vertices)
        {
            positionById[vertex.Id] = vertex.Position;
        }

        foreach (NetworkFace face in context.Network.Faces)
        {
            AddWaterFace(positionById, face, uvScale, vertices, normals, uvs, indices);
        }

        int liquidTypeId = context.Int(WowLiquidParameters.LiquidTypeIdLookup);
        float frameDuration = context.Float(WowLiquidParameters.FrameDuration);
        output.AddSurface(Output, "Water", vertices, indices, WowLiquidParameters.BuildMaterial(liquidTypeId, frameDuration), normals, uvs);
    }

    private static void AddWaterFace(
        IReadOnlyDictionary<int, Vector3> positionById,
        NetworkFace face,
        float uvScale,
        List<Vector3> vertices,
        List<Vector3> normals,
        List<Vector2> uvs,
        List<int> indices)
    {
        IReadOnlyList<int> loop = face.Vertices;
        if (loop.Count < 3)
        {
            return;
        }

        var positions = new Vector3[loop.Count];
        for (int i = 0; i < loop.Count; i++)
        {
            positions[i] = positionById.TryGetValue(loop[i], out Vector3 position) ? position : Vector3.Zero;
        }

        Vector3 normal = FaceNormal(positions);
        if (normal.LengthSquared() <= 1e-10f)
        {
            return;
        }

        int start = vertices.Count;
        var plane = new System.Numerics.Vector2[loop.Count];
        for (int i = 0; i < positions.Length; i++)
        {
            Vector3 position = positions[i];
            vertices.Add(position);
            normals.Add(normal);
            uvs.Add(new Vector2(position.X * uvScale, position.Z * uvScale));

            // Triangulation only needs a 2D projection of the loop, not the UV itself — the loop's own
            // XZ is a fine projection for a near-horizontal water face, and matches what is actually
            // being authored (NetworkEditTool has no notion of "this loop is vertical").
            plane[i] = new System.Numerics.Vector2(position.X, position.Z);
        }

        List<int> triangles = PolygonTriangulator.Triangulate(plane);
        for (int i = 0; i < triangles.Count; i++)
        {
            indices.Add(start + triangles[i]);
        }
    }

    /// <summary>Newell's method — robust for a near-planar n-gon, unlike a three-point cross product.</summary>
    private static Vector3 FaceNormal(IReadOnlyList<Vector3> loop)
    {
        Vector3 normal = Vector3.Zero;
        for (int i = 0; i < loop.Count; i++)
        {
            Vector3 current = loop[i];
            Vector3 next = loop[(i + 1) % loop.Count];
            normal.X += (current.Y - next.Y) * (current.Z + next.Z);
            normal.Y += (current.Z - next.Z) * (current.X + next.X);
            normal.Z += (current.X - next.X) * (current.Y + next.Y);
        }

        return normal.LengthSquared() <= 1e-10f ? normal : normal.Normalized();
    }
}
