using System.Numerics;

namespace WorldMapStudio.WoW;

/// <summary>Reads the value of an imported vertex without starting an edit: for readouts and the eyedropper.</summary>
public static class AdtVertexProbe
{
    /// <summary>The value of <paramref name="vertex"/>, as the edit tools see it. False when the map has no such
    /// raster or the vertex's data is not loaded.</summary>
    public static bool TryRead(AdtMapWorld world, AdtVertexKind kind, AdtVertex vertex, out Vector3 value)
    {
        value = default;
        AdtVertexCodec codec = AdtVertexCodec.For(kind);
        if (world.PlacementFor(AdtVertexSession.ChannelOf(kind)) is not { Image: { } image } placement ||
            image.Stride != codec.Stride ||
            world.PixelsPerChunk(placement) != AdtVertexLattice.Edge)
        {
            return false;
        }

        var lattice = new AdtVertexLattice(new AdtImageVertexStore(world, placement, codec), codec);
        return lattice.TryGet(vertex, out value);
    }

    /// <summary>The first imported chunk that holds <paramref name="vertex"/>. Throws when none does.</summary>
    public static AdtChunkRef ImportedHolder(AdtMapWorld world, AdtVertex vertex)
    {
        foreach (ChunkCoord holder in AdtVertexLattice.ChunksHolding(vertex))
        {
            if (world.TryChunk(holder, out AdtChunkRef chunk))
            {
                return chunk;
            }
        }

        throw new System.InvalidOperationException("There is no imported chunk at that point.");
    }
}
