using System;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Why a chunk does not render the textures its slots name: layers other than the imported ones won
/// slots, and imported textures that lost theirs.</summary>
public sealed record AdtChunkOverride(IReadOnlyList<string> ForeignLayers, IReadOnlyList<string> DroppedTextures);

/// <summary>
/// What the texture tools edit on one map: the shared <see cref="AdtMapWorld"/>, the alpha image and the
/// placement that maps it into the world, and the slot materials.
/// </summary>
public sealed class AdtTextureWorld
{
    public AdtTextureWorld(
        LandscapeGrid grid,
        ImageComponent alphaPlacement,
        IEnumerable<(SceneEntity Entity, AdtTileComponent Tile)> tiles,
        AdtSlotMaterials materials,
        Action changed,
        Func<ChunkCoord, LandscapeChunkOutput?>? outputAt = null)
        : this(new AdtMapWorld(grid, tiles, [alphaPlacement], changed, outputAt), alphaPlacement, materials)
    {
    }

    public AdtTextureWorld(AdtMapWorld map, ImageComponent alphaPlacement, AdtSlotMaterials materials)
    {
        Map = map;
        Placement = alphaPlacement;
        Image = alphaPlacement.Image ?? throw new ArgumentException("The alpha placement has no image.", nameof(alphaPlacement));
        Materials = materials;
    }

    public AdtMapWorld Map { get; }

    public LandscapeGrid Grid => Map.Grid;

    public ImageComponent Placement { get; }

    /// <summary>The map-wide alpha image, R/G/B holding slots 1-3.</summary>
    public PaintImage Image { get; }

    public AdtSlotMaterials Materials { get; }

    public int TileCount => Map.TileCount;

    /// <summary>The imported tiles loaded for this map.</summary>
    public IEnumerable<AdtTileComponent> Tiles => Map.Tiles;

    /// <summary>Builds the world for the editor's current map. Null, with a reason, when the map has no
    /// imported alpha data or no landscape.</summary>
    public static AdtTextureWorld? FromEditor(EditorContext editor, MapId map, out string? problem)
    {
        if (AdtMapWorld.FromEditor(editor, map, out problem) is not { } world)
        {
            return null;
        }

        if (world.PlacementFor(AdtImportCatalog.AlphaChannelName) is not { } placement)
        {
            problem = "This map has no imported ADT alpha data.";
            return null;
        }

        return new AdtTextureWorld(world, placement, new AdtSlotMaterials(editor, map));
    }

    public bool TryChunk(ChunkCoord coord, out AdtChunkRef chunk) => Map.TryChunk(coord, out chunk);

    public bool TryChunkAt(Vector3 world, out AdtChunkRef chunk) => Map.TryChunkAt(world, out chunk);

    /// <summary>The chunk's 64x64 alpha block: its top-left pixel in the alpha image. False when the block
    /// does not lie inside the image.</summary>
    public bool TryPixelBase(ChunkCoord coord, out int pixelX, out int pixelY) =>
        Map.TryPixelBase(Placement, coord, AdtChunkTextureSet.Edge, out pixelX, out pixelY);

    /// <summary>The chunk's alpha texels in world space, for <see cref="BrushRaster"/>.</summary>
    public BrushGrid GridOf(ChunkCoord coord)
    {
        Vector3 origin = Grid.OriginOf(coord);
        float texel = Grid.ChunkSize / AdtChunkTextureSet.Edge;
        return new BrushGrid(origin.X, origin.Z, texel, texel, AdtChunkTextureSet.Edge, AdtChunkTextureSet.Edge);
    }

    /// <summary>Whether the landscape has built terrain for a chunk.</summary>
    public bool HasTerrain(ChunkCoord coord) => Map.HasTerrain(coord);

    /// <summary>
    /// Compares what the landscape built for a chunk with what the chunk's slots say. A layer the tile did not
    /// put there took a slot, or a slot's texture did not make it into the build, and the tool's idea of the
    /// chunk's four textures disagrees with what renders. Null when they agree, or nothing is built yet.
    /// </summary>
    public AdtChunkOverride? OverrideAt(AdtChunkRef chunk)
    {
        if (Map.OutputAt(chunk.Coord) is not { } output)
        {
            return null;
        }

        AdtChunkFacts facts = chunk.Tile.Chunks[(chunk.Col, chunk.Row)];
        HashSet<int> slotIds = facts.Slots.Select(slot => slot.MaterialId).OfType<int>().ToHashSet();
        HashSet<int> builtIds = output.Layers.Select(layer => layer.Material?.RecordId).OfType<int>().ToHashSet();

        string[] foreign = output.Layers
            .Where(layer => layer.Material is { } material && !slotIds.Contains(material.RecordId ?? -1))
            .Select(layer => layer.Material!.DisplayName)
            .ToArray();
        string[] dropped = facts.Slots
            .Where(slot => slot.MaterialId is int id && !builtIds.Contains(id))
            .Select(slot => Materials.TextureOf(slot.MaterialId) ?? "?")
            .ToArray();

        return foreign.Length == 0 && dropped.Length == 0 ? null : new AdtChunkOverride(foreign, dropped);
    }

    /// <summary>Tells the landscape its content changed, so it rebuilds what was edited.</summary>
    public void NoticeChanged() => Map.NoticeChanged();

    /// <summary>The textures a chunk stacks, in slot order. Null when a slot has no known material, so the
    /// chunk cannot be edited safely.</summary>
    public IReadOnlyList<string>? TexturesOf(AdtChunkFacts facts)
    {
        var textures = new List<string>(facts.Slots.Count);
        foreach (AdtSlotAssignment slot in facts.Slots.OrderBy(slot => slot.SlotIndex))
        {
            if (slot.SlotIndex != textures.Count || Materials.TextureOf(slot.MaterialId) is not { } texture)
            {
                return null;
            }

            textures.Add(texture);
        }

        return textures;
    }

    /// <summary>The facts for a chunk whose stack is now <paramref name="textures"/>, creating any slot
    /// materials that do not exist yet.</summary>
    public AdtChunkFacts FactsFor(AdtChunkFacts original, IReadOnlyList<string> textures)
    {
        Materials.EnsurePairs(textures.Select((texture, slot) => (texture, slot)));
        var slots = new List<AdtSlotAssignment>(textures.Count);
        for (int slot = 0; slot < textures.Count; slot++)
        {
            slots.Add(new AdtSlotAssignment(slot, LayerId: null, Materials.MaterialFor(textures[slot], slot)));
        }

        return original with { Slots = slots };
    }
}
