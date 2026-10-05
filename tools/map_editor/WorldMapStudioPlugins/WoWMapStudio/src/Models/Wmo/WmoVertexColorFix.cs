using System;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Port of the client's <c>CMapObjGroup::FixColorVertexAlpha</c>: raw MOCV is not renderable as-is.
/// This rebases a group's vertex colours in place so RGB becomes a precomputed additive-light term
/// (root ambient removed, halved, clamped) and A becomes a clean interior/exterior selector
/// (1 = exterior, 0 = interior) for every non-transition vertex. Skipping it leaves exterior groups'
/// near-black raw MOCV RGB blacking out the surface and their alpha never marking them exterior.
/// </summary>
internal static class WmoVertexColorFix
{
    // Byte-space math is `(r * a) / 64 + r - base`, then `/ 2`, then clamp 0..255. In 0..1 space that
    // `/ 64` factor becomes 255/64 (see the derivation: r_byte*a_byte/64/510 == r*a*(255/64)/2).
    private const float ExteriorAlphaGain = 255f / 64f;

    public static void Apply(
        Color[] colors,
        WmoBatch[] batches,
        ushort transBatchCount,
        bool exterior,
        bool exteriorLit,
        in WmoRootLighting lighting)
    {
        if (colors.Length == 0)
        {
            return;
        }

        int secondFixupStart = 0;
        if (transBatchCount > 0 && transBatchCount <= batches.Length)
        {
            secondFixupStart = Math.Min(colors.Length, batches[transBatchCount - 1].VertexEnd + 1);
        }

        float exteriorAlpha = exterior || exteriorLit ? 1f : 0f;

        // Interiors-lit WMOs (e.g. Stormwind) keep raw MOCV RGB and only get the alpha selector.
        if (lighting.LightenInteriors)
        {
            for (int i = secondFixupStart; i < colors.Length; i++)
            {
                colors[i].A = exteriorAlpha;
            }

            return;
        }

        Color baseAmbient = lighting.SkipBaseColor ? new Color(0f, 0f, 0f) : lighting.AmbientColor;

        // Transition vertices: subtract base ambient, then fade RGB out by alpha; alpha itself is kept.
        for (int i = 0; i < secondFixupStart; i++)
        {
            float a = colors[i].A;
            float r = Math.Max(0f, colors[i].R - baseAmbient.R);
            float g = Math.Max(0f, colors[i].G - baseAmbient.G);
            float b = Math.Max(0f, colors[i].B - baseAmbient.B);

            colors[i].R = Math.Max(0f, (r - a * r) * 0.5f);
            colors[i].G = Math.Max(0f, (g - a * g) * 0.5f);
            colors[i].B = Math.Max(0f, (b - a * b) * 0.5f);
        }

        // Interior/exterior vertices: gain RGB by its own alpha, subtract base ambient, halve, clamp;
        // alpha is replaced by the interior/exterior selector.
        for (int i = secondFixupStart; i < colors.Length; i++)
        {
            float a = colors[i].A;
            float r = colors[i].R;
            float g = colors[i].G;
            float b = colors[i].B;

            colors[i].R = Math.Clamp((r * a * ExteriorAlphaGain + r - baseAmbient.R) * 0.5f, 0f, 1f);
            colors[i].G = Math.Clamp((g * a * ExteriorAlphaGain + g - baseAmbient.G) * 0.5f, 0f, 1f);
            colors[i].B = Math.Clamp((b * a * ExteriorAlphaGain + b - baseAmbient.B) * 0.5f, 0f, 1f);
            colors[i].A = exteriorAlpha;
        }
    }
}
