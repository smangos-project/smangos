using System;

namespace WorldMapStudio.WoW;

/// <summary>
/// Converts one texel between the stored lerp-chain alphas and per-layer weights that sum to 1. With
/// alphas <c>a1..a3</c> the client blends <c>lerp(lerp(lerp(t0, t1, a1), t2, a2), t3, a3)</c>, so
/// <c>w3 = a3</c>, <c>w2 = a2 (1 - a3)</c>, <c>w1 = a1 (1 - a2)(1 - a3)</c>, <c>w0 = (1 - a1)(1 - a2)(1 - a3)</c>.
/// Weights are the space edits happen in; alphas are what gets stored.
/// </summary>
public static class AdtBlendSpace
{
    /// <summary>Layers per chunk, including the base.</summary>
    public const int Layers = AdtTileComponent.SlotCount;

    private const float Epsilon = 1e-6f;

    /// <summary>Alphas (0-255 each, slots 1-3) to <paramref name="weights"/> (length <see cref="Layers"/>).</summary>
    public static void ToWeights(byte alpha1, byte alpha2, byte alpha3, Span<float> weights)
    {
        float a1 = alpha1 / 255.0f;
        float a2 = alpha2 / 255.0f;
        float a3 = alpha3 / 255.0f;
        weights[3] = a3;
        weights[2] = a2 * (1.0f - a3);
        weights[1] = a1 * (1.0f - a2) * (1.0f - a3);
        weights[0] = (1.0f - a1) * (1.0f - a2) * (1.0f - a3);
    }

    /// <summary>Weights (length <see cref="Layers"/>, summing to 1) to alphas. A layer with nothing
    /// visible beneath it has nothing to recover, so its alpha is written as 0.</summary>
    public static void ToAlphas(ReadOnlySpan<float> weights, out byte alpha1, out byte alpha2, out byte alpha3)
    {
        float w3 = Math.Clamp(weights[3], 0.0f, 1.0f);
        float w2 = Math.Clamp(weights[2], 0.0f, 1.0f);
        float w1 = Math.Clamp(weights[1], 0.0f, 1.0f);

        float below3 = 1.0f - w3;
        float a2 = below3 > Epsilon ? w2 / below3 : 0.0f;
        float below2 = below3 - w2;
        float a1 = below2 > Epsilon ? w1 / below2 : 0.0f;

        alpha3 = ToByte(w3);
        alpha2 = ToByte(a2);
        alpha1 = ToByte(a1);
    }

    private static byte ToByte(float alpha) => (byte)Math.Clamp((int)MathF.Round(alpha * 255.0f), 0, 255);
}
