using System;
using System.Collections.Generic;
using System.Linq;

namespace WorldMapStudio.WoW;

/// <summary>A flat, empty <see cref="ImportedTile"/> to build a tile that has no client file yet from the editor's terrain images.</summary>
public static class WowBlankTile
{
    public static ImportedTile Create(ImportedTileId id, uint textureFdid, uint areaId = 0)
    {
        var chunks = new List<ImportedChunk>(ImportedTile.ChunkCount);
        for (int row = 0; row < ImportedTile.ChunkEdge; row++)
        {
            for (int col = 0; col < ImportedTile.ChunkEdge; col++)
            {
                chunks.Add(new ImportedChunk
                {
                    LocalCol = col,
                    LocalRow = row,
                    OuterHeights = new float[ImportedChunk.OuterEdge * ImportedChunk.OuterEdge],
                    InnerHeights = new float[ImportedChunk.InnerEdge * ImportedChunk.InnerEdge],
                    Layers = [new ImportedLayer(0, 0, null)],
                    Holes = new bool[ImportedChunk.HoleEdge * ImportedChunk.HoleEdge],
                    AreaId = areaId,
                    Flags = 0,
                });
            }
        }

        return new ImportedTile
        {
            TileId = id,
            Textures = [textureFdid],
            HeightTextures = [0],
            Doodads = [],
            Objects = [],
            Chunks = chunks,
        };
    }

    /// <summary>The MCNK header (x, y) pair for a chunk: server-world north and west coordinates of its corner.</summary>
    public static (float X, float Y) HeaderOrigin(int fileX, int fileY, int col, int row)
    {
        const float mapOrigin = 32 * WowTileAddressing.TileSize;
        return (mapOrigin - (fileY * WowTileAddressing.TileSize) - (row * WowTileAddressing.CellSize),
                mapOrigin - (fileX * WowTileAddressing.TileSize) - (col * WowTileAddressing.CellSize));
    }
}
