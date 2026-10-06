using System.Collections.Generic;
using Godot;
using ImGuiNET;
using NVector4 = System.Numerics.Vector4;

namespace WorldMapStudio.WoW;

/// <summary>
/// The texture tool's viewport overlays, drawn over the chunks around the cursor: the chunk grid, a
/// layer-count heat map, chunks that cannot take the current texture, chunks the tool will not touch (not
/// imported, or their alpha data not loaded yet), and the chunk selection.
/// </summary>
public sealed class AdtTextureOverlay
{
    private const int MaxVerdictLoadsPerFrame = 3;

    private static readonly NVector4[] LayerColors =
    [
        new(0.35f, 0.55f, 1.00f, 0.22f),
        new(0.35f, 0.85f, 0.45f, 0.22f),
        new(1.00f, 0.85f, 0.25f, 0.24f),
        new(1.00f, 0.35f, 0.30f, 0.28f),
    ];

    private readonly record struct Verdict(int Revision, AdtChunkFacts Facts, string Texture, bool CanTake);

    private readonly Dictionary<ChunkCoord, Verdict> _verdicts = [];

    public void Draw(
        AdtTextureToolFactory factory,
        AdtTextureWorld world,
        Vector3 centre,
        BrushSurface surface,
        in ViewportProjector projector)
    {
        if (!factory.ShowChunkGrid && !factory.ShowLayerCount && !factory.ShowUnpaintable && !factory.ShowNotImported &&
            factory.Selection.Count == 0)
        {
            return;
        }

        ChunkCoord middle = world.Grid.CoordAt(centre);
        int loadsLeft = MaxVerdictLoadsPerFrame;
        ImDrawListPtr drawList = ImGui.GetWindowDrawList();

        for (int x = middle.X - AdtChunkOverlay.ViewChunks; x <= middle.X + AdtChunkOverlay.ViewChunks; x++)
        {
            for (int y = middle.Y - AdtChunkOverlay.ViewChunks; y <= middle.Y + AdtChunkOverlay.ViewChunks; y++)
            {
                var coord = new ChunkCoord(x, y);
                if (!AdtChunkOverlay.TryQuad(world.Map, coord, surface, projector, out AdtChunkQuad quad))
                {
                    continue;
                }

                if (world.TryChunk(coord, out AdtChunkRef chunk))
                {
                    DrawImported(factory, world, chunk, ref loadsLeft, drawList, quad);
                }
                else if (factory.ShowNotImported && world.HasTerrain(coord))
                {
                    AdtChunkOverlay.NotImported(drawList, quad);
                }

                if (factory.ShowChunkGrid)
                {
                    AdtChunkOverlay.Grid(drawList, quad);
                }

                if (factory.Selection.Contains(coord))
                {
                    AdtChunkOverlay.Selected(drawList, quad);
                }
            }
        }
    }

    private void DrawImported(
        AdtTextureToolFactory factory,
        AdtTextureWorld world,
        AdtChunkRef chunk,
        ref int loadsLeft,
        ImDrawListPtr drawList,
        in AdtChunkQuad quad)
    {
        AdtChunkFacts facts = chunk.Tile.Chunks[(chunk.Col, chunk.Row)];

        if (factory.ShowLayerCount && facts.Slots.Count is > 0 and <= AdtChunkTextureSet.MaxLayers)
        {
            drawList.AddQuadFilled(quad.A, quad.B, quad.C, quad.D, ImGui.GetColorU32(LayerColors[facts.Slots.Count - 1]));
        }

        if (factory.ShowNotImported &&
            (!world.TryPixelBase(chunk.Coord, out int px, out int py) ||
             !world.Image.CanEditRegion(px, py, AdtChunkTextureSet.Edge, AdtChunkTextureSet.Edge)))
        {
            AdtChunkOverlay.Loading(drawList, quad);
        }

        if (factory.ShowUnpaintable && facts.Slots.Count == AdtChunkTextureSet.MaxLayers &&
            factory.Options.Texture.Length > 0 && !CanTake(world, chunk, facts, factory.Options.Texture, ref loadsLeft))
        {
            drawList.AddQuadFilled(quad.A, quad.B, quad.C, quad.D, ImGui.GetColorU32(new NVector4(0.9f, 0.1f, 0.1f, 0.4f)));
            drawList.AddQuad(quad.A, quad.B, quad.C, quad.D, ImGui.GetColorU32(new NVector4(1.0f, 0.2f, 0.2f, 0.95f)), 2.0f);
        }
    }

    // Whether a full chunk can take the texture. Loaded lazily and cached, since it needs the alpha data;
    // a chunk not yet judged is assumed fine until it is.
    private bool CanTake(AdtTextureWorld world, AdtChunkRef chunk, AdtChunkFacts facts, string texture, ref int loadsLeft)
    {
        int revision = world.Image.ContentRevision;
        if (_verdicts.TryGetValue(chunk.Coord, out Verdict known) &&
            known.Revision == revision && ReferenceEquals(known.Facts, facts) && known.Texture == texture)
        {
            return known.CanTake;
        }

        if (loadsLeft <= 0 || AdtChunkEdit.TryLoad(world, chunk.Coord, out AdtChunkEdit? edit) != AdtChunkLoadResult.Loaded)
        {
            return known.CanTake || known.Facts == null;
        }

        loadsLeft--;
        bool canTake = edit!.Set.CanAdd(texture);
        _verdicts[chunk.Coord] = new Verdict(revision, facts, texture, canTake);
        return canTake;
    }
}
