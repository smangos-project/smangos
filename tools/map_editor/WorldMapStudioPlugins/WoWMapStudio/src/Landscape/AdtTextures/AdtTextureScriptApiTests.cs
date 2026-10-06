using System;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Covers <c>wms.adtTextures</c> on the same small imported map <see cref="AdtTextureEditTests"/> builds.</summary>
public static class AdtTextureScriptApiTests
{
    private static readonly float Cell = WowTileAddressing.CellSize;

    private static (AdtTextureEditTests.Rig Rig, ScriptEngineHost Host) NewHost()
    {
        AdtTextureEditTests.Rig rig = AdtTextureEditTests.NewRig();
        var api = new AdtTextureScriptApi(rig.Context.Scripting) { WorldSource = () => rig.World };
        return (rig, new ScriptEngineHost([api]));
    }

    private static string F(double value) => value.ToString(System.Globalization.CultureInfo.InvariantCulture);

    private static string Run(ScriptEngineHost host, string expression) => host.Evaluate($"JSON.stringify({expression})");

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void Chunk_and_at_describe_the_texture_stack_with_weights()
    {
        (_, ScriptEngineHost host) = NewHost();

        Assert.AreEqual("[\"a.blp\",\"b.blp\"]", Run(host, "wms.adtTextures.Chunk(32, 32, 1, 1).Layers.map(l => l.Texture)"));
        Assert.AreEqual("1", Run(host, "wms.adtTextures.Chunk(32, 32, 1, 1).Col"));

        double weight = double.Parse(Run(host, $"wms.adtTextures.At(-50, {F(Cell + 16.6)}).Layers[0].Weight"), System.Globalization.CultureInfo.InvariantCulture);
        Assert.IsTrue(weight is > 0.0 and < 1.0, "a gradient chunk has a partial weight for the base");

        Assert.IsTrue(host.Evaluate("wms.adtTextures.At(9999, 9999)").StartsWith("Error"), "no chunk there");
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void Paint_lays_one_undoable_stroke_and_reports_full_chunks()
    {
        (AdtTextureEditTests.Rig rig, ScriptEngineHost host) = NewHost();

        string result = Run(host, $"wms.adtTextures.Paint('c.blp', [[-50, {F(Cell - 6)}], [-50, {F(Cell + 6)}]], {{ Radius: 8, Strength: 1 }})");

        Assert.IsTrue(result.Contains("\"Changed\":true"), result);
        Assert.AreEqual(1, rig.Context.EditSessions.Active.History.UndoStack.Count);
        Assert.AreEqual("true", Run(host, "wms.adtTextures.Chunk(32, 32, 0, 1).Layers.map(l => l.Texture).indexOf('c.blp') >= 0"));

        string full = Run(host, "wms.adtTextures.Paint('c.blp', [[-10, 10]], { Radius: 6, Strength: 1 })");
        Assert.IsTrue(full.Contains("\"Unpaintable\":[{"), full);
        Assert.IsTrue(full.Contains("\"Col\":0,\"Row\":0"), full);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void Replace_swaps_in_chunks_a_tile_or_under_the_brush()
    {
        (AdtTextureEditTests.Rig rig, ScriptEngineHost host) = NewHost();

        Assert.AreEqual("1", Run(host, "wms.adtTextures.Replace('b.blp', 'grass.blp', { Chunks: [[32, 32, 1, 1]] }).Changed"));
        Assert.AreEqual("true", Run(host, "wms.adtTextures.Chunk(32, 32, 1, 1).Layers.map(l => l.Texture).indexOf('grass.blp') >= 0"));

        Assert.AreEqual("1", Run(host, "wms.adtTextures.Replace('grass.blp', 'moss.blp', { Tile: [32, 32] }).Changed"));

        Assert.AreEqual("1", Run(host, $"wms.adtTextures.Replace('moss.blp', 'sand.blp', {{ Points: [[-50, {F(Cell + 16.6)}]], Radius: 6 }}).Changed"));
        Assert.AreEqual(3, rig.Context.EditSessions.Active.History.UndoStack.Count);

        Assert.IsTrue(host.Evaluate("wms.adtTextures.Replace('a', 'b', {})").StartsWith("Error"), "no target given");
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void Remove_swap_cleanup_and_duplicates_change_the_stack()
    {
        (AdtTextureEditTests.Rig rig, ScriptEngineHost host) = NewHost();

        Assert.AreEqual("1", Run(host, "wms.adtTextures.Swap([32, 32, 1, 1], 0, 1).Changed"));
        Assert.AreEqual("[\"b.blp\",\"a.blp\"]", Run(host, "wms.adtTextures.Chunk(32, 32, 1, 1).Layers.map(l => l.Texture)"));

        Assert.AreEqual("1", Run(host, "wms.adtTextures.Remove('b.blp', [{ TileX: 32, TileY: 32, Col: 1, Row: 1 }]).Changed"));
        Assert.AreEqual("[\"a.blp\"]", Run(host, "wms.adtTextures.Chunk(32, 32, 1, 1).Layers.map(l => l.Texture)"));

        Assert.AreEqual("0", Run(host, "wms.adtTextures.Cleanup().Changed"));
        Assert.AreEqual("0", Run(host, "wms.adtTextures.RemoveDuplicates().Changed"));
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void Unpaintable_and_textures_query_a_region()
    {
        (_, ScriptEngineHost host) = NewHost();

        Assert.AreEqual("1", Run(host, $"wms.adtTextures.Unpaintable('c.blp', {{ MinX: {F(-Cell * 2)}, MinZ: 0, MaxX: 0, MaxZ: {F(Cell * 2)} }}).length"));

        Assert.IsTrue(Run(host, "wms.adtTextures.Textures().map(t => t.Texture + ':' + t.Chunks)").Contains("\"a.blp:4\""));
        Assert.IsTrue(Run(host, $"wms.adtTextures.Textures([{F(-Cell * 2 + 1)}, 1, {F(-Cell - 1)}, {F(Cell - 1)}]).map(t => t.Texture + ':' + t.Chunks)").Contains("\"a.blp:1\""));
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void The_brush_uses_the_shared_vocabulary_and_survives_a_call()
    {
        (AdtTextureEditTests.Rig rig, ScriptEngineHost host) = NewHost();

        host.Evaluate("wms.adtTextures.SetBrush({ Radius: 9, Hardness: 0.5, Texture: 'x.blp', TargetWeight: 0.4, Opacity: 0.7 })");

        Assert.AreEqual("9", Run(host, "wms.adtTextures.CurrentBrush.Radius"));
        Assert.AreEqual("0.5", Run(host, "wms.adtTextures.CurrentBrush.Hardness"));
        Assert.AreEqual("\"x.blp\"", Run(host, "wms.adtTextures.CurrentBrush.Texture"));
        Assert.AreApproximatelyEqual(0.7, double.Parse(Run(host, "wms.adtTextures.CurrentBrush.Strength"), System.Globalization.CultureInfo.InvariantCulture), 1e-4);

        host.Evaluate("wms.adtTextures.Paint('c.blp', [[-50, 10]], { Radius: 2 })");
        Assert.AreEqual("9", Run(host, "wms.adtTextures.CurrentBrush.Radius"), "call options do not change the tool");
        Assert.IsTrue(host.Evaluate("wms.adtTextures.SetBrush({ Nonsense: 1 })").StartsWith("Error"), "unknown option");
    }
}
