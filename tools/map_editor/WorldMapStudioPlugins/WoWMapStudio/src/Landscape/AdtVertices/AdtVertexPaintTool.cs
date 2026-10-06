using System;
using Godot;
using ImGuiNET;
using NVector3 = System.Numerics.Vector3;

namespace WorldMapStudio.WoW;

/// <summary>
/// Paints the vertex colour or vertex light of imported ADT chunks. Nothing has to be selected; the tool edits the
/// vertices under the brush. The brush, cursor and stroke handling are the shared <see cref="BrushViewport"/>; this
/// adds the modes, the swatch controls, overlays, the readout, and Alt+click to take the colour under the cursor.
/// </summary>
public sealed class AdtVertexPaintTool : ITool
{
    private readonly AdtVertexPaintToolFactory _factory;
    private readonly BrushSurface _surface;
    private readonly BrushViewport _viewport;
    private readonly AdtVertexStrokeTarget _colorTarget;
    private readonly AdtVertexStrokeTarget _lightTarget;

    private Vector3 _lastHit;
    private bool _hasLastHit;
    private string _status = "";

    public AdtVertexPaintTool(ToolContext context, AdtVertexPaintToolFactory factory)
    {
        _factory = factory;
        _colorTarget = NewTarget(AdtVertexKind.Color, "Paint vertex colour");
        _lightTarget = NewTarget(AdtVertexKind.Light, "Paint vertex light");
        _surface = new BrushSurface(new TerrainProbe(context.Editor.Landscape)) { FallbackPlane = false };
        _surface.Accepts = world => Target.Covers(world);
        _viewport = new BrushViewport(factory.Brush, _surface, context.Sessions);
        EditSessions = context.Editor.EditSessions;
    }

    public string Name => "ADT Vertex Paint";

    public bool CapturesMouse => _viewport.IsStroking;

    private EditSessionManager EditSessions { get; }

    private AdtVertexKind Kind => _factory.Options.Kind;

    private AdtVertexStrokeTarget Target => Kind == AdtVertexKind.Light ? _lightTarget : _colorTarget;

    public void DrawToolbar()
    {
        AdtVertexPaintOptions options = _factory.Options;

        ImGui.BeginDisabled(_viewport.IsStroking);
        DrawKindSwitch(options);
        ImGui.EndDisabled();
        ImGui.SameLine();

        foreach (AdtVertexPaintMode mode in Enum.GetValues<AdtVertexPaintMode>())
        {
            if (ImGui.RadioButton(mode.ToString(), options.Mode == mode))
            {
                options.Mode = mode;
            }

            ImGui.SameLine();
        }

        if (options.Mode != AdtVertexPaintMode.Pick)
        {
            BrushControls.Draw(_factory.Brush);
            ImGui.SameLine();
        }

        switch (options.Mode)
        {
            case AdtVertexPaintMode.Paint:
            case AdtVertexPaintMode.Tint:
            case AdtVertexPaintMode.Pick:
                DrawSwatch(options);
                break;
            case AdtVertexPaintMode.Brighten:
                DrawRate(options);
                break;
        }

        AdtVertexOverlay.DrawMenu("adt_vertex_paint_overlays", _factory.Overlays);
        DrawChunkActions();

        AdtMapWorld? world = _factory.WorldOf(Kind).Get();
        if (world == null && _factory.WorldOf(Kind).Problem is { } problem)
        {
            ImGui.SameLine();
            ImGui.TextDisabled(problem);
        }

        if (Target.Loading.Count > 0)
        {
            ImGui.SameLine();
            ImGui.TextColored(new System.Numerics.Vector4(1.0f, 0.6f, 0.2f, 1.0f), $"{Target.Loading.Count} chunk(s) still loading");
        }

        if (_status.Length > 0)
        {
            ImGui.SameLine();
            ImGui.TextDisabled(_status);
        }
    }

    public void UpdateViewport(in ViewportContext context)
    {
        AdtVertexKind kind = Kind;
        bool pick = _factory.Options.Mode == AdtVertexPaintMode.Pick || ImGui.GetIO().KeyAlt;
        if (!pick)
        {
            _viewport.Update(context, Target);
        }
        else
        {
            _viewport.Finish(record: true);
        }

        Vector3 point = _viewport.HitPoint;
        bool hit = !pick ? _viewport.HasHit : !context.CameraFlying && _surface.TryHit(context, out point);
        if (hit)
        {
            _lastHit = point;
            _hasLastHit = true;
        }

        AdtMapWorld? world = _factory.WorldOf(kind).Get();
        if (world != null && _hasLastHit && !context.CameraFlying)
        {
            var projector = new ViewportProjector(context.Camera, context.ImageMin, context.ImageSize);
            AdtVertexOverlay.Draw(
                _factory.Overlays, world, world.PlacementFor(AdtVertexSession.ChannelOf(kind)), _lastHit, _factory.Brush.Radius, _surface, projector);
        }

        if (!hit || _viewport.IsStroking || world == null)
        {
            return;
        }

        AdtVertex vertex = world.NearestVertex(point);
        bool read = AdtVertexProbe.TryRead(world, kind, vertex, out NVector3 value);
        DrawReadout(world, vertex, read ? value : null);

        if (pick && read && context.Hovered && ImGui.IsMouseClicked(ImGuiMouseButton.Left))
        {
            _factory.Use(kind, AdtSwatch.FromValue(value, _factory.Options.Swatch.Tint));
        }
    }

    public void Deactivate()
    {
        _viewport.Finish(record: true);
    }

    private AdtVertexStrokeTarget NewTarget(AdtVertexKind kind, string description) =>
        new(_factory.WorldOf(kind).Get, kind, () => _factory.Options.CreateOp(), description);

    private void DrawKindSwitch(AdtVertexPaintOptions options)
    {
        if (ImGui.RadioButton("Colour", options.Kind == AdtVertexKind.Color))
        {
            options.Kind = AdtVertexKind.Color;
        }

        ImGui.SameLine();
        if (ImGui.RadioButton("Light", options.Kind == AdtVertexKind.Light))
        {
            options.Kind = AdtVertexKind.Light;
        }
    }

    private void DrawSwatch(AdtVertexPaintOptions options)
    {
        AdtSwatch swatch = options.Swatch;
        if (AdtColorPalette.Draw(options.Kind, ref swatch, _factory.Recents(options.Kind)))
        {
            _factory.Use(options.Kind, swatch);
        }

        ImGui.SameLine();
    }

    private static void DrawRate(AdtVertexPaintOptions options)
    {
        float percent = options.Rate * 100.0f;
        ImGui.SetNextItemWidth(90.0f);
        if (ImGui.DragFloat("Rate", ref percent, 1.0f, 1.0f, 200.0f, "%.0f%%"))
        {
            options.Rate = percent / 100.0f;
        }

        ImGui.SameLine();
    }

    // Whole-chunk actions on the chunk the cursor last rested on, for when a brush is the wrong instrument.
    private void DrawChunkActions()
    {
        ImGui.SameLine();
        if (ImGui.Button("Chunk"))
        {
            ImGui.OpenPopup("adt_vertex_paint_chunk");
        }

        if (!ImGui.BeginPopup("adt_vertex_paint_chunk"))
        {
            return;
        }

        AdtVertexKind kind = Kind;
        string noun = kind == AdtVertexKind.Light ? "light" : "colour";
        AdtMapWorld? world = _factory.WorldOf(kind).Get();
        ChunkCoord? chunk = world != null && _hasLastHit && world.TryChunkAt(_lastHit, out AdtChunkRef holder)
            ? holder.Coord
            : null;

        ImGui.TextDisabled(chunk == null ? "Hover a chunk first." : $"Chunk under the cursor: {noun}");
        ImGui.BeginDisabled(chunk == null);
        if (ImGui.Button($"Fill with swatch"))
        {
            Report(new AdtVertexEdit(world!, EditSessions).Fill(kind, [chunk!.Value], _factory.Options.Swatch.Value));
        }

        if (ImGui.Button($"Reset to neutral"))
        {
            Report(new AdtVertexEdit(world!, EditSessions).Reset(kind, [chunk!.Value]));
        }

        if (ImGui.Button($"Remove vertex {noun}"))
        {
            Report(new AdtVertexEdit(world!, EditSessions).Remove(kind, [chunk!.Value]));
        }

        ImGui.EndDisabled();
        ImGui.EndPopup();
    }

    private void Report(AdtVertexEditResult result)
    {
        _status = result.Changed == 0
            ? "Nothing to change."
            : $"Changed {result.Changed} chunk(s).{(result.Skipped > 0 ? $" {result.Skipped} could not be edited." : "")}";
    }

    private void DrawReadout(AdtMapWorld world, AdtVertex vertex, NVector3? value)
    {
        ImGui.BeginTooltip();
        AdtChunkRef? owner = null;
        foreach (ChunkCoord holder in AdtVertexLattice.ChunksHolding(vertex))
        {
            if (world.TryChunk(holder, out AdtChunkRef chunk))
            {
                owner = chunk;
                break;
            }
        }

        if (owner is { } imported)
        {
            (int localX, int localZ) = vertex.IndexIn(imported.Coord);
            ImGui.TextDisabled($"Tile {imported.Tile.FileX},{imported.Tile.FileY}  chunk {imported.Col},{imported.Row}  {(vertex.IsCentre ? "cell" : "vertex")} {localX},{localZ}");
        }
        else
        {
            ImGui.TextDisabled("Not imported");
        }

        if (value is { } found)
        {
            ImGui.Text(Kind == AdtVertexKind.Light
                ? $"Light +{found.X:0.00} +{found.Y:0.00} +{found.Z:0.00}"
                : $"Colour x{found.X:0.00} x{found.Y:0.00} x{found.Z:0.00}");
        }
        else
        {
            ImGui.Text("Not loaded");
        }

        ImGui.EndTooltip();
    }
}
