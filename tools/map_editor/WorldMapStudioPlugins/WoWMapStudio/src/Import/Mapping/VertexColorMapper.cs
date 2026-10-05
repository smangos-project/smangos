using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Encodes a chunk's 9×9 outer and 8×8 cell-centre MCCV / MCLV grids as the 32×32 RGBA byte block the
/// map-wide <c>adt_vcolor</c> / <c>adt_vlight</c> image stores. Same texel layout and per-chunk axis
/// rotation as <see cref="HeightMapper"/> (both are vertex-sampled), read back through
/// <c>builtin.vertex_color.channel_paint</c> / <c>builtin.vertex_light.channel_paint</c>.
///
/// Vertex colour forces alpha to 255 so the paint function's weight is 1 and the RGB replaces the
/// build's white start exactly (decoded by that function's own Exponent/Scale — this mapper stores
/// the raw on-disk bytes, unchanged); vertex light needs no alpha (it adds RGB onto the black start).
/// </summary>
public static class VertexColorMapper
{
    public const int TexelEdge = HeightMapper.TexelEdge;

    private const int VertexEdge = ImportedChunk.OuterEdge;

    private const int CentreEdge = ImportedChunk.InnerEdge;

    /// <summary>MCCV's own on-disk neutral (mid-grey, raw <c>0.498</c>) — not white, since this mapper
    /// doesn't decode the byte into the engine's multiplier space.</summary>
    private const byte NeutralByte = 0x7F;

    /// <summary>Compared as bytes so float round-trip noise can never flag a neutral chunk as
    /// coloured.</summary>
    public static bool HasVertexColor(Color[]? outer, Color[]? inner = null) =>
        (outer != null && HasAnyRgbByteNot(outer, NeutralByte)) || (inner != null && HasAnyRgbByteNot(inner, NeutralByte));

    /// <summary>RGB only — <c>Colors.Black</c> is opaque, but MCLV's own alpha byte is commonly 0, so
    /// comparing all four components would flag an all-black chunk as having vertex light.</summary>
    public static bool HasVertexLight(Color[]? outer, Color[]? inner = null) =>
        (outer != null && HasAnyRgbNot(outer, Colors.Black)) || (inner != null && HasAnyRgbNot(inner, Colors.Black));

    /// <summary>One chunk's 32×32 RGBA block for vertex colour — alpha forced opaque. A null
    /// <paramref name="innerColors"/> takes the mean of each cell's four corners.</summary>
    public static byte[] ColorBlock(Color[] outerColors, Color[]? innerColors = null) => Block(outerColors, innerColors, opaque: true);

    /// <summary>One chunk's 32×32 RGBA block for vertex light — alpha left zero.</summary>
    public static byte[] LightBlock(Color[] outerColors, Color[]? innerColors = null) => Block(outerColors, innerColors, opaque: false);

    /// <summary>Reads one component (0 R, 1 G, 2 B, 3 A) of a block back at editor corner
    /// (<paramref name="vx"/>, <paramref name="vy"/>), as the channel's bilinear sample would.</summary>
    public static float SampleAtVertex(byte[] block, int component, int vx, int vy) =>
        CellCentreTexels.Sample(
            (x, y) => block[(((y * TexelEdge) + x) * 4) + component],
            TexelEdge,
            CellCentreTexels.CornerCoordinate(vx),
            CellCentreTexels.CornerCoordinate(vy));

    /// <summary>As <see cref="SampleAtVertex"/>, at the centre of editor cell (<paramref name="cx"/>, <paramref name="cy"/>).</summary>
    public static float SampleAtCentre(byte[] block, int component, int cx, int cy) =>
        CellCentreTexels.Sample(
            (x, y) => block[(((y * TexelEdge) + x) * 4) + component],
            TexelEdge,
            CellCentreTexels.CentreCoordinate(cx),
            CellCentreTexels.CentreCoordinate(cy));

    private static byte[] Block(Color[] outer, Color[]? inner, bool opaque)
    {
        var block = new byte[TexelEdge * TexelEdge * 4];
        for (int ty = 0; ty < TexelEdge; ty++)
        {
            for (int tx = 0; tx < TexelEdge; tx++)
            {
                Color c = CellCentreTexels.Texel(
                    tx,
                    ty,
                    (x, y) => EditorVertex(outer, x, y),
                    (x, y) => inner != null ? EditorCentre(inner, x, y) : CornerMean(outer, x, y),
                    static (a, b) => new Color((a.R + b.R) * 0.5f, (a.G + b.G) * 0.5f, (a.B + b.B) * 0.5f, (a.A + b.A) * 0.5f));
                int o = (((ty * TexelEdge) + tx) * 4);
                block[o + 0] = ToByte(c.R);
                block[o + 1] = ToByte(c.G);
                block[o + 2] = ToByte(c.B);
                block[o + 3] = opaque ? (byte)255 : (byte)0;
            }
        }

        return block;
    }

    /// <summary>editor[y][x] = mcnk[edge-1-x][y] — the inverse of the export quarter-turn, in the 9×9
    /// grid.</summary>
    private static Color EditorVertex(Color[] mcnk, int x, int y) => mcnk[((VertexEdge - 1 - x) * VertexEdge) + y];

    /// <summary>The same quarter-turn over the 8×8 cell-centre grid.</summary>
    private static Color EditorCentre(Color[] mcnk, int x, int y) => mcnk[((CentreEdge - 1 - x) * CentreEdge) + y];

    private static Color CornerMean(Color[] outer, int x, int y)
    {
        Color a = EditorVertex(outer, x, y);
        Color b = EditorVertex(outer, x + 1, y);
        Color c = EditorVertex(outer, x, y + 1);
        Color d = EditorVertex(outer, x + 1, y + 1);
        return new Color(
            (a.R + b.R + c.R + d.R) * 0.25f,
            (a.G + b.G + c.G + d.G) * 0.25f,
            (a.B + b.B + c.B + d.B) * 0.25f,
            (a.A + b.A + c.A + d.A) * 0.25f);
    }

    private static byte ToByte(float channel) => (byte)Mathf.Clamp(Mathf.RoundToInt(channel * 255.0f), 0, 255);

    private static bool HasAnyRgbByteNot(Color[] grid, byte value)
    {
        foreach (Color c in grid)
        {
            if (ToByte(c.R) != value || ToByte(c.G) != value || ToByte(c.B) != value)
            {
                return true;
            }
        }

        return false;
    }

    private static bool HasAnyRgbNot(Color[] grid, Color value)
    {
        foreach (Color c in grid)
        {
            if (c.R != value.R || c.G != value.G || c.B != value.B)
            {
                return true;
            }
        }

        return false;
    }
}
