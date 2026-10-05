using System;

namespace WorldMapStudio.WoW;

/// <summary>
/// The texel layout of a vertex-sampled image with cell-centre vertices: 4 texels per cell along each
/// axis. Per axis, in cell <c>V</c>:
///
/// <list type="table">
/// <item><term><c>4V</c></term><description>corner <c>V</c> (shared with <c>4V-1</c> of the previous cell)</description></item>
/// <item><term><c>4V+1</c>, <c>4V+2</c></term><description>centre of cell <c>V</c></description></item>
/// <item><term><c>4V+3</c></term><description>corner <c>V+1</c></description></item>
/// </list>
///
/// A channel is read bilinearly at vertex positions. A corner reads the 2x2 block <c>(4V-1, 4V)</c>
/// and a centre reads <c>(4V+1, 4V+2)</c>; the blocks are disjoint, so both read back exactly. The
/// texels that are a corner along one axis and a centre along the other are never read at a vertex;
/// they hold the mean of the two corners on their edge, so the image still looks right in a preview.
/// </summary>
public static class CellCentreTexels
{
    public const int PerCell = 4;

    /// <summary>Texel coordinate a corner's read sits on: midway between texels <c>4V-1</c> and <c>4V</c>.</summary>
    public static float CornerCoordinate(int corner) => (corner * PerCell) - 0.5f;

    /// <summary>Texel coordinate a cell centre's read sits on: midway between texels <c>4V+1</c> and <c>4V+2</c>.</summary>
    public static float CentreCoordinate(int cell) => (cell * PerCell) + 1.5f;

    /// <summary>
    /// The value of texel (<paramref name="tx"/>, <paramref name="ty"/>) of a chunk block, given how to
    /// read a corner vertex, a centre vertex and how to average two values. Coordinates are editor axes.
    /// </summary>
    public static T Texel<T>(
        int tx,
        int ty,
        Func<int, int, T> corner,
        Func<int, int, T> centre,
        Func<T, T, T> mean)
    {
        (int x, bool xCorner) = Axis(tx);
        (int y, bool yCorner) = Axis(ty);

        if (xCorner && yCorner)
        {
            return corner(x, y);
        }

        if (!xCorner && !yCorner)
        {
            return centre(x, y);
        }

        return xCorner
            ? mean(corner(x, y), corner(x, y + 1))
            : mean(corner(x, y), corner(x + 1, y));
    }

    // Corner axes give the corner index; centre axes give the cell index.
    private static (int Index, bool IsCorner) Axis(int texel)
    {
        int cell = texel / PerCell;
        return (texel % PerCell) switch
        {
            0 => (cell, true),
            3 => (cell + 1, true),
            _ => (cell, false),
        };
    }

    /// <summary>
    /// Reads a block back the way <c>LandscapeChannelPool.SampleComponent</c> would at a position given
    /// in texel coordinates (<see cref="CornerCoordinate"/> / <see cref="CentreCoordinate"/>): bilinear
    /// about texel centres, edge-clamped.
    /// </summary>
    public static float Sample(Func<int, int, float> texel, int edge, float fx, float fy)
    {
        int x0 = (int)MathF.Floor(fx);
        int y0 = (int)MathF.Floor(fy);
        float tx = fx - x0;
        float ty = fy - y0;

        float At(int x, int y) => texel(Math.Clamp(x, 0, edge - 1), Math.Clamp(y, 0, edge - 1));

        float top = (At(x0, y0) * (1.0f - tx)) + (At(x0 + 1, y0) * tx);
        float bottom = (At(x0, y0 + 1) * (1.0f - tx)) + (At(x0 + 1, y0 + 1) * tx);
        return (top * (1.0f - ty)) + (bottom * ty);
    }
}
