using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Names and colours for the navmesh's polygon areas, shared by the drawing and any legend over it.</summary>
public static class WowNavAreaStyle
{
    public static string NameOf(byte area) => area switch
    {
        WowNavArea.Ground or WowNavArea.GroundModel => "Ground",
        WowNavArea.SteepSlope or WowNavArea.SteepSlopeModel => "Steep ground",
        WowNavArea.WaterTransition => "Water edge",
        WowNavArea.Water => "Water",
        WowNavArea.Magma => "Magma",
        WowNavArea.Slime => "Slime",
        _ => $"Area {area}",
    };

    public static Color ColorOf(byte area) => area switch
    {
        WowNavArea.Ground or WowNavArea.GroundModel => new Color(0.2f, 0.85f, 0.3f),
        WowNavArea.SteepSlope or WowNavArea.SteepSlopeModel => new Color(1.0f, 0.6f, 0.1f),
        WowNavArea.WaterTransition => new Color(0.2f, 0.85f, 0.8f),
        WowNavArea.Water => new Color(0.2f, 0.5f, 1.0f),
        WowNavArea.Magma => new Color(0.95f, 0.2f, 0.15f),
        WowNavArea.Slime => new Color(0.5f, 0.8f, 0.1f),
        _ => new Color(0.9f, 0.2f, 0.9f),
    };
}

/// <summary>
/// The walkable surface of one navmesh tile: translucent triangles coloured by polygon area, polygon
/// edges as lines, and off-mesh connections as arrows. Water polygons get edges but no fill — their
/// surface is the liquid layer's to draw, and filling both would fight for the same pixels.
/// </summary>
public static class WowServerNavmeshMesh
{
    /// <summary>Raised off the ground so the navmesh reads above the terrain layers.</summary>
    public const float Lift = 0.15f;

    private const float FillAlpha = 0.4f;
    private const float ArrowHeadLength = 1.2f;
    private const float ArrowHeadWidth = 0.6f;

    public static IReadOnlyList<WowServerMeshData> Build(WowNavMeshTile tile)
    {
        var meshes = new List<WowServerMeshData>();
        if (BuildFill(tile) is { } fill)
        {
            meshes.Add(fill);
        }

        if (BuildLines(tile) is { } lines)
        {
            meshes.Add(lines);
        }

        return meshes;
    }

    private static Vector3 Position(WowNavMeshTile tile, float[] positions, int index) =>
        WowNavCoordinates.ToGodot(new Vector3(positions[index * 3], positions[(index * 3) + 1], positions[(index * 3) + 2])) + (Vector3.Up * Lift);

    private static WowServerMeshData? BuildFill(WowNavMeshTile tile)
    {
        (float[] positions, int[] indices, byte[] areas) = tile.BuildSurface();
        var vertices = new List<Vector3>(indices.Length);
        var colors = new List<Color>(indices.Length);

        for (int triangle = 0; triangle < areas.Length; triangle++)
        {
            if (areas[triangle] == WowNavArea.Water)
            {
                continue;
            }

            Color color = WowNavAreaStyle.ColorOf(areas[triangle]);
            color.A = FillAlpha;
            for (int corner = 0; corner < 3; corner++)
            {
                vertices.Add(Position(tile, positions, indices[(triangle * 3) + corner]));
                colors.Add(color);
            }
        }

        return vertices.Count == 0 ? null : new WowServerMeshData { Vertices = vertices.ToArray(), Colors = colors.ToArray(), Translucent = true };
    }

    private static WowServerMeshData? BuildLines(WowNavMeshTile tile)
    {
        var vertices = new List<Vector3>();
        var colors = new List<Color>();

        for (int poly = 0; poly < tile.PolygonCount; poly++)
        {
            if (tile.PolyType[poly] != 0)
            {
                continue;
            }

            int count = tile.PolyVertexCount[poly];
            Color color = WowNavAreaStyle.ColorOf(tile.PolyArea[poly]) * 0.6f;
            color.A = 1f;
            for (int edge = 0; edge < count; edge++)
            {
                int from = tile.PolyVertices[(poly * WowNavMeshTile.VertsPerPoly) + edge];
                int to = tile.PolyVertices[(poly * WowNavMeshTile.VertsPerPoly) + ((edge + 1) % count)];
                vertices.Add(Position(tile, tile.Vertices, from));
                vertices.Add(Position(tile, tile.Vertices, to));
                colors.Add(color);
                colors.Add(color);
            }
        }

        var arrow = new Color(1f, 0.9f, 0.1f);
        foreach (WowNavOffMeshConnection link in tile.OffMeshConnections)
        {
            Vector3 start = WowNavCoordinates.ToGodot(link.Start) + (Vector3.Up * Lift);
            Vector3 end = WowNavCoordinates.ToGodot(link.End) + (Vector3.Up * Lift);
            Vector3 direction = (end - start).Normalized();
            Vector3 side = direction.Cross(Vector3.Up).Normalized() * ArrowHeadWidth;
            Vector3 back = end - (direction * ArrowHeadLength);
            vertices.AddRange([start, end, end, back + side, end, back - side]);
            colors.AddRange([arrow, arrow, arrow, arrow, arrow, arrow]);
        }

        return vertices.Count == 0
            ? null
            : new WowServerMeshData { Vertices = vertices.ToArray(), Colors = colors.ToArray(), Primitive = Mesh.PrimitiveType.Lines };
    }
}
