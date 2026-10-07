using System;
using System.IO;

namespace WorldMapStudio.WoW;

/// <summary>Height differences between the editor terrain and a tile's extracted server heights.</summary>
public readonly record struct WowHeightDrift(int Samples, int Drifted, float MaxAbs, float MeanAbs)
{
    public bool IsZero => Drifted == 0;

    /// <summary>
    /// Compares every corner vertex of <paramref name="tile"/> with <paramref name="terrainHeight"/>
    /// (row, col, returning null where the editor has no terrain); holes are skipped.
    /// </summary>
    public static WowHeightDrift Measure(WowMapTile tile, Func<int, int, float?> terrainHeight, float tolerance)
    {
        int samples = 0, drifted = 0;
        float max = 0, sum = 0;
        for (int row = 0; row < WowMapTile.VertexEdge; row++)
        {
            for (int col = 0; col < WowMapTile.VertexEdge; col++)
            {
                if (row < WowMapTile.CellEdge && col < WowMapTile.CellEdge && tile.IsHole(row, col))
                {
                    continue;
                }

                if (terrainHeight(row, col) is not float height)
                {
                    continue;
                }

                float diff = MathF.Abs(height - tile.CornerHeight(row, col));
                samples++;
                sum += diff;
                max = MathF.Max(max, diff);
                if (diff > tolerance)
                {
                    drifted++;
                }
            }
        }

        return new WowHeightDrift(samples, drifted, max, samples == 0 ? 0 : sum / samples);
    }

    /// <summary>One-line drift summary of server tile (<paramref name="fileX"/>, <paramref name="fileY"/>) against the loaded editor terrain.</summary>
    public static string Describe(EditorContext context, int wowMap, int fileX, int fileY)
    {
        if (context.WowServerPaths.Roots.MapFile(new WowServerTileKey(wowMap, fileX, fileY)) is not { } file || !File.Exists(file))
        {
            return "no server .map for tile";
        }

        var tile = WowMapTileReader.Read(File.ReadAllBytes(file));
        if (AdtMapWorld.FromEditor(context, context.Maps.CurrentMap, out _) is not { } world)
        {
            return "no imported ADT height data";
        }

        WowHeightDrift drift = Measure(tile, (row, col) =>
        {
            Godot.Vector3 at = WowMapGrid.ToGodot(row, col, 0f, fileX, fileY);
            AdtVertex vertex = world.NearestVertex(at);
            return AdtVertexProbe.TryRead(world, AdtVertexKind.Height, vertex, out System.Numerics.Vector3 height) ? height.X : null;
        }, 0.1f);
        return drift.Samples == 0
            ? "no terrain loaded under tile"
            : $"{drift.Drifted}/{drift.Samples} verts >0.1, max {drift.MaxAbs:F2}, mean {drift.MeanAbs:F3}";
    }
}
