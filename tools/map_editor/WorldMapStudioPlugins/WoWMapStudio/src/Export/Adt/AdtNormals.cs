using System;
using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>
/// MCNR normals from a tile's heights. Each of a chunk's 145 vertices (9x9 corners interleaved with
/// 8x8 cell centres) takes the gradient from the four vertices diagonally around it at half a cell;
/// across a chunk edge it reads the neighbouring chunk of the same tile, and where there is none (tile
/// border) the vertex's own height.
/// </summary>
public static class AdtNormals
{
    private const int Outer = ImportedChunk.OuterEdge;
    private const int Inner = ImportedChunk.InnerEdge;
    private const int Edge = ImportedTile.ChunkEdge;
    private const int Span = 2 * Inner;
    private const float Half = WowTileAddressing.CellSize / Inner * 0.5f;

    /// <summary>The 145 normals of chunk (<paramref name="col"/>, <paramref name="row"/>), three signed bytes each, in on-disk order.</summary>
    public static sbyte[] Compute(IReadOnlyList<ImportedChunk> chunks, int col, int row)
    {
        var result = new sbyte[(Outer * Outer + Inner * Inner) * 3];
        int index = 0;

        void Write(int hx, int hz)
        {
            float own = Height(chunks, col, row, hx, hz) ?? 0f;
            float H(int dx, int dz) => Height(chunks, col, row, hx + dx, hz + dz) ?? own;
            float gr, gc;
            if (((hx | hz) & 1) == 0)
            {
                gr = (H(0, 2) - H(0, -2)) / (4f * Half);
                gc = (H(2, 0) - H(-2, 0)) / (4f * Half);
            }
            else
            {
                float d1 = H(1, 1) - H(-1, -1);
                float d2 = H(1, -1) - H(-1, 1);
                gc = (d1 + d2) / (4f * Half);
                gr = (d1 - d2) / (4f * Half);
            }

            float len = MathF.Sqrt((gr * gr) + (gc * gc) + 1f);
            result[(index * 3) + 0] = Encode(gr / len);
            result[(index * 3) + 1] = Encode(gc / len);
            result[(index * 3) + 2] = Encode(1f / len);
            index++;
        }

        for (int r = 0; r < Outer; r++)
        {
            for (int c = 0; c < Outer; c++)
            {
                Write(c * 2, r * 2);
            }

            if (r < Inner)
            {
                for (int c = 0; c < Inner; c++)
                {
                    Write((c * 2) + 1, (r * 2) + 1);
                }
            }
        }

        return result;
    }

    private static sbyte Encode(float v) => (sbyte)Math.Clamp((int)MathF.Round(v * 127f), -127, 127);

    // Height at half-step (hx, hz) of chunk (col, row); coordinates past 0..16 land in the neighbour.
    private static float? Height(IReadOnlyList<ImportedChunk> chunks, int col, int row, int hx, int hz)
    {
        while (hx < 0) { hx += Span; col--; }
        while (hx > Span) { hx -= Span; col++; }
        while (hz < 0) { hz += Span; row--; }
        while (hz > Span) { hz -= Span; row++; }
        if (col < 0 || row < 0 || col >= Edge || row >= Edge)
        {
            return null;
        }

        var chunk = chunks[(row * Edge) + col];
        return (hx & 1) == 0
            ? chunk.OuterHeights[((hz / 2) * Outer) + (hx / 2)]
            : chunk.InnerHeights[((hz / 2) * Inner) + (hx / 2)];
    }
}
