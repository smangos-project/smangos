namespace WorldMapStudio.WoW;

/// <summary>One ADT tile's identity: the map directory name plus its two filename indices, e.g.
/// <c>World\Maps\Azeroth\Azeroth_32_49</c> (the caller appends <c>.adt</c>/<c>_tex0.adt</c>/etc).</summary>
public readonly record struct WowTileId(string Map, int FileX, int FileY)
{
    /// <summary>The shared stem every one of this tile's physical files is named from.</summary>
    public string Stem => $"World\\Maps\\{Map}\\{Map}_{FileX}_{FileY}";
}

/// <summary>Where one <see cref="ChunkCoord"/> lands inside its tile: which of the 256 MCNK slots,
/// row-major (<see cref="LocalRow"/> outer, <see cref="LocalCol"/> inner) — the order real ADT files
/// store them in.</summary>
public readonly record struct WowMcnkAddress(WowTileId Tile, int LocalCol, int LocalRow)
{
    /// <summary>This chunk's position among the tile's 256 MCNKs, in on-disk order.</summary>
    public int Index => (LocalRow * WowTileAddressing.ChunksPerTileEdge) + LocalCol;
}

/// <summary>
/// Converts between <see cref="ChunkCoord"/> (this editor's own addressing — absolute, unbounded,
/// centred on the map's own origin chunk) and a WoW ADT tile's filename indices plus MCNK slot.
///
/// Derived and locked against a real client tile (<c>Azeroth_32_49.adt</c>; this math is unchanged from Vanilla through Cata+).
/// The one relationship this relies on that isn't re-derivable from the dump alone: an MCNK header's
/// position field and an MDDF/MODF placement's position field are the same client "world space"
/// <see cref="WowCoordinates"/> already converts — a basic, format-level fact, not a per-map guess.
///
/// The result is a clean but asymmetric-looking pair of formulas — <see cref="LocalCol"/> tracks
/// <see cref="ChunkCoord.Y"/> directly while <see cref="LocalRow"/> tracks <see cref="ChunkCoord.X"/>
/// negated <em>and shifted by one</em> — which is expected, not a bug: it is the well-known "ADT
/// filename axes are swapped and one is mirrored relative to world space" quirk, now composed with
/// this editor's own Y-up Godot convention. The <c>-1</c> on the row axis is the other half of that
/// mirror: a WoW MCNK's grid is laid <em>south</em> from its header position (the chunk's north
/// edge), where <see cref="LandscapeGrid.OriginOf"/> is a chunk's <em>south</em> corner with the grid
/// running north. Same header value, opposite side — so the editor chunk whose terrain belongs in a
/// given MCNK slot is the one a step further south than the un-shifted formula names, and
/// <see cref="HeaderPositionOf"/> adds a <see cref="LandscapeGrid.ChunkSize"/> back to land on that
/// north edge. <see cref="WowTileAddressingTests"/> pins the whole thing against the real tile's raw
/// bytes so a sign error here fails loudly instead of shipping a map that loads a chunk off.
/// </summary>
public static class WowTileAddressing
{
    /// <summary>MCNK chunks along one tile edge. Fixed by the format — not a landscape setting.</summary>
    public const int ChunksPerTileEdge = 16;

    /// <summary>World-space edge length of one ADT tile, in yards.</summary>
    public const float TileSize = 533.33333f;

    /// <summary>World-space edge length of one MCNK chunk, in yards.</summary>
    public const float CellSize = TileSize / ChunksPerTileEdge;

    /// <summary>Tiles along one map edge (a WDT's MAIN table is this square).</summary>
    public const int TilesPerMapEdge = 64;

    /// <summary>The filename index of the tile straddling the map's world-space origin.</summary>
    public const int OriginTileIndex = TilesPerMapEdge / 2;

    /// <summary>Which tile file and MCNK slot a chunk belongs to, without a map name — the pure
    /// addressing math <see cref="AddressOf"/> and <see cref="ChunkAt"/> are both built on.</summary>
    public static (int FileX, int FileY, int LocalCol, int LocalRow) SlotOf(ChunkCoord coord)
    {
        int fileX = OriginTileIndex + FloorDiv(coord.Y, ChunksPerTileEdge);
        int localCol = FloorMod(coord.Y, ChunksPerTileEdge);
        int fileY = OriginTileIndex + FloorDiv(-coord.X - 1, ChunksPerTileEdge);
        int localRow = FloorMod(-coord.X - 1, ChunksPerTileEdge);
        return (fileX, fileY, localCol, localRow);
    }

    /// <summary>Which tile and MCNK slot a chunk belongs to.</summary>
    public static WowMcnkAddress AddressOf(string map, ChunkCoord coord)
    {
        (int fileX, int fileY, int localCol, int localRow) = SlotOf(coord);
        return new WowMcnkAddress(new WowTileId(map, fileX, fileY), localCol, localRow);
    }

    /// <summary>The inverse of <see cref="AddressOf"/>: which chunk a tile's (localCol, localRow) MCNK slot is.</summary>
    public static ChunkCoord ChunkAt(WowTileId tile, int localCol, int localRow)
    {
        int y = ((tile.FileX - OriginTileIndex) * ChunksPerTileEdge) + localCol;
        int x = -(((tile.FileY - OriginTileIndex) * ChunksPerTileEdge) + localRow) - 1;
        return new ChunkCoord(x, y);
    }

    /// <summary>Every one of a tile's 256 chunks, in on-disk MCNK order — the single enumeration every
    /// per-file writer (root, tex0, obj0) walks, so they can never disagree on chunk order with each
    /// other.</summary>
    public static System.Collections.Generic.IEnumerable<(int LocalCol, int LocalRow, ChunkCoord Coord)> EnumerateChunks(WowTileId tile)
    {
        for (int row = 0; row < ChunksPerTileEdge; row++)
        {
            for (int col = 0; col < ChunksPerTileEdge; col++)
            {
                yield return (col, row, ChunkAt(tile, col, row));
            }
        }
    }

    /// <summary>
    /// The raw <c>(zpos, xpos)</c> pair an MCNK header stores for this chunk — named for the on-disk
    /// struct fields, which famously do not match the axis they hold (<c>zpos</c> is the chunk's world
    /// X, <c>xpos</c> its world Y). Computed straight from <paramref name="grid"/>'s own corner, so it
    /// stays correct under a non-default <see cref="LandscapeSettings.OriginChunkX"/>/<c>Y</c> with no
    /// extra bookkeeping here.
    ///
    /// The <c>+ ChunkSize</c> on <c>zpos</c> is the row-axis mirror described on the class: the client
    /// lays an MCNK's grid south from this point, so it is the chunk's <em>north</em> edge, one chunk
    /// above <see cref="LandscapeGrid.OriginOf"/>'s own south corner. <c>xpos</c> needs no such term
    /// because the east/west axis is not mirrored — both conventions start at the west edge.
    /// </summary>
    public static (float Zpos, float Xpos) HeaderPositionOf(LandscapeGrid grid, ChunkCoord coord)
    {
        Godot.Vector3 origin = grid.OriginOf(coord);
        return (origin.X + grid.ChunkSize, -origin.Z);
    }

    /// <summary>
    /// The server-world X/Y at the centre of tile <c>(fileX, fileY)</c> — where a <c>.go xyz</c> puts
    /// you standing in the middle of that ADT. Z is the caller's to pick. Inverts the client's
    /// <c>file = OriginTileIndex - server / TileSize</c> tile lookup; pinned against the same real tile
    /// as the rest of this class in <see cref="WowTileAddressingTests"/>.
    /// </summary>
    public static (float ServerX, float ServerY) TileCentreServer(int fileX, int fileY)
    {
        float half = WowTileAddressing.TileSize * 0.5f;
        float serverX = ((OriginTileIndex - fileY) * WowTileAddressing.TileSize) - half;
        float serverY = ((OriginTileIndex - fileX) * WowTileAddressing.TileSize) - half;
        return (serverX, serverY);
    }

    /// <summary>Which tile's filename indices contain a given server-world X/Y. The inverse of
    /// <see cref="TileCentreServer"/> for any point inside the tile, not just its centre.</summary>
    public static (int FileX, int FileY) TileAtServer(float serverX, float serverY)
    {
        int fileX = OriginTileIndex - 1 - (int)System.MathF.Floor(serverY / WowTileAddressing.TileSize);
        int fileY = OriginTileIndex - 1 - (int)System.MathF.Floor(serverX / WowTileAddressing.TileSize);
        return (fileX, fileY);
    }

    internal static int FloorDiv(int a, int b)
    {
        int q = a / b;
        int r = a % b;
        return r != 0 && (r < 0) != (b < 0) ? q - 1 : q;
    }

    internal static int FloorMod(int a, int b)
    {
        int r = a % b;
        return r != 0 && (r < 0) != (b < 0) ? r + b : r;
    }
}
