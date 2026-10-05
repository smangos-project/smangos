using System;
using System.Collections.Generic;
using System.Linq;

namespace WorldMapStudio.WoW;

public static class WowAdtImportBatchOperationTests
{
    private const int ChunkSize = AdtImportCatalog.HeightPxPerTile; // 256 — one image chunk per ADT tile

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Height_chunks_place_each_mcnk_block_at_its_mapwide_texel_position()
    {
        var tile = new WowTileId("TestMap", 33, 30);
        ImportedTile parsed = SyntheticTile(new ImportedTileId(tile.Map, tile.FileX, tile.FileY));

        IReadOnlyList<ImportedImageChunk> chunks =
            WowAdtImportBatchOperation.BuildHeightChunks(tile, parsed, ChunkSize);

        // A single ADT tile occupies exactly one 256×256 image chunk (plus apron spilling into
        // neighbours).
        Assert.IsTrue(chunks.Count >= 1);

        var byCoord = chunks.ToDictionary(entry => entry.Coord, entry => ToFloats(entry.Pixels));

        foreach ((int localCol, int localRow, ChunkCoord coord) in WowTileAddressing.EnumerateChunks(tile))
        {
            int mapMin = -(WowTileAddressing.TilesPerMapEdge / 2) * WowTileAddressing.ChunksPerTileEdge;
            int texelX = HeightMapper.TexelEdge * (coord.X - mapMin);
            int texelY = HeightMapper.TexelEdge * (coord.Y - mapMin);

            var imageChunk = new ImageChunkCoord(texelX / ChunkSize, texelY / ChunkSize);
            Assert.IsTrue(byCoord.TryGetValue(imageChunk, out float[]? buffer), $"chunk {localCol},{localRow} has an image chunk");

            // The synthetic tile gives every vertex of MCNK slot n the value n, so its whole 16×16
            // block is that value — check the block's centre texel.
            int centre = (((texelY % ChunkSize) + 8) * ChunkSize) + (texelX % ChunkSize) + 8;
            float expected = (localRow * WowTileAddressing.ChunksPerTileEdge) + localCol;
            Assert.AreApproximatelyEqual(expected, buffer![centre], 1e-3, $"slot {localCol},{localRow} value at its texel position");
        }
    }

    /// <summary>
    /// A tile's apron reaches into its neighbours' image chunks, and a chunk row is written whole, so
    /// an apron pixel must be marked unowned — otherwise writing one tile replaces the whole chunk its
    /// neighbour had already filled, and every tile but the last one imported comes out flat.
    /// </summary>
    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void An_apron_never_claims_a_neighbouring_tiles_image_chunk()
    {
        var tile = new WowTileId("TestMap", 33, 30);
        var neighbour = new WowTileId("TestMap", 33, 31);

        ImportedImageChunk own = ChunkOwnedBy(tile);
        Assert.IsTrue(own.Owned.All(owned => owned), "a tile owns every pixel of its own image chunk");

        ImportedImageChunk spill = WowAdtImportBatchOperation
            .BuildHeightChunks(neighbour, SyntheticTile(new ImportedTileId(neighbour.Map, neighbour.FileX, neighbour.FileY)), ChunkSize)
            .Single(chunk => chunk.Coord.Equals(own.Coord));

        Assert.IsTrue(spill.Owned.All(owned => !owned), "a neighbour's apron owns nothing in that chunk");
    }

    /// <summary>The one image chunk holding <paramref name="tile"/>'s own 256×256 block.</summary>
    private static ImportedImageChunk ChunkOwnedBy(WowTileId tile) => WowAdtImportBatchOperation
        .BuildHeightChunks(tile, SyntheticTile(new ImportedTileId(tile.Map, tile.FileX, tile.FileY)), ChunkSize)
        .Single(chunk => chunk.Owned.Any(owned => owned));

    private static ImportedTile SyntheticTile(ImportedTileId id)
    {
        var chunks = new ImportedChunk[ImportedTile.ChunkCount];
        for (int slot = 0; slot < chunks.Length; slot++)
        {
            var outer = new float[ImportedChunk.OuterEdge * ImportedChunk.OuterEdge];
            Array.Fill(outer, (float)slot);
            chunks[slot] = new ImportedChunk
            {
                LocalCol = slot % WowTileAddressing.ChunksPerTileEdge,
                LocalRow = slot / WowTileAddressing.ChunksPerTileEdge,
                OuterHeights = outer,
                InnerHeights = new float[ImportedChunk.InnerEdge * ImportedChunk.InnerEdge],
                Layers = Array.Empty<ImportedLayer>(),
                Holes = new bool[ImportedChunk.HoleEdge * ImportedChunk.HoleEdge],
                AreaId = 0u,
                Flags = 0u,
            };
        }

        return new ImportedTile
        {
            TileId = id,
            Textures = Array.Empty<uint>(),
            HeightTextures = Array.Empty<uint>(),
            Doodads = Array.Empty<ImportedDoodad>(),
            Objects = Array.Empty<ImportedObject>(),
            Chunks = chunks,
        };
    }

    private static float[] ToFloats(byte[] bytes)
    {
        var floats = new float[bytes.Length / 4];
        Buffer.BlockCopy(bytes, 0, floats, 0, bytes.Length);
        return floats;
    }
}
