using System.Collections.Generic;
using WoWLib;
using WoWLib.Filesystem;
using WoWLib.Formats.WDT;
using WoWLib.Formats.WDT.Root;
using WoWLib.Formats.WDT.Root.Chunks;

namespace WorldMapStudio.WoW;

/// <summary>Edits a map's WDT tile table (MAIN) and file ids (MAID); every other byte is kept.</summary>
public static class WowWdtWriter
{
    /// <summary>Marks the tiles as present and points their MAID entries at the given files.</summary>
    public static void AddTiles(FileSystem fs, string map, IEnumerable<((int FileX, int FileY) Tile, WowTileFiles Files)> tiles) =>
        Edit(fs, map, tiles, (area, entry, files) =>
        {
            area.Flags |= AreaInfoFlags.HasAdt;
            Assign(entry, files);
        });

    /// <summary>Clears the tiles' presence and MAID entries.</summary>
    public static void RemoveTiles(FileSystem fs, string map, IEnumerable<(int FileX, int FileY)> tiles)
    {
        var none = new WowTileFiles(0, 0, 0, 0, 0, 0, 0, 0);
        var list = new List<((int, int), WowTileFiles)>();
        foreach (var tile in tiles)
        {
            list.Add((tile, none));
        }

        Edit(fs, map, list, (area, entry, files) =>
        {
            area.Flags &= ~AreaInfoFlags.HasAdt;
            Assign(entry, files);
        });
    }

    private static void Assign(MapFileDataIDs? entry, WowTileFiles files)
    {
        if (entry == null)
        {
            return;
        }

        entry.RootAdt = files.Root;
        entry.Obj0Adt = files.Obj0;
        entry.Obj1Adt = files.Obj1;
        entry.Tex0Adt = files.Tex0;
    }

    private static void Edit(
        FileSystem fs,
        string map,
        IEnumerable<((int FileX, int FileY) Tile, WowTileFiles Files)> tiles,
        System.Action<SmAreaInfo, MapFileDataIDs?, WowTileFiles> apply)
    {
        var key = new FileKey($"world/maps/{map}/{map}.wdt");
        var wdt = WDT.ForVersion(fs.Version);
        using (wdt)
        {
            wdt.Read(fs, key);
            foreach (((int fileX, int fileY), WowTileFiles files) in tiles)
            {
                int index = (fileY * WowWdt.GridEdge) + fileX;
                MapFileDataIDs? entry = wdt.Root is WDTRootShadowlandsPlus plus && index < plus.MapFdids.Count ? plus.MapFdids[index] : null;
                apply(wdt.Root.Tiles[index], entry, files);
            }

            wdt.Write(fs, key);
        }
    }
}
