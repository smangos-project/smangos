using System;
using WoWLib;
using WoWLib.Filesystem;
using WoWLib.Formats.ADT;
using WoWLib.Formats.ADT.Chunks;

namespace WorldMapStudio.WoW;

/// <summary>
/// Writes an <see cref="ImportedTile"/> back to a client ADT through wowlib. The tile is applied onto a
/// freshly read copy of the client's own file, so everything the editor does not model (shadow maps,
/// normals, texture params, water, flying bounds) survives untouched.
/// </summary>
public static class WowAdtTileWriter
{
    private const int Outer = ImportedChunk.OuterEdge;
    private const int Inner = ImportedChunk.InnerEdge;
    private const int AlphaSize = 64 * 64;

    /// <summary>Applies <paramref name="tile"/> onto <paramref name="adt"/> (terrain, layers, textures); placements are not yet touched.</summary>
    public static void Apply(ADTBfaPlus adt, ImportedTile tile)
    {
        if (tile.Chunks.Count != ImportedTile.ChunkCount || adt.Chunks.Count != ImportedTile.ChunkCount)
        {
            throw new ArgumentException("an ADT tile has exactly 256 chunks");
        }

        adt.DiffuseTextureIds.Clear();
        adt.HeightTextureIds.Clear();
        for (int i = 0; i < tile.Textures.Count; i++)
        {
            adt.DiffuseTextureIds.Add(tile.Textures[i]);
            adt.HeightTextureIds.Add(i < tile.HeightTextures.Count ? tile.HeightTextures[i] : 0);
        }

        for (int i = 0; i < tile.Chunks.Count; i++)
        {
            ApplyChunk(adt.Chunks[i], tile.Chunks[i]);
        }
    }

    /// <summary>Reads the tile's current client file, applies <paramref name="tile"/> and writes it into the filesystem's project overlay.</summary>
    public static void Write(FileSystem fs, WowWdt wdt, ImportedTile tile)
    {
        var id = tile.TileId;
        var key = new FileKey($"world/maps/{id.Map}/{id.Map}_{id.FileX}_{id.FileY}.adt");
        var alpha = wdt.BigAlpha ? AlphaFormat.Highres8Bit : AlphaFormat.Lowres4Bit;
        using var adt = new ADTBfaPlus();
        adt.Read(fs, key, alpha);
        Apply(adt, tile);
        adt.Write(fs, key, alpha);
    }

    private static void ApplyChunk(MapChunkCataPlus c, ImportedChunk src)
    {
        var header = c.Header;
        float baseZ = header.Position.Z;
        c.Heights.Clear();
        for (int y = 0; y < Outer; y++)
        {
            for (int x = 0; x < Outer; x++)
            {
                c.Heights.Add(src.OuterHeights[(y * Outer) + x] - baseZ);
            }

            if (y < Inner)
            {
                for (int x = 0; x < Inner; x++)
                {
                    c.Heights.Add(src.InnerHeights[(y * Inner) + x] - baseZ);
                }
            }
        }

        header.AreaId = src.AreaId;
        header.Flags = (MapChunkFlags)src.Flags;
        SetHoles(header, src.Holes);

        c.Layers.Clear();
        c.AlphaMaps.Clear();
        for (int i = 0; i < src.Layers.Count; i++)
        {
            c.Layers.Add(new SMLayer { TextureId = (uint)src.Layers[i].TextureIndex, EffectId = src.Layers[i].EffectId });
            var map = new Vector<byte>();
            if (src.Layers[i].Alpha is { Length: AlphaSize } a)
            {
                map.CopyFrom(a);
            }

            c.AlphaMaps.Add(map);
        }
    }

    private static void SetHoles(SMChunk header, bool[] holes)
    {
        // Written as the 8x8 high-res mask; each 4x4 hole cell covers a 2x2 block of bits.
        ulong mask = 0;
        for (int y = 0; y < 4; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                if (!holes[(y * 4) + x])
                {
                    continue;
                }

                for (int q = 0; q < 4; q++)
                {
                    mask |= 1UL << ((((y * 2) + (q >> 1)) * 8) + (x * 2) + (q & 1));
                }
            }
        }

        header.Flags |= MapChunkFlags.HighResHoles;
        header.SetHolesHighRes(mask);
    }
}
