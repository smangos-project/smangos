using System;
using System.Collections.Generic;
using System.Linq;
using Godot;
using NVector3 = System.Numerics.Vector3;

namespace WorldMapStudio.WoW;

/// <summary>Covers <see cref="AdtVertexStrokeTarget"/> and <see cref="AdtVertexEdit"/> on a small imported
/// map: one tile whose four chunks sit at chunk coords (-2..-1, 0..1), with height, colour and light images
/// covering those and a chunk of margin all round (chunk coords -3..0, -1..2).</summary>
public static class AdtVertexEditTests
{
    private const int Edge = AdtVertexLattice.Edge;
    private static readonly float Cell = WowTileAddressing.CellSize;

    // The corner with these corner indices, on the half-step lattice.
    internal static AdtVertex Corner(int x, int z) => new(x * 2, z * 2);

    internal static readonly ChunkCoord TopLeft = new(-2, 0);
    internal static readonly ChunkCoord TopRight = new(-2, 1);
    internal static readonly ChunkCoord BottomLeft = new(-1, 0);
    internal static readonly ChunkCoord BottomRight = new(-1, 1);

    internal sealed class Rig
    {
        public required EditorContext Context { get; init; }
        public required AdtMapWorld World { get; init; }
        public required AdtTileComponent Tile { get; init; }
        public required PaintImage Height { get; init; }
        public required PaintImage Color { get; init; }
        public required PaintImage Light { get; init; }

        public PaintImage ImageOf(AdtVertexKind kind) => kind switch
        {
            AdtVertexKind.Height => Height,
            AdtVertexKind.Color => Color,
            _ => Light,
        };

        public AdtChunkFacts Facts(ChunkCoord coord)
        {
            World.TryChunk(coord, out AdtChunkRef chunk);
            return chunk.Tile.Chunks[(chunk.Col, chunk.Row)];
        }

        /// <summary>A vertex's value: the average of those of its four texels that hold data, or the neutral value
        /// when none does.</summary>
        public NVector3 ValueAt(AdtVertexKind kind, AdtVertex vertex)
        {
            AdtVertexCodec codec = AdtVertexCodec.For(kind);
            PaintImage image = ImageOf(kind);
            NVector3 sum = NVector3.Zero;
            int count = 0;
            var texels = AdtVertexLattice.Texels(vertex).ToList();
            var imported = texels.Where(texel => World.TryChunk(texel.Chunk, out _)).ToList();
            foreach ((ChunkCoord chunk, int tx, int tz) in imported.Count > 0 ? imported : texels)
            {
                World.TryPixelBase(World.PlacementFor(AdtVertexSession.ChannelOf(kind))!, chunk, Edge, out int px, out int py);
                if (codec.TryDecode(image.ReadPixels(px + tx, py + tz, 1, 1), out NVector3 value))
                {
                    sum += value;
                    count++;
                }
            }

            return count == 0 ? codec.Neutral : sum / count;
        }

        /// <summary>Whether a vertex's four texels hold one value, whichever chunk they are in.</summary>
        public bool Welded(AdtVertexKind kind, AdtVertex vertex)
        {
            PaintImage image = ImageOf(kind);
            var texels = new List<byte[]>();
            foreach ((ChunkCoord chunk, int tx, int tz) in AdtVertexLattice.Texels(vertex))
            {
                World.TryPixelBase(World.PlacementFor(AdtVertexSession.ChannelOf(kind))!, chunk, Edge, out int px, out int py);
                texels.Add(image.ReadPixels(px + tx, py + tz, 1, 1));
            }

            return texels.All(texel => texel.SequenceEqual(texels[0]));
        }
    }

    // A slope, the same at every chunk that holds a vertex.
    internal static float TerrainHeight(AdtVertex vertex) => 10.0f + (vertex.X * 0.5f) + (vertex.Z * 0.25f);

    // TopLeft's colour: a gradient along x that runs from 1 to 1.4 across the chunk.
    private static NVector3 TerrainColor(AdtVertex vertex) => new(1.0f + (0.025f * (vertex.X + 32)), 1.0f, 1.0f);

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void A_raise_across_a_chunk_corner_moves_every_copy_and_leaves_no_seam()
    {
        Rig rig = NewRig();
        var corner = Corner(-8, 8);
        float before = rig.ValueAt(AdtVertexKind.Height, corner).X;
        Assert.AreEqual(TerrainHeight(corner), before);

        Stroke(rig, AdtVertexKind.Height, () => new AdtRaiseOp { Rate = 4.0f }, [new Vector3(-Cell, 0, Cell)], radius: 9.0f);

        Assert.IsTrue(rig.ValueAt(AdtVertexKind.Height, corner).X > before + 1.0f, "the corner rose");
        foreach (AdtVertex vertex in NearbyVertices(corner, 3))
        {
            Assert.IsTrue(rig.Welded(AdtVertexKind.Height, vertex), $"vertex {vertex} reads the same from every chunk");
        }

        foreach ((ChunkCoord chunk, int tx, int tz) in AdtVertexLattice.Texels(corner))
        {
            rig.World.TryPixelBase(rig.World.PlacementFor(AdtImportCatalog.HeightChannelName)!, chunk, Edge, out int px, out int py);
            Assert.IsTrue(BitConverter.ToSingle(rig.Height.ReadPixels(px + tx, py + tz, 1, 1)) > before + 1.0f, $"{chunk} copy moved");
        }
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void A_raise_changes_only_the_vertices_it_reaches()
    {
        Rig rig = NewRig();
        byte[] before = rig.Height.CopyPixels();

        Stroke(rig, AdtVertexKind.Height, () => new AdtRaiseOp(), [new Vector3(-Cell - 12, 0, 12)], radius: 5.0f);

        byte[] after = rig.Height.CopyPixels();
        Assert.IsFalse(after.SequenceEqual(before));
        var changed = new HashSet<AdtVertex>();
        foreach (AdtVertex vertex in AllVertices())
        {
            if (Math.Abs(rig.ValueAt(AdtVertexKind.Height, vertex).X - TerrainHeight(vertex)) > 1e-4f)
            {
                changed.Add(vertex);
            }
        }

        Assert.IsTrue(changed.Count > 0);
        Vector3 centre = new(-Cell - 12, 0, 12);
        foreach (AdtVertex vertex in changed)
        {
            Vector3 position = rig.World.PositionOf(vertex);
            Assert.IsTrue(new Vector2(position.X - centre.X, position.Z - centre.Z).Length() <= 5.0f + 1e-3f, $"{vertex} is inside the brush");
        }
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void A_vertex_on_the_border_of_the_import_is_written_into_the_padding_too()
    {
        Rig rig = NewRig();
        var border = Corner(-16, 4);
        Assert.IsFalse(rig.World.TryChunk(new ChunkCoord(-3, 0), out _), "the chunk beyond is not imported");

        Stroke(rig, AdtVertexKind.Height, () => new AdtRaiseOp { Rate = 4.0f }, [rig.World.PositionOf(border)], radius: 3.0f);

        Assert.IsTrue(rig.ValueAt(AdtVertexKind.Height, border).X > TerrainHeight(border) + 1.0f);
        Assert.IsTrue(rig.Welded(AdtVertexKind.Height, border), "the padding holds the edge too, so the build does not half-sample it");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void One_stroke_is_one_undo_step_that_restores_the_bytes_exactly()
    {
        Rig rig = NewRig();
        byte[] before = rig.Height.CopyPixels();

        Stroke(rig, AdtVertexKind.Height, () => new AdtRaiseOp(), [new Vector3(-Cell, 0, Cell - 3), new Vector3(-Cell, 0, Cell + 3)], radius: 8.0f);
        byte[] after = rig.Height.CopyPixels();

        Assert.IsFalse(after.SequenceEqual(before));
        Assert.AreEqual(1, rig.Context.EditSessions.Active.History.UndoStack.Count);

        rig.Context.EditSessions.Undo();
        Assert.IsTrue(rig.Height.CopyPixels().SequenceEqual(before), "undo puts the bytes back");

        rig.Context.EditSessions.Redo();
        Assert.IsTrue(rig.Height.CopyPixels().SequenceEqual(after), "redo reapplies the stroke");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void The_undo_command_reports_impacts_for_the_image()
    {
        Rig rig = NewRig();

        Stroke(rig, AdtVertexKind.Height, () => new AdtRaiseOp(), [new Vector3(-Cell - 12, 0, 12)], radius: 5.0f);

        var command = (IChunkChangeCommand)rig.Context.EditSessions.Active.History.UndoStack[0];
        Assert.IsTrue(command.ChunkImpacts.Any(impact => impact.Entity.Component<ImageComponent>() != null));
        Assert.IsTrue(command.ChunkImpacts.All(impact => impact.Before != impact.After));
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void A_stroke_outside_the_imported_chunks_does_nothing()
    {
        Rig rig = NewRig();
        byte[] before = rig.Height.CopyPixels();
        var target = new AdtVertexStrokeTarget(() => rig.World, AdtVertexKind.Height, () => new AdtRaiseOp(), "Raise");
        var stroke = new BrushStroke(new Brush { Radius = 5.0f, Strength = 1.0f }, target, rig.Context.EditSessions);

        Assert.IsTrue(stroke.Begin());
        stroke.MoveTo(new Vector3(500, 0, 500));
        stroke.Finish(record: true);

        Assert.IsTrue(rig.Height.CopyPixels().SequenceEqual(before));
        Assert.AreEqual(0, rig.Context.EditSessions.Active.History.UndoStack.Count);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Flatten_takes_the_height_under_the_first_dab_and_flattens_to_it()
    {
        Rig rig = NewRig();
        var centre = new Vector3(-Cell - 16.0f, 0, 16.0f);
        AdtVertex middle = rig.World.NearestVertex(centre);
        float target = rig.ValueAt(AdtVertexKind.Height, middle).X;

        Stroke(rig, AdtVertexKind.Height, () => new AdtFlattenOp(), [centre, centre, centre, centre, centre, centre], radius: 12.0f, hardness: 1.0f, strength: 1.0f);

        foreach (AdtVertex vertex in NearbyVertices(middle, 1))
        {
            Assert.AreApproximatelyEqual(target, rig.ValueAt(AdtVertexKind.Height, vertex).X, 1e-3, $"{vertex}");
        }
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Painting_colour_on_a_chunk_without_it_turns_it_on_and_leaves_the_neighbour_outside_the_brush()
    {
        Rig rig = NewRig();
        Assert.IsTrue(rig.Facts(TopLeft).HasVertexColor);
        Assert.IsFalse(rig.Facts(TopRight).HasVertexColor);

        // The shared edge of TopLeft and TopRight is z = Cell. The stroke sits just inside TopRight.
        var centre = new Vector3(-Cell - 16.0f, 0, Cell + 3.0f);
        var outsideBrush = Corner(-12, 7);
        NVector3 neighbourBefore = rig.ValueAt(AdtVertexKind.Color, outsideBrush);

        Stroke(rig, AdtVertexKind.Color, () => new AdtPaintOp(new NVector3(1.5f, 1.0f, 0.5f), NVector3.One),
            [centre], radius: 6.0f, hardness: 1.0f, strength: 1.0f);

        Assert.IsTrue(rig.Facts(TopRight).HasVertexColor, "the painted chunk claims colour now");
        Assert.IsTrue(rig.Facts(TopLeft).HasVertexColor);
        Assert.IsFalse(rig.Facts(BottomRight).HasVertexColor, "a chunk the stroke never reached is unchanged");
        Assert.IsFalse(rig.Facts(BottomLeft).HasVertexColor);

        AdtVertex onEdge = rig.World.NearestVertex(centre with { Z = Cell });
        Assert.AreEqual(16, onEdge.Z, "the corner on the chunk edge, in half-steps");
        NVector3 painted = rig.ValueAt(AdtVertexKind.Color, onEdge);
        Assert.AreApproximatelyEqual(1.5, painted.X, 0.01);
        Assert.AreApproximatelyEqual(0.5, painted.Z, 0.01);
        Assert.IsTrue(rig.Welded(AdtVertexKind.Color, onEdge), "both chunks hold the painted edge, so it is not half-sampled");

        Assert.AreEqual(neighbourBefore, rig.ValueAt(AdtVertexKind.Color, outsideBrush), "the neighbour's vertices outside the brush are as they were");
        Assert.AreEqual(NVector3.One, rig.ValueAt(AdtVertexKind.Color, Corner(-10, 15)), "texels the stroke never set were seeded neutral");
        Assert.IsTrue(rig.Welded(AdtVertexKind.Color, Corner(-10, 15)));
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Undoing_a_colour_stroke_turns_the_layer_back_off_and_restores_the_bytes()
    {
        Rig rig = NewRig();
        byte[] before = rig.Color.CopyPixels();

        Stroke(rig, AdtVertexKind.Color, () => new AdtPaintOp(new NVector3(1.5f, 1.0f, 0.5f), NVector3.One),
            [new Vector3(-Cell - 16.0f, 0, Cell + 16.0f)], radius: 6.0f, hardness: 1.0f, strength: 1.0f);
        Assert.IsTrue(rig.Facts(TopRight).HasVertexColor);
        Assert.AreEqual(1, rig.Context.EditSessions.Active.History.UndoStack.Count);

        rig.Context.EditSessions.Undo();

        Assert.IsFalse(rig.Facts(TopRight).HasVertexColor);
        Assert.IsTrue(rig.Color.CopyPixels().SequenceEqual(before));
        rig.Context.EditSessions.Redo();
        Assert.IsTrue(rig.Facts(TopRight).HasVertexColor);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Light_is_added_without_touching_colour_and_turns_the_light_flag_on()
    {
        Rig rig = NewRig();
        byte[] colourBefore = rig.Color.CopyPixels();

        Stroke(rig, AdtVertexKind.Light, () => new AdtPaintOp(new NVector3(1.0f, 0.5f, 0.0f), NVector3.Zero),
            [new Vector3(-Cell - 16.0f, 0, 16.0f)], radius: 6.0f, hardness: 1.0f, strength: 1.0f);

        Assert.IsTrue(rig.Facts(TopLeft).HasVertexLight);
        Assert.IsTrue(rig.Facts(TopLeft).HasVertexColor, "colour is as it was");
        Assert.IsTrue(rig.Color.CopyPixels().SequenceEqual(colourBefore));
        AdtVertex painted = rig.World.NearestVertex(new Vector3(-Cell - 16.0f, 0, 16.0f));
        Assert.AreApproximatelyEqual(1.0, rig.ValueAt(AdtVertexKind.Light, painted).X, 0.01);
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Filling_chunks_sets_every_vertex_as_one_undo_step()
    {
        Rig rig = NewRig();
        var edit = new AdtVertexEdit(rig.World, rig.Context.EditSessions);
        byte[] before = rig.Height.CopyPixels();

        AdtVertexEditResult result = edit.Fill(AdtVertexKind.Height, [TopLeft, TopRight], new NVector3(77.0f, 0, 0));

        Assert.AreEqual(2, result.Changed);
        foreach (ChunkCoord chunk in new[] { TopLeft, TopRight })
        {
            foreach (AdtVertex vertex in AdtVertexEdit.VerticesOf(chunk))
            {
                Assert.AreEqual(77.0f, rig.ValueAt(AdtVertexKind.Height, vertex).X, $"{vertex}");
            }
        }

        Assert.AreEqual(1, rig.Context.EditSessions.Active.History.UndoStack.Count);
        rig.Context.EditSessions.Undo();
        Assert.IsTrue(rig.Height.CopyPixels().SequenceEqual(before));
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void Removing_colour_clears_a_chunk_but_keeps_an_edge_a_neighbour_still_uses()
    {
        Rig rig = NewRig();
        var edit = new AdtVertexEdit(rig.World, rig.Context.EditSessions);
        edit.Fill(AdtVertexKind.Color, [TopLeft, TopRight], new NVector3(1.2f, 1.2f, 1.2f));
        Assert.IsTrue(rig.Facts(TopRight).HasVertexColor);

        AdtVertexEditResult result = edit.Remove(AdtVertexKind.Color, [TopRight]);

        Assert.AreEqual(1, result.Changed);
        Assert.IsFalse(rig.Facts(TopRight).HasVertexColor);
        Assert.IsTrue(rig.Facts(TopLeft).HasVertexColor);
        var shared = Corner(-16 + 4, 8);
        Assert.AreApproximatelyEqual(1.2, rig.ValueAt(AdtVertexKind.Color, shared).X, 0.01, "the edge TopLeft still uses");
        Assert.AreEqual(NVector3.One, rig.ValueAt(AdtVertexKind.Color, Corner(-16 + 4, 8 + 4)), "inside the removed chunk: no data reads neutral");
    }

    [EditorTest(Category = "WoW ADT Vertices", Thread = TestThread.Main)]
    public static void A_map_without_the_raster_cannot_be_edited()
    {
        Rig rig = NewRig(withLight: false);
        var target = new AdtVertexStrokeTarget(() => rig.World, AdtVertexKind.Light, () => new AdtPaintOp(NVector3.One, NVector3.Zero), "Paint");

        Assert.IsFalse(new BrushStroke(new Brush(), target, rig.Context.EditSessions).Begin());
        Assert.AreEqual(1, new AdtVertexEdit(rig.World, rig.Context.EditSessions).Fill(AdtVertexKind.Light, [TopLeft], NVector3.One).Skipped);
    }

    internal static void Stroke(
        Rig rig,
        AdtVertexKind kind,
        Func<IAdtVertexOp> op,
        IReadOnlyList<Vector3> points,
        float radius = 8.0f,
        float hardness = 0.0f,
        float strength = 1.0f)
    {
        var target = new AdtVertexStrokeTarget(() => rig.World, kind, op, "Stroke");
        var stroke = new BrushStroke(new Brush { Radius = radius, Strength = strength, Hardness = hardness }, target, rig.Context.EditSessions);

        Assert.IsTrue(stroke.Begin());
        foreach (Vector3 point in points)
        {
            stroke.MoveTo(point);
        }

        stroke.Finish(record: true);
    }

    private static IEnumerable<AdtVertex> NearbyVertices(AdtVertex centre, int reach)
    {
        for (int z = -reach; z <= reach; z++)
        {
            for (int x = -reach; x <= reach; x++)
            {
                yield return new AdtVertex(centre.X + (2 * x), centre.Z + (2 * z));
            }
        }
    }

    private static IEnumerable<AdtVertex> AllVertices() =>
        new[] { TopLeft, TopRight, BottomLeft, BottomRight }.SelectMany(AdtVertexEdit.VerticesOf).Distinct();

    // World layout: chunk coord (x, y) spans world x in [x*Cell, (x+1)*Cell] and z in [y*Cell, (y+1)*Cell].
    // TopLeft claims vertex colour and holds a gradient in it; the other chunks have none.
    internal static Rig NewRig(bool withLight = true)
    {
        var context = AdtTestDatabase.Attach(new EditorContext(new Node3D(), new Project { Name = "__wms_adt_vertex_test__" }));

        PaintImage MakeImage(int id, string name, PaintImagePixelFormat format, int components)
        {
            var image = new PaintImage { RecordId = id, Name = name };
            image.ConfigureNew(Edge * 4, Edge * 4, chunkSize: Edge, components: components, format: format);
            context.Catalog.Add(image);
            return image;
        }

        SceneEntity Place(PaintImage image, string channel)
        {
            var placement = new MapSceneEntity { Transform = new Transform3D(Basis.Identity, new Vector3(-Cell, 0, Cell)) };
            placement.AddComponent(new ImageComponent(context.Images)
            {
                ImageId = image.RecordId,
                Channel = channel,
                WorldSizeX = Cell * 4,
                WorldSizeZ = Cell * 4,
            });
            context.Scene.Add(placement);
            return placement;
        }

        PaintImage height = MakeImage(1, "ADT Heights", PaintImagePixelFormat.Float32, 1);
        PaintImage color = MakeImage(2, "ADT VColor", PaintImagePixelFormat.Byte, 4);
        PaintImage light = MakeImage(3, "ADT VLight", PaintImagePixelFormat.Byte, 4);
        var placements = new List<ImageComponent>
        {
            Place(height, AdtImportCatalog.HeightChannelName).Component<ImageComponent>()!,
            Place(color, AdtImportCatalog.VColorChannelName).Component<ImageComponent>()!,
        };
        if (withLight)
        {
            placements.Add(Place(light, AdtImportCatalog.VLightChannelName).Component<ImageComponent>()!);
        }

        var tile = new AdtTileComponent { FileX = 32, FileY = 32 };
        var tileEntity = new MapSceneEntity();
        tileEntity.AddComponent(tile);
        context.Scene.Add(tileEntity);

        var world = new AdtMapWorld(
            new LandscapeGrid(new WowAdtProfile(null!).CreateSettings()),
            [(tileEntity, tile)],
            placements,
            () => { });

        foreach (ChunkCoord coord in new[] { TopLeft, TopRight, BottomLeft, BottomRight })
        {
            (_, _, int col, int row) = WowTileAddressing.SlotOf(coord);
            tile.SetChunk(col, row, new AdtChunkFacts(Array.Empty<AdtSlotAssignment>(), HasVertexColor: coord == TopLeft));

            ImageComponent heightPlacement = world.PlacementFor(AdtImportCatalog.HeightChannelName)!;
            world.TryPixelBase(heightPlacement, coord, Edge, out int px, out int py);
            var heights = new byte[Edge * Edge * sizeof(float)];
            var colours = new byte[Edge * Edge * 4];
            ChunkCoord chunkCoord = coord;
            for (int tz = 0; tz < Edge; tz++)
            {
                for (int tx = 0; tx < Edge; tx++)
                {
                    float texelHeight = CellCentreTexels.Texel(
                        tx,
                        tz,
                        (x, z) => TerrainHeight(AdtVertex.Corner(chunkCoord, x, z)),
                        (x, z) => TerrainHeight(AdtVertex.Centre(chunkCoord, x, z)),
                        static (a, b) => (a + b) * 0.5f);
                    BitConverter.TryWriteBytes(heights.AsSpan(((tz * Edge) + tx) * sizeof(float)), texelHeight);
                    if (coord == TopLeft)
                    {
                        NVector3 texelColor = CellCentreTexels.Texel(
                            tx,
                            tz,
                            (x, z) => TerrainColor(AdtVertex.Corner(chunkCoord, x, z)),
                            (x, z) => TerrainColor(AdtVertex.Centre(chunkCoord, x, z)),
                            static (a, b) => (a + b) * 0.5f);
                        AdtVertexCodec.Color.Encode(texelColor, colours.AsSpan(((tz * Edge) + tx) * 4, 4));
                    }
                }
            }

            height.WritePixels(px, py, Edge, Edge, heights);
            color.WritePixels(px, py, Edge, Edge, colours);
        }

        tile.ConsumeDirtyRegions();
        return new Rig { Context = context, World = world, Tile = tile, Height = height, Color = color, Light = light };
    }
}
