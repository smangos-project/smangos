using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace WorldMapStudio.WoW;

/// <summary>An <see cref="ITerrainImageSource"/> over the stored chunks of the ADT import's map-wide images.</summary>
public sealed class StoredTerrainImages : ITerrainImageSource
{
    private readonly Dictionary<TerrainImage, int> _chunkSizes;
    private readonly Dictionary<TerrainImage, Dictionary<ImageChunkCoord, byte[]>> _chunks;

    private StoredTerrainImages(
        Dictionary<TerrainImage, int> chunkSizes,
        Dictionary<TerrainImage, Dictionary<ImageChunkCoord, byte[]>> chunks)
    {
        _chunkSizes = chunkSizes;
        _chunks = chunks;
    }

    public int ChunkSize(TerrainImage image) => _chunkSizes[image];

    public byte[]? Chunk(TerrainImage image, ImageChunkCoord coord) =>
        _chunks.TryGetValue(image, out Dictionary<ImageChunkCoord, byte[]>? byCoord) && byCoord.TryGetValue(coord, out byte[]? pixels)
            ? pixels
            : null;

    /// <summary>Loads every image chunk that holds terrain of <paramref name="tile"/>.</summary>
    public static async Task<StoredTerrainImages> LoadAsync(
        EditorStorage storage,
        IEnumerable<PaintImage> images,
        AdtImportCatalog catalog,
        WowTileId tile)
    {
        (TerrainImage Image, int Id, int ChunkSize, int Edge)[] specs =
        [
            (TerrainImage.Height, catalog.HeightImageId, catalog.HeightImageChunkSize, HeightMapper.TexelEdge),
            (TerrainImage.Alpha, catalog.AlphaImageId, catalog.AlphaImageChunkSize, TextureMapper.AlphaEdge),
            (TerrainImage.Holes, catalog.HoleImageId, catalog.HoleImageChunkSize, HoleMapper.HoleEdge),
            (TerrainImage.Area, catalog.AreaImageId, catalog.ChunkImageChunkSize, 1),
            (TerrainImage.Flags, catalog.FlagsImageId, catalog.ChunkImageChunkSize, 1),
            (TerrainImage.VertexColor, catalog.VertexColorImageId, catalog.VertexImageChunkSize, VertexColorMapper.TexelEdge),
            (TerrainImage.VertexLight, catalog.VertexLightImageId, catalog.VertexImageChunkSize, VertexColorMapper.TexelEdge),
        ];

        List<ChunkCoord> coords = WowTileAddressing.EnumerateChunks(tile).Select(c => c.Coord).ToList();
        int minX = coords.Min(c => c.X) - WowAdtImportBatchOperation.MapMinChunk.X;
        int maxX = coords.Max(c => c.X) - WowAdtImportBatchOperation.MapMinChunk.X;
        int minY = coords.Min(c => c.Y) - WowAdtImportBatchOperation.MapMinChunk.Y;
        int maxY = coords.Max(c => c.Y) - WowAdtImportBatchOperation.MapMinChunk.Y;

        Dictionary<int, PaintImage> byId = images.Where(i => i.RecordId != null).ToDictionary(i => i.RecordId!.Value);
        var wanted = new Dictionary<PaintImage, IReadOnlyCollection<ImageChunkCoord>>();
        var imageOf = new Dictionary<PaintImage, List<TerrainImage>>();
        foreach ((TerrainImage kind, int id, int size, int edge) in specs)
        {
            if (!byId.TryGetValue(id, out PaintImage? paint))
            {
                continue;
            }

            var set = new HashSet<ImageChunkCoord>();
            for (int cy = (edge * minY) / size; cy <= ((edge * (maxY + 1)) - 1) / size; cy++)
            {
                for (int cx = (edge * minX) / size; cx <= ((edge * (maxX + 1)) - 1) / size; cx++)
                {
                    set.Add(new ImageChunkCoord(cx, cy));
                }
            }

            wanted[paint] = set;
            if (!imageOf.TryGetValue(paint, out List<TerrainImage>? kinds))
            {
                imageOf[paint] = kinds = [];
            }

            kinds.Add(kind);
        }

        var chunks = new Dictionary<TerrainImage, Dictionary<ImageChunkCoord, byte[]>>();
        foreach ((PaintImage paint, ImageChunkCoord coord, byte[] pixels) in await storage.LoadImageChunksAsync(wanted).ConfigureAwait(false))
        {
            foreach (TerrainImage kind in imageOf[paint])
            {
                if (!chunks.TryGetValue(kind, out Dictionary<ImageChunkCoord, byte[]>? byCoord))
                {
                    chunks[kind] = byCoord = [];
                }

                byCoord[coord] = pixels;
            }
        }

        return new StoredTerrainImages(specs.ToDictionary(s => s.Image, s => s.ChunkSize), chunks);
    }
}
