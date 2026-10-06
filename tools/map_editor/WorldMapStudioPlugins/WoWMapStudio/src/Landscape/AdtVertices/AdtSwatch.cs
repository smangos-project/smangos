using System;
using System.Collections.Generic;
using System.Numerics;

namespace WorldMapStudio.WoW;

/// <summary>
/// A colour or light as a person picks it: a tint, whose brightest channel is 1, and a brightness. The value
/// it stands for is their product, in the same terms the client uses: for colour a multiplier of the ground
/// (1 is unchanged, 0.5 half as bright, 2 twice as bright), for light the light added on top.
/// </summary>
public readonly record struct AdtSwatch(Vector3 Tint, float Brightness)
{
    public static readonly AdtSwatch NeutralColor = new(Vector3.One, 1.0f);

    public static readonly AdtSwatch NoLight = new(Vector3.One, 0.0f);

    public Vector3 Value => Tint * Brightness;

    /// <summary>Splits a value into tint and brightness. A value with no brightness keeps <paramref name="fallbackTint"/>.</summary>
    public static AdtSwatch FromValue(Vector3 value, Vector3 fallbackTint)
    {
        float brightness = MathF.Max(value.X, MathF.Max(value.Y, value.Z));
        return brightness <= 1e-4f ? new AdtSwatch(fallbackTint, 0.0f) : new AdtSwatch(value / brightness, brightness);
    }

    /// <summary>Scales a picked colour so its brightest channel is 1.</summary>
    public static Vector3 Normalize(Vector3 tint)
    {
        float brightest = MathF.Max(tint.X, MathF.Max(tint.Y, tint.Z));
        return brightest <= 1e-4f ? Vector3.One : tint / brightest;
    }
}

/// <summary>A named swatch a person can pick with one click, and that a script can pick by name.</summary>
public sealed record AdtSwatchPreset(string Name, AdtSwatch Swatch);

/// <summary>The preset swatches for vertex colour and vertex light.</summary>
public static class AdtSwatches
{
    private static readonly AdtSwatchPreset[] ColorPresets =
    [
        new("Neutral", AdtSwatch.NeutralColor),
        new("Shade 50%", new AdtSwatch(Vector3.One, 0.5f)),
        new("Shade 75%", new AdtSwatch(Vector3.One, 0.75f)),
        new("Bright 125%", new AdtSwatch(Vector3.One, 1.25f)),
        new("Warm", new AdtSwatch(new Vector3(1.0f, 0.85f, 0.65f), 1.0f)),
        new("Cool", new AdtSwatch(new Vector3(0.7f, 0.85f, 1.0f), 1.0f)),
        new("Moss", new AdtSwatch(new Vector3(0.65f, 0.85f, 0.5f), 0.9f)),
        new("Dust", new AdtSwatch(new Vector3(1.0f, 0.88f, 0.7f), 1.05f)),
    ];

    private static readonly AdtSwatchPreset[] LightPresets =
    [
        new("None", AdtSwatch.NoLight),
        new("Warm lamp", new AdtSwatch(new Vector3(1.0f, 0.7f, 0.35f), 0.6f)),
        new("Cool glow", new AdtSwatch(new Vector3(0.4f, 0.6f, 1.0f), 0.5f)),
        new("Fel green", new AdtSwatch(new Vector3(0.3f, 1.0f, 0.2f), 0.6f)),
        new("Fire", new AdtSwatch(new Vector3(1.0f, 0.4f, 0.1f), 0.8f)),
    ];

    public static IReadOnlyList<AdtSwatchPreset> Presets(AdtVertexKind kind) =>
        kind == AdtVertexKind.Light ? LightPresets : ColorPresets;

    /// <summary>The preset called <paramref name="name"/> (case does not matter), or null.</summary>
    public static AdtSwatchPreset? Find(AdtVertexKind kind, string name)
    {
        foreach (AdtSwatchPreset preset in Presets(kind))
        {
            if (string.Equals(preset.Name, name, StringComparison.OrdinalIgnoreCase))
            {
                return preset;
            }
        }

        return null;
    }
}
