using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>
/// Turns one <see cref="ImportedTile"/>'s texture layers into what the editor stores: per chunk, the
/// slot assignments the tile component carries (which <c>(texture, slot)</c> material
/// occupies each MCNK slot, plus its ground-effect id), and one editor-oriented RGBA block for the
/// map-wide <c>adt_alpha</c> image — slot 1 → R, slot 2 → G, slot 3 → B, the base slot opaque with no
/// alpha. Every non-empty alpha byte round-trips exactly through the identity <c>channel_mask</c>
/// remap the material binds.
/// </summary>
public static class TextureMapper
{
    /// <summary>MCLY layers per MCNK.</summary>
    public const int SlotCount = 4;

    /// <summary>Alpha texels along one chunk edge — 1:1 with a decoded MCAL grid.</summary>
    public const int AlphaEdge = 64;

    /// <summary>Every <c>(texture path, slot, effectId)</c> the tile's chunks reference — the input to
    /// the material catalog. The MCLY <c>effectId</c> is now a
    /// property of the texture (the material), so it rides along here; a texture that carries
    /// different effect ids in different chunks yields several triples and the catalog picks one.</summary>
    public static IEnumerable<(uint Texture, int Slot, int EffectId)> RequiredMaterials(ImportedTile tile)
    {
        foreach (ImportedChunk chunk in tile.Chunks)
        {
            for (int slot = 0; slot < chunk.Layers.Count && slot < SlotCount; slot++)
            {
                ImportedLayer layer = chunk.Layers[slot];
                if (layer.TextureIndex >= 0 && layer.TextureIndex < tile.Textures.Count)
                {
                    yield return (tile.Textures[layer.TextureIndex], slot, layer.EffectId);
                }
            }
        }
    }

    /// <summary>One chunk's slot assignments, in slot order. The slot's layer is resolved by the
    /// component from its own slot layer ids, so only the material is
    /// carried here.</summary>
    public static IReadOnlyList<AdtSlotAssignment> SlotAssignments(
        ImportedChunk chunk,
        IReadOnlyList<uint> textures,
        IReadOnlyDictionary<(uint Texture, int Slot), int> materialByPair)
    {
        var result = new List<AdtSlotAssignment>();
        for (int slot = 0; slot < chunk.Layers.Count && slot < SlotCount; slot++)
        {
            ImportedLayer layer = chunk.Layers[slot];
            int? materialId = layer.TextureIndex >= 0 && layer.TextureIndex < textures.Count
                && materialByPair.TryGetValue((textures[layer.TextureIndex], slot), out int id)
                ? id
                : null;

            result.Add(new AdtSlotAssignment(slot, LayerId: null, materialId));
        }

        return result;
    }

    /// <summary>The chunk's coverage as one editor-oriented RGBA 64×64 block, row-major, byte per
    /// component: slot 1 → R, slot 2 → G, slot 3 → B, A left zero. The base slot has no alpha.</summary>
    public static byte[] AlphaBlock(ImportedChunk chunk)
    {
        var block = new byte[AlphaEdge * AlphaEdge * 4];
        for (int slot = 1; slot < chunk.Layers.Count && slot < SlotCount; slot++)
        {
            byte[]? mcal = chunk.Layers[slot].Alpha;
            if (mcal == null)
            {
                continue;
            }

            int component = slot - 1;
            for (int y = 0; y < AlphaEdge; y++)
            {
                for (int x = 0; x < AlphaEdge; x++)
                {
                    // Inverse of WowChunkGridOrientation's per-chunk quarter turn: editor[y][x] = wow[n-1-x][y].
                    block[(((y * AlphaEdge) + x) * 4) + component] = mcal[((AlphaEdge - 1 - x) * AlphaEdge) + y];
                }
            }
        }

        return block;
    }

    /// <summary>The inverse of <see cref="AlphaBlock"/>: the 64×64 MCAL grid of <paramref name="slot"/> (1-3).</summary>
    public static byte[] AlphaFromBlock(byte[] block, int slot)
    {
        var mcal = new byte[AlphaEdge * AlphaEdge];
        int component = slot - 1;
        for (int y = 0; y < AlphaEdge; y++)
        {
            for (int x = 0; x < AlphaEdge; x++)
            {
                mcal[((AlphaEdge - 1 - x) * AlphaEdge) + y] = block[(((y * AlphaEdge) + x) * 4) + component];
            }
        }

        return mcal;
    }
}
