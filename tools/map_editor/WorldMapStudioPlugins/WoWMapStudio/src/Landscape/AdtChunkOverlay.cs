using Godot;
using ImGuiNET;
using NVector2 = System.Numerics.Vector2;
using NVector4 = System.Numerics.Vector4;

namespace WorldMapStudio.WoW;

/// <summary>A chunk's footprint as four projected screen corners, clockwise from its minimum corner.</summary>
public readonly record struct AdtChunkQuad(NVector2 A, NVector2 B, NVector2 C, NVector2 D);

/// <summary>
/// The chunk drawing the ADT tools' viewport overlays share: projecting a chunk onto the screen, and the
/// grid, not-imported, not-loaded-yet and selected looks.
/// </summary>
public static class AdtChunkOverlay
{
    /// <summary>How many chunks around the cursor an overlay draws in each direction.</summary>
    public const int ViewChunks = 7;

    public static bool TryQuad(
        AdtMapWorld world,
        ChunkCoord coord,
        BrushSurface surface,
        in ViewportProjector projector,
        out AdtChunkQuad quad)
    {
        Vector3 origin = world.Grid.OriginOf(coord);
        float size = world.Grid.ChunkSize;
        quad = default;
        if (!projector.TryProject(surface.Lift(origin), out NVector2 a) ||
            !projector.TryProject(surface.Lift(origin + new Vector3(size, 0.0f, 0.0f)), out NVector2 b) ||
            !projector.TryProject(surface.Lift(origin + new Vector3(size, 0.0f, size)), out NVector2 c) ||
            !projector.TryProject(surface.Lift(origin + new Vector3(0.0f, 0.0f, size)), out NVector2 d))
        {
            return false;
        }

        quad = new AdtChunkQuad(a, b, c, d);
        return true;
    }

    public static void Grid(ImDrawListPtr drawList, in AdtChunkQuad quad) =>
        drawList.AddQuad(quad.A, quad.B, quad.C, quad.D, ImGui.GetColorU32(new NVector4(1.0f, 1.0f, 1.0f, 0.25f)), 1.0f);

    /// <summary>A chunk with terrain that the tool will not touch.</summary>
    public static void NotImported(ImDrawListPtr drawList, in AdtChunkQuad quad) =>
        drawList.AddQuadFilled(quad.A, quad.B, quad.C, quad.D, ImGui.GetColorU32(new NVector4(0.1f, 0.1f, 0.1f, 0.45f)));

    /// <summary>An imported chunk whose image data is not loaded yet, or lies outside the image.</summary>
    public static void Loading(ImDrawListPtr drawList, in AdtChunkQuad quad) =>
        drawList.AddQuad(quad.A, quad.B, quad.C, quad.D, ImGui.GetColorU32(new NVector4(1.0f, 0.6f, 0.1f, 0.9f)), 2.0f);

    public static void Selected(ImDrawListPtr drawList, in AdtChunkQuad quad) =>
        drawList.AddQuad(quad.A, quad.B, quad.C, quad.D, ImGui.GetColorU32(new NVector4(0.3f, 1.0f, 1.0f, 1.0f)), 2.5f);
}
