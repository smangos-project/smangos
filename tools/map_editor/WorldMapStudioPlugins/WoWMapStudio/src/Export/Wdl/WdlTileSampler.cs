using System;
using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>
/// Builds a WDL tile heightmap (MARE) from a tile's chunks: a 17x17 grid of chunk-corner heights and a
/// 16x16 grid of chunk-centre heights, as absolute int16 heights.
/// </summary>
public static class WdlTileSampler
{
    public const int OuterSize = 17;
    public const int InnerSize = 16;

    public static (short[] Outer, short[] Inner) Build(IReadOnlyList<ImportedChunk> chunks)
    {
        const int edge = ImportedTile.ChunkEdge;
        const int last = ImportedChunk.OuterEdge - 1;
        var outer = new short[OuterSize * OuterSize];
        var inner = new short[InnerSize * InnerSize];
        for (int row = 0; row < OuterSize; row++)
        {
            for (int col = 0; col < OuterSize; col++)
            {
                var chunk = chunks[(Math.Min(row, edge - 1) * edge) + Math.Min(col, edge - 1)];
                int lx = col == edge ? last : 0;
                int lz = row == edge ? last : 0;
                outer[(row * OuterSize) + col] = ToInt16(chunk.OuterHeights[(lz * ImportedChunk.OuterEdge) + lx]);
            }
        }

        for (int row = 0; row < InnerSize; row++)
        {
            for (int col = 0; col < InnerSize; col++)
            {
                var chunk = chunks[(row * edge) + col];
                inner[(row * InnerSize) + col] = ToInt16(chunk.OuterHeights[(last / 2 * ImportedChunk.OuterEdge) + (last / 2)]);
            }
        }

        return (outer, inner);
    }

    private static short ToInt16(float v) => (short)Math.Clamp(MathF.Round(v), short.MinValue, short.MaxValue);
}
