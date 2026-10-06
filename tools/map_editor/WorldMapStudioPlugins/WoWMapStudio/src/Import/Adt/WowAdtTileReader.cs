using System;
using System.Collections.Generic;
using Godot;
using WoWLib;
using WoWLib.Filesystem;
using WoWLib.Formats.ADT;
using WoWLib.Formats.ADT.Chunks;
using WoWLib.Formats;
using WoWLib.Formats.Common;

namespace WorldMapStudio.WoW;

/// <summary>Decodes one client ADT tile (root, <c>_tex0</c> and <c>_obj0</c>) through wowlib into the
/// version-neutral <see cref="ImportedTile"/>.</summary>
public static class WowAdtTileReader
{
    private const int Outer = ImportedChunk.OuterEdge;
    private const int Inner = ImportedChunk.InnerEdge;
    private const int RowStride = Outer + Inner;
    private const int AlphaSize = 64 * 64;

    public static ImportedTile Read(FileSystem fs, WowWdt wdt, ImportedTileId id)
    {
        var key = TileKey(wdt, id);
        var adt = ADT.ForVersion(fs.Version);
        using (adt)
        {
            adt.Read(fs, key, wdt.BigAlpha ? AlphaFormat.Highres8Bit : AlphaFormat.Lowres4Bit);
            return Convert(adt, id, WowFileDataIds.Shared);
        }
    }

    private static FileKey TileKey(WowWdt wdt, ImportedTileId id)
    {
        if (wdt.Files != null)
        {
            uint fdid = wdt.Files[(id.FileY * WowWdt.GridEdge) + id.FileX].Root;
            if (fdid != 0)
            {
                return new FileKey(new FileDataId(fdid));
            }
        }

        return new FileKey($"world/maps/{id.Map}/{id.Map}_{id.FileX}_{id.FileY}.adt");
    }

    internal static ImportedTile Convert(ADT adt, ImportedTileId id, WowFileDataIds ids)
    {
        var diffuse = new List<uint>();
        var height = new List<uint>();
        if (adt is ADTBfaPlus bfa)
        {
            foreach (uint d in bfa.DiffuseTextureIds)
            {
                diffuse.Add(d);
            }
            for (int i = 0; i < diffuse.Count; i++)
            {
                height.Add(i < bfa.HeightTextureIds.Count ? bfa.HeightTextureIds[i] : 0);
            }
        }

        var doodads = new List<ImportedDoodad>(adt.DoodadPlacements.Count);
        foreach (var d in adt.DoodadPlacements)
        {
            uint fdid = (d.Flags & DoodadDefFlags.EntryIsFdid) != 0 ? d.NameId : PathFdid(adt.ModelFilenames, adt.ModelNameOffsets, d.NameId, ids);
            doodads.Add(new ImportedDoodad(fdid, d.UniqueId, ToVector(d.Position), ToVector(d.Rotation), d.Scale, (ushort)d.Flags));
        }

        var objects = new List<ImportedObject>(adt.WmoPlacements.Count);
        foreach (var o in adt.WmoPlacements)
        {
            uint fdid = (o.Flags & MapObjDefFlags.EntryIsFdid) != 0 ? o.NameId : PathFdid(adt.WmoFilenames, adt.WmoNameOffsets, o.NameId, ids);
            objects.Add(new ImportedObject(fdid, o.UniqueId, ToVector(o.Position), ToVector(o.Rotation), ToVector(o.Extents.Min), ToVector(o.Extents.Max), (ushort)o.Flags, o.DoodadSet, o.NameSet, o.Scale));
        }

        Mh2OData? water = adt is ADTBfaPlus w ? w.Water : null;
        var chunks = new List<ImportedChunk>(adt.Chunks.Count);
        for (int i = 0; i < adt.Chunks.Count; i++)
        {
            var liquid = water != null && i < water.Cells.Count ? ConvertLiquid(water.Cells[i]) : null;
            chunks.Add(ConvertChunk((MapChunkCataPlus)adt.Chunks[i], liquid));
        }

        return new ImportedTile
        {
            TileId = id,
            Textures = diffuse,
            HeightTextures = height,
            Doodads = doodads,
            Objects = objects,
            Chunks = chunks,
        };
    }

    private static uint PathFdid(StringBlock names, Vector<uint> offsets, uint index, WowFileDataIds ids)
    {
        if (index >= offsets.Count)
        {
            return 0;
        }
        return ids.IdOf(names.At(offsets[(int)index])) ?? 0;
    }

    private static Vector3 ToVector(C3Vector v) => new(v.X, v.Y, v.Z);

    private static ImportedChunk ConvertChunk(MapChunkCataPlus c, ImportedChunkLiquid? liquid)
    {
        var header = c.Header;
        float baseZ = header.Position.Z;
        var outer = new float[Outer * Outer];
        var inner = new float[Inner * Inner];
        if (c.Heights.Count >= Outer * Outer + (Inner * Inner))
        {
            for (int y = 0; y < Outer; y++)
            {
                for (int x = 0; x < Outer; x++)
                {
                    outer[(y * Outer) + x] = c.Heights[(y * RowStride) + x] + baseZ;
                }
            }
            for (int y = 0; y < Inner; y++)
            {
                for (int x = 0; x < Inner; x++)
                {
                    inner[(y * Inner) + x] = c.Heights[(y * RowStride) + Outer + x] + baseZ;
                }
            }
        }

        bool hasMccv = (header.Flags & MapChunkFlags.HasMccv) != 0 && c.VertexColors.Count >= Outer * Outer + (Inner * Inner);
        Color[]? mccv = null, mccvInner = null, mclv = null, mclvInner = null;
        if (hasMccv)
        {
            (mccv, mccvInner) = Grids(i => Bgra(c.VertexColors[i]));
        }
        if (c.VertexLighting.Count >= Outer * Outer + (Inner * Inner))
        {
            (mclv, mclvInner) = Grids(i => Argb(c.VertexLighting[i]));
        }

        var layers = new List<ImportedLayer>(c.Layers.Count);
        for (int i = 0; i < c.Layers.Count; i++)
        {
            var l = c.Layers[i];
            byte[]? alpha = null;
            if (i > 0 && i < c.AlphaMaps.Count && c.AlphaMaps[i].Count == AlphaSize)
            {
                alpha = c.AlphaMaps[i].ToArray();
            }
            layers.Add(new ImportedLayer((int)l.TextureId, l.EffectId, alpha));
        }

        return new ImportedChunk
        {
            LocalCol = (int)header.IndexX,
            LocalRow = (int)header.IndexY,
            OuterHeights = outer,
            InnerHeights = inner,
            VertexColors = mccv,
            InnerVertexColors = mccvInner,
            VertexLight = mclv,
            InnerVertexLight = mclvInner,
            Layers = layers,
            Holes = Holes(header),
            AreaId = header.AreaId,
            Flags = (uint)header.Flags,
            Liquid = liquid,
        };
    }

    private static (Color[] Outer, Color[] Inner) Grids(Func<int, Color> at)
    {
        var o = new Color[Outer * Outer];
        for (int y = 0; y < Outer; y++)
        {
            for (int x = 0; x < Outer; x++)
            {
                o[(y * Outer) + x] = at((y * RowStride) + x);
            }
        }
        var n = new Color[Inner * Inner];
        for (int y = 0; y < Inner; y++)
        {
            for (int x = 0; x < Inner; x++)
            {
                n[(y * Inner) + x] = at((y * RowStride) + Outer + x);
            }
        }
        return (o, n);
    }

    private static Color Bgra(CImVector v) => new(v.R / 255f, v.G / 255f, v.B / 255f, v.A / 255f);

    private static Color Argb(CArgb v) => new(v.R / 255f, v.G / 255f, v.B / 255f, v.A / 255f);

    private static bool[] Holes(SMChunk h)
    {
        var holes = new bool[ImportedChunk.HoleEdge * ImportedChunk.HoleEdge];
        if ((h.Flags & MapChunkFlags.HighResHoles) != 0)
        {
            // 8x8 bits (row-major); a 4x4 cell is cut when any of its four quarter-cells is.
            for (int y = 0; y < 4; y++)
            {
                for (int x = 0; x < 4; x++)
                {
                    bool cut = false;
                    for (int q = 0; q < 4; q++)
                    {
                        int bit = (((y * 2) + (q >> 1)) * 8) + (x * 2) + (q & 1);
                        cut |= ((h.HolesHighRes >> bit) & 1) != 0;
                    }
                    holes[(y * 4) + x] = cut;
                }
            }
        }
        else
        {
            for (int i = 0; i < holes.Length; i++)
            {
                holes[i] = ((h.HolesLowRes >> i) & 1) != 0;
            }
        }
        return holes;
    }

    internal static ImportedChunkLiquid? ConvertLiquid(MapChunkLiquid cell)
    {
        if (cell.Empty)
        {
            return null;
        }

        var instances = new List<ImportedLiquidInstance>(cell.Instances.Count);
        foreach (var i in cell.Instances)
        {
            instances.Add(new ImportedLiquidInstance
            {
                LiquidType = i.LiquidType,
                LiquidObjectOrFormat = i.LiquidObjectOrLvf,
                MinHeight = i.MinHeight,
                MaxHeight = i.MaxHeight,
                XOffset = i.XOffset,
                YOffset = i.YOffset,
                Width = i.Width,
                Height = i.Height,
                Exists = i.ExistsBitmap.Count > 0 ? i.ExistsBitmap.ToArray() : null,
                Heights = i.Heightmap.Count > 0 ? i.Heightmap.ToArray() : null,
                Depths = i.Depthmap.Count > 0 ? i.Depthmap.ToArray() : null,
            });
        }

        return new ImportedChunkLiquid { HasAttributes = cell.HasAttributes, Fishable = cell.Fishable, Fatigue = cell.Deep, Instances = instances };
    }
}
