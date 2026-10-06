using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Covers <see cref="AdtTextureToolFactory"/>'s registration and shared settings.</summary>
public static class AdtTextureToolTests
{
    private static EditorContext NewContext() => new(new Node3D(), new Project { Name = "__wms_adt_tool_test__" });

    private static AdtTextureToolFactory NewFactory() =>
        NewContext().Tools.Factories.OfType<AdtTextureToolFactory>().Single();

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void The_tool_registers_itself_with_its_own_brush()
    {
        EditorContext context = NewContext();
        AdtTextureToolFactory factory = context.Tools.Factories.OfType<AdtTextureToolFactory>().Single();
        PaintToolFactory paint = context.Tools.PaintToolFactory;

        Assert.AreEqual("ADT Textures", factory.Name);
        Assert.IsFalse(ReferenceEquals(factory.Brush, paint.Brush), "each tool keeps its own brush settings");
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void Using_a_texture_sets_it_and_keeps_recents_newest_first_without_duplicates()
    {
        AdtTextureToolFactory factory = NewFactory();

        for (int i = 0; i < AdtTextureToolFactory.MaxRecents + 3; i++)
        {
            factory.Use($"tileset\\t{i}.blp");
        }

        factory.Use("TILESET/T5.BLP");

        Assert.AreEqual("TILESET/T5.BLP", factory.Options.Texture);
        Assert.AreEqual(AdtTextureToolFactory.MaxRecents, factory.Recents.Count);
        Assert.AreEqual("TILESET/T5.BLP", factory.Recents[0]);
        Assert.AreEqual(1, factory.Recents.Count(recent => AdtTexturePath.Same(recent, "tileset\\t5.blp")));
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void Selecting_chunks_replaces_or_toggles_and_falls_back_to_the_hovered_chunk()
    {
        AdtTextureToolFactory factory = NewFactory();
        var a = new ChunkCoord(1, 2);
        var b = new ChunkCoord(3, 4);

        Assert.AreEqual(0, factory.Targets().Count);
        factory.HoverChunk = a;
        Assert.AreEqual(a, factory.Targets().Single());

        factory.Select(b, additive: false);
        Assert.AreEqual(b, factory.Targets().Single(), "a selection wins over the hover");

        factory.Select(a, additive: true);
        Assert.AreEqual(2, factory.Selection.Count);
        factory.Select(b, additive: true);
        Assert.IsFalse(factory.Selection.Contains(b), "a second additive click deselects");

        factory.ClearSelection();
        Assert.AreEqual(a, factory.Targets().Single());
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void The_tool_activates_and_deactivates_cleanly()
    {
        EditorContext context = NewContext();
        AdtTextureToolFactory factory = context.Tools.Factories.OfType<AdtTextureToolFactory>().Single();

        context.Tools.Activate(factory);

        Assert.IsTrue(context.Tools.Active is AdtTextureTool);
        Assert.IsFalse(context.Tools.Active!.CapturesMouse);

        context.Tools.Activate(context.Tools.PaintToolFactory);
        Assert.IsTrue(context.Tools.Active is PaintTool);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Background)]
    public static void Texture_paths_compare_without_case_or_slash_direction()
    {
        Assert.IsTrue(AdtTexturePath.Same("Tileset\\Generic\\Grass.blp", "tileset/generic/grass.blp"));
        Assert.IsFalse(AdtTexturePath.Same("tileset/a.blp", "tileset/b.blp"));
        Assert.AreEqual(AdtTexturePath.Key("A\\B.blp"), AdtTexturePath.Key("a/b.BLP"));
    }
}
