using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace WorldMapStudio.WoW;

/// <summary>One ADT tile of one WoW map, in the tile's own file indices — never a project map's chunk space.</summary>
public readonly record struct WowServerTileKey(int WowMapId, int FileX, int FileY)
{
    public override string ToString() => $"map {WowMapId} tile ({FileX}, {FileY})";
}

/// <summary>The extracted-data directories, resolved once on the main thread so workers never touch the project.</summary>
public readonly record struct WowServerRoots(string? Maps, string? Vmaps, string? Mmaps)
{
    public string? MapFile(WowServerTileKey tile) =>
        Maps == null ? null : Path.Combine(Maps, WowMapGrid.FileName(tile.WowMapId, tile.FileX, tile.FileY));

    public string? VmapTileFile(WowServerTileKey tile) =>
        Vmaps == null ? null : Path.Combine(Vmaps, WowMapGrid.VmapTileFileName(tile.WowMapId, tile.FileX, tile.FileY));

    public string? NavTileFile(WowServerTileKey tile) =>
        Mmaps == null ? null : Path.Combine(Mmaps, $"{tile.WowMapId:D3}{tile.FileY:D2}{tile.FileX:D2}.mmtile");
}

/// <summary>Which tiles a square around a point touches.</summary>
public static class WowServerRegion
{
    /// <summary>
    /// The tiles intersecting the square of half-size <paramref name="radius"/> around a server-world X/Y,
    /// nearest first so what the camera is over loads before what it is merely near.
    /// </summary>
    public static IReadOnlyList<(int FileX, int FileY)> TilesAround(float serverX, float serverY, float radius)
    {
        (int lowFileX, int lowFileY) = WowTileAddressing.TileAtServer(serverX + radius, serverY + radius);
        (int highFileX, int highFileY) = WowTileAddressing.TileAtServer(serverX - radius, serverY - radius);
        (int centreFileX, int centreFileY) = WowTileAddressing.TileAtServer(serverX, serverY);

        var tiles = new List<(int FileX, int FileY)>();
        for (int fileY = Math.Max(0, lowFileY); fileY <= Math.Min(WowTileAddressing.TilesPerMapEdge - 1, highFileY); fileY++)
        {
            for (int fileX = Math.Max(0, lowFileX); fileX <= Math.Min(WowTileAddressing.TilesPerMapEdge - 1, highFileX); fileX++)
            {
                tiles.Add((fileX, fileY));
            }
        }

        return tiles
            .OrderBy(tile => Math.Max(Math.Abs(tile.FileX - centreFileX), Math.Abs(tile.FileY - centreFileY)))
            .ThenBy(tile => tile.FileY)
            .ThenBy(tile => tile.FileX)
            .ToList();
    }
}
