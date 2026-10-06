using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Godot;
using ImGuiNET;
using NVector2 = System.Numerics.Vector2;

namespace WorldMapStudio.WoW;

/// <summary>
/// Paints textures onto imported ADT chunks the way a terrain texture painter does: pick a texture and
/// brush over the ground. Nothing has to be selected; the tool edits whichever imported chunks are under the
/// cursor, adding the texture to a chunk's stack when it is not there. The brush, cursor and stroke handling
/// are the shared <see cref="BrushViewport"/>; this adds the texture palette, replace and select modes, the
/// eyedropper, overlays, and a readout of what is under the cursor.
/// </summary>
public sealed class AdtTextureTool : ITool
{
    private const float ThumbnailSize = 28.0f;

    private readonly EditorContext _editor;
    private readonly AdtTextureToolFactory _factory;
    private readonly BrushSurface _surface;
    private readonly BrushViewport _viewport;
    private readonly AdtTexturePaintTarget _target;
    private readonly AdtTextureOverlay _overlay = new();
    private readonly TextureAssetPicker _picker;
    private readonly Dictionary<string, Task<Texture2D?>> _thumbnails = new(StringComparer.OrdinalIgnoreCase);

    private AdtChunkEdit? _hover;
    private AdtChunkFacts? _hoverFacts;
    private ChunkCoord _hoverCoord;
    private int _hoverRevision = -1;
    private Vector3 _lastHit;
    private bool _hasLastHit;

    public AdtTextureTool(ToolContext context, AdtTextureToolFactory factory)
    {
        _editor = context.Editor;
        _factory = factory;
        _surface = new BrushSurface(new TerrainProbe(_editor.Landscape)) { FallbackPlane = false };
        _target = new AdtTexturePaintTarget(factory.World, factory.Options);
        _surface.Accepts = _target.Covers;
        _viewport = new BrushViewport(factory.Brush, _surface, context.Sessions);
        _picker = new TextureAssetPicker(_editor.Assets);
    }

    public string Name => "ADT Textures";

    public bool CapturesMouse => _viewport.IsStroking;

    public void DrawToolbar()
    {
        foreach (AdtTextureMode mode in Enum.GetValues<AdtTextureMode>())
        {
            if (ImGui.RadioButton(mode.ToString(), _factory.Mode == mode))
            {
                _factory.Mode = mode;
            }

            ImGui.SameLine();
        }

        if (_factory.Mode is AdtTextureMode.Paint or AdtTextureMode.Replace)
        {
            BrushControls.Draw(_factory.Brush);
            ImGui.SameLine();
        }

        if (_factory.Mode == AdtTextureMode.Paint)
        {
            float target = _factory.Options.TargetWeight;
            ImGui.SetNextItemWidth(100.0f);
            if (ImGui.SliderFloat("Target", ref target, 0.0f, 1.0f))
            {
                _factory.Options.TargetWeight = target;
            }

            ImGui.SameLine();
        }

        if (_factory.Mode == AdtTextureMode.Replace)
        {
            DrawReplaceSource();
        }

        DrawTexturePalette();
        ImGui.SameLine();
        DrawOverlayMenu();
        _picker.Draw();

        if (_target.Unpaintable.Count > 0)
        {
            ImGui.SameLine();
            ImGui.TextColored(new System.Numerics.Vector4(1.0f, 0.6f, 0.2f, 1.0f), $"{_target.Unpaintable.Count} chunk(s) are full");
        }

        if (_factory.World() == null && _factory.WorldProblem is { } problem)
        {
            ImGui.SameLine();
            ImGui.TextDisabled(problem);
        }
    }

    public void UpdateViewport(in ViewportContext context)
    {
        AdtTextureMode mode = _factory.Mode;
        ImGuiIOPtr io = ImGui.GetIO();
        bool eyedrop = mode == AdtTextureMode.Pick || io.KeyAlt || (mode == AdtTextureMode.Replace && io.KeyCtrl);
        bool select = mode == AdtTextureMode.Select;
        bool brush = !select && !eyedrop;
        if (!_viewport.IsStroking)
        {
            _factory.Options.Kind = mode == AdtTextureMode.Replace ? AdtTextureStrokeKind.Replace : AdtTextureStrokeKind.Paint;
        }

        if (brush)
        {
            _viewport.Update(context, _target);
        }
        else
        {
            _viewport.Finish(record: true);
        }

        Vector3 point = _viewport.HitPoint;
        bool hit = brush ? _viewport.HasHit : !context.CameraFlying && _surface.TryHit(context, out point);

        if (hit)
        {
            _lastHit = point;
            _hasLastHit = true;
        }

        AdtTextureWorld? world = _factory.World();
        var projector = new ViewportProjector(context.Camera, context.ImageMin, context.ImageSize);
        if (world != null && _hasLastHit && !context.CameraFlying)
        {
            _overlay.Draw(_factory, world, _lastHit, _surface, projector);
        }

        _factory.HoverChunk = hit && world != null && world.TryChunkAt(point, out AdtChunkRef hovered) ? hovered.Coord : null;
        if (!hit || _viewport.IsStroking)
        {
            return;
        }

        DrawReadout(point);
        if (!context.Hovered || !ImGui.IsMouseClicked(ImGuiMouseButton.Left))
        {
            return;
        }

        if (select && world != null && world.TryChunkAt(point, out AdtChunkRef clicked))
        {
            _factory.Select(clicked.Coord, additive: io.KeyShift);
        }
        else if (eyedrop)
        {
            EyedropAt(point, replaceSource: mode == AdtTextureMode.Replace && io.KeyCtrl && !io.KeyAlt);
        }
    }

    public void Deactivate()
    {
        _viewport.Finish(record: true);
    }

    private void DrawReplaceSource()
    {
        string from = _factory.Options.ReplaceFrom;
        ImGui.TextUnformatted("Replace");
        ImGui.SameLine();
        DrawThumbnail(from, ThumbnailSize);
        ImGui.SameLine();
        ImGui.TextUnformatted(from.Length == 0 ? "(Ctrl+click a texture)" : NameOf(from));
        ImGui.SameLine();

        bool whole = _factory.Options.ReplaceWholeChunk;
        if (ImGui.Checkbox("Whole chunk", ref whole))
        {
            _factory.Options.ReplaceWholeChunk = whole;
        }

        ImGui.SameLine();
        ImGui.TextUnformatted("with");
        ImGui.SameLine();
    }

    private void DrawTexturePalette()
    {
        string current = _factory.Options.Texture;
        DrawThumbnail(current, ThumbnailSize);
        ImGui.SameLine();
        ImGui.TextUnformatted(current.Length == 0 ? "(no texture)" : NameOf(current));
        ImGui.SameLine();
        if (ImGui.Button("Browse..."))
        {
            _picker.Browse(current, selected =>
            {
                if (selected.Length > 0)
                {
                    _factory.Use(selected);
                }
            }, "tileset");
        }

        foreach (string recent in _factory.Recents)
        {
            ImGui.SameLine();
            DrawThumbnail(recent, ThumbnailSize);
            if (ImGui.IsItemClicked())
            {
                _factory.Use(recent);
            }

            if (ImGui.IsItemHovered())
            {
                ImGui.SetTooltip(NameOf(recent));
            }
        }
    }

    private void DrawOverlayMenu()
    {
        if (ImGui.Button("Overlays"))
        {
            ImGui.OpenPopup("adt_overlays");
        }

        if (!ImGui.BeginPopup("adt_overlays"))
        {
            return;
        }

        bool grid = _factory.ShowChunkGrid;
        if (ImGui.Checkbox("Chunk grid", ref grid))
        {
            _factory.ShowChunkGrid = grid;
        }

        bool layers = _factory.ShowLayerCount;
        if (ImGui.Checkbox("Layer count", ref layers))
        {
            _factory.ShowLayerCount = layers;
        }

        bool full = _factory.ShowUnpaintable;
        if (ImGui.Checkbox("Chunks that cannot take the texture", ref full))
        {
            _factory.ShowUnpaintable = full;
        }

        bool notImported = _factory.ShowNotImported;
        if (ImGui.Checkbox("Chunks the tool will not touch", ref notImported))
        {
            _factory.ShowNotImported = notImported;
        }

        ImGui.EndPopup();
    }

    private void DrawThumbnail(string texture, float size)
    {
        var box = new NVector2(size, size);
        if (texture.Length > 0 && Thumbnail(texture) is { IsCompletedSuccessfully: true, Result: { } image })
        {
            ImGui.Image((IntPtr)image.GetRid().Id, box);
        }
        else
        {
            ImGui.Dummy(box);
        }
    }

    private Task<Texture2D?> Thumbnail(string texture)
    {
        if (!_thumbnails.TryGetValue(texture, out Task<Texture2D?>? task))
        {
            task = _editor.Assets.LoadTextureAssetAsync(texture);
            _thumbnails[texture] = task;
        }

        return task;
    }

    private void EyedropAt(Vector3 world, bool replaceSource)
    {
        if (!TexelUnder(world, out AdtChunkEdit? edit, out int x, out int y) ||
            edit!.Set.DominantAt(x, y) is not (>= 0 and var slot))
        {
            return;
        }

        string texture = edit.Set.Textures[slot];
        if (replaceSource)
        {
            _factory.Options.ReplaceFrom = texture;
        }
        else
        {
            _factory.Use(texture);
        }
    }

    private void DrawReadout(Vector3 world)
    {
        if (!TexelUnder(world, out AdtChunkEdit? edit, out int x, out int y))
        {
            return;
        }

        AdtChunkRef chunk = edit!.Chunk;
        ImGui.BeginTooltip();
        ImGui.TextDisabled($"Tile {chunk.Tile.FileX},{chunk.Tile.FileY}  chunk {chunk.Col},{chunk.Row}");
        for (int slot = edit.Set.Count - 1; slot >= 0; slot--)
        {
            string texture = edit.Set.Textures[slot];
            string marker = AdtTexturePath.Same(texture, _factory.Options.Texture) ? ">" : " ";
            ImGui.Text($"{marker} {slot}  {edit.Set.WeightAt(slot, x, y) * 100.0f,3:0}%  {NameOf(texture)}");
        }

        if (_factory.World()?.OverrideAt(chunk) is { } overridden)
        {
            ImGui.TextColored(new System.Numerics.Vector4(1.0f, 0.6f, 0.2f, 1.0f), OverrideText(overridden));
        }

        ImGui.EndTooltip();
    }

    /// <summary>The line the readout and the chunk inspector show for a chunk whose build disagrees with its slots.</summary>
    public static string OverrideText(AdtChunkOverride overridden) =>
        overridden.ForeignLayers.Count > 0
            ? $"(overridden by layer {string.Join(", ", overridden.ForeignLayers)})"
            : $"(not rendered: {string.Join(", ", overridden.DroppedTextures.Select(NameOf))})";

    // The chunk under a point and the alpha texel in it. Reloaded only when the chunk, its slot facts or the
    // alpha image changed, so the readout costs nothing on a still frame.
    private bool TexelUnder(Vector3 world, out AdtChunkEdit? edit, out int x, out int y)
    {
        edit = null;
        x = y = 0;
        if (_factory.World() is not { } textureWorld || !textureWorld.TryChunkAt(world, out AdtChunkRef chunk))
        {
            return false;
        }

        AdtChunkFacts facts = chunk.Tile.Chunks[(chunk.Col, chunk.Row)];
        int revision = textureWorld.Image.ContentRevision;
        if (_hover == null || _hoverCoord != chunk.Coord || _hoverRevision != revision || !ReferenceEquals(_hoverFacts, facts))
        {
            AdtChunkEdit.TryLoad(textureWorld, chunk.Coord, out _hover);
            _hoverCoord = chunk.Coord;
            _hoverRevision = revision;
            _hoverFacts = facts;
        }

        if (_hover == null)
        {
            return false;
        }

        BrushGrid grid = textureWorld.GridOf(chunk.Coord);
        x = Math.Clamp((int)((world.X - grid.OriginX) / grid.TexelWidth), 0, AdtChunkTextureSet.Edge - 1);
        y = Math.Clamp((int)((world.Z - grid.OriginZ) / grid.TexelHeight), 0, AdtChunkTextureSet.Edge - 1);
        edit = _hover;
        return true;
    }

    private static string NameOf(string texture)
    {
        string normal = texture.Replace('\\', '/');
        return normal[(normal.LastIndexOf('/') + 1)..];
    }
}
