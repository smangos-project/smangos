using System;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Covers <see cref="AdtTexturePaintTarget"/> and <see cref="AdtTextureEdit"/> on a small imported
/// map: one tile whose four chunks sit at chunk coords (-2..-1, 0..1), the alpha image 128 px across.</summary>
public static class AdtTextureEditTests
{
    private const int Edge = AdtChunkTextureSet.Edge;
    private static readonly float Cell = WowTileAddressing.CellSize;

    // For the tile at file 32,32 a chunk coord (x, y) is tile slot col = y, row = -x - 1. Alpha pixel (px, py)
    // of a chunk is ((x + 2) * 64, y * 64).
    private static readonly ChunkCoord TopLeft = new(-2, 0);
    private static readonly ChunkCoord TopRight = new(-2, 1);
    private static readonly ChunkCoord BottomLeft = new(-1, 0);
    private static readonly ChunkCoord BottomRight = new(-1, 1);

    internal sealed class Rig
    {
        public required EditorContext Context { get; init; }
        public required AdtTextureWorld World { get; init; }
        public required AdtTileComponent Tile { get; init; }
        public required PaintImage Image { get; init; }
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void A_stroke_across_a_chunk_border_paints_both_sides_including_a_chunk_without_the_texture()
    {
        Rig rig = NewRig();
        Assert.IsFalse(Textures(rig, TopLeft).Contains("c.blp"));
        Assert.IsFalse(Textures(rig, TopRight).Contains("c.blp"));

        Stroke(rig, new AdtTexturePaintOptions { Texture = "c.blp" }, [new Vector3(-50, 0, Cell - 6), new Vector3(-50, 0, Cell + 6)]);

        foreach (ChunkCoord coord in new[] { TopLeft, TopRight })
        {
            Assert.IsTrue(Textures(rig, coord).Contains("c.blp"), $"{coord} should have the texture");
        }

        AdtChunkEdit.TryLoad(rig.World, TopLeft, out AdtChunkEdit? left);
        AdtChunkEdit.TryLoad(rig.World, TopRight, out AdtChunkEdit? right);
        int leftSlot = left!.Set.IndexOf("c.blp");
        int rightSlot = right!.Set.IndexOf("c.blp");
        Assert.IsTrue(left.Set.WeightAt(leftSlot, 30, Edge - 1) > 0.9f, "painted right up to the border");
        Assert.IsTrue(right.Set.WeightAt(rightSlot, 30, 0) > 0.9f, "and continuing across it");
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void A_stroke_leaves_chunks_it_did_not_reach_untouched()
    {
        Rig rig = NewRig();
        byte[] before = rig.Image.ReadPixels(0, 64, Edge, Edge);
        AdtChunkFacts factsBefore = rig.Tile.Chunks[Slot(TopRight)];

        Stroke(rig, new AdtTexturePaintOptions { Texture = "c.blp" }, [new Vector3(-50, 0, 10)]);

        Assert.IsTrue(rig.Image.ReadPixels(0, 64, Edge, Edge).SequenceEqual(before), "the chunk to the north is unchanged");
        Assert.IsTrue(rig.Tile.Chunks[Slot(TopRight)].Slots.SequenceEqual(factsBefore.Slots));
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void One_stroke_is_one_undo_step_that_restores_bytes_and_slots_exactly()
    {
        Rig rig = NewRig();
        byte[] pixelsBefore = rig.Image.CopyPixels();
        var slotsBefore = rig.Tile.Chunks.ToDictionary(pair => pair.Key, pair => pair.Value.Slots.ToArray());

        Stroke(rig, new AdtTexturePaintOptions { Texture = "c.blp" },
            [new Vector3(-50, 0, Cell - 15), new Vector3(-50, 0, Cell + 15), new Vector3(-20, 0, Cell + 15)]);
        byte[] pixelsAfter = rig.Image.CopyPixels();
        Assert.IsFalse(pixelsAfter.SequenceEqual(pixelsBefore));
        Assert.AreEqual(1, rig.Context.EditSessions.Active.History.UndoStack.Count);

        rig.Context.EditSessions.Undo();

        Assert.IsTrue(rig.Image.CopyPixels().SequenceEqual(pixelsBefore), "the alpha bytes are back");
        foreach ((var key, AdtChunkFacts facts) in rig.Tile.Chunks)
        {
            Assert.IsTrue(facts.Slots.SequenceEqual(slotsBefore[key]), $"chunk {key} slots are back");
        }

        rig.Context.EditSessions.Redo();
        Assert.IsTrue(rig.Image.CopyPixels().SequenceEqual(pixelsAfter), "redo reapplies the stroke");
        Assert.IsTrue(Textures(rig, TopLeft).Contains("c.blp"));
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void The_undo_command_reports_impacts_for_the_alpha_image_and_the_tile()
    {
        Rig rig = NewRig();

        Stroke(rig, new AdtTexturePaintOptions { Texture = "c.blp" }, [new Vector3(-50, 0, Cell - 6)]);

        var command = (IChunkChangeCommand)rig.Context.EditSessions.Active.History.UndoStack[0];
        Assert.IsTrue(command.ChunkImpacts.Any(impact => impact.Entity.Component<AdtTileComponent>() != null));
        Assert.IsTrue(command.ChunkImpacts.Any(impact => impact.Entity.Component<ImageComponent>() != null));
        Assert.IsTrue(command.ChunkImpacts.All(impact => impact.Before != impact.After));
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void A_chunk_with_four_visible_layers_is_unpaintable_and_stays_unchanged()
    {
        Rig rig = NewRig();
        byte[] before = rig.Image.ReadPixels(64, 0, Edge, Edge);
        var target = new AdtTexturePaintTarget(() => rig.World, new AdtTexturePaintOptions { Texture = "c.blp" });
        var brush = new Brush { Radius = 6.0f, Strength = 1.0f };
        var stroke = new BrushStroke(brush, target, rig.Context.EditSessions);

        stroke.Begin();
        stroke.MoveTo(new Vector3(-10, 0, 10));
        stroke.Finish(record: true);

        Assert.IsTrue(target.Unpaintable.Contains(BottomLeft));
        Assert.IsTrue(rig.Image.ReadPixels(64, 0, Edge, Edge).SequenceEqual(before));
        Assert.AreEqual(4, Textures(rig, BottomLeft).Count);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void Painting_removes_a_layer_nothing_shows_at_the_end_of_the_stroke()
    {
        Rig rig = NewRig();
        Assert.IsTrue(Textures(rig, TopRight).Contains("b.blp"));

        // A hard brush wider than the chunk paints the base over all of it, so b ends up at 0 everywhere.
        Stroke(rig, new AdtTexturePaintOptions { Texture = "a.blp" },
            [new Vector3(-50, 0, Cell + 16.5f)], radius: 40.0f, hardness: 1.0f);

        Assert.IsFalse(Textures(rig, TopRight).Contains("b.blp"), "b is invisible everywhere now and was dropped");
        Assert.AreEqual(1, Textures(rig, TopRight).Count);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void Replace_swaps_a_texture_in_the_given_chunks_as_one_undo_step()
    {
        Rig rig = NewRig();
        var edit = new AdtTextureEdit(rig.World, rig.Context.EditSessions);

        AdtTextureEditResult result = edit.Replace("b.blp", "grass.blp", [TopRight, TopLeft]);

        Assert.AreEqual(1, result.Changed, "only one of the two chunks had b");
        Assert.IsTrue(Textures(rig, TopRight).Contains("grass.blp"));
        Assert.IsFalse(Textures(rig, TopRight).Contains("b.blp"));

        rig.Context.EditSessions.Undo();
        Assert.IsTrue(Textures(rig, TopRight).Contains("b.blp"));
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void Removing_a_layer_shifts_the_slots_and_undo_brings_it_back()
    {
        Rig rig = NewRig();
        var edit = new AdtTextureEdit(rig.World, rig.Context.EditSessions);
        byte[] pixelsBefore = rig.Image.CopyPixels();

        Assert.AreEqual(1, edit.Remove("a.blp", [TopRight]).Changed);

        IReadOnlyList<string> after = Textures(rig, TopRight);
        Assert.AreEqual(1, after.Count);
        Assert.AreEqual("b.blp", after[0]);
        Assert.AreEqual(0, rig.Tile.Chunks[Slot(TopRight)].Slots[0].SlotIndex);

        rig.Context.EditSessions.Undo();
        Assert.AreEqual(2, Textures(rig, TopRight).Count);
        Assert.IsTrue(rig.Image.CopyPixels().SequenceEqual(pixelsBefore));
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void A_tile_reports_only_the_chunks_whose_slots_changed()
    {
        Rig rig = NewRig();
        rig.Tile.ConsumeDirtyRegions();
        Assert.AreEqual(0, rig.Tile.ConsumeDirtyRegions().Count);

        Stroke(rig, new AdtTexturePaintOptions { Texture = "c.blp" }, [new Vector3(-50, 0, 10)]);

        IReadOnlyList<Aabb> regions = rig.Tile.ConsumeDirtyRegions();
        Assert.AreEqual(1, regions.Count);
        Assert.IsTrue(regions[0].Size.X <= Cell + 0.01f, "one chunk, not the tile");
        Assert.AreEqual(0, rig.Tile.ConsumeDirtyRegions().Count);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void A_replace_stroke_swaps_the_texture_only_under_the_brush()
    {
        Rig rig = NewRig();
        AdtChunkEdit.TryLoad(rig.World, TopRight, out AdtChunkEdit? before);
        float centreBefore = before!.Set.WeightAt(1, 32, 32);
        float farBefore = before.Set.WeightAt(1, 60, 60);
        Assert.IsTrue(centreBefore > 0.0f);

        Stroke(rig, new AdtTexturePaintOptions { Texture = "grass.blp", Kind = AdtTextureStrokeKind.Replace, ReplaceFrom = "b.blp" },
            [new Vector3(-50, 0, Cell + 16.67f)], radius: 6.0f);

        AdtChunkEdit.TryLoad(rig.World, TopRight, out AdtChunkEdit? after);
        Assert.AreEqual(3, after!.Set.Count);
        int grass = after.Set.IndexOf("grass.blp");
        int b = after.Set.IndexOf("b.blp");
        Assert.IsTrue(MathF.Abs(after.Set.WeightAt(grass, 32, 32) - centreBefore) <= 3.0f / 255.0f, "the weight moved across");
        Assert.IsTrue(after.Set.WeightAt(b, 32, 32) <= 3.0f / 255.0f);
        Assert.IsTrue(MathF.Abs(after.Set.WeightAt(b, 60, 60) - farBefore) <= 3.0f / 255.0f, "outside the brush is untouched");

        rig.Context.EditSessions.Undo();
        Assert.AreEqual(2, Textures(rig, TopRight).Count);
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void A_whole_chunk_replace_stroke_renames_the_layer_in_every_chunk_it_touches()
    {
        Rig rig = NewRig();

        Stroke(rig, new AdtTexturePaintOptions
            {
                Texture = "grass.blp",
                Kind = AdtTextureStrokeKind.Replace,
                ReplaceFrom = "b.blp",
                ReplaceWholeChunk = true,
            },
            [new Vector3(-50, 0, Cell + 2)], radius: 3.0f);

        IReadOnlyList<string> textures = Textures(rig, TopRight);
        Assert.AreEqual(2, textures.Count);
        Assert.IsTrue(textures.Contains("grass.blp"));
        Assert.IsFalse(textures.Contains("b.blp"));
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void A_chunk_whose_build_took_a_foreign_layer_reports_the_override()
    {
        LandscapeMaterial? foreign = null;
        Rig? rig = null;
        rig = NewRig(coord =>
        {
            AdtTextureWorld world = rig!.World;
            world.TryChunk(coord, out AdtChunkRef chunk);
            var layers = new List<LandscapeChunkLayer>();
            foreach (AdtSlotAssignment slot in chunk.Tile.Chunks[(chunk.Col, chunk.Row)].Slots.Take(1))
            {
                layers.Add(new LandscapeChunkLayer { Material = MaterialById(rig, slot.MaterialId), Alpha = null });
            }

            layers.Add(new LandscapeChunkLayer { Material = foreign, Alpha = new byte[Edge * Edge] });
            return Output(coord, layers);
        });
        foreign = new LandscapeMaterial { Name = "Native Rock", TexturePath = "rock.blp" };
        rig.Context.Catalog.AssignId(foreign);
        rig.Context.Catalog.Add(foreign);

        rig.World.TryChunk(TopRight, out AdtChunkRef chunk);
        AdtChunkOverride? result = rig.World.OverrideAt(chunk);

        Assert.IsNotNull(result);
        Assert.AreEqual(1, result!.ForeignLayers.Count);
        Assert.AreEqual("Native Rock", result.ForeignLayers[0]);
        Assert.IsTrue(result.DroppedTextures.Contains("b.blp"), "b's slot did not survive into the build");
        Assert.IsTrue(AdtTextureTool.OverrideText(result).Contains("overridden by layer Native Rock"));
    }

    [EditorTest(Category = "WoW ADT Textures", Thread = TestThread.Main)]
    public static void A_chunk_that_builds_as_its_slots_say_reports_no_override()
    {
        Rig? rig = null;
        rig = NewRig(coord =>
        {
            rig!.World.TryChunk(coord, out AdtChunkRef chunk);
            var layers = chunk.Tile.Chunks[(chunk.Col, chunk.Row)].Slots
                .Select(slot => new LandscapeChunkLayer { Material = MaterialById(rig, slot.MaterialId), Alpha = null })
                .ToList();
            return Output(coord, layers);
        });

        rig.World.TryChunk(TopRight, out AdtChunkRef chunk);

        Assert.IsNull(rig.World.OverrideAt(chunk));
        Assert.IsTrue(rig.World.HasTerrain(TopRight));
        Assert.IsFalse(new AdtTextureWorld(rig.World.Grid, rig.World.Placement, [], rig.World.Materials, () => { }).HasTerrain(TopRight));
    }

    private static LandscapeMaterial? MaterialById(Rig? rig, int? id) =>
        rig!.Context.Catalog.OfType<LandscapeMaterial>().FirstOrDefault(material => material.RecordId == id);

    private static LandscapeChunkOutput Output(ChunkCoord coord, IReadOnlyList<LandscapeChunkLayer> layers) => new()
    {
        Coord = coord,
        HeightResolution = 9,
        Heights = new float[81],
        AlphaResolution = Edge,
        Layers = layers,
        HoleResolution = 4,
        Holes = new bool[16],
        VertexColors = [],
        VertexLight = [],
    };

    private static (int Col, int Row) Slot(ChunkCoord coord)
    {
        (_, _, int col, int row) = WowTileAddressing.SlotOf(coord);
        return (col, row);
    }

    private static IReadOnlyList<string> Textures(Rig rig, ChunkCoord coord)
    {
        Assert.IsTrue(rig.World.TryChunk(coord, out AdtChunkRef chunk));
        return rig.World.TexturesOf(chunk.Tile.Chunks[(chunk.Col, chunk.Row)])!;
    }

    private static void Stroke(Rig rig, AdtTexturePaintOptions options, IReadOnlyList<Vector3> points, float radius = 8.0f, float hardness = 0.0f)
    {
        var target = new AdtTexturePaintTarget(() => rig.World, options);
        var brush = new Brush { Radius = radius, Strength = 1.0f, Hardness = hardness };
        var stroke = new BrushStroke(brush, target, rig.Context.EditSessions);

        Assert.IsTrue(stroke.Begin());
        foreach (Vector3 point in points)
        {
            stroke.MoveTo(point);
        }

        stroke.Finish(record: true);
    }

    // World layout: chunk coord (x, y) spans world x in [x*Cell, (x+1)*Cell] and z in [y*Cell, (y+1)*Cell].
    // TopLeft holds [a], TopRight [a, b] with a diagonal alpha for b, BottomLeft four visible layers, BottomRight [a].
    internal static Rig NewRig(Func<ChunkCoord, LandscapeChunkOutput?>? outputAt = null)
    {
        var context = AdtTestDatabase.Attach(new EditorContext(new Node3D(), new Project { Name = "__wms_adt_texture_test__" }));

        var image = new PaintImage { RecordId = 1, Name = "ADT Alpha" };
        image.ConfigureNew(Edge * 2, Edge * 2, chunkSize: Edge, components: 4);
        context.Catalog.Add(image);

        var placement = new MapSceneEntity { Transform = new Transform3D(Basis.Identity, new Vector3(-Cell, 0, Cell)) };
        placement.AddComponent(new ImageComponent(context.Images)
        {
            ImageId = image.RecordId,
            Channel = AdtImportCatalog.AlphaChannelName,
            WorldSizeX = Cell * 2,
            WorldSizeZ = Cell * 2,
        });
        context.Scene.Add(placement);

        var materials = new AdtSlotMaterials(context, new MapId(0), _ => { });
        foreach (string texture in new[] { "a.blp", "b.blp", "d.blp", "e.blp" })
        {
            materials.EnsureTexture(texture);
        }

        var tile = new AdtTileComponent { FileX = 32, FileY = 32 };
        var tileEntity = new MapSceneEntity();
        tileEntity.AddComponent(tile);
        context.Scene.Add(tileEntity);

        var world = new AdtTextureWorld(
            new LandscapeGrid(new WowAdtProfile(null!).CreateSettings()),
            placement.Component<ImageComponent>()!,
            [(tileEntity, tile)],
            materials,
            () => { },
            outputAt);

        void Chunk(ChunkCoord coord, string[] textures, Func<int, int, byte[]> alpha)
        {
            (_, _, int col, int row) = WowTileAddressing.SlotOf(coord);
            tile.SetChunk(col, row, world.FactsFor(AdtChunkFacts.Empty, textures));
            world.TryPixelBase(coord, out int px, out int py);
            var block = new byte[Edge * Edge * 4];
            for (int y = 0; y < Edge; y++)
            {
                for (int x = 0; x < Edge; x++)
                {
                    byte[] texel = alpha(x, y);
                    Array.Copy(texel, 0, block, ((y * Edge) + x) * 4, 3);
                }
            }

            image.WritePixels(px, py, Edge, Edge, block);
        }

        Chunk(TopLeft, ["a.blp"], (_, _) => [0, 0, 0]);
        Chunk(TopRight, ["a.blp", "b.blp"], (x, _) => [(byte)(x * 4), 0, 0]);
        Chunk(BottomLeft, ["a.blp", "b.blp", "d.blp", "e.blp"], (x, y) => [(byte)(60 + x), (byte)(60 + y), (byte)(60 + ((x + y) / 2))]);
        Chunk(BottomRight, ["a.blp"], (_, _) => [0, 0, 0]);

        tile.ConsumeDirtyRegions();
        return new Rig { Context = context, World = world, Tile = tile, Image = image };
    }
}
