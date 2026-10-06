using System;
using System.Collections.Generic;
using System.Linq;
using Godot;
using WoWLib;
using WoWLib.Filesystem;
using WoWLib.Formats.ADT;
using WoWLib.Formats.ADT.Chunks;
using WoWLib.Formats.Common;

namespace WorldMapStudio.WoW;

/// <summary>
/// Writes an <see cref="ImportedTile"/> back to a client ADT through wowlib. The tile is applied onto a
/// freshly read copy of the client's own file, so everything the editor does not model (shadow maps,
/// texture params, water, flying bounds) survives untouched.
/// </summary>
public static class WowAdtTileWriter
{
    private const int Outer = ImportedChunk.OuterEdge;
    private const int Inner = ImportedChunk.InnerEdge;
    private const int AlphaSize = 64 * 64;

    /// <summary>Applies <paramref name="tile"/> onto <paramref name="adt"/> (terrain, layers, textures) and placements matched by unique id: existing ones are moved, added ones appended (by FileDataID) and removed ones dropped, with the per-chunk reference lists rebuilt.</summary>
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

        var heightsChanged = new bool[tile.Chunks.Count];
        for (int i = 0; i < tile.Chunks.Count; i++)
        {
            heightsChanged[i] = ApplyChunk(adt.Chunks[i], tile.Chunks[i]);
        }

        RebuildNormals(adt, tile, heightsChanged);

        ApplyPlacements(adt, tile);
        ApplyWater(adt, tile);
    }

    /// <summary>Rewrites only the cells whose liquid differs from what the file holds, so untouched water stays byte-equal.</summary>
    private static void ApplyWater(ADTBfaPlus adt, ImportedTile tile)
    {
        Mh2OData? water = adt.Water;
        if (water == null)
        {
            return;
        }

        while (water.Cells.Count < ImportedTile.ChunkCount)
        {
            water.Cells.Add(new MapChunkLiquid());
        }

        for (int i = 0; i < ImportedTile.ChunkCount; i++)
        {
            ImportedChunkLiquid? wanted = tile.Chunks[i].Liquid;
            if (ImportedTileComparer.LiquidEqual(WowAdtTileReader.ConvertLiquid(water.Cells[i]), wanted))
            {
                continue;
            }

            MapChunkLiquid cell = water.Cells[i];
            cell.Instances.Clear();
            cell.HasAttributes = wanted?.HasAttributes ?? false;
            cell.Fishable = wanted?.Fishable ?? 0;
            cell.Deep = wanted?.Fatigue ?? 0;
            foreach (ImportedLiquidInstance source in wanted?.Instances ?? [])
            {
                cell.Instances.Add(ToWowLib(source));
            }
        }
    }

    private static LiquidInstance ToWowLib(ImportedLiquidInstance source)
    {
        var instance = new LiquidInstance
        {
            LiquidType = source.LiquidType,
            LiquidObjectOrLvf = source.LiquidObjectOrFormat,
            MinHeight = source.MinHeight,
            MaxHeight = source.MaxHeight,
            XOffset = (byte)source.XOffset,
            YOffset = (byte)source.YOffset,
            Width = (byte)source.Width,
            Height = (byte)source.Height,
        };
        foreach (byte b in source.Exists ?? [])
        {
            instance.ExistsBitmap.Add(b);
        }

        foreach (float h in source.Heights ?? [])
        {
            instance.Heightmap.Add(h);
        }

        foreach (byte d in source.Depths ?? [])
        {
            instance.Depthmap.Add(d);
        }

        if ((LiquidVertexFormat)source.LiquidObjectOrFormat is LiquidVertexFormat.HeightUv or LiquidVertexFormat.HeightUvDepth)
        {
            for (int i = 0; i < (source.Width + 1) * (source.Height + 1); i++)
            {
                instance.Uvmap.Add(new UvMapEntry());
            }
        }

        return instance;
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

        string lodPath = $"world/maps/{id.Map}/{id.Map}_{id.FileX}_{id.FileY}_obj1.adt";
        if (fs.Exists(lodPath) &&
            AdtObj1Patcher.Patch(
                fs.ReadFile(lodPath),
                tile.Doodads.Select(d => d.UniqueId).ToHashSet(),
                tile.Objects.Select(o => o.UniqueId).ToHashSet()) is { } patched)
        {
            fs.AddFile(lodPath, patched);
        }
    }

    private static void ApplyPlacements(ADTBfaPlus adt, ImportedTile tile)
    {
        var doodadUids = new HashSet<uint>(tile.Doodads.Select(d => d.UniqueId));
        var objectUids = new HashSet<uint>(tile.Objects.Select(o => o.UniqueId));
        var existingDoodads = Items(adt.DoodadPlacements);
        var existingObjects = Items(adt.WmoPlacements);
        bool membershipChanged =
            !doodadUids.SetEquals(existingDoodads.Select(p => p.UniqueId)) ||
            !objectUids.SetEquals(existingObjects.Select(p => p.UniqueId));

        if (membershipChanged)
        {
            adt.DoodadPlacements.Clear();
            foreach (var p in existingDoodads.Where(p => doodadUids.Contains(p.UniqueId)))
            {
                adt.DoodadPlacements.Add(p);
            }

            adt.WmoPlacements.Clear();
            foreach (var p in existingObjects.Where(p => objectUids.Contains(p.UniqueId)))
            {
                adt.WmoPlacements.Add(p);
            }

            var knownDoodads = new HashSet<uint>(Items(adt.DoodadPlacements).Select(p => p.UniqueId));
            foreach (var d in tile.Doodads)
            {
                if (knownDoodads.Add(d.UniqueId))
                {
                    adt.DoodadPlacements.Add(new SmDoodadDef { NameId = d.ModelFdid, UniqueId = d.UniqueId });
                }
            }

            var knownObjects = new HashSet<uint>(Items(adt.WmoPlacements).Select(p => p.UniqueId));
            foreach (var o in tile.Objects)
            {
                if (knownObjects.Add(o.UniqueId))
                {
                    adt.WmoPlacements.Add(new SmMapObjDef { NameId = o.WmoFdid, UniqueId = o.UniqueId });
                }
            }
        }

        foreach (var d in tile.Doodads)
        {
            foreach (var p in adt.DoodadPlacements)
            {
                if (p.UniqueId == d.UniqueId)
                {
                    p.Position = ToC3(d.Position);
                    p.Rotation = ToC3(d.RotationDegrees);
                    p.Scale = d.Scale;
                    p.Flags = (DoodadDefFlags)d.Flags;
                    if (membershipChanged && p.NameId == d.ModelFdid)
                    {
                        p.Flags |= DoodadDefFlags.EntryIsFdid;
                    }
                }
            }
        }

        foreach (var o in tile.Objects)
        {
            foreach (var p in adt.WmoPlacements)
            {
                if (p.UniqueId == o.UniqueId)
                {
                    p.Position = ToC3(o.Position);
                    p.Rotation = ToC3(o.RotationDegrees);
                    p.Extents.Min = ToC3(o.LowerBounds);
                    p.Extents.Max = ToC3(o.UpperBounds);
                    p.Flags = (MapObjDefFlags)o.Flags;
                    p.DoodadSet = o.DoodadSet;
                    p.NameSet = o.NameSet;
                    p.Scale = o.Scale;
                    if (membershipChanged && p.NameId == o.WmoFdid)
                    {
                        p.Flags |= MapObjDefFlags.EntryIsFdid;
                    }
                }
            }
        }

        if (membershipChanged)
        {
            RebuildRefs(adt, tile.TileId);
        }
    }

    /// <summary>Rewrites every chunk's MCRD/MCRW lists from the placement positions; indices into the placement lists shift when records are added or removed.</summary>
    private static void RebuildRefs(ADTBfaPlus adt, ImportedTileId id)
    {
        foreach (var chunk in adt.Chunks)
        {
            chunk.DoodadRefs.Clear();
            chunk.ObjectRefs.Clear();
        }

        for (int i = 0; i < adt.DoodadPlacements.Count; i++)
        {
            var p = adt.DoodadPlacements[i];
            foreach (int c in PlacementRefs.ChunksOverlapping(id.FileX, id.FileY, p.Position.X, p.Position.X, p.Position.Z, p.Position.Z))
            {
                adt.Chunks[c].DoodadRefs.Add((uint)i);
            }
        }

        for (int i = 0; i < adt.WmoPlacements.Count; i++)
        {
            var e = adt.WmoPlacements[i].Extents;
            foreach (int c in PlacementRefs.ChunksOverlapping(id.FileX, id.FileY,
                         Math.Min(e.Min.X, e.Max.X), Math.Max(e.Min.X, e.Max.X),
                         Math.Min(e.Min.Z, e.Max.Z), Math.Max(e.Min.Z, e.Max.Z)))
            {
                adt.Chunks[c].ObjectRefs.Add((uint)i);
            }
        }
    }

    private static List<T> Items<T>(Vector<T> v)
    {
        var list = new List<T>(v.Count);
        for (int i = 0; i < v.Count; i++)
        {
            list.Add(v[i]);
        }

        return list;
    }

    private static C3Vector ToC3(Vector3 v) => new() { X = v.X, Y = v.Y, Z = v.Z };

    /// <summary>Recomputes MCNR for chunks whose heights changed and the neighbours that read across their edges.</summary>
    private static void RebuildNormals(ADTBfaPlus adt, ImportedTile tile, bool[] heightsChanged)
    {
        const int edge = ImportedTile.ChunkEdge;
        for (int i = 0; i < heightsChanged.Length; i++)
        {
            if (!heightsChanged[i])
            {
                continue;
            }

            for (int dr = -1; dr <= 1; dr++)
            {
                for (int dc = -1; dc <= 1; dc++)
                {
                    int row = (i / edge) + dr, col = (i % edge) + dc;
                    if (row < 0 || col < 0 || row >= edge || col >= edge)
                    {
                        continue;
                    }

                    var normals = AdtNormals.Compute(tile.Chunks, col, row);
                    var target = adt.Chunks[(row * edge) + col].Normals;
                    for (int v = 0; v < target.Count; v++)
                    {
                        for (int k = 0; k < 3; k++)
                        {
                            target[v].Normal[k] = normals[(v * 3) + k];
                        }
                    }
                }
            }
        }
    }

    // True when the chunk's heights differ from the ones it held.
    private static bool ApplyChunk(MapChunkCataPlus c, ImportedChunk src)
    {
        var header = c.Header;
        float baseZ = header.Position.Z;
        var old = new float[c.Heights.Count];
        for (int i = 0; i < old.Length; i++)
        {
            old[i] = c.Heights[i];
        }

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

        ApplyColors(c.VertexColors, src.VertexColors, src.InnerVertexColors, (v, k) => { v.R = k.R; v.G = k.G; v.B = k.B; v.A = k.A; });
        ApplyColors(c.VertexLighting, src.VertexLight, src.InnerVertexLight, (v, k) => { v.R = k.R; v.G = k.G; v.B = k.B; v.A = k.A; });

        var oldFlags = new LayerFlags[c.Layers.Count];
        for (int i = 0; i < oldFlags.Length; i++)
        {
            oldFlags[i] = c.Layers[i].Flags;
        }

        c.Layers.Clear();
        c.AlphaMaps.Clear();
        for (int i = 0; i < src.Layers.Count; i++)
        {
            c.Layers.Add(new SMLayer
            {
                TextureId = (uint)src.Layers[i].TextureIndex,
                EffectId = src.Layers[i].EffectId,
                Flags = (i < oldFlags.Length ? oldFlags[i] & ~LayerFlags.UseAlphaMap : 0)
                    | (src.Layers[i].Alpha != null ? LayerFlags.UseAlphaMap : 0),
            });
            var map = new Vector<byte>();
            if (src.Layers[i].Alpha is { Length: AlphaSize } a)
            {
                map.CopyFrom(a);
            }

            c.AlphaMaps.Add(map);
        }

        if (old.Length != c.Heights.Count)
        {
            return true;
        }

        for (int i = 0; i < old.Length; i++)
        {
            if (MathF.Abs(old[i] - c.Heights[i]) > 0.001f)
            {
                return true;
            }
        }

        return false;
    }

    private static void ApplyColors<T>(Vector<T> dst, Color[]? outer, Color[]? inner, Action<T, (byte R, byte G, byte B, byte A)> set)
        where T : class
    {
        if (outer == null || inner == null || dst.Count < (Outer * Outer) + (Inner * Inner))
        {
            return;
        }

        const int rowStride = Outer + Inner;
        for (int y = 0; y < Outer; y++)
        {
            for (int x = 0; x < Outer; x++)
            {
                set(dst[(y * rowStride) + x], Bytes(outer[(y * Outer) + x]));
            }

            if (y < Inner)
            {
                for (int x = 0; x < Inner; x++)
                {
                    set(dst[(y * rowStride) + Outer + x], Bytes(inner[(y * Inner) + x]));
                }
            }
        }
    }

    private static (byte, byte, byte, byte) Bytes(Color c) =>
        ((byte)MathF.Round(c.R * 255f), (byte)MathF.Round(c.G * 255f), (byte)MathF.Round(c.B * 255f), (byte)MathF.Round(c.A * 255f));

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
