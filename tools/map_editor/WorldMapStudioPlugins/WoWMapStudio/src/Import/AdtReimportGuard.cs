using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace WorldMapStudio.WoW;

/// <summary>Finds imported tiles that were edited after they were imported, which a re-import would overwrite.</summary>
public static class AdtReimportGuard
{
    /// <summary>The chunk-coordinate rectangle a tile's 256 chunks fill.</summary>
    public static (ChunkCoord Min, ChunkCoord Max) ChunkRange(int fileX, int fileY)
    {
        var tile = new WowTileId("", fileX, fileY);
        ChunkCoord a = WowTileAddressing.ChunkAt(tile, 0, 0);
        ChunkCoord b = WowTileAddressing.ChunkAt(tile, WowTileAddressing.ChunksPerTileEdge - 1, WowTileAddressing.ChunksPerTileEdge - 1);
        return (new ChunkCoord(System.Math.Min(a.X, b.X), System.Math.Min(a.Y, b.Y)),
                new ChunkCoord(System.Math.Max(a.X, b.X), System.Math.Max(a.Y, b.Y)));
    }

    /// <summary>The tiles among <paramref name="rows"/> with a chunk edit stamped after the tile's import time.</summary>
    public static async Task<IReadOnlyList<(int X, int Y)>> EditedTilesAsync(
        EditorStorage storage,
        MapId map,
        IEnumerable<AdtImportTileRecord> rows)
    {
        var edited = new List<(int X, int Y)>();
        foreach (AdtImportTileRecord row in rows)
        {
            (ChunkCoord min, ChunkCoord max) = ChunkRange(row.FileX, row.FileY);
            IReadOnlyList<ChunkChange> changes = await storage.LoadChunksInRangeAsync(map, min, max).ConfigureAwait(false);
            if (changes.Any(change => change.LastEditedUtc > row.ImportedAtUtc))
            {
                edited.Add((row.FileX, row.FileY));
            }
        }

        return edited;
    }

    public static string Describe(IReadOnlyList<(int X, int Y)> edited) =>
        string.Join(", ", edited.Take(12).Select(tile => $"{tile.X},{tile.Y}")) + (edited.Count > 12 ? ", ..." : "");
}
