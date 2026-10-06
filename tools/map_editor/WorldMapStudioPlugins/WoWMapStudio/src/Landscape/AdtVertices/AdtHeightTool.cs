using System;
using Godot;
using ImGuiNET;
using NVector3 = System.Numerics.Vector3;

namespace WorldMapStudio.WoW;

/// <summary>
/// Sculpts the ground of imported ADT chunks: raise and lower, flatten, smooth. Nothing has to be selected; the
/// tool edits the vertices under the brush. The brush, cursor and stroke handling are the shared
/// <see cref="BrushViewport"/>; this adds the modes, overlays, the height readout, and Alt+click to take the height
/// under the cursor as the flatten target.
/// </summary>
public sealed class AdtHeightTool : ITool
{
    private readonly AdtHeightToolFactory _factory;
    private readonly BrushSurface _surface;
    private readonly BrushViewport _viewport;
    private readonly AdtVertexStrokeTarget _target;

    private Vector3 _lastHit;
    private bool _hasLastHit;
    private bool _wasStroking;

    public AdtHeightTool(ToolContext context, AdtHeightToolFactory factory)
    {
        _factory = factory;
        _surface = new BrushSurface(new TerrainProbe(context.Editor.Landscape)) { FallbackPlane = false };
        _target = new AdtVertexStrokeTarget(
            factory.World.Get, AdtVertexKind.Height, () => factory.Options.CreateOp(factory.Mode), "Sculpt terrain");
        _surface.Accepts = _target.Covers;
        _viewport = new BrushViewport(factory.Brush, _surface, context.Sessions);
    }

    public string Name => "ADT Height";

    public bool CapturesMouse => _viewport.IsStroking;

    public void DrawToolbar()
    {
        foreach (AdtHeightMode mode in Enum.GetValues<AdtHeightMode>())
        {
            if (ImGui.RadioButton(mode.ToString(), _factory.Mode == mode))
            {
                _factory.Mode = mode;
            }

            ImGui.SameLine();
        }

        if (_factory.Mode != AdtHeightMode.Pick)
        {
            BrushControls.Draw(_factory.Brush);
            ImGui.SameLine();
        }

        switch (_factory.Mode)
        {
            case AdtHeightMode.Raise:
                DrawRate();
                break;
            case AdtHeightMode.Flatten:
            case AdtHeightMode.Pick:
                DrawFlattenTarget();
                break;
        }

        AdtVertexOverlay.DrawMenu("adt_height_overlays", _factory.Overlays);

        AdtMapWorld? world = _factory.World.Get();
        if (world == null && _factory.World.Problem is { } problem)
        {
            ImGui.SameLine();
            ImGui.TextDisabled(problem);
        }

        if (_target.Loading.Count > 0)
        {
            ImGui.SameLine();
            ImGui.TextColored(new System.Numerics.Vector4(1.0f, 0.6f, 0.2f, 1.0f), $"{_target.Loading.Count} chunk(s) still loading");
        }
    }

    public void UpdateViewport(in ViewportContext context)
    {
        AdtHeightMode mode = _factory.Mode;
        bool pick = mode == AdtHeightMode.Pick || ImGui.GetIO().KeyAlt;
        if (!pick)
        {
            _viewport.Update(context, _target);
            FollowFlattenTarget();
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

        AdtMapWorld? world = _factory.World.Get();
        if (world != null && _hasLastHit && !context.CameraFlying)
        {
            var projector = new ViewportProjector(context.Camera, context.ImageMin, context.ImageSize);
            AdtVertexOverlay.Draw(
                _factory.Overlays, world, world.PlacementFor(AdtImportCatalog.HeightChannelName), _lastHit, _factory.Brush.Radius, _surface, projector);
        }

        if (!hit || _viewport.IsStroking || world == null)
        {
            return;
        }

        AdtVertex vertex = world.NearestVertex(point);
        bool read = AdtVertexProbe.TryRead(world, AdtVertexKind.Height, vertex, out NVector3 height);
        DrawReadout(world, vertex, read ? height.X : null);

        if (pick && read && context.Hovered && ImGui.IsMouseClicked(ImGuiMouseButton.Left))
        {
            _factory.Options.TargetHeight = height.X;
        }
    }

    public void Deactivate()
    {
        _viewport.Finish(record: true);
    }

    private void DrawRate()
    {
        float rate = _factory.Options.Rate;
        ImGui.SetNextItemWidth(90.0f);
        if (ImGui.DragFloat("Rate", ref rate, 0.05f, 0.01f, 50.0f))
        {
            _factory.Options.Rate = rate;
        }

        ImGui.SameLine();
    }

    private void DrawFlattenTarget()
    {
        float target = _factory.Options.TargetHeight ?? 0.0f;
        ImGui.SetNextItemWidth(100.0f);
        if (ImGui.DragFloat("Target", ref target, 0.1f))
        {
            _factory.Options.TargetHeight = target;
        }

        if (ImGui.IsItemHovered())
        {
            ImGui.SetTooltip("The height flatten aims for. Alt+click the ground to take its height.");
        }

        ImGui.SameLine();
        bool locked = _factory.Options.LockTarget;
        if (ImGui.Checkbox("Lock", ref locked))
        {
            _factory.Options.LockTarget = locked;
            if (!locked)
            {
                _factory.Options.TargetHeight = null;
            }
        }

        if (ImGui.IsItemHovered())
        {
            ImGui.SetTooltip("Keeps the target between strokes. Off, a target is used by one stroke; with none set, flatten takes the height under its first dab.");
        }

        ImGui.SameLine();
        int side = (int)_factory.Options.Side;
        ImGui.SetNextItemWidth(110.0f);
        if (ImGui.Combo("Move", ref side, "Both\0Raise only\0Lower only\0"))
        {
            _factory.Options.Side = (AdtFlattenSide)side;
        }

        ImGui.SameLine();
    }

    // A locked flatten with no target keeps the height its first stroke started from; an unlocked target is
    // used by one stroke and then dropped.
    private void FollowFlattenTarget()
    {
        AdtHeightOptions options = _factory.Options;
        if (options is { LockTarget: true, TargetHeight: null } && _target.Op is AdtFlattenOp { Resolved: true } flatten)
        {
            options.TargetHeight = flatten.ResolvedTarget;
        }

        if (_wasStroking && !_viewport.IsStroking && !options.LockTarget)
        {
            options.TargetHeight = null;
        }

        _wasStroking = _viewport.IsStroking;
    }

    private void DrawReadout(AdtMapWorld world, AdtVertex vertex, float? height)
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

        ImGui.Text(height is { } value ? $"Height {value:0.00}" : "Height not loaded");
        if (_factory.Options.TargetHeight is { } target)
        {
            ImGui.Text($"Flatten target {target:0.00}");
        }

        ImGui.EndTooltip();
    }
}
