using System;
using System.Collections.Generic;
using System.Linq;

namespace WorldMapStudio.WoW;

public static class TextureMapperTests
{
    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Alpha_block_routes_slots_to_rgb_and_rotates_to_editor_axes()
    {
        const int edge = TextureMapper.AlphaEdge;

        // slot 1 alpha: value = row; slot 2 alpha: value = col. slot 3 absent.
        var slot1 = new byte[edge * edge];
        var slot2 = new byte[edge * edge];
        for (int r = 0; r < edge; r++)
        {
            for (int c = 0; c < edge; c++)
            {
                slot1[(r * edge) + c] = (byte)r;
                slot2[(r * edge) + c] = (byte)c;
            }
        }

        ImportedChunk chunk = Chunk(
            new ImportedLayer(0, -1, null),
            new ImportedLayer(1, -1, slot1),
            new ImportedLayer(2, -1, slot2));

        byte[] block = TextureMapper.AlphaBlock(chunk);

        for (int y = 0; y < edge; y++)
        {
            for (int x = 0; x < edge; x++)
            {
                int o = ((y * edge) + x) * 4;
                // editor[y][x] = wow[edge-1-x][y]
                Assert.AreEqual((byte)(edge - 1 - x), block[o + 0], $"R at {x},{y}");
                Assert.AreEqual((byte)y, block[o + 1], $"G at {x},{y}");
                Assert.AreEqual((byte)0, block[o + 2], "B — slot 3 absent");
                Assert.AreEqual((byte)0, block[o + 3], "A — always zero");
            }
        }
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Slot_assignments_resolve_material_per_texture_and_slot()
    {
        ImportedChunk chunk = Chunk(
            new ImportedLayer(TextureIndex: 0, EffectId: -1, null),
            new ImportedLayer(TextureIndex: 1, EffectId: 7, new byte[TextureMapper.AlphaEdge * TextureMapper.AlphaEdge]));

        var textures = new uint[] { 100, 200 };
        var materials = new Dictionary<(uint, int), int> { [(100, 0)] = 10, [(200, 1)] = 20 };

        IReadOnlyList<AdtSlotAssignment> slots = TextureMapper.SlotAssignments(chunk, textures, materials);

        Assert.AreEqual(2, slots.Count);
        Assert.AreEqual(0, slots[0].SlotIndex);
        Assert.AreEqual(10, slots[0].MaterialId);
        Assert.AreEqual(20, slots[1].MaterialId);
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Required_materials_lists_each_pair_capped_at_four_slots()
    {
        ImportedChunk chunk = Chunk(
            new ImportedLayer(0, -1, null),
            new ImportedLayer(1, -1, null),
            new ImportedLayer(0, -1, null),
            new ImportedLayer(2, -1, null),
            new ImportedLayer(3, -1, null)); // 5th layer — ignored

        var tile = SingleChunkTile(chunk, [1, 2, 3, 4]);

        var pairs = TextureMapper.RequiredMaterials(tile)
            .Select(request => (request.Texture, request.Slot))
            .ToHashSet();

        Assert.IsTrue(pairs.SetEquals(new HashSet<(uint, int)>
        {
            (1, 0), (2, 1), (1, 2), (3, 3),
        }));
    }

    private static ImportedChunk Chunk(params ImportedLayer[] layers) => new()
    {
        LocalCol = 0,
        LocalRow = 0,
        OuterHeights = new float[ImportedChunk.OuterEdge * ImportedChunk.OuterEdge],
        InnerHeights = new float[ImportedChunk.InnerEdge * ImportedChunk.InnerEdge],
        Layers = layers,
        Holes = new bool[ImportedChunk.HoleEdge * ImportedChunk.HoleEdge],
        AreaId = 0u,
        Flags = 0u,
    };

    private static ImportedTile SingleChunkTile(ImportedChunk chunk, uint[] textures)
    {
        var chunks = new ImportedChunk[ImportedTile.ChunkCount];
        Array.Fill(chunks, chunk);
        return new ImportedTile
        {
            TileId = new ImportedTileId("m", 32, 32),
            Textures = textures,
            HeightTextures = new uint[textures.Length],
            Doodads = Array.Empty<ImportedDoodad>(),
            Objects = Array.Empty<ImportedObject>(),
            Chunks = chunks,
        };
    }
}
