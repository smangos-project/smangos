using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// One authored water surface's triangles in world space, with its liquid type resolved. The seam
/// between gathering geometry from the scene (<see cref="WowWaterPatches"/>) and turning triangles into
/// per-cell liquid data (<see cref="WowLiquidRasterizer"/>); manual lakes and river ribbons both arrive
/// as an ordinary indexed triangle mesh.
/// </summary>
public sealed record WaterSurfacePatch(
    int LiquidTypeId,
    LiquidBasicType BasicType,
    IReadOnlyList<Vector3> Vertices,
    IReadOnlyList<int> Indices,
    bool Fishable = true,
    bool Fatigue = false)
{
    /// <summary>World-space XZ bounding rectangle, for the rasterizer's cell/subchunk broad-phase.</summary>
    public Rect2 BoundsXZ { get; } = ComputeBoundsXZ(Vertices);

    private static Rect2 ComputeBoundsXZ(IReadOnlyList<Vector3> vertices)
    {
        if (vertices.Count == 0)
        {
            return new Rect2();
        }

        float minX = vertices[0].X;
        float maxX = vertices[0].X;
        float minZ = vertices[0].Z;
        float maxZ = vertices[0].Z;
        foreach (Vector3 vertex in vertices)
        {
            minX = Mathf.Min(minX, vertex.X);
            maxX = Mathf.Max(maxX, vertex.X);
            minZ = Mathf.Min(minZ, vertex.Z);
            maxZ = Mathf.Max(maxZ, vertex.Z);
        }

        return new Rect2(minX, minZ, maxX - minX, maxZ - minZ);
    }
}
