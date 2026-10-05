using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

public static class AdtTileComponentTests
{
    private static readonly WowTileId Tile = new("TestMap", 32, 32);

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void SlotOf_inverts_ChunkAt_for_every_slot_in_a_tile()
    {
        for (int row = 0; row < 16; row += 5)
        {
            for (int col = 0; col < 16; col += 5)
            {
                ChunkCoord coord = WowTileAddressing.ChunkAt(Tile, col, row);
                (int fileX, int fileY, int localCol, int localRow) = WowTileAddressing.SlotOf(coord);

                Assert.AreEqual(Tile.FileX, fileX);
                Assert.AreEqual(Tile.FileY, fileY);
                Assert.AreEqual(col, localCol);
                Assert.AreEqual(row, localRow);
            }
        }
    }

    [EditorTest(Category = "WoW Import", Thread = TestThread.Background)]
    public static void Claims_the_height_and_slot_layers_only_in_imported_chunks()
    {
        var functions = new LandscapeFunctions();

        var heightMaterial = new LandscapeMaterial { Name = "adt terrain", RecordId = 1, TexturePath = "res://terrain.png" };
        var slotMaterial = new LandscapeMaterial { Name = "grass", RecordId = 2, TexturePath = "res://grass.png" };
        var heightLayer = new LandscapeLayer { Name = "ADT Terrain", RecordId = 1, DrawOrder = 0, IsBase = true };
        var slotLayer1 = new LandscapeLayer { Name = "ADT Layer 1", RecordId = 3, DrawOrder = 2 };

        var settings = new LandscapeSettings
        {
            ChunkWorldSize = 64.0f,
            ChunkHeightResolution = 9,
            ChunkAlphaResolution = 16,
            TextureLimit = 4,
            FallbackMaterialId = 99,
        };
        var catalog = new LandscapeCatalog([], [heightLayer, slotLayer1], [heightMaterial, slotMaterial], functions);

        ChunkCoord importedCoord = WowTileAddressing.ChunkAt(Tile, localCol: 0, localRow: 0);
        ChunkCoord otherCoord = WowTileAddressing.ChunkAt(Tile, localCol: 1, localRow: 0);

        var component = new AdtTileComponent
        {
            FileX = Tile.FileX,
            FileY = Tile.FileY,
            HeightLayerId = heightLayer.RecordId,
            HeightMaterialId = heightMaterial.RecordId,
            SlotLayerIds = [null, slotLayer1.RecordId, null, null],
        };
        component.SetChunk(0, 0, new AdtChunkFacts(
            Slots: [new AdtSlotAssignment(SlotIndex: 1, LayerId: slotLayer1.RecordId, MaterialId: slotMaterial.RecordId)]));

        var entity = new MapSceneEntity();
        entity.AddComponent(new StampComponent { Radius = 200.0f });
        entity.AddComponent(component);

        LandscapeBuildResult result = new LandscapeBuilder(settings, catalog, functions)
            .Build([importedCoord, otherCoord], [component]);

        Assert.IsTrue(result.Chunks[importedCoord].Layers.Any(output => output.Material == heightMaterial),
            "the imported chunk gets the height layer's material");
        Assert.IsTrue(result.Chunks[importedCoord].Layers.Any(output => output.Material == slotMaterial),
            "and its assigned slot material");
        Assert.IsFalse(result.Chunks[otherCoord].Layers.Any(output => output.Material == heightMaterial || output.Material == slotMaterial),
            "a chunk this tile did not import is untouched by the component");
    }
}
