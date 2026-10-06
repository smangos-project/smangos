using System.Globalization;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Covers <c>wms.adtHeights</c> and the height tool's registration on the map
/// <see cref="AdtVertexEditTests"/> builds.</summary>
public static class AdtHeightScriptApiTests
{
    private static readonly float Cell = WowTileAddressing.CellSize;
    private static readonly float Step = Cell / AdtVertex.CellsPerChunk;

    private static (AdtVertexEditTests.Rig Rig, ScriptEngineHost Host) NewHost()
    {
        AdtVertexEditTests.Rig rig = AdtVertexEditTests.NewRig();
        var api = new AdtHeightScriptApi(rig.Context.Scripting) { WorldSource = () => rig.World };
        return (rig, new ScriptEngineHost([api]));
    }

    private static string F(double value) => value.ToString(CultureInfo.InvariantCulture);

    private static string Run(ScriptEngineHost host, string expression) => host.Evaluate($"JSON.stringify({expression})");

    private static double Number(ScriptEngineHost host, string expression) =>
        double.Parse(Run(host, expression), CultureInfo.InvariantCulture);

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void The_tool_registers_itself_with_its_own_brush()
    {
        var context = AdtTestDatabase.Attach(new EditorContext(new Node3D(), new Project { Name = "__wms_adt_height_tool_test__" }));
        AdtHeightToolFactory factory = context.Tools.Factories.OfType<AdtHeightToolFactory>().Single();

        Assert.AreEqual("ADT Height", factory.Name);
        Assert.IsFalse(ReferenceEquals(factory.Brush, context.Tools.PaintToolFactory.Brush));
        Assert.IsFalse(ReferenceEquals(factory.Brush, context.Tools.Factories.OfType<AdtTextureToolFactory>().Single().Brush));

        context.Tools.Activate(factory);
        Assert.IsTrue(context.Tools.Active is AdtHeightTool);
        Assert.IsFalse(context.Tools.Active!.CapturesMouse);
        context.Tools.Activate(context.Tools.PaintToolFactory);
        Assert.IsTrue(context.Tools.Active is PaintTool);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void At_and_vertices_read_the_imported_heights()
    {
        (_, ScriptEngineHost host) = NewHost();

        var vertex = AdtVertexEditTests.Corner(-12, 4);
        Assert.AreApproximatelyEqual(AdtVertexEditTests.TerrainHeight(vertex), Number(host, $"wms.adtHeights.At({F(-12 * Step)}, {F(4 * Step)}).Height"), 1e-4);
        Assert.AreEqual("[0,1,4,4]", Run(host, $"[wms.adtHeights.At({F(-12 * Step)}, {F(4 * Step)}).Col, wms.adtHeights.At({F(-12 * Step)}, {F(4 * Step)}).Row, wms.adtHeights.At({F(-12 * Step)}, {F(4 * Step)}).Vx, wms.adtHeights.At({F(-12 * Step)}, {F(4 * Step)}).Vz]"));

        Assert.AreEqual("false", Run(host, $"wms.adtHeights.At({F(-12 * Step)}, {F(4 * Step)}).Centre"));
        Assert.AreEqual("81", Run(host, "wms.adtHeights.Vertices(32, 32, 0, 1).Corners.length"));
        Assert.AreEqual("64", Run(host, "wms.adtHeights.Vertices(32, 32, 0, 1).Centres.length"));
        Assert.AreApproximatelyEqual(AdtVertexEditTests.TerrainHeight(AdtVertexEditTests.Corner(-16, 0)), Number(host, "wms.adtHeights.Vertices(32, 32, 0, 1).Corners[0]"), 1e-4);
        Assert.AreApproximatelyEqual(AdtVertexEditTests.TerrainHeight(AdtVertexEditTests.Corner(-8, 8)), Number(host, "wms.adtHeights.Vertices(32, 32, 0, 1).Corners[80]"), 1e-4);
        Assert.AreApproximatelyEqual(AdtVertexEditTests.TerrainHeight(new AdtVertex(-31, 1)), Number(host, "wms.adtHeights.Vertices(32, 32, 0, 1).Centres[0]"), 1e-4, "the first cell's centre");
        Assert.AreApproximatelyEqual(AdtVertexEditTests.TerrainHeight(new AdtVertex(-17, 15)), Number(host, "wms.adtHeights.Vertices(32, 32, 0, 1).Centres[63]"), 1e-4, "the last cell's centre");

        // Half a cell in from a corner, the nearest vertex is that cell's centre.
        double centreX = (-31 * Step) / 2.0;
        double centreZ = (1 * Step) / 2.0;
        Assert.AreEqual("[true,0,0]", Run(host, $"[wms.adtHeights.At({F(centreX)}, {F(centreZ)}).Centre, wms.adtHeights.At({F(centreX)}, {F(centreZ)}).Vx, wms.adtHeights.At({F(centreX)}, {F(centreZ)}).Vz]"));
        Assert.AreApproximatelyEqual(AdtVertexEditTests.TerrainHeight(new AdtVertex(-31, 1)), Number(host, $"wms.adtHeights.At({F(centreX)}, {F(centreZ)}).Height"), 1e-4);

        Assert.IsTrue(host.Evaluate("wms.adtHeights.At(9999, 9999)").StartsWith("Error"), "no chunk there");
        Assert.IsTrue(host.Evaluate("wms.adtHeights.Vertices(1, 1, 0, 0)").StartsWith("Error"), "not imported");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Raise_and_lower_move_the_ground_as_one_undoable_stroke_each()
    {
        (AdtVertexEditTests.Rig rig, ScriptEngineHost host) = NewHost();
        var vertex = AdtVertexEditTests.Corner(-12, 4);
        double before = AdtVertexEditTests.TerrainHeight(vertex);
        string at = $"wms.adtHeights.At({F(-12 * Step)}, {F(4 * Step)}).Height";

        string result = Run(host, $"wms.adtHeights.Raise([[{F(-12 * Step)}, {F(4 * Step)}]], {{ Radius: 6, Strength: 1, Rate: 3 }})");
        Assert.IsTrue(result.Contains("\"Changed\":true"), result);
        Assert.IsTrue(Number(host, at) > before + 2.0);
        Assert.AreEqual(1, rig.Context.EditSessions.Active.History.UndoStack.Count);

        Run(host, $"wms.adtHeights.Lower([[{F(-12 * Step)}, {F(4 * Step)}]], {{ Radius: 6, Strength: 1, Rate: 3 }})");
        Assert.AreApproximatelyEqual(before, Number(host, at), 1e-3, "lowered by as much as it was raised");
        Assert.AreEqual(2, rig.Context.EditSessions.Active.History.UndoStack.Count);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Flatten_takes_a_target_or_the_first_height_and_smooth_evens_a_spike()
    {
        (AdtVertexEditTests.Rig rig, ScriptEngineHost host) = NewHost();
        string around = $"[[{F(-12 * Step)}, {F(4 * Step)}]]";
        string at = $"wms.adtHeights.At({F(-12 * Step)}, {F(4 * Step)}).Height";

        Run(host, $"wms.adtHeights.Flatten({around}, {{ Radius: 8, Strength: 1, Hardness: 1, Target: 40 }})");
        Assert.AreApproximatelyEqual(40.0, Number(host, at), 1e-3);

        double first = AdtVertexEditTests.TerrainHeight(AdtVertexEditTests.Corner(-5, 4));
        Run(host, $"wms.adtHeights.Flatten([[{F(-5 * Step)}, {F(4 * Step)}]], {{ Radius: 5, Strength: 1, Hardness: 1 }})");
        Assert.AreApproximatelyEqual(first, Number(host, $"wms.adtHeights.At({F(-4 * Step)}, {F(4 * Step)}).Height"), 1e-3, "a neighbour flattened to the height under the first dab");

        string edge = $"wms.adtHeights.At({F(-11 * Step)}, {F(4 * Step)}).Height";
        Assert.AreApproximatelyEqual(40.0, Number(host, edge), 1e-3);
        Run(host, $"wms.adtHeights.Smooth({around}, {{ Radius: 12, Strength: 1, Hardness: 1 }})");
        Assert.IsTrue(Number(host, edge) < 40.0, "the edge of the plateau came down");
        Assert.AreEqual(3, rig.Context.EditSessions.Active.History.UndoStack.Count);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Set_chunks_fills_whole_chunks_and_reports_what_it_changed()
    {
        (AdtVertexEditTests.Rig rig, ScriptEngineHost host) = NewHost();

        Assert.AreEqual("2", Run(host, "wms.adtHeights.SetChunks([[32, 32, 0, 1], { TileX: 32, TileY: 32, Col: 1, Row: 1 }], 55).Changed"));
        Assert.AreApproximatelyEqual(55.0, Number(host, "wms.adtHeights.Vertices(32, 32, 0, 1).Corners[40]"), 1e-4);
        Assert.AreApproximatelyEqual(55.0, Number(host, "wms.adtHeights.Vertices(32, 32, 0, 1).Centres[20]"), 1e-4, "cell centres are filled too");
        Assert.AreEqual(1, rig.Context.EditSessions.Active.History.UndoStack.Count);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void The_brush_uses_the_shared_vocabulary_and_survives_a_call()
    {
        (_, ScriptEngineHost host) = NewHost();

        host.Evaluate("wms.adtHeights.SetBrush({ Radius: 9, Hardness: 0.5, Rate: 2, Side: 'RaiseOnly' })");

        Assert.AreEqual("9", Run(host, "wms.adtHeights.CurrentBrush.Radius"));
        Assert.AreEqual("2", Run(host, "wms.adtHeights.CurrentBrush.Rate"));
        Assert.AreEqual("\"RaiseOnly\"", Run(host, "wms.adtHeights.CurrentBrush.Side"));

        host.Evaluate($"wms.adtHeights.Raise([[{F(-12 * Step)}, {F(4 * Step)}]], {{ Radius: 2, Rate: 9 }})");
        Assert.AreEqual("9", Run(host, "wms.adtHeights.CurrentBrush.Radius"), "call options do not change the tool");
        Assert.AreEqual("2", Run(host, "wms.adtHeights.CurrentBrush.Rate"));
        Assert.IsTrue(host.Evaluate("wms.adtHeights.SetBrush({ Nonsense: 1 })").StartsWith("Error"), "unknown option");
    }
}
