using System;
using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>What a click does in the ADT texture tool.</summary>
public enum AdtTextureMode
{
    Paint,

    /// <summary>Swaps one texture for another under the brush.</summary>
    Replace,

    /// <summary>Makes the texture under the cursor the current one.</summary>
    Pick,

    /// <summary>Selects chunks for the chunk texture inspector.</summary>
    Select,
}

/// <summary>
/// Registers the ADT texture tool and owns its settings, so they outlive a tool instance and are shared with
/// the chunk inspector and the script API: the brush, the texture being painted, the recently used textures,
/// the overlays and the chunk selection.
/// </summary>
[Subsystem(nameof(ToolSystem))]
public sealed class AdtTextureToolFactory : IToolFactory
{
    public const int MaxRecents = 8;

    private const double WorldRefreshSeconds = 0.5;

    private readonly EditorContext _context;
    private readonly List<string> _recents = [];
    private readonly HashSet<ChunkCoord> _selection = [];

    private AdtTextureWorld? _world;
    private double _worldTime = double.NegativeInfinity;

    public AdtTextureToolFactory(ToolSystem tools)
    {
        _context = tools.Context.Editor;
    }

    public float Priority => 2.0f;

    public string Name => "ADT Textures";

    public Brush Brush { get; } = new() { Radius = 12.0f, Strength = 0.5f };

    public AdtTexturePaintOptions Options { get; } = new();

    public AdtTextureMode Mode { get; set; }

    public bool ShowChunkGrid { get; set; }

    public bool ShowLayerCount { get; set; }

    public bool ShowUnpaintable { get; set; } = true;

    public bool ShowNotImported { get; set; } = true;

    /// <summary>Recently painted textures, newest first.</summary>
    public IReadOnlyList<string> Recents => _recents;

    /// <summary>The chunks picked with the select mode, for the chunk inspector.</summary>
    public IReadOnlySet<ChunkCoord> Selection => _selection;

    /// <summary>The imported chunk last under the cursor, or null.</summary>
    public ChunkCoord? HoverChunk { get; set; }

    public bool CanActivate() => _context.Landscape.IsEnabled;

    public ITool Create(ToolContext context) => new AdtTextureTool(context, this);

    /// <summary>What the tool edits on the current map, rebuilt at most twice a second. Null, with a reason in
    /// <see cref="WorldProblem"/>, when the map has no imported ADT data.</summary>
    public AdtTextureWorld? World()
    {
        double now = Time.GetTicksMsec() / 1000.0;
        if (now - _worldTime >= WorldRefreshSeconds)
        {
            _world = AdtTextureWorld.FromEditor(_context, _context.Maps.CurrentMap, out string? problem);
            WorldProblem = problem;
            _worldTime = now;
        }

        return _world;
    }

    public string? WorldProblem { get; private set; }

    /// <summary>Forgets the cached world, so the next <see cref="World"/> rebuilds it.</summary>
    public void InvalidateWorld() => _worldTime = double.NegativeInfinity;

    /// <summary>Makes <paramref name="texture"/> the one being painted and moves it to the front of the recents.</summary>
    public void Use(string texture)
    {
        Options.Texture = texture;
        _recents.RemoveAll(existing => AdtTexturePath.Same(existing, texture));
        _recents.Insert(0, texture);
        if (_recents.Count > MaxRecents)
        {
            _recents.RemoveRange(MaxRecents, _recents.Count - MaxRecents);
        }
    }

    /// <summary>Selects a chunk, replacing the selection or, when <paramref name="additive"/>, toggling it in.</summary>
    public void Select(ChunkCoord chunk, bool additive)
    {
        if (!additive)
        {
            _selection.Clear();
            _selection.Add(chunk);
        }
        else if (!_selection.Remove(chunk))
        {
            _selection.Add(chunk);
        }
    }

    public void SetSelection(IEnumerable<ChunkCoord> chunks)
    {
        _selection.Clear();
        _selection.UnionWith(chunks);
    }

    public void ClearSelection() => _selection.Clear();

    /// <summary>The selected chunks, or the one under the cursor when nothing is selected.</summary>
    public IReadOnlyList<ChunkCoord> Targets() =>
        _selection.Count > 0 ? _selection.ToList() : HoverChunk is { } hover ? [hover] : [];
}
