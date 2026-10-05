namespace WorldMapStudio.WoW;

/// <summary>
/// Turns one MCNK's 4×4 hole bitmap into the byte block the map-wide <c>adt_holes</c> image stores —
/// <c>255</c> where a cell is cut, <c>0</c> elsewhere, read back through <c>builtin.hole.channel_mask</c>
/// at threshold 0.5. The block is in editor axes (the inverse of <see cref="WowChunkGridOrientation"/>'s
/// per-chunk quarter turn), 1:1 with the channel's 4-texel-per-chunk resolution.
/// </summary>
public static class HoleMapper
{
    /// <summary>Hole cells along one chunk edge.</summary>
    public const int HoleEdge = ImportedChunk.HoleEdge;

    public static bool HasAny(bool[] holes)
    {
        foreach (bool hole in holes)
        {
            if (hole)
            {
                return true;
            }
        }

        return false;
    }

    /// <summary>The chunk's 4×4 hole grid as a row-major byte block in editor axes.</summary>
    public static byte[] HoleBlock(bool[] holes)
    {
        var block = new byte[HoleEdge * HoleEdge];
        for (int y = 0; y < HoleEdge; y++)
        {
            for (int x = 0; x < HoleEdge; x++)
            {
                // editor[y][x] = wow[edge-1-x][y]
                block[(y * HoleEdge) + x] = holes[((HoleEdge - 1 - x) * HoleEdge) + y] ? (byte)255 : (byte)0;
            }
        }

        return block;
    }
}
