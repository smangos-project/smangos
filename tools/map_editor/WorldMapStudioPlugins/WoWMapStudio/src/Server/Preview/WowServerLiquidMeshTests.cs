using System;
using System.IO;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

public static class WowServerLiquidMeshTests
{
    private static WowMapTile Map(string name)
    {
        string plugin = PluginDirectories.Find("WoWMapStudio") ?? throw new InvalidOperationException("WoWMapStudio plugin directory not found");
        return WowMapTileReader.Read(File.ReadAllBytes(Path.Combine(plugin, "src", "Server", "Fixtures", name)));
    }

    [EditorTest(Category = "WoW Server Preview", Thread = TestThread.Background)]
    public static void A_tile_wide_ocean_becomes_one_quad_per_cell()
    {
        WowServerMeshData mesh = WowServerLiquidMesh.Build(Map("0002035.map"), 35, 20)!;

        Assert.AreEqual(16384 * 4, mesh.Vertices.Length);
        Assert.AreEqual(16384 * 6, mesh.Indices!.Length);
        Assert.IsTrue(mesh.Translucent);
        Assert.IsTrue(mesh.Vertices.All(v => Math.Abs(v.Y) < 0.001f), "the ocean sits at level zero");
        Assert.IsTrue(mesh.Colors.All(color => color == WowServerLiquidMesh.ColorOf(18)));
    }

    [EditorTest(Category = "WoW Server Preview", Thread = TestThread.Background)]
    public static void A_tile_without_liquid_builds_nothing()
    {
        Assert.IsNull(WowServerLiquidMesh.Build(new WowMapTile { HasLiquid = false }, 0, 0));
    }

    [EditorTest(Category = "WoW Server Preview", Thread = TestThread.Background)]
    public static void Liquid_colour_follows_kind_with_the_hazardous_kind_winning()
    {
        Color water = WowServerLiquidMesh.ColorOf(0x01);
        Color ocean = WowServerLiquidMesh.ColorOf(0x02);
        Color magma = WowServerLiquidMesh.ColorOf(0x04);
        Color slime = WowServerLiquidMesh.ColorOf(0x08);

        Assert.IsTrue(water.B > water.R && ocean.B > ocean.R);
        Assert.IsTrue(magma.R > magma.B);
        Assert.IsTrue(slime.G > slime.R && slime.G > slime.B);
        Assert.AreEqual(magma, WowServerLiquidMesh.ColorOf(0x05), "magma outranks water");
        Assert.IsTrue(WowServerLiquidMesh.ColorOf(0x11).B < water.B, "dark water is dimmer");
    }
}
