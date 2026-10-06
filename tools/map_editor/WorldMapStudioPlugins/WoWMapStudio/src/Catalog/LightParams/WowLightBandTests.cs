using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Covers <see cref="WowLightBand{T}"/> sampling against hand-computed values.</summary>
public static class WowLightBandTests
{
    [EditorTest(Category = "WoW Lighting")]
    public static void Band_interpolates_between_two_keys()
    {
        var band = new WowLightBand<float>();
        band.Add(0, 0.0f);
        band.Add(1440, 1.0f);

        Assert.AreApproximatelyEqual(0.5, band.Sample(720, -1.0f, Mathf.Lerp), tolerance: 1e-5);
    }

    [EditorTest(Category = "WoW Lighting")]
    public static void Band_sampled_exactly_on_a_key_returns_that_keys_value()
    {
        var band = new WowLightBand<float>();
        band.Add(0, 0.0f);
        band.Add(1440, 1.0f);

        Assert.AreApproximatelyEqual(1.0, band.Sample(1440, -1.0f, Mathf.Lerp), tolerance: 1e-5);
        Assert.AreApproximatelyEqual(0.0, band.Sample(0, -1.0f, Mathf.Lerp), tolerance: 1e-5);
    }

    [EditorTest(Category = "WoW Lighting")]
    public static void Band_with_a_single_key_is_constant_at_any_time()
    {
        var band = new WowLightBand<float>();
        band.Add(500, 0.25f);

        Assert.AreApproximatelyEqual(0.25, band.Sample(0, -1.0f, Mathf.Lerp), tolerance: 1e-5);
        Assert.AreApproximatelyEqual(0.25, band.Sample(500, -1.0f, Mathf.Lerp), tolerance: 1e-5);
        Assert.AreApproximatelyEqual(0.25, band.Sample(2000, -1.0f, Mathf.Lerp), tolerance: 1e-5);
    }

    [EditorTest(Category = "WoW Lighting")]
    public static void Band_wraps_around_before_the_first_key()
    {
        var band = new WowLightBand<float>();
        band.Add(1000, 0.2f);
        band.Add(2000, 0.8f);

        // time=100 falls before the first key, so it interpolates from key[2000] (from "yesterday")
        // forward through key[1000]+DayDuration.
        Assert.AreApproximatelyEqual(0.487234, band.Sample(100, -1.0f, Mathf.Lerp), tolerance: 1e-4);
    }

    [EditorTest(Category = "WoW Lighting")]
    public static void Band_with_no_keys_returns_the_empty_value()
    {
        var band = new WowLightBand<float>();
        Assert.AreApproximatelyEqual(-7.0, band.Sample(500, -7.0f, Mathf.Lerp), tolerance: 1e-5);
    }

    [EditorTest(Category = "WoW Lighting")]
    public static void Band_interpolates_colors_the_same_way_as_floats()
    {
        var band = new WowLightBand<Color>();
        band.Add(0, Colors.Black);
        band.Add(1440, Colors.White);

        Color mid = band.Sample(720, Colors.Magenta, static (a, b, t) => a.Lerp(b, t));
        Assert.AreApproximatelyEqual(0.5, mid.R, tolerance: 1e-5);
    }
}
