using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// The liquid the server would find in a tile, drawn as one flat quad per cell at the level the server
/// resolves for it. A cell is left out where the server treats it as dry: no liquid type, the empty-level
/// sentinel, or ground standing above the surface.
/// </summary>
public static class WowServerLiquidMesh
{
    /// <summary>Levels at or below this are the extractor's "no liquid here" marker (-2000 in practice).</summary>
    public const float EmptyLevelBelow = -1000f;

    /// <summary>The server's tolerance (<c>GROUND_LEVEL_OFFSET_HACK</c>) for ground poking through a surface.</summary>
    public const float GroundTolerance = 0.02f;

    private const byte Water = 0x01;
    private const byte Ocean = 0x02;
    private const byte Magma = 0x04;
    private const byte Slime = 0x08;
    private const byte DarkWater = 0x10;

    /// <summary>Null when the tile has no liquid the server would report.</summary>
    public static WowServerMeshData? Build(WowMapTile tile, int fileX, int fileY)
    {
        if (!tile.HasLiquid)
        {
            return null;
        }

        var vertices = new List<Vector3>();
        var colors = new List<Color>();
        var indices = new List<int>();

        for (int row = 0; row < WowMapTile.CellEdge; row++)
        {
            for (int col = 0; col < WowMapTile.CellEdge; col++)
            {
                if (tile.LiquidAt(row, col) is not { } cell || IsDry(tile, row, col, cell))
                {
                    continue;
                }

                Color color = ColorOf(cell.TypeFlags);
                int first = vertices.Count;
                vertices.Add(WowMapGrid.ToGodot(row, col, cell.Level, fileX, fileY));
                vertices.Add(WowMapGrid.ToGodot(row + 1, col, cell.Level, fileX, fileY));
                vertices.Add(WowMapGrid.ToGodot(row, col + 1, cell.Level, fileX, fileY));
                vertices.Add(WowMapGrid.ToGodot(row + 1, col + 1, cell.Level, fileX, fileY));
                colors.AddRange([color, color, color, color]);
                indices.AddRange([first, first + 1, first + 2, first + 2, first + 1, first + 3]);
            }
        }

        return vertices.Count == 0
            ? null
            : new WowServerMeshData { Vertices = vertices.ToArray(), Colors = colors.ToArray(), Indices = indices.ToArray(), Translucent = true };
    }

    private static bool IsDry(WowMapTile tile, int row, int col, WowMapLiquidCell cell) =>
        cell.Level < EmptyLevelBelow || cell.Level < tile.CentreHeight(row, col) - GroundTolerance;

    /// <summary>Colour by liquid kind. A cell can carry several kind bits; the more hazardous one wins.</summary>
    public static Color ColorOf(byte typeFlags)
    {
        Color color =
            (typeFlags & Magma) != 0 ? new Color(1.0f, 0.4f, 0.05f)
            : (typeFlags & Slime) != 0 ? new Color(0.3f, 0.9f, 0.2f)
            : (typeFlags & Ocean) != 0 ? new Color(0.05f, 0.3f, 0.75f)
            : (typeFlags & Water) != 0 ? new Color(0.2f, 0.55f, 1.0f)
            : new Color(0.7f, 0.7f, 0.7f);

        if ((typeFlags & DarkWater) != 0)
        {
            color = new Color(color.R * 0.6f, color.G * 0.6f, color.B * 0.6f);
        }

        color.A = 0.45f;
        return color;
    }
}
