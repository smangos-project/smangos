using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>
/// One vertex edit in progress on an imported map: the lattice over one raster, the image stroke it writes
/// inside, and the chunk facts it changed. A brush stroke and a one-shot edit both run on one, so both turn on a
/// chunk's vertex colour or light the same way and both end as the same single undo command.
/// </summary>
public sealed class AdtVertexSession
{
    private readonly Dictionary<ChunkCoord, (AdtChunkRef Chunk, AdtChunkFacts Original)> _facts = [];
    private readonly PaintImage _image;
    private bool _open = true;

    private AdtVertexSession(AdtMapWorld world, ImageComponent placement, AdtVertexKind kind)
    {
        World = world;
        Placement = placement;
        Kind = kind;
        _image = placement.Image!;

        AdtVertexCodec codec = AdtVertexCodec.For(kind);
        Lattice = new AdtVertexLattice(new AdtImageVertexStore(world, placement, codec), codec);
        _image.BeginStroke();
    }

    public AdtMapWorld World { get; }

    public ImageComponent Placement { get; }

    public AdtVertexKind Kind { get; }

    public AdtVertexLattice Lattice { get; }

    /// <summary>The name of the landscape channel that carries a kind of vertex data.</summary>
    public static string ChannelOf(AdtVertexKind kind) => kind switch
    {
        AdtVertexKind.Height => AdtImportCatalog.HeightChannelName,
        AdtVertexKind.Color => AdtImportCatalog.VColorChannelName,
        _ => AdtImportCatalog.VLightChannelName,
    };

    /// <summary>Starts an edit. Null when the map has no imported raster for <paramref name="kind"/>.</summary>
    public static AdtVertexSession? TryOpen(AdtMapWorld world, AdtVertexKind kind) =>
        world.PlacementFor(ChannelOf(kind)) is { } placement && placement.Image is { } image &&
        image.Stride == AdtVertexCodec.For(kind).Stride &&
        world.PixelsPerChunk(placement) == AdtVertexLattice.Edge
            ? new AdtVertexSession(world, placement, kind)
            : null;

    public bool Has(AdtChunkFacts facts) => Kind switch
    {
        AdtVertexKind.Color => facts.HasVertexColor,
        AdtVertexKind.Light => facts.HasVertexLight,
        _ => true,
    };

    /// <summary>Follows a batch of set vertices: turns on the layer in the chunks that now carry it, writes the
    /// changed blocks and tells the landscape.</summary>
    public void Settle()
    {
        foreach (ChunkCoord coord in Lattice.TakeNewlyChanged())
        {
            EnableLayer(coord);
        }

        Lattice.Flush();
        World.NoticeChanged();
    }

    /// <summary>Ends the edit. The one undo command for everything it changed, or null when nothing did.</summary>
    public IEditCommand? End(string description)
    {
        if (!_open)
        {
            return null;
        }

        _open = false;
        Lattice.Flush();
        return AdtEditCommit.Commit(_image, Placement, _facts.Values, description);
    }

    /// <summary>Ends the edit without a command. What was written stays.</summary>
    public void Cancel()
    {
        if (_open)
        {
            _open = false;
            _image.EndStroke();
        }
    }

    /// <summary>Sets whether an imported chunk claims vertex colour or light.</summary>
    public void SetLayer(ChunkCoord coord, bool enabled)
    {
        if (Kind == AdtVertexKind.Height || !World.TryChunk(coord, out AdtChunkRef chunk))
        {
            return;
        }

        AdtChunkFacts facts = chunk.Tile.Chunks[(chunk.Col, chunk.Row)];
        if (Has(facts) == enabled)
        {
            return;
        }

        _facts.TryAdd(coord, (chunk, facts));
        chunk.Tile.SetChunk(chunk.Col, chunk.Row, Kind == AdtVertexKind.Color
            ? facts with { HasVertexColor = enabled }
            : facts with { HasVertexLight = enabled });
    }

    // A chunk that had no colour or light starts using it once a vertex it holds is set. The texels it has not
    // set are filled with the neutral value first, since the raster now claims all of them.
    private void EnableLayer(ChunkCoord coord)
    {
        if (Kind == AdtVertexKind.Height ||
            !World.TryChunk(coord, out AdtChunkRef chunk) ||
            Has(chunk.Tile.Chunks[(chunk.Col, chunk.Row)]))
        {
            return;
        }

        Lattice.Seed(coord);
        SetLayer(coord, enabled: true);
    }
}
