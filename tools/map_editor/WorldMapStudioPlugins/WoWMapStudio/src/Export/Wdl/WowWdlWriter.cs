using System;
using WoWLib;
using WoWLib.Filesystem;
using WoWLib.Formats.WDL;

namespace WorldMapStudio.WoW;

/// <summary>Updates the low-resolution heightmaps of edited tiles in a map's WDL; every other tile keeps its bytes.</summary>
public static class WowWdlWriter
{
    public static void Apply(WDL wdl, ImportedTile tile)
    {
        var id = tile.TileId;
        int slot = 0;
        for (int i = 0; i < id.FileY * 64 + id.FileX; i++)
        {
            if (wdl.TileOffsets[i] != 0)
            {
                slot++;
            }
        }

        if (wdl.TileOffsets[(id.FileY * 64) + id.FileX] == 0)
        {
            return;
        }

        var (outer, inner) = WdlTileSampler.Build(tile.Chunks);
        var heights = wdl.Heightmaps[slot];
        for (int i = 0; i < outer.Length; i++)
        {
            heights.Outer[i] = outer[i];
        }

        for (int i = 0; i < inner.Length; i++)
        {
            heights.Inner[i] = inner[i];
        }
    }

    public static void Write(FileSystem fs, string map, params ImportedTile[] tiles)
    {
        var key = new FileKey($"world/maps/{map}/{map}.wdl");
        if (!fs.Exists($"world/maps/{map}/{map}.wdl"))
        {
            return;
        }

        using var wdl = WDL.ForVersion(Expansion.Bfa);
        wdl.Read(fs, key);
        foreach (var tile in tiles)
        {
            Apply(wdl, tile);
        }

        wdl.Write(fs, key);
    }
}
