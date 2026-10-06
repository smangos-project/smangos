namespace WorldMapStudio.WoW;

/// <summary>An <see cref="IAdtVertexStore"/> over an imported raster: each chunk's 16x16 block of the placement's
/// image, and the imported chunks as its sources.</summary>
public sealed class AdtImageVertexStore : IAdtVertexStore
{
    private readonly AdtMapWorld _world;
    private readonly ImageComponent _placement;
    private readonly PaintImage _image;
    private readonly int _stride;

    public AdtImageVertexStore(AdtMapWorld world, ImageComponent placement, AdtVertexCodec codec)
    {
        _world = world;
        _placement = placement;
        _image = placement.Image ?? throw new System.ArgumentException("The placement has no image.", nameof(placement));
        _stride = codec.Stride;
    }

    public AdtBlockState Open(ChunkCoord chunk, out byte[]? block)
    {
        block = null;
        if (_image.Stride != _stride)
        {
            return AdtBlockState.Outside;
        }

        AdtBlockState state = _world.BlockStateOf(_placement, chunk, AdtVertexLattice.Edge);
        if (state == AdtBlockState.Ready)
        {
            _world.TryPixelBase(_placement, chunk, AdtVertexLattice.Edge, out int x, out int y);
            block = _image.ReadPixels(x, y, AdtVertexLattice.Edge, AdtVertexLattice.Edge);
        }

        return state;
    }

    public bool IsSource(ChunkCoord chunk) => _world.TryChunk(chunk, out _);

    public void Write(ChunkCoord chunk, byte[] block)
    {
        if (_world.TryPixelBase(_placement, chunk, AdtVertexLattice.Edge, out int x, out int y))
        {
            _image.WritePixels(x, y, AdtVertexLattice.Edge, AdtVertexLattice.Edge, block);
        }
    }
}
