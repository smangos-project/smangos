using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

public static class PlacementExtractorTests
{
    private static readonly Aabb Bounds = new(new Vector3(-1, -2, -3), new Vector3(2, 4, 6));

    private static ImportedTile Template(params ImportedDoodad[] doodads) => new()
    {
        TileId = new ImportedTileId("azeroth", 32, 48),
        Textures = [],
        HeightTextures = [],
        Doodads = doodads,
        Objects = [],
        Chunks = [],
    };

    private static ImportedDoodad Doodad(uint uid) =>
        new(100, uid, new Vector3((32 * 533.33333f) + 40, 90, (48 * 533.33333f) + 50), new Vector3(10, 20, 30), 1024, 0);

    private static EditorPlacement PlacementOf(long record, ImportedDoodad d) =>
        new(record, "world/x.m2", PlacementMapper.InvertTransform(d.Position, d.RotationDegrees, d.Scale, Bounds.GetCenter()), Bounds, 0);

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Placements_keep_moved_add_and_drop()
    {
        ImportedDoodad kept = Doodad(1), moved = Doodad(2), gone = Doodad(3);
        ImportedTile template = Template(kept, moved, gone);
        EditorPlacement keptP = PlacementOf(11, kept);
        EditorPlacement movedP = PlacementOf(12, moved);
        movedP = movedP with { Transform = movedP.Transform.Translated(new Vector3(0, 5, 0)) };
        EditorPlacement added = PlacementOf(14, Doodad(9)) with { RecordId = 14 };
        added = added with { Transform = added.Transform.Translated(new Vector3(1, 0, 0)) };

        var uids = new Dictionary<long, uint> { [11] = 1, [12] = 2 };
        uint next = 500;
        var (doodads, objects) = PlacementExtractor.Apply(
            template, [keptP, movedP, added], r => uids.TryGetValue(r, out uint u) ? u : null, _ => next++, _ => 100);

        Assert.AreEqual(0, objects.Count);
        Assert.AreEqual(3, doodads.Count);
        Assert.IsTrue(doodads[0] == kept, "unmoved record untouched");
        Assert.AreEqual(2u, doodads[1].UniqueId);
        Assert.IsTrue(System.MathF.Abs(doodads[1].Position.Y - (moved.Position.Y + 5)) < 0.01f, "moved height");
        Assert.AreEqual(500u, doodads[2].UniqueId);
    }
}
