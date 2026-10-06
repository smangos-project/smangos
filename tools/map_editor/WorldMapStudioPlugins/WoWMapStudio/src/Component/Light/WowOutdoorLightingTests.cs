using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Covers <see cref="WowOutdoorLighting"/>'s night-intensity table and sun-direction invariant.</summary>
public static class WowOutdoorLightingTests
{
    [EditorTest(Category = "WoW Lighting")]
    public static void Outdoor_lighting_is_full_night_at_midnight()
    {
        Assert.AreApproximatelyEqual(1.0, WowOutdoorLighting.Sample(0).NightIntensity, tolerance: 1e-5);
    }

    [EditorTest(Category = "WoW Lighting")]
    public static void Outdoor_lighting_is_full_day_at_noon()
    {
        Assert.AreApproximatelyEqual(0.0, WowOutdoorLighting.Sample(1440).NightIntensity, tolerance: 1e-5);
    }

    [EditorTest(Category = "WoW Lighting")]
    public static void Outdoor_lighting_is_full_night_deep_in_the_night_hours()
    {
        // Hour 3 (time = 3 * 120): both its own and the next hour are night in the table, so the
        // result is 1.0 regardless of where in the hour the query falls.
        Assert.AreApproximatelyEqual(1.0, WowOutdoorLighting.Sample(3 * 120).NightIntensity, tolerance: 1e-5);
    }

    [EditorTest(Category = "WoW Lighting")]
    public static void Outdoor_lighting_is_full_day_at_dawn_and_dusk_hour_boundaries()
    {
        // Hours 6 and 18 (dawn/dusk) are both inside the table's day block, one hour past the actual
        // night/day transitions (hour 5→6 and 21→22), so both come out fully day.
        Assert.AreApproximatelyEqual(0.0, WowOutdoorLighting.Sample(6 * 120).NightIntensity, tolerance: 1e-5);
        Assert.AreApproximatelyEqual(0.0, WowOutdoorLighting.Sample(18 * 120).NightIntensity, tolerance: 1e-5);
    }

    [EditorTest(Category = "WoW Lighting")]
    public static void Outdoor_lighting_sun_direction_is_always_unit_length()
    {
        foreach (int hour in new[] { 0, 3, 6, 9, 12, 15, 18, 21 })
        {
            Vector3 dayDir = WowOutdoorLighting.Sample(hour * 120).DayDir;
            Assert.AreApproximatelyEqual(1.0, dayDir.Length(), tolerance: 1e-4, message: $"hour {hour}");
        }
    }
}
