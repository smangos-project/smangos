using System;

namespace WorldMapStudio.WoW;

/// <summary>
/// Encodes one MCNK's 145 height vertices — the 9×9 outer grid and the 8×8 cell-centre grid — as the
/// 32×32 Float32 texel block the map-wide <c>adt_height</c> image stores for that chunk. Two things
/// happen here and nowhere else:
///
/// <b>Texel layout.</b> Four texels per cell along each axis (see <see cref="CellCentreTexels"/>), so
/// the channel's bilinear read at any corner or centre vertex returns exactly that vertex's height —
/// the round trip is lossless, not smoothed.
///
/// <b>Orientation.</b> The reader hands heights in MCNK grid order (fast index East, slow index
/// South); the editor's grid runs fast +X, slow +Z. This applies the inverse of
/// <see cref="WowChunkGridOrientation"/>'s per-chunk quarter-turn so the block is in editor axes.
/// </summary>
public static class HeightMapper
{
    /// <summary>Vertices along one chunk edge (9×9 outer grid).</summary>
    public const int VertexEdge = ImportedChunk.OuterEdge;

    /// <summary>Cell-centre vertices along one chunk edge (8×8 inner grid).</summary>
    public const int CentreEdge = ImportedChunk.InnerEdge;

    /// <summary>Texels along one chunk edge in the height image.</summary>
    public const int TexelEdge = CentreEdge * CellCentreTexels.PerCell;

    /// <summary>
    /// One chunk's 32×32 texel block, row-major, in editor axes. <paramref name="mcnkOuterHeights"/> and
    /// <paramref name="mcnkInnerHeights"/> are the chunk's 9×9 and 8×8 grids in MCNK order (row-major),
    /// absolute world height — exactly <see cref="ImportedChunk.OuterHeights"/> and
    /// <see cref="ImportedChunk.InnerHeights"/>.
    /// </summary>
    public static float[] ChunkBlock(float[] mcnkOuterHeights, float[] mcnkInnerHeights)
    {
        if (mcnkOuterHeights.Length != VertexEdge * VertexEdge)
        {
            throw new ArgumentException($"Expected {VertexEdge * VertexEdge} outer heights, got {mcnkOuterHeights.Length}.", nameof(mcnkOuterHeights));
        }

        if (mcnkInnerHeights.Length != CentreEdge * CentreEdge)
        {
            throw new ArgumentException($"Expected {CentreEdge * CentreEdge} inner heights, got {mcnkInnerHeights.Length}.", nameof(mcnkInnerHeights));
        }

        var block = new float[TexelEdge * TexelEdge];
        for (int ty = 0; ty < TexelEdge; ty++)
        {
            for (int tx = 0; tx < TexelEdge; tx++)
            {
                block[(ty * TexelEdge) + tx] = CellCentreTexels.Texel(
                    tx,
                    ty,
                    (x, y) => EditorCorner(mcnkOuterHeights, x, y),
                    (x, y) => EditorCentre(mcnkInnerHeights, x, y),
                    static (a, b) => (a + b) * 0.5f);
            }
        }

        return block;
    }

    /// <summary>
    /// The height at editor-grid corner <paramref name="x"/>, <paramref name="y"/>, read out of an
    /// MCNK-ordered grid. The export rotation is <c>wow[slow][fast] = editor[fast][edge-1-slow]</c>;
    /// inverted, <c>editor[y][x] = wow[edge-1-x][y]</c>.
    /// </summary>
    private static float EditorCorner(float[] mcnk, int x, int y) =>
        mcnk[((VertexEdge - 1 - x) * VertexEdge) + y];

    /// <summary>The same quarter-turn over the 8×8 cell-centre grid.</summary>
    private static float EditorCentre(float[] mcnk, int x, int y) =>
        mcnk[((CentreEdge - 1 - x) * CentreEdge) + y];

    /// <summary>
    /// Reads the block back the way <c>LandscapeChannelPool.SampleComponent</c> would at editor corner
    /// <paramref name="vx"/>, <paramref name="vy"/>. Exposed for the round-trip test that is this
    /// file's whole correctness argument.
    /// </summary>
    public static float SampleAtVertex(float[] block, int vx, int vy) =>
        CellCentreTexels.Sample(
            (x, y) => block[(y * TexelEdge) + x], TexelEdge, CellCentreTexels.CornerCoordinate(vx), CellCentreTexels.CornerCoordinate(vy));

    /// <summary>As <see cref="SampleAtVertex"/>, at the centre vertex of editor cell (<paramref name="cx"/>, <paramref name="cy"/>).</summary>
    public static float SampleAtCentre(float[] block, int cx, int cy) =>
        CellCentreTexels.Sample(
            (x, y) => block[(y * TexelEdge) + x], TexelEdge, CellCentreTexels.CentreCoordinate(cx), CellCentreTexels.CentreCoordinate(cy));
}
