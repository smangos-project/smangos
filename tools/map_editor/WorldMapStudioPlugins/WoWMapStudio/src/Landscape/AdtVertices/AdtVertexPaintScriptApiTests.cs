using System.Globalization;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Covers <c>wms.adtVertexColors</c> and the vertex paint tool's registration on the map
/// <see cref="AdtVertexEditTests"/> builds. Its top left chunk claims vertex colour, holding a gradient
/// that reads 1.2 at vertex (-12, 4); the other chunks claim neither colour nor light.</summary>
public static class AdtVertexPaintScriptApiTests
{
    private static readonly float Step = WowTileAddressing.CellSize / AdtVertex.CellsPerChunk;

    private const double ColorTolerance = 0.01;

    private static (AdtVertexEditTests.Rig Rig, ScriptEngineHost Host) NewHost()
    {
        AdtVertexEditTests.Rig rig = AdtVertexEditTests.NewRig();
        var api = new AdtVertexPaintScriptApi(rig.Context.Scripting) { WorldSource = () => rig.World };
        return (rig, new ScriptEngineHost([api]));
    }

    private static string F(double value) => value.ToString(CultureInfo.InvariantCulture);

    private static string Run(ScriptEngineHost host, string expression) => host.Evaluate($"JSON.stringify({expression})");

    private static double Number(ScriptEngineHost host, string expression) =>
        double.Parse(Run(host, expression), CultureInfo.InvariantCulture);

    // A point on vertex (x, z) of the map-wide lattice.
    private static string Point(int x, int z) => $"[{F(x * Step)}, {F(z * Step)}]";

    private static string At(int x, int z) => $"wms.adtVertexColors.At({F(x * Step)}, {F(z * Step)})";

    private static string Chunk(ChunkCoord coord)
    {
        (int fileX, int fileY, int col, int row) = WowTileAddressing.SlotOf(coord);
        return $"[{fileX}, {fileY}, {col}, {row}]";
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void The_tool_registers_itself_with_its_own_brush()
    {
        var context = AdtTestDatabase.Attach(new EditorContext(new Node3D(), new Project { Name = "__wms_adt_vertex_paint_tool_test__" }));
        AdtVertexPaintToolFactory factory = context.Tools.Factories.OfType<AdtVertexPaintToolFactory>().Single();

        Assert.AreEqual("ADT Vertex Paint", factory.Name);
        Assert.IsFalse(ReferenceEquals(factory.Brush, context.Tools.PaintToolFactory.Brush));
        Assert.IsFalse(ReferenceEquals(factory.Brush, context.Tools.Factories.OfType<AdtTextureToolFactory>().Single().Brush));
        Assert.IsFalse(ReferenceEquals(factory.Brush, context.Tools.Factories.OfType<AdtHeightToolFactory>().Single().Brush));

        context.Tools.Activate(factory);
        Assert.IsTrue(context.Tools.Active is AdtVertexPaintTool);
        Assert.IsFalse(context.Tools.Active!.CapturesMouse);
        context.Tools.Activate(context.Tools.PaintToolFactory);
        Assert.IsTrue(context.Tools.Active is PaintTool);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Using_a_swatch_moves_it_to_the_front_of_the_recents_once()
    {
        var context = AdtTestDatabase.Attach(new EditorContext(new Node3D(), new Project { Name = "__wms_adt_vertex_recents_test__" }));
        AdtVertexPaintToolFactory factory = context.Tools.Factories.OfType<AdtVertexPaintToolFactory>().Single();
        AdtSwatch warm = AdtSwatches.Find(AdtVertexKind.Color, "Warm")!.Swatch;
        AdtSwatch cool = AdtSwatches.Find(AdtVertexKind.Color, "Cool")!.Swatch;

        factory.Use(AdtVertexKind.Color, warm);
        factory.Use(AdtVertexKind.Color, cool);
        factory.Use(AdtVertexKind.Color, warm);

        Assert.AreEqual(2, factory.Recents(AdtVertexKind.Color).Count);
        Assert.AreEqual(warm, factory.Recents(AdtVertexKind.Color)[0]);
        Assert.AreEqual(warm, factory.Options.Color);
        Assert.AreEqual(0, factory.Recents(AdtVertexKind.Light).Count, "light keeps its own recents");

        for (int i = 0; i < AdtVertexPaintToolFactory.MaxRecents + 3; i++)
        {
            factory.Use(AdtVertexKind.Light, new AdtSwatch(System.Numerics.Vector3.One, 0.05f * (i + 1)));
        }

        Assert.AreEqual(AdtVertexPaintToolFactory.MaxRecents, factory.Recents(AdtVertexKind.Light).Count);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void At_reads_colour_light_and_which_layers_a_chunk_claims()
    {
        (_, ScriptEngineHost host) = NewHost();

        Assert.AreApproximatelyEqual(1.2, Number(host, $"{At(-12, 4)}.Color[0]"), ColorTolerance);
        Assert.AreEqual("[true,false]", Run(host, $"[{At(-12, 4)}.HasColor, {At(-12, 4)}.HasLight]"));
        Assert.AreEqual("[0,0,0]", Run(host, $"{At(-12, 4)}.Light"));

        Assert.AreEqual("[1,1,1]", Run(host, $"{At(-12, 12)}.Color"), "a chunk with no colour reads neutral");
        Assert.AreEqual("[false,false]", Run(host, $"[{At(-12, 12)}.HasColor, {At(-12, 12)}.HasLight]"));
        Assert.IsTrue(host.Evaluate("wms.adtVertexColors.At(9999, 9999)").StartsWith("Error"), "no chunk there");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Paint_sets_a_colour_and_invert_paints_it_back_to_neutral()
    {
        (AdtVertexEditTests.Rig rig, ScriptEngineHost host) = NewHost();
        string brush = "Radius: 6, Strength: 1, Hardness: 1";

        string result = Run(host, $"wms.adtVertexColors.Paint([{Point(-12, 4)}], {{ {brush}, Color: [1.5, 1, 0.5] }})");
        Assert.IsTrue(result.Contains("\"Changed\":true"), result);
        Assert.AreEqual("[1.5,1,0.5]", Run(host, $"{At(-12, 4)}.Color.map(c => Math.round(c * 50) / 50)"));
        Assert.AreEqual(1, rig.Context.EditSessions.Active.History.UndoStack.Count);

        Run(host, $"wms.adtVertexColors.Paint([{Point(-12, 4)}], {{ {brush}, Invert: true, Color: [1.5, 1, 0.5] }})");
        Assert.AreEqual("[1,1,1]", Run(host, $"{At(-12, 4)}.Color.map(c => Math.round(c * 50) / 50)"), "erasing paints towards neutral");
        Assert.AreEqual(2, rig.Context.EditSessions.Active.History.UndoStack.Count);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Painting_a_chunk_without_colour_turns_it_on()
    {
        (AdtVertexEditTests.Rig rig, ScriptEngineHost host) = NewHost();

        Run(host, $"wms.adtVertexColors.Paint([{Point(-12, 12)}], {{ Radius: 4, Strength: 1, Hardness: 1, Color: [0.5, 0.5, 0.5] }})");

        Assert.AreEqual("true", Run(host, $"{At(-12, 12)}.HasColor"));
        Assert.IsTrue(rig.Facts(AdtVertexEditTests.TopRight).HasVertexColor);
        Assert.AreEqual("false", Run(host, $"{At(-12, 12)}.HasLight"), "light is a layer of its own");
        rig.Context.EditSessions.Undo();
        Assert.AreEqual("false", Run(host, $"{At(-12, 12)}.HasColor"));
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void A_colour_can_be_a_value_a_hue_with_a_brightness_a_hex_string_or_a_preset()
    {
        (_, ScriptEngineHost host) = NewHost();
        string where = $"[{Point(-12, 4)}]";
        string read = $"{At(-12, 4)}.Color.map(c => Math.round(c * 50) / 50)";
        string brush = "Radius: 6, Strength: 1, Hardness: 1";

        Run(host, $"wms.adtVertexColors.Paint({where}, {{ {brush}, Color: '#ff8000', Brightness: 0.5 }})");
        Assert.AreEqual("[0.5,0.26,0]", Run(host, read), "the colour gives the hue and the brightness the level");

        Run(host, $"wms.adtVertexColors.Paint({where}, {{ {brush}, Preset: 'shade 50%' }})");
        Assert.AreEqual("[0.5,0.5,0.5]", Run(host, read), "presets are found by name, whatever the case");

        Run(host, $"wms.adtVertexColors.Paint({where}, {{ {brush}, Color: '#f80' }})");
        Assert.AreEqual("[1,0.54,0]", Run(host, read), "a hex string without a brightness is the value itself");

        Assert.IsTrue(host.Evaluate($"wms.adtVertexColors.Paint({where}, {{ Preset: 'nope' }})").StartsWith("Error"), "unknown preset");
        Assert.IsTrue(host.Evaluate($"wms.adtVertexColors.Paint({where}, {{ Color: [1, 2] }})").StartsWith("Error"), "a colour has three channels");
        Assert.IsTrue(host.Evaluate($"wms.adtVertexColors.Paint({where}, {{ Target: 'height' }})").StartsWith("Error"), "unknown target");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Light_is_painted_on_its_own_layer_and_leaves_colour_alone()
    {
        (AdtVertexEditTests.Rig rig, ScriptEngineHost host) = NewHost();
        byte[] colourBefore = rig.Color.CopyPixels();

        Run(host, $"wms.adtVertexColors.Paint([{Point(-12, 4)}], {{ Target: 'light', Radius: 6, Strength: 1, Hardness: 1, Color: [1, 0.5, 0] }})");

        Assert.AreEqual("[1,0.5,0]", Run(host, $"{At(-12, 4)}.Light.map(c => Math.round(c * 50) / 50)"));
        Assert.AreEqual("true", Run(host, $"{At(-12, 4)}.HasLight"));
        Assert.IsTrue(rig.Color.CopyPixels().SequenceEqual(colourBefore), "colour is as it was");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Brighten_and_darken_scale_a_colour_and_keep_its_hue()
    {
        (_, ScriptEngineHost host) = NewHost();
        string where = $"[{Point(-12, 4)}]";
        string brush = "Radius: 6, Strength: 1, Hardness: 1, Rate: 0.5";
        double before = Number(host, $"{At(-12, 4)}.Color[0]");
        double greenBefore = Number(host, $"{At(-12, 4)}.Color[1]");

        Run(host, $"wms.adtVertexColors.Brighten({where}, {{ {brush} }})");
        Assert.AreApproximatelyEqual(before * 1.5, Number(host, $"{At(-12, 4)}.Color[0]"), ColorTolerance * 2);
        Assert.AreApproximatelyEqual(greenBefore * 1.5, Number(host, $"{At(-12, 4)}.Color[1]"), ColorTolerance * 2);

        Run(host, $"wms.adtVertexColors.Darken({where}, {{ {brush} }})");
        Assert.AreApproximatelyEqual(before * 0.75, Number(host, $"{At(-12, 4)}.Color[0]"), ColorTolerance * 2);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Smooth_pulls_a_painted_spike_towards_its_neighbours()
    {
        (_, ScriptEngineHost host) = NewHost();
        string brush = "Radius: 1, Strength: 1, Hardness: 1";

        Run(host, $"wms.adtVertexColors.Paint([{Point(-12, 4)}], {{ {brush}, Color: [2, 2, 2] }})");
        double spike = Number(host, $"{At(-12, 4)}.Color[0]");
        Run(host, $"wms.adtVertexColors.Smooth([{Point(-12, 4)}], {{ {brush} }})");

        Assert.IsTrue(Number(host, $"{At(-12, 4)}.Color[0]") < spike - 0.2, "the spike came down");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Fill_reset_and_remove_edit_whole_chunks_as_one_undo_step_each()
    {
        (AdtVertexEditTests.Rig rig, ScriptEngineHost host) = NewHost();
        string chunks = $"[{Chunk(AdtVertexEditTests.TopLeft)}, {Chunk(AdtVertexEditTests.TopRight)}]";

        Assert.AreEqual("2", Run(host, $"wms.adtVertexColors.Fill({chunks}, {{ Color: [1.25, 1.25, 1.25] }}).Changed"));
        Assert.AreEqual("[1.25,1.25,1.25]", Run(host, $"{At(-12, 4)}.Color.map(c => Math.round(c * 20) / 20)"));
        Assert.AreEqual("true", Run(host, $"{At(-12, 12)}.HasColor"));
        Assert.AreEqual(1, rig.Context.EditSessions.Active.History.UndoStack.Count);

        Assert.AreEqual("2", Run(host, $"wms.adtVertexColors.Reset({chunks}, 'color').Changed"));
        Assert.AreEqual("[1,1,1]", Run(host, $"{At(-12, 4)}.Color"));
        Assert.AreEqual("true", Run(host, $"{At(-12, 12)}.HasColor"), "reset leaves a chunk using colour");

        Assert.AreEqual("1", Run(host, $"wms.adtVertexColors.Remove([{Chunk(AdtVertexEditTests.TopRight)}], 'color').Changed"));
        Assert.AreEqual("false", Run(host, $"{At(-12, 12)}.HasColor"));
        Assert.AreEqual("true", Run(host, $"{At(-12, 4)}.HasColor"));
        Assert.AreEqual(3, rig.Context.EditSessions.Active.History.UndoStack.Count);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Presets_list_the_named_swatches_of_a_target()
    {
        (_, ScriptEngineHost host) = NewHost();

        Assert.AreEqual(AdtSwatches.Presets(AdtVertexKind.Color).Count.ToString(), Run(host, "wms.adtVertexColors.Presets('color').length"));
        Assert.AreEqual("\"None\"", Run(host, "wms.adtVertexColors.Presets('light')[0].Name"));
        Assert.AreEqual("[1,1,1]", Run(host, "wms.adtVertexColors.Presets('color')[0].Color"), "neutral colour is a multiplier of one");
        Assert.AreEqual("\"Shade 50%\"", Run(host, "wms.adtVertexColors.Presets('color')[1].Name"));
        Assert.AreEqual("0.5", Run(host, "wms.adtVertexColors.Presets('color')[1].Brightness"));
        Assert.IsTrue(host.Evaluate("wms.adtVertexColors.Presets('height')").StartsWith("Error"));
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void The_brush_uses_the_shared_vocabulary_and_survives_a_call()
    {
        (_, ScriptEngineHost host) = NewHost();

        host.Evaluate("wms.adtVertexColors.SetBrush({ Radius: 9, Hardness: 0.5, Target: 'light', Color: [1, 0.5, 0.25], Rate: 0.4 })");

        Assert.AreEqual("9", Run(host, "wms.adtVertexColors.CurrentBrush.Radius"));
        Assert.AreEqual("\"light\"", Run(host, "wms.adtVertexColors.CurrentBrush.Target"));
        Assert.AreEqual("0.4", Run(host, "wms.adtVertexColors.CurrentBrush.Rate").Substring(0, 3));
        Assert.AreEqual("[1,0.5,0.25]", Run(host, "wms.adtVertexColors.CurrentBrush.Light"));

        host.Evaluate($"wms.adtVertexColors.Paint([{Point(-12, 4)}], {{ Radius: 2, Color: [1.4, 1, 1] }})");
        Assert.AreEqual("9", Run(host, "wms.adtVertexColors.CurrentBrush.Radius"), "call options do not change the tool");
        Assert.AreEqual("[1,1,1]", Run(host, "wms.adtVertexColors.CurrentBrush.Color"), "nor its swatches");
        Assert.IsTrue(host.Evaluate("wms.adtVertexColors.SetBrush({ Nonsense: 1 })").StartsWith("Error"), "unknown option");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void A_map_without_the_raster_reports_it()
    {
        AdtVertexEditTests.Rig rig = AdtVertexEditTests.NewRig(withLight: false);
        var api = new AdtVertexPaintScriptApi(rig.Context.Scripting) { WorldSource = () => rig.World };
        var host = new ScriptEngineHost([api]);

        string error = host.Evaluate($"wms.adtVertexColors.Paint([{Point(-12, 4)}], {{ Target: 'light' }})");
        Assert.IsTrue(error.StartsWith("Error") && error.Contains("light"), error);
        Assert.IsTrue(Run(host, $"wms.adtVertexColors.Paint([{Point(-12, 4)}], {{ Color: [1.3, 1, 1] }})").Contains("\"Changed\":true"));
    }
}
