using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Where a <see cref="WowMapTile"/>'s grids sit in the world. A tile's row axis runs along server X
/// decreasing and its column axis along server Y decreasing, from the tile's north-west corner.
/// <c>GridMap::getHeight</c> derives the same mapping (<c>128 * (32 - x / TileSize)</c>, wrapped into the
/// tile); <see cref="WowTileAddressing.TileAtServer"/> is its tile-index half.
///
/// The file name is <c>MMMYYXX</c> — the tile's Y index before its X — so a tile's <c>fileY</c> selects
/// the server-X band and its <c>fileX</c> the server-Y band.
/// </summary>
public static class WowMapGrid
{
    public const float TileSize = WowTileAddressing.TileSize;

    /// <summary>Height cells along one tile edge (the <c>.map</c> V8 grid; V9 has one more vertex per edge).</summary>
    public const int CellEdge = 128;

    public const float CellSize = TileSize / CellEdge;

    /// <summary><c>maps/MMMYYXX.map</c>, as the extractor names it.</summary>
    public static string FileName(int wowMapId, int fileX, int fileY) => $"{wowMapId:D3}{fileY:D2}{fileX:D2}.map";

    /// <summary>
    /// <c>vmaps/MMM_XX_YY.vmtile</c>. Unlike the <c>.map</c> and <c>.mmtile</c> names, the server's
    /// <c>StaticMapTree::getTileFileName</c> writes the tile's row first, so the file's first index is the
    /// ADT's <c>fileX</c>.
    /// </summary>
    public static string VmapTileFileName(int wowMapId, int fileX, int fileY) => $"{wowMapId:D3}_{fileX:D2}_{fileY:D2}.vmtile";

    /// <summary>The server-world X/Y of a grid position (fractional cells) inside tile (<paramref name="fileX"/>, <paramref name="fileY"/>).</summary>
    public static (float ServerX, float ServerY) ToServer(float row, float col, int fileX, int fileY) => (
        ((WowTileAddressing.OriginTileIndex - fileY) * TileSize) - (row * CellSize),
        ((WowTileAddressing.OriginTileIndex - fileX) * TileSize) - (col * CellSize));

    /// <summary>The inverse of <see cref="ToServer"/>: fractional grid position of a server-world X/Y relative to a tile.</summary>
    public static (float Row, float Col) ToGrid(float serverX, float serverY, int fileX, int fileY) => (
        (((WowTileAddressing.OriginTileIndex - fileY) * TileSize) - serverX) / CellSize,
        (((WowTileAddressing.OriginTileIndex - fileX) * TileSize) - serverY) / CellSize);

    /// <summary>A grid position and height as a Godot-space point.</summary>
    public static Vector3 ToGodot(float row, float col, float height, int fileX, int fileY)
    {
        (float serverX, float serverY) = ToServer(row, col, fileX, fileY);
        return WowCoordinates.ToGodot(new Vector3(serverX, serverY, height));
    }
}
