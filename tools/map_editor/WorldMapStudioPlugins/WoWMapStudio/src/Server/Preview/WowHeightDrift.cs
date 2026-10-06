using System;

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
}
