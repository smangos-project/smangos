using System;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using Godot;
using NVector3 = System.Numerics.Vector3;

namespace WorldMapStudio.WoW;

/// <summary>The vertex colour and light at the vertex nearest a point, as scripts see them. <c>Color</c> is the
/// multiplier of the ground texture (1 is unchanged) and <c>Light</c> the light added on top; both are
/// <c>[r, g, b]</c>. A value that is not loaded reads as neutral and has <c>Loaded</c> false.</summary>
public sealed class AdtVertexColorDescriptor
{
    public AdtVertexColorDescriptor(
        AdtChunkRef chunk, int vx, int vz, bool centre, bool colorLoaded, NVector3 color, bool lightLoaded, NVector3 light)
    {
        Centre = centre;
        TileX = chunk.Tile.FileX;
        TileY = chunk.Tile.FileY;
        Col = chunk.Col;
        Row = chunk.Row;
        Vx = vx;
        Vz = vz;
        AdtChunkFacts facts = chunk.Tile.Chunks[(chunk.Col, chunk.Row)];
        HasColor = facts.HasVertexColor;
        HasLight = facts.HasVertexLight;
        ColorLoaded = colorLoaded;
        LightLoaded = lightLoaded;
        Color = Rgb(color);
        Light = Rgb(light);
    }

    [ScriptProperty] public int TileX { get; }
    [ScriptProperty] public int TileY { get; }
    [ScriptProperty] public int Col { get; }
    [ScriptProperty] public int Row { get; }
    [ScriptProperty] public int Vx { get; }
    [ScriptProperty] public int Vz { get; }

    /// <summary>Whether this is the vertex at the centre of a cell rather than a corner, in which case
    /// <c>Vx</c>, <c>Vz</c> are the cell's index, 0 to 7.</summary>
    [ScriptProperty] public bool Centre { get; }

    /// <summary>Whether the imported chunk claims vertex colour.</summary>
    [ScriptProperty] public bool HasColor { get; }

    /// <summary>Whether the imported chunk claims vertex light.</summary>
    [ScriptProperty] public bool HasLight { get; }

    [ScriptProperty] public bool ColorLoaded { get; }
    [ScriptProperty] public bool LightLoaded { get; }
    [ScriptProperty] public double[] Color { get; }
    [ScriptProperty] public double[] Light { get; }

    internal static double[] Rgb(NVector3 value) => [value.X, value.Y, value.Z];
}

/// <summary>A named swatch: the value it paints, and the tint and brightness it is made of.</summary>
public sealed class AdtSwatchDescriptor
{
    public AdtSwatchDescriptor(string name, AdtSwatch swatch)
    {
        Name = name;
        Color = AdtVertexColorDescriptor.Rgb(swatch.Value);
        Tint = AdtVertexColorDescriptor.Rgb(swatch.Tint);
        Brightness = swatch.Brightness;
    }

    [ScriptProperty] public string Name { get; }
    [ScriptProperty] public double[] Color { get; }
    [ScriptProperty] public double[] Tint { get; }
    [ScriptProperty] public double Brightness { get; }
}

/// <summary>The vertex paint tool's settings as scripts see them.</summary>
public sealed class AdtVertexPaintBrushDescriptor : BrushDescriptor
{
    public AdtVertexPaintBrushDescriptor(Brush brush, AdtVertexPaintOptions options)
        : base(brush)
    {
        Target = options.Kind == AdtVertexKind.Light ? "light" : "color";
        Rate = options.Rate;
        Color = AdtVertexColorDescriptor.Rgb(options.Color.Value);
        Light = AdtVertexColorDescriptor.Rgb(options.Light.Value);
    }

    /// <summary>What the tool edits: <c>color</c> or <c>light</c>.</summary>
    [ScriptProperty] public string Target { get; }

    [ScriptProperty] public double Rate { get; }
    [ScriptProperty] public double[] Color { get; }
    [ScriptProperty] public double[] Light { get; }
}

/// <summary>
/// The ADT vertex paint tool as <c>wms.adtVertexColors</c>: everything the tool does, on imported chunks. Brush
/// options are the shared vocabulary plus <c>Target</c> (<c>"color"</c> or <c>"light"</c>, colour when omitted),
/// <c>Color</c> (<c>[r, g, b]</c> or <c>"#rrggbb"</c>), <c>Brightness</c>, <c>Preset</c> (a name from
/// <c>Presets</c>) and <c>Rate</c> for brightening. Without <c>Brightness</c>, <c>Color</c> is the value painted:
/// for colour a multiplier of the ground (1 is unchanged), for light the light added. With it, <c>Color</c> only
/// gives the hue and <c>Brightness</c> the level. Each edit is one undo step.
/// </summary>
[Subsystem(nameof(ScriptingSystem))]
public sealed class AdtVertexPaintScriptApi : IScriptModule
{
    private readonly EditorContext _context;

    public AdtVertexPaintScriptApi(ScriptingSystem system)
    {
        _context = system.Context;
    }

    public string Name => "adtVertexColors";

    /// <summary>Supplies the world in place of the current map's. Tests only.</summary>
    internal Func<AdtMapWorld?>? WorldSource { get; set; }

    private AdtVertexPaintToolFactory Factory => _context.Tools.Factories.OfType<AdtVertexPaintToolFactory>().Single();

    /// <summary>The tool's brush and paint settings.</summary>
    [ScriptProperty]
    public AdtVertexPaintBrushDescriptor CurrentBrush => new(Factory.Brush, Factory.Options);

    /// <summary>Changes the tool's settings: any brush option, <c>Target</c>, <c>Color</c>, <c>Brightness</c>,
    /// <c>Preset</c>, <c>Rate</c>. The swatch set applies to the target the call names, or else the tool's.</summary>
    [ScriptFunction]
    public AdtVertexPaintBrushDescriptor SetBrush(object? options)
    {
        AdtVertexPaintOptions tool = Factory.Options;
        Settings settings = Read(Factory.Brush, options);
        AdtVertexKind kind = settings.Kind ?? tool.Kind;
        tool.Kind = kind;
        if (settings.Rate is { } rate)
        {
            tool.Rate = rate;
        }

        if (settings.HasSwatch)
        {
            Factory.Use(kind, settings.SwatchFor(kind, tool));
        }

        return CurrentBrush;
    }

    /// <summary>The colour and light at the vertex nearest a world point, corner or cell centre.</summary>
    [ScriptFunction]
    public AdtVertexColorDescriptor At(double x, double z)
    {
        AdtMapWorld world = RequireWorld(null);
        AdtVertex vertex = world.NearestVertex(new Vector3((float)x, 0.0f, (float)z));
        AdtChunkRef holder = AdtVertexProbe.ImportedHolder(world, vertex);
        bool colorLoaded = AdtVertexProbe.TryRead(world, AdtVertexKind.Color, vertex, out NVector3 color);
        bool lightLoaded = AdtVertexProbe.TryRead(world, AdtVertexKind.Light, vertex, out NVector3 light);
        (int vx, int vz) = vertex.IndexIn(holder.Coord);
        return new AdtVertexColorDescriptor(
            holder,
            vx,
            vz,
            vertex.IsCentre,
            colorLoaded,
            colorLoaded ? color : AdtVertexCodec.Color.Neutral,
            lightLoaded,
            lightLoaded ? light : AdtVertexCodec.Light.Neutral);
    }

    /// <summary>The preset swatches of a target: <c>"color"</c> or <c>"light"</c>.</summary>
    [ScriptFunction]
    public AdtSwatchDescriptor[] Presets(string target) =>
        AdtSwatches.Presets(ParseTarget(target)).Select(preset => new AdtSwatchDescriptor(preset.Name, preset.Swatch)).ToArray();

    /// <summary>Paints along <paramref name="points"/> as one stroke; with <c>Invert</c> it paints towards
    /// neutral, which erases.</summary>
    [ScriptFunction]
    public AdtVertexStrokeResultDescriptor Paint(object? points, object? options = null) =>
        Stroke(points, options, invert: false, call => new AdtPaintOp(call.Swatch.Value, AdtVertexCodec.For(call.Kind).Neutral));

    /// <summary>Moves the hue and saturation along the stroke towards the swatch and keeps the brightness.</summary>
    [ScriptFunction]
    public AdtVertexStrokeResultDescriptor Tint(object? points, object? options = null) =>
        Stroke(points, options, invert: false, call => new AdtTintOp(call.Swatch.Value));

    /// <summary>Brightens along <paramref name="points"/> by <c>Rate</c> per dab at full strength.</summary>
    [ScriptFunction]
    public AdtVertexStrokeResultDescriptor Brighten(object? points, object? options = null) =>
        Stroke(points, options, invert: false, call => new AdtBrightnessOp { Rate = call.Rate });

    /// <summary>Darkens along <paramref name="points"/> by <c>Rate</c> per dab at full strength.</summary>
    [ScriptFunction]
    public AdtVertexStrokeResultDescriptor Darken(object? points, object? options = null) =>
        Stroke(points, options, invert: true, call => new AdtBrightnessOp { Rate = call.Rate });

    /// <summary>Smooths along <paramref name="points"/>.</summary>
    [ScriptFunction]
    public AdtVertexStrokeResultDescriptor Smooth(object? points, object? options = null) =>
        Stroke(points, options, invert: false, _ => new AdtSmoothOp());

    /// <summary>Sets every vertex of the chunks to the swatch the options name.</summary>
    [ScriptFunction]
    public AdtVertexEditResultDescriptor Fill(object? chunks, object? options = null)
    {
        Call call = Resolve(options);
        return new(new AdtVertexEdit(RequireWorld(call.Kind), _context.EditSessions)
            .Fill(call.Kind, AdtScriptArguments.Chunks(chunks), call.Swatch.Value));
    }

    /// <summary>Puts colour or light back to neutral in the chunks, leaving them using it.</summary>
    [ScriptFunction]
    public AdtVertexEditResultDescriptor Reset(object? chunks, string target)
    {
        AdtVertexKind kind = ParseTarget(target);
        return new(new AdtVertexEdit(RequireWorld(kind), _context.EditSessions).Reset(kind, AdtScriptArguments.Chunks(chunks)));
    }

    /// <summary>Takes colour or light off the chunks: their data is cleared and they stop claiming it.</summary>
    [ScriptFunction]
    public AdtVertexEditResultDescriptor Remove(object? chunks, string target)
    {
        AdtVertexKind kind = ParseTarget(target);
        return new(new AdtVertexEdit(RequireWorld(kind), _context.EditSessions).Remove(kind, AdtScriptArguments.Chunks(chunks)));
    }

    private sealed record Call(AdtVertexKind Kind, Brush Brush, AdtSwatch Swatch, float Rate);

    // What one call's options ask for, before they are combined with the tool's own settings.
    private sealed class Settings
    {
        public AdtVertexKind? Kind { get; set; }
        public NVector3? Color { get; set; }
        public float? Brightness { get; set; }
        public string? Preset { get; set; }
        public float? Rate { get; set; }

        public bool HasSwatch => Color != null || Brightness != null || Preset != null;

        public AdtSwatch SwatchFor(AdtVertexKind kind, AdtVertexPaintOptions tool)
        {
            AdtSwatch swatch = kind == AdtVertexKind.Light ? tool.Light : tool.Color;
            if (Preset != null)
            {
                swatch = AdtSwatches.Find(kind, Preset)?.Swatch
                    ?? throw new ArgumentException($"There is no {(kind == AdtVertexKind.Light ? "light" : "colour")} preset called '{Preset}'.");
            }

            if (Color is { } color)
            {
                swatch = Brightness is { } level
                    ? new AdtSwatch(AdtSwatch.Normalize(color), level)
                    : AdtSwatch.FromValue(color, swatch.Tint);
            }
            else if (Brightness is { } only)
            {
                swatch = swatch with { Brightness = only };
            }

            return swatch;
        }
    }

    private Call Resolve(object? options)
    {
        Brush brush = Factory.Brush.Clone();
        Settings settings = Read(brush, options);
        AdtVertexKind kind = settings.Kind ?? AdtVertexKind.Color;
        return new Call(kind, brush, settings.SwatchFor(kind, Factory.Options), settings.Rate ?? Factory.Options.Rate);
    }

    private AdtVertexStrokeResultDescriptor Stroke(object? points, object? options, bool invert, Func<Call, IAdtVertexOp> op)
    {
        Call call = Resolve(options);
        AdtMapWorld world = RequireWorld(call.Kind);
        if (invert)
        {
            call.Brush.Invert = true;
        }

        string description = call.Kind == AdtVertexKind.Light ? "Paint vertex light" : "Paint vertex colour";
        return new AdtVertexStrokeResultDescriptor(AdtVertexStroke.Run(
            world, _context.EditSessions, call.Kind, op(call), call.Brush, AdtScriptArguments.Points(points), description));
    }

    private static Settings Read(Brush brush, object? options)
    {
        var settings = new Settings();
        BrushScriptOptions.Apply(brush, options, (key, value) =>
        {
            switch (key.ToLowerInvariant())
            {
                case "target":
                    settings.Kind = ParseTarget(Convert.ToString(value, CultureInfo.InvariantCulture));
                    return true;
                case "color":
                case "colour":
                    settings.Color = ParseColor(value);
                    return true;
                case "brightness":
                    settings.Brightness = (float)BrushScriptOptions.ToDouble(value);
                    return true;
                case "preset":
                    settings.Preset = Convert.ToString(value, CultureInfo.InvariantCulture);
                    return true;
                case "rate":
                    settings.Rate = (float)BrushScriptOptions.ToDouble(value);
                    return true;
                case "points":
                    return true;
                default:
                    return false;
            }
        });

        return settings;
    }

    private static AdtVertexKind ParseTarget(string? name) => name?.Trim().ToLowerInvariant() switch
    {
        "color" or "colour" => AdtVertexKind.Color,
        "light" => AdtVertexKind.Light,
        _ => throw new ArgumentException("Target must be \"color\" or \"light\"."),
    };

    private static NVector3 ParseColor(object? value)
    {
        if (value is string text)
        {
            string hex = text.TrimStart('#');
            if (hex.Length == 3)
            {
                hex = string.Concat(hex.Select(digit => new string(digit, 2)));
            }

            if (hex.Length == 6 && int.TryParse(hex, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out int rgb))
            {
                return new NVector3(((rgb >> 16) & 0xFF) / 255.0f, ((rgb >> 8) & 0xFF) / 255.0f, (rgb & 0xFF) / 255.0f);
            }
        }
        else if (value is IEnumerable && AdtScriptArguments.Numbers(value) is { Length: 3 } channels)
        {
            return new NVector3((float)channels[0], (float)channels[1], (float)channels[2]);
        }

        throw new ArgumentException("Color must be [r, g, b] or \"#rrggbb\".");
    }

    // The map's world, when it has the raster a kind of edit works on, or either colour raster when the kind is null.
    private AdtMapWorld RequireWorld(AdtVertexKind? kind)
    {
        AdtMapWorld? world = WorldSource != null
            ? WorldSource()
            : AdtMapWorld.FromEditor(_context, _context.Maps.CurrentMap, out _);
        bool has(AdtVertexKind of) => world?.PlacementFor(AdtVertexSession.ChannelOf(of)) != null;
        return world is { } found && (kind is { } required ? has(required) : has(AdtVertexKind.Color) || has(AdtVertexKind.Light))
            ? found
            : throw new InvalidOperationException(kind == AdtVertexKind.Light
                ? "The current map has no imported ADT vertex light data."
                : "The current map has no imported ADT vertex colour data.");
    }
}
