using Godot;
using ImGuiNET;
using NVector2 = System.Numerics.Vector2;
using NVector4 = System.Numerics.Vector4;

namespace WorldMapStudio.WoW;

/// <summary>Which overlays the vertex tools draw.</summary>
public sealed class AdtVertexOverlayOptions
{
    public bool ShowChunkGrid { get; set; }

    /// <summary>The lattice vertices inside the brush, when the brush is small enough for them to be legible.</summary>
    public bool ShowVertices { get; set; } = true;

    /// <summary>Chunks the tool will not touch: not imported, or their data not loaded yet.</summary>
    public bool ShowNotImported { get; set; } = true;
}

/// <summary>
/// The vertex tools' viewport overlays, drawn over the chunks around the cursor: the chunk grid, chunks the tool
/// will not touch, and the vertices the brush reaches.
/// </summary>
public static class AdtVertexOverlay
{
    /// <summary>Beyond this many vertex steps across the brush radius the dots are too dense to read.</summary>
    private const float MaxDotRadiusInSteps = 9.0f;

    public static void Draw(
        AdtVertexOverlayOptions options,
        AdtMapWorld world,
        ImageComponent? placement,
        Vector3 centre,
        float brushRadius,
        BrushSurface surface,
        in ViewportProjector projector)
    {
        if (!options.ShowChunkGrid && !options.ShowNotImported && !options.ShowVertices)
        {
            return;
        }

        ChunkCoord middle = world.Grid.CoordAt(centre);
        ImDrawListPtr drawList = ImGui.GetWindowDrawList();
        for (int x = middle.X - AdtChunkOverlay.ViewChunks; x <= middle.X + AdtChunkOverlay.ViewChunks; x++)
        {
            for (int y = middle.Y - AdtChunkOverlay.ViewChunks; y <= middle.Y + AdtChunkOverlay.ViewChunks; y++)
            {
                var coord = new ChunkCoord(x, y);
                if (!AdtChunkOverlay.TryQuad(world, coord, surface, projector, out AdtChunkQuad quad))
                {
                    continue;
                }

                if (world.TryChunk(coord, out _))
                {
                    if (options.ShowNotImported && placement != null &&
                        world.BlockStateOf(placement, coord, AdtVertexLattice.Edge) != AdtBlockState.Ready)
                    {
                        AdtChunkOverlay.Loading(drawList, quad);
                    }
                }
                else if (options.ShowNotImported && world.HasTerrain(coord))
                {
                    AdtChunkOverlay.NotImported(drawList, quad);
                }

                if (options.ShowChunkGrid)
                {
                    AdtChunkOverlay.Grid(drawList, quad);
                }
            }
        }

        if (options.ShowVertices && brushRadius <= MaxDotRadiusInSteps * world.VertexStep)
        {
            DrawVertices(world, centre, brushRadius, surface, projector, drawList);
        }
    }

    /// <summary>The overlay toggles, for a tool's "Overlays" menu.</summary>
    public static void DrawMenu(string popupId, AdtVertexOverlayOptions options)
    {
        if (ImGui.Button("Overlays"))
        {
            ImGui.OpenPopup(popupId);
        }

        if (!ImGui.BeginPopup(popupId))
        {
            return;
        }

        bool grid = options.ShowChunkGrid;
        if (ImGui.Checkbox("Chunk grid", ref grid))
        {
            options.ShowChunkGrid = grid;
        }

        bool vertices = options.ShowVertices;
        if (ImGui.Checkbox("Vertices under the brush", ref vertices))
        {
            options.ShowVertices = vertices;
        }

        bool notImported = options.ShowNotImported;
        if (ImGui.Checkbox("Chunks the tool will not touch", ref notImported))
        {
            options.ShowNotImported = notImported;
        }

        ImGui.EndPopup();
    }

    private static void DrawVertices(
        AdtMapWorld world,
        Vector3 centre,
        float radius,
        BrushSurface surface,
        in ViewportProjector projector,
        ImDrawListPtr drawList)
    {
        AdtVertex nearest = world.NearestVertex(centre);
        int reach = ((int)(radius / world.VertexStep) + 1) * 2;
        uint inside = ImGui.GetColorU32(new NVector4(1.0f, 1.0f, 1.0f, 0.9f));
        uint outside = ImGui.GetColorU32(new NVector4(1.0f, 1.0f, 1.0f, 0.25f));
        uint centreInside = ImGui.GetColorU32(new NVector4(0.55f, 0.85f, 1.0f, 0.9f));
        uint centreOutside = ImGui.GetColorU32(new NVector4(0.55f, 0.85f, 1.0f, 0.25f));

        for (int dz = -reach; dz <= reach; dz++)
        {
            for (int dx = -reach; dx <= reach; dx++)
            {
                var vertex = new AdtVertex(nearest.X + dx, nearest.Z + dz);

                // Corners and cell centres are the only vertices; the other parities are between them.
                if ((vertex.X & 1) != (vertex.Z & 1) || !HoldsImported(world, vertex))
                {
                    continue;
                }

                Vector3 position = world.PositionOf(vertex);
                if (projector.TryProject(surface.Lift(position), out NVector2 screen))
                {
                    bool reached = new Vector2(position.X - centre.X, position.Z - centre.Z).Length() <= radius;
                    float size = reached ? 2.5f : 1.5f;
                    if (vertex.IsCentre)
                    {
                        drawList.AddCircleFilled(screen, size * 0.8f, reached ? centreInside : centreOutside);
                    }
                    else
                    {
                        drawList.AddCircleFilled(screen, size, reached ? inside : outside);
                    }
                }
            }
        }
    }

    private static bool HoldsImported(AdtMapWorld world, AdtVertex vertex)
    {
        foreach (ChunkCoord chunk in AdtVertexLattice.ChunksHolding(vertex))
        {
            if (world.TryChunk(chunk, out _))
            {
                return true;
            }
        }

        return false;
    }
}
