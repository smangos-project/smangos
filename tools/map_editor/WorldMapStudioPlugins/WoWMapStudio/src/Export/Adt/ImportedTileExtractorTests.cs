using System.Collections.Generic;
using System.Linq;

namespace WorldMapStudio.WoW;

public static class ImportedTileExtractorTests
{
    private const int Size = 128;

    private sealed class DictSource : ITerrainImageSource
    {
        public readonly Dictionary<(TerrainImage, ImageChunkCoord), byte[]> Chunks = [];

        public int ChunkSize(TerrainImage image) => Size;

        public byte[]? Chunk(TerrainImage image, ImageChunkCoord coord) => Chunks.GetValueOrDefault((image, coord));

        public void Add(TerrainImage image, IEnumerable<(ImageChunkCoord Coord, byte[] Pixels)> chunks)
        {
            foreach (var (coord, pixels) in chunks)
            {
                Chunks[(image, coord)] = pixels;
            }
        }
    }

    [EditorTest(Category = "WoW Export", Thread = TestThread.Background)]
    public static void Extract_inverts_the_import_image_build()
    {
        WowTestContent.RequireFile("world/maps/azeroth/azeroth_32_48.adt");
        string project = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "wms-extract-test-" + System.Guid.NewGuid().ToString("N"));
        var fs = WowClientFiles.OpenIsolated(project);
        var wdt = WowWdt.Read(fs, "azeroth");
        var tile = WowAdtTileReader.Read(fs, wdt, new ImportedTileId("azeroth", 32, 48));
        System.IO.Directory.Delete(project, true);

        var id = new WowTileId("azeroth", 32, 48);
        var source = new DictSource();
        source.Add(TerrainImage.Height, WowAdtImportBatchOperation.BuildHeightChunks(id, tile, Size).Select(c => (c.Coord, c.Pixels)));
        source.Add(TerrainImage.Alpha, WowAdtImportBatchOperation.BuildAlphaChunks(id, tile, Size));
        source.Add(TerrainImage.Holes, WowAdtImportBatchOperation.BuildHoleChunks(id, tile, Size));
        source.Add(TerrainImage.Area, WowAdtImportBatchOperation.BuildScalarChunks(id, tile, Size, AreaMapper.AreaValue));
        source.Add(TerrainImage.Flags, WowAdtImportBatchOperation.BuildScalarChunks(id, tile, Size, AreaMapper.FlagsValue));
        source.Add(TerrainImage.VertexColor, WowAdtImportBatchOperation.BuildVertexChunks(id, tile, Size, true).Select(c => (c.Coord, c.Pixels)));
        source.Add(TerrainImage.VertexLight, WowAdtImportBatchOperation.BuildVertexChunks(id, tile, Size, false).Select(c => (c.Coord, c.Pixels)));

        var back = ImportedTileExtractor.Extract(id, tile, source);

        for (int i = 0; i < tile.Chunks.Count; i++)
        {
            var a = tile.Chunks[i];
            var b = back.Chunks[i];
            Assert.IsTrue(a.OuterHeights.Zip(b.OuterHeights).All(p => System.MathF.Abs(p.First - p.Second) < 1e-3f), $"outer heights {i}");
            Assert.IsTrue(a.InnerHeights.Zip(b.InnerHeights).All(p => System.MathF.Abs(p.First - p.Second) < 1e-3f), $"inner heights {i}");
            Assert.IsTrue(a.Holes.SequenceEqual(b.Holes), $"holes {i}");
            Assert.AreEqual(a.AreaId, b.AreaId);
            Assert.AreEqual(a.Flags & AreaMapper.RoundTrippedFlagMask, b.Flags & AreaMapper.RoundTrippedFlagMask);
            for (int l = 1; l < a.Layers.Count; l++)
            {
                if (a.Layers[l].Alpha != null)
                {
                    Assert.IsTrue(a.Layers[l].Alpha!.SequenceEqual(b.Layers[l].Alpha!), $"alpha {i}/{l}");
                }
            }
        }
    }
}
