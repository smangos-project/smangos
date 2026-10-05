using System;
using System.Collections.Generic;
using WoWLib;
using WoWLib.Filesystem;
using WoWLib.Formats.WDT;
using WoWLib.Formats.WDT.Root.Chunks;

namespace WorldMapStudio.WoW;

/// <summary>Per-tile FileDataIDs from a WDT's MAID chunk; 0 where the file does not exist.</summary>
public readonly record struct WowTileFiles(uint Root, uint Obj0, uint Obj1, uint Tex0, uint Lod, uint MapTexture, uint MapTextureN, uint MinimapTexture);

/// <summary>The parts of a map's WDT the importer needs: tile presence, MPHD flags and MAID.</summary>
public sealed class WowWdt
{
    public const int GridEdge = 64;

    public required MapHeaderFlags Flags { get; init; }
    public required bool[] HasAdt { get; init; }

    /// <summary>MAID entries in tile order (<c>y * 64 + x</c>), or null when the WDT has no MAID.</summary>
    public WowTileFiles[]? Files { get; init; }

    public bool BigAlpha => (Flags & MapHeaderFlags.AdtHasBigAlpha) != 0;
    public bool HeightTexturing => (Flags & MapHeaderFlags.AdtHasHeightTexturing) != 0;

    public bool Has(int fileX, int fileY) => HasAdt[(fileY * GridEdge) + fileX];

    public static WowWdt Read(FileSystem fs, string map)
    {
        var wdt = WDT.ForVersion(fs.Version);
        wdt.Read(fs, new FileKey($"world/maps/{map}/{map}.wdt"));
        var root = wdt.Root;

        var has = new bool[GridEdge * GridEdge];
        for (int i = 0; i < has.Length && i < root.Tiles.Count; i++)
        {
            has[i] = (root.Tiles[i].Flags & AreaInfoFlags.HasAdt) != 0;
        }

        WowTileFiles[]? files = null;
        if (root is WoWLib.Formats.WDT.Root.WDTRootShadowlandsPlus plus && plus.MapFdids.Count > 0)
        {
            files = new WowTileFiles[plus.MapFdids.Count];
            for (int i = 0; i < files.Length; i++)
            {
                var m = plus.MapFdids[i];
                files[i] = new WowTileFiles(m.RootAdt, m.Obj0Adt, m.Obj1Adt, m.Tex0Adt, m.LodAdt, m.MapTexture, m.MapTextureN, m.MinimapTexture);
            }
        }

        return new WowWdt { Flags = root.Header.Flags, HasAdt = has, Files = files };
    }
}
