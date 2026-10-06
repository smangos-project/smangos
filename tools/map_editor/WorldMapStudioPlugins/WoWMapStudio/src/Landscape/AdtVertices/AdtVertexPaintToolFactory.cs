using System;
using System.Collections.Generic;
using System.Numerics;

namespace WorldMapStudio.WoW;

/// <summary>What a click does in the ADT vertex paint tool.</summary>
public enum AdtVertexPaintMode
{
    /// <summary>Paints the colour or light; Ctrl paints towards neutral, which erases.</summary>
    Paint,

    /// <summary>Moves hue and saturation towards the tint and keeps the brightness.</summary>
    Tint,

    /// <summary>Scales brightness and keeps the hue; Ctrl darkens.</summary>
    Brighten,

    Smooth,

    /// <summary>Reads the colour or light under the cursor into the swatch.</summary>
    Pick,
}

/// <summary>What the vertex paint tool paints with, on top of the shared <see cref="Brush"/>.</summary>
public sealed class AdtVertexPaintOptions
{
    /// <summary>Whether the tool edits vertex colour or vertex light.</summary>
    public AdtVertexKind Kind { get; set; } = AdtVertexKind.Color;

    public AdtVertexPaintMode Mode { get; set; }

    public AdtSwatch Color { get; set; } = new(Vector3.One, 0.75f);

    public AdtSwatch Light { get; set; } = new(new Vector3(1.0f, 0.7f, 0.35f), 0.6f);

    /// <summary>Relative brightness change per dab at full strength, for <see cref="AdtVertexPaintMode.Brighten"/>.</summary>
    public float Rate { get; set; } = 0.25f;

    public AdtSwatch Swatch
    {
        get => Kind == AdtVertexKind.Light ? Light : Color;
        set
        {
            if (Kind == AdtVertexKind.Light)
            {
                Light = value;
            }
            else
            {
                Color = value;
            }
        }
    }

    public AdtVertexPaintOptions Clone() => new()
    {
        Kind = Kind,
        Mode = Mode,
        Color = Color,
        Light = Light,
        Rate = Rate,
    };

    public IAdtVertexOp CreateOp() => Mode switch
    {
        AdtVertexPaintMode.Tint => new AdtTintOp(Swatch.Value),
        AdtVertexPaintMode.Brighten => new AdtBrightnessOp { Rate = Rate },
        AdtVertexPaintMode.Smooth => new AdtSmoothOp(),
        _ => new AdtPaintOp(Swatch.Value, AdtVertexCodec.For(Kind).Neutral),
    };
}

/// <summary>
/// Registers the ADT vertex paint tool and owns its settings, so they outlive a tool instance and are shared with
/// the script API: the brush, the colour and light swatches, the recently used swatches and the overlays.
/// </summary>
[Subsystem(nameof(ToolSystem))]
public sealed class AdtVertexPaintToolFactory : IToolFactory
{
    public const int MaxRecents = 8;

    private readonly EditorContext _context;
    private readonly List<AdtSwatch> _colorRecents = [];
    private readonly List<AdtSwatch> _lightRecents = [];

    public AdtVertexPaintToolFactory(ToolSystem tools)
    {
        _context = tools.Context.Editor;
        ColorWorld = new AdtMapWorldSource(_context, AdtImportCatalog.VColorChannelName, "This map has no imported ADT vertex colour data.");
        LightWorld = new AdtMapWorldSource(_context, AdtImportCatalog.VLightChannelName, "This map has no imported ADT vertex light data.");
    }

    public float Priority => 2.2f;

    public string Name => "ADT Vertex Paint";

    public Brush Brush { get; } = new() { Radius = 12.0f, Strength = 0.35f };

    public AdtVertexPaintOptions Options { get; } = new();

    public AdtVertexOverlayOptions Overlays { get; } = new();

    public AdtMapWorldSource ColorWorld { get; }

    public AdtMapWorldSource LightWorld { get; }

    public bool CanActivate() => _context.Landscape.IsEnabled;

    public ITool Create(ToolContext context) => new AdtVertexPaintTool(context, this);

    public AdtMapWorldSource WorldOf(AdtVertexKind kind) => kind == AdtVertexKind.Light ? LightWorld : ColorWorld;

    /// <summary>Recently painted swatches of a kind, newest first.</summary>
    public IReadOnlyList<AdtSwatch> Recents(AdtVertexKind kind) => kind == AdtVertexKind.Light ? _lightRecents : _colorRecents;

    /// <summary>Makes <paramref name="swatch"/> the one being painted with and moves it to the front of the recents.</summary>
    public void Use(AdtVertexKind kind, AdtSwatch swatch)
    {
        if (kind == AdtVertexKind.Light)
        {
            Options.Light = swatch;
        }
        else
        {
            Options.Color = swatch;
        }

        List<AdtSwatch> recents = kind == AdtVertexKind.Light ? _lightRecents : _colorRecents;
        recents.RemoveAll(existing => Near(existing, swatch));
        recents.Insert(0, swatch);
        if (recents.Count > MaxRecents)
        {
            recents.RemoveRange(MaxRecents, recents.Count - MaxRecents);
        }
    }

    private static bool Near(AdtSwatch a, AdtSwatch b) =>
        Vector3.DistanceSquared(a.Value, b.Value) < 1e-4f;
}
