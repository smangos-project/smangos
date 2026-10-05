using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>A tile's server height grid as lines, so the heights the server walks on can be compared with the terrain.</summary>
public static class WowServerTerrainMesh
{
    /// <summary>Raised off the ground so the lines read above the terrain layers.</summary>
    public const float Lift = 0.05f;

    private const int Step = 4;
    private static readonly Color LineColor = new(0.95f, 0.9f, 0.2f);

    public static WowServerMeshData Build(WowMapTile tile, int fileX, int fileY)
    {
        var vertices = new List<Vector3>();
        var indices = new List<int>();
        int edge = WowMapTile.VertexEdge;
        int[,] ids = new int[edge, edge];
        for (int row = 0; row < edge; row += Step)
        {
            for (int col = 0; col < edge; col += Step)
            {
                ids[row, col] = vertices.Count;
                vertices.Add(WowMapGrid.ToGodot(row, col, tile.CornerHeight(row, col), fileX, fileY) + (Vector3.Up * Lift));
            }
        }

        for (int row = 0; row < edge; row += Step)
        {
            for (int col = 0; col < edge; col += Step)
            {
                if (col + Step < edge)
                {
                    indices.Add(ids[row, col]);
                    indices.Add(ids[row, col + Step]);
                }

                if (row + Step < edge)
                {
                    indices.Add(ids[row, col]);
                    indices.Add(ids[row + Step, col]);
                }
            }
        }

        var colors = new Color[vertices.Count];
        System.Array.Fill(colors, LineColor);
        return new WowServerMeshData
        {
            Vertices = [.. vertices],
            Colors = colors,
            Indices = [.. indices],
            Primitive = Mesh.PrimitiveType.Lines,
        };
    }
}
