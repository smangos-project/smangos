using System;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

public static class AdtHoleEditTests
{
    private const int Edge = HoleMapper.HoleEdge;
    private static readonly float Cell = WowTileAddressing.CellSize;
    private static readonly ChunkCoord TopLeft = WowTileAddressing.ChunkAt(new WowTileId("", 32, 32), 0, 1);

    private static (EditorContext Context, AdtMapWorld World, PaintImage Image) NewRig()
    {
        var context = AdtTestDatabase.Attach(new EditorContext(new Node3D(), new Project { Name = "__wms_adt_hole_test__" }));
        var image = new PaintImage { RecordId = 1, Name = "ADT Holes" };
        image.ConfigureNew(Edge * 4, Edge * 4, chunkSize: Edge, components: 1, format: PaintImagePixelFormat.Byte);
        context.Catalog.Add(image);

        var placement = new MapSceneEntity { Transform = new Transform3D(Basis.Identity, new Vector3(-Cell, 0, Cell)) };
        placement.AddComponent(new ImageComponent(context.Images)
        {
            ImageId = 1,
            Channel = AdtImportCatalog.HoleChannelName,
            WorldSizeX = Cell * 4,
            WorldSizeZ = Cell * 4,
        });
        context.Scene.Add(placement);

        var tile = new AdtTileComponent { FileX = 32, FileY = 32 };
        var tileEntity = new MapSceneEntity();
        tileEntity.AddComponent(tile);
        context.Scene.Add(tileEntity);
        (_, _, int col, int row) = WowTileAddressing.SlotOf(TopLeft);
        tile.SetChunk(col, row, new AdtChunkFacts(Array.Empty<AdtSlotAssignment>()));

        var world = new AdtMapWorld(
            new LandscapeGrid(new WowAdtProfile(null!).CreateSettings()),
            [(tileEntity, tile)],
            [placement.Component<ImageComponent>()!],
            () => { });
        return (context, world, image);
    }

    [EditorTest(Category = "WoW ADT Holes", Thread = TestThread.Main)]
    public static void Cut_and_fill_set_the_chosen_cells_and_undo_as_one_step()
    {
        (EditorContext context, AdtMapWorld world, _) = NewRig();
        var edit = new AdtHoleEdit(world, context.EditSessions);

        Assert.IsTrue(edit.Read(TopLeft)!.All(cut => !cut));
        Assert.AreEqual(1, edit.Set([TopLeft], cut: true, [0, 5]));
        bool[] cells = edit.Read(TopLeft)!;
        Assert.AreEqual(2, cells.Count(cut => cut));
        Assert.IsTrue(cells[0] && cells[5]);

        Assert.AreEqual(0, edit.Set([TopLeft], cut: true, [0]), "already cut");
        Assert.AreEqual(1, edit.Set([TopLeft], cut: false, [0]));
        Assert.AreEqual(1, edit.Read(TopLeft)!.Count(cut => cut));

        context.EditSessions.Undo();
        Assert.AreEqual(2, edit.Read(TopLeft)!.Count(cut => cut));
    }

    [EditorTest(Category = "WoW ADT Holes", Thread = TestThread.Main)]
    public static void Cut_at_hits_the_cell_under_the_point()
    {
        (EditorContext context, AdtMapWorld world, _) = NewRig();
        var edit = new AdtHoleEdit(world, context.EditSessions);
        Vector3 origin = world.Grid.OriginOf(TopLeft);
        float size = Cell / Edge;

        Assert.AreEqual(1, edit.SetAt([origin + new Vector3(size * 2.5f, 0, size * 1.5f)], cut: true));
        bool[] cells = edit.Read(TopLeft)!;
        Assert.AreEqual(1, cells.Count(cut => cut));
        Assert.IsTrue(cells[(1 * Edge) + 2]);
    }

    [EditorTest(Category = "WoW ADT Holes", Thread = TestThread.Main)]
    public static void Chunks_that_are_not_imported_are_left_alone()
    {
        (EditorContext context, AdtMapWorld world, _) = NewRig();
        var edit = new AdtHoleEdit(world, context.EditSessions);
        ChunkCoord other = WowTileAddressing.ChunkAt(new WowTileId("", 32, 32), 5, 5);
        Assert.AreEqual(0, edit.Set([other], cut: true));
        Assert.IsNull(edit.Read(other));
    }
}
