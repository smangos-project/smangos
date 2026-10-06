namespace WorldMapStudio.WoW;

/// <summary>What a click does in the ADT height tool.</summary>
public enum AdtHeightMode
{
    /// <summary>Raises the ground; Ctrl lowers it.</summary>
    Raise,

    Flatten,

    Smooth,

    /// <summary>Reads the height under the cursor into the flatten target.</summary>
    Pick,
}

/// <summary>What the height tool sculpts with, on top of the shared <see cref="Brush"/>.</summary>
public sealed class AdtHeightOptions
{
    /// <summary>World units a raise stroke moves the ground per dab at full strength.</summary>
    public float Rate { get; set; } = 0.5f;

    public AdtFlattenSide Side { get; set; }

    /// <summary>The height a flatten stroke aims for. Null takes the height under the stroke's first dab.</summary>
    public float? TargetHeight { get; set; }

    /// <summary>Keeps <see cref="TargetHeight"/> across strokes. Off, a target is used by one stroke.</summary>
    public bool LockTarget { get; set; }

    public AdtHeightOptions Clone() => new()
    {
        Rate = Rate,
        Side = Side,
        TargetHeight = TargetHeight,
        LockTarget = LockTarget,
    };

    public IAdtVertexOp CreateOp(AdtHeightMode mode) => mode switch
    {
        AdtHeightMode.Flatten => new AdtFlattenOp { Target = TargetHeight, Side = Side },
        AdtHeightMode.Smooth => new AdtSmoothOp(),
        _ => new AdtRaiseOp { Rate = Rate },
    };
}

/// <summary>
/// Registers the ADT height tool and owns its settings, so they outlive a tool instance and are shared with the
/// script API: the brush, the sculpt options and the overlays.
/// </summary>
[Subsystem(nameof(ToolSystem))]
public sealed class AdtHeightToolFactory : IToolFactory
{
    private readonly EditorContext _context;

    public AdtHeightToolFactory(ToolSystem tools)
    {
        _context = tools.Context.Editor;
        World = new AdtMapWorldSource(_context, AdtImportCatalog.HeightChannelName, "This map has no imported ADT height data.");
    }

    public float Priority => 2.1f;

    public string Name => "ADT Height";

    public Brush Brush { get; } = new() { Radius = 12.0f, Strength = 0.25f };

    public AdtHeightOptions Options { get; } = new();

    public AdtHeightMode Mode { get; set; }

    public AdtVertexOverlayOptions Overlays { get; } = new();

    public AdtMapWorldSource World { get; }

    public bool CanActivate() => _context.Landscape.IsEnabled;

    public ITool Create(ToolContext context) => new AdtHeightTool(context, this);
}
