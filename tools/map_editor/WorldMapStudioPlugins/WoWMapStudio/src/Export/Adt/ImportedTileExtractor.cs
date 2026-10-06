using System;
using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>The map-wide images a tile's terrain lives in.</summary>
public enum TerrainImage
{
    Height,
    Alpha,
    Holes,
    Area,
    Flags,
    VertexColor,
    VertexLight,
}

/// <summary>Read access to the stored chunks of the map-wide terrain images.</summary>
public interface ITerrainImageSource
{
    int ChunkSize(TerrainImage image);

    /// <summary>The raw pixels of one image chunk, or null when nothing is stored there.</summary>
    byte[]? Chunk(TerrainImage image, ImageChunkCoord coord);
}

/// <summary>
/// The inverse of the import: rebuilds an <see cref="ImportedTile"/> from the editor's map-wide terrain
/// images. Anything the editor does not model — layer textures, placements, liquid, unmapped flag bits —
/// is carried over from <c>template</c>, the tile as last read from the client.
/// </summary>
public static class ImportedTileExtractor
{
    /// <param name="slotTextures">Texture FileDataID per MCNK slot (null = no layer) by chunk (col, row); chunks absent keep the template's layers.</param>
    public static ImportedTile Extract(
        WowTileId tile,
        ImportedTile template,
        ITerrainImageSource source,
        IReadOnlyDictionary<(int Col, int Row), IReadOnlyList<uint?>>? slotTextures = null)
    {
        var textures = new List<uint>(template.Textures);
        var heightTextures = new List<uint>(template.HeightTextures);
        var chunks = new List<ImportedChunk>(ImportedTile.ChunkCount);
        foreach ((int col, int row, ChunkCoord coord) in WowTileAddressing.EnumerateChunks(tile))
        {
            ImportedChunk src = template.Chunks[(row * ImportedTile.ChunkEdge) + col];
            IReadOnlyList<uint?>? slots = null;
            slotTextures?.TryGetValue((src.LocalCol, src.LocalRow), out slots);
            chunks.Add(ExtractChunk(src, coord, source, slots, textures, heightTextures));
        }

        return new ImportedTile
        {
            TileId = template.TileId,
            Textures = textures,
            HeightTextures = heightTextures,
            Doodads = template.Doodads,
            Objects = template.Objects,
            Chunks = chunks,
        };
    }

    /// <summary>The tile with its placements replaced.</summary>
    public static ImportedTile WithPlacements(ImportedTile tile, IReadOnlyList<ImportedDoodad> doodads, IReadOnlyList<ImportedObject> objects) => new()
    {
        TileId = tile.TileId,
        Textures = tile.Textures,
        HeightTextures = tile.HeightTextures,
        Doodads = doodads,
        Objects = objects,
        Chunks = tile.Chunks,
    };

    private static ImportedChunk ExtractChunk(
        ImportedChunk src,
        ChunkCoord coord,
        ITerrainImageSource source,
        IReadOnlyList<uint?>? slotTextures,
        List<uint> textures,
        List<uint> heightTextures)
    {
        float[] outer = src.OuterHeights;
        float[] inner = src.InnerHeights;
        if (ReadBlock(source, TerrainImage.Height, HeightMapper.TexelEdge, 4, coord) is { } height)
        {
            var floats = new float[height.Length / 4];
            Buffer.BlockCopy(height, 0, floats, 0, height.Length);
            (outer, inner) = HeightMapper.FromBlock(floats);
        }

        bool[] holes = ReadBlock(source, TerrainImage.Holes, HoleMapper.HoleEdge, 1, coord) is { } holeBlock
            ? HoleMapper.FromBlock(holeBlock)
            : src.Holes;

        uint areaId = ReadScalar(source, TerrainImage.Area, coord) is { } area ? (uint)area : src.AreaId;
        uint flags = src.Flags;
        if (ReadScalar(source, TerrainImage.Flags, coord) is { } flagValue)
        {
            flags = (src.Flags & ~AreaMapper.RoundTrippedFlagMask) | ((uint)flagValue & AreaMapper.RoundTrippedFlagMask);
        }

        Color[]? colors = src.VertexColors;
        Color[]? innerColors = src.InnerVertexColors;
        if (ReadVertexBlock(source, TerrainImage.VertexColor, coord) is { } colorBlock)
        {
            (colors, innerColors) = VertexColorMapper.FromBlock(colorBlock);
        }

        Color[]? light = src.VertexLight;
        Color[]? innerLight = src.InnerVertexLight;
        if (ReadVertexBlock(source, TerrainImage.VertexLight, coord) is { } lightBlock)
        {
            (light, innerLight) = VertexColorMapper.FromBlock(lightBlock);
        }

        var layers = new List<ImportedLayer>(src.Layers.Count);
        byte[]? alphaBlock = ReadBlock(source, TerrainImage.Alpha, TextureMapper.AlphaEdge, 4, coord);
        int slotCount = slotTextures == null || LastAssigned(slotTextures) < 0 ? src.Layers.Count : Math.Min(TextureMapper.SlotCount, LastAssigned(slotTextures) + 1);
        for (int slot = 0; slot < slotCount; slot++)
        {
            bool existing = slot < src.Layers.Count;
            ImportedLayer layer = existing ? src.Layers[slot] : new ImportedLayer(0, 0, null);
            if (slotTextures != null && slot < slotTextures.Count)
            {
                uint? fdid = slotTextures[slot];
                if (fdid is { } id && (!existing || TextureOf(textures, layer) != id))
                {
                    int index = textures.IndexOf(id);
                    if (index < 0)
                    {
                        index = textures.Count;
                        textures.Add(id);
                        heightTextures.Add(0);
                    }

                    layer = layer with { TextureIndex = index };
                }
            }

            if (alphaBlock != null && slot >= 1 && slot < TextureMapper.SlotCount && (layer.Alpha != null || !existing))
            {
                layer = layer with { Alpha = TextureMapper.AlphaFromBlock(alphaBlock, slot) };
            }

            layers.Add(layer);
        }

        return new ImportedChunk
        {
            LocalCol = src.LocalCol,
            LocalRow = src.LocalRow,
            OuterHeights = outer,
            InnerHeights = inner,
            VertexColors = colors,
            InnerVertexColors = innerColors,
            VertexLight = light,
            InnerVertexLight = innerLight,
            Layers = layers,
            Holes = holes,
            AreaId = areaId,
            Flags = flags,
            Liquid = src.Liquid,
        };
    }

    private static uint? TextureOf(List<uint> textures, ImportedLayer layer) =>
        layer.TextureIndex >= 0 && layer.TextureIndex < textures.Count ? textures[layer.TextureIndex] : null;

    private static int LastAssigned(IReadOnlyList<uint?> slots)
    {
        for (int i = slots.Count - 1; i >= 0; i--)
        {
            if (slots[i] != null)
            {
                return i;
            }
        }

        return -1;
    }

    /// <summary>A vertex colour/light block, or null when it is blank (the import stores nothing for a chunk without one).</summary>
    private static byte[]? ReadVertexBlock(ITerrainImageSource source, TerrainImage image, ChunkCoord coord)
    {
        byte[]? block = ReadBlock(source, image, VertexColorMapper.TexelEdge, 4, coord);
        return block != null && Array.Exists(block, b => b != 0) ? block : null;
    }

    private static float? ReadScalar(ITerrainImageSource source, TerrainImage image, ChunkCoord coord)
    {
        byte[]? texel = ReadBlock(source, image, 1, 4, coord);
        return texel == null ? null : BitConverter.ToSingle(texel, 0);
    }

    /// <summary>One chunk's <paramref name="edge"/>² texel block out of the map-wide image, or null when any of its image chunks is missing.</summary>
    private static byte[]? ReadBlock(ITerrainImageSource source, TerrainImage image, int edge, int stride, ChunkCoord coord)
    {
        int size = source.ChunkSize(image);
        int baseX = edge * (coord.X - WowAdtImportBatchOperation.MapMinChunk.X);
        int baseY = edge * (coord.Y - WowAdtImportBatchOperation.MapMinChunk.Y);
        if (baseX < 0 || baseY < 0)
        {
            return null;
        }

        var block = new byte[edge * edge * stride];
        byte[]? cached = null;
        ImageChunkCoord cachedAt = default;
        for (int by = 0; by < edge; by++)
        {
            for (int bx = 0; bx < edge; bx++)
            {
                int tx = baseX + bx;
                int ty = baseY + by;
                var at = new ImageChunkCoord(tx / size, ty / size);
                if (cached == null || at != cachedAt)
                {
                    cached = source.Chunk(image, at);
                    cachedAt = at;
                    if (cached == null)
                    {
                        return null;
                    }
                }

                Array.Copy(cached, (((ty % size) * size) + (tx % size)) * stride, block, ((by * edge) + bx) * stride, stride);
            }
        }

        return block;
    }
}
