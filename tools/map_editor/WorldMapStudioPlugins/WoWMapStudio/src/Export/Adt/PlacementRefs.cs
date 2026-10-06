using System;
using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>Which of a tile's 256 chunks reference a placement (the MCRD/MCRW lists), from on-disk placement coordinates.</summary>
public static class PlacementRefs
{
    /// <summary>Chunk indices (<c>row * 16 + col</c>) overlapped by the box <c>[minX,maxX] × [minZ,maxZ]</c> of raw MDDF/MODF x and z, clipped to the tile.</summary>
    public static IEnumerable<int> ChunksOverlapping(int fileX, int fileY, float minX, float maxX, float minZ, float maxZ)
    {
        const int edge = WowTileAddressing.ChunksPerTileEdge;
        float originX = fileX * WowTileAddressing.TileSize;
        float originZ = fileY * WowTileAddressing.TileSize;
        int col0 = Math.Clamp((int)MathF.Floor((minX - originX) / WowTileAddressing.CellSize), 0, edge - 1);
        int col1 = Math.Clamp((int)MathF.Floor((maxX - originX) / WowTileAddressing.CellSize), 0, edge - 1);
        int row0 = Math.Clamp((int)MathF.Floor((minZ - originZ) / WowTileAddressing.CellSize), 0, edge - 1);
        int row1 = Math.Clamp((int)MathF.Floor((maxZ - originZ) / WowTileAddressing.CellSize), 0, edge - 1);
        for (int row = row0; row <= row1; row++)
        {
            for (int col = col0; col <= col1; col++)
            {
                yield return (row * edge) + col;
            }
        }
    }
}
