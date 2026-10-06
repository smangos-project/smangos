using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Why a chunk could not be loaded for editing.</summary>
public enum AdtChunkLoadResult
{
    Loaded,

    /// <summary>No loaded imported tile owns this chunk.</summary>
    NotImported,

    /// <summary>A slot has no known material, or the alpha block lies outside the image.</summary>
    Unresolved,

    /// <summary>The chunk's alpha data is stored but not loaded yet.</summary>
    Loading,
}

/// <summary>
/// One chunk being edited: its texture stack loaded from the tile's slot facts and the alpha image, kept in
/// memory for the length of one stroke or operation, and written back after each change. The alpha image
/// must be inside a stroke (<see cref="PaintImage.BeginStroke"/>) for the writes to be undoable.
/// </summary>
public sealed class AdtChunkEdit
{
    private readonly AdtTextureWorld _world;
    private readonly int _pixelX;
    private readonly int _pixelY;

    private AdtChunkEdit(AdtTextureWorld world, AdtChunkRef chunk, AdtChunkFacts original, AdtChunkTextureSet set, int pixelX, int pixelY)
    {
        _world = world;
        Chunk = chunk;
        Original = original;
        Set = set;
        _pixelX = pixelX;
        _pixelY = pixelY;
    }

    public AdtChunkRef Chunk { get; }

    /// <summary>The slot facts as they were before the edit.</summary>
    public AdtChunkFacts Original { get; }

    public AdtChunkTextureSet Set { get; }

    public static AdtChunkLoadResult TryLoad(AdtTextureWorld world, ChunkCoord coord, out AdtChunkEdit? edit)
    {
        edit = null;
        if (!world.TryChunk(coord, out AdtChunkRef chunk))
        {
            return AdtChunkLoadResult.NotImported;
        }

        AdtChunkFacts facts = chunk.Tile.Chunks[(chunk.Col, chunk.Row)];
        IReadOnlyList<string>? textures = world.TexturesOf(facts);
        if (textures == null || !world.TryPixelBase(coord, out int pixelX, out int pixelY))
        {
            return AdtChunkLoadResult.Unresolved;
        }

        if (!world.Image.CanEditRegion(pixelX, pixelY, AdtChunkTextureSet.Edge, AdtChunkTextureSet.Edge))
        {
            return AdtChunkLoadResult.Loading;
        }

        byte[] block = world.Image.ReadPixels(pixelX, pixelY, AdtChunkTextureSet.Edge, AdtChunkTextureSet.Edge);
        edit = new AdtChunkEdit(world, chunk, facts, AdtChunkTextureSet.FromStored(textures, block), pixelX, pixelY);
        return AdtChunkLoadResult.Loaded;
    }

    /// <summary>Writes the stack back: the alpha block, and the tile's slot facts when the slot list changed.</summary>
    public void Store()
    {
        AdtStoredChunk stored = Set.ToStored();
        _world.Image.WritePixels(_pixelX, _pixelY, AdtChunkTextureSet.Edge, AdtChunkTextureSet.Edge, stored.Rgba);
        if (stored.SlotsChanged)
        {
            Chunk.Tile.SetChunk(Chunk.Col, Chunk.Row, _world.FactsFor(Original, stored.Textures));
        }
    }

    /// <summary>Ends the alpha image's stroke and returns the one undo command for the alpha edits and the
    /// slot changes of <paramref name="edits"/>, or null when nothing changed.</summary>
    public static IEditCommand? Commit(AdtTextureWorld world, IEnumerable<AdtChunkEdit> edits, string description) =>
        AdtEditCommit.Commit(world.Image, world.Placement, edits.Select(edit => (edit.Chunk, edit.Original)), description);
}
