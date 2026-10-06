using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Sun direction (in WoW's own axis convention — see <c>WowCoordinates</c> for the swap into
/// Godot space) and night intensity at one instant.</summary>
public readonly record struct WowOutdoorLightStats(float NightIntensity, Vector3 DayDir);

/// <summary>
/// Sun direction and night intensity from the time of day: a hardcoded 24-hour night-intensity table
/// and a fixed spherical sun path (a 4-entry phi table, constant theta) — not derived from any DBC,
/// so there is nothing to load.
/// </summary>
public static class WowOutdoorLighting
{
    private const long DayNightSecondsPerDay = 86400;
    private const float Theta = 3.926991f;

    private static readonly float[] PhiTable = [2.2165682f, 1.9198623f, 2.2165682f, 1.9198623f];

    // 1 = night, 0 = day, one entry per whole hour.
    private static readonly float[] NightHours =
    [
        1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 1,
    ];

    /// <summary>Samples at <paramref name="time"/>, a WoW day unit (0..<see cref="WowDay.Duration"/>; wraps).</summary>
    public static WowOutdoorLightStats Sample(int time)
    {
        int normalizedTime = (((time % WowDay.Duration) + WowDay.Duration) % WowDay.Duration) / 2;

        long progress = normalizedTime * 120L;
        progress = ((progress % DayNightSecondsPerDay) + DayNightSecondsPerDay) % DayNightSecondsPerDay;

        int hourA = normalizedTime / 60;
        int hourB = (hourA + 1) % 24;

        return Interpolate(NightHours[hourA], NightHours[hourB], progress);
    }

    private static WowOutdoorLightStats Interpolate(float nightA, float nightB, long progressSeconds)
    {
        float progress = progressSeconds / (float)DayNightSecondsPerDay;

        int currentPhiIndex = (int)(progress / 0.25f);
        int nextPhiIndex = currentPhiIndex < 3 ? currentPhiIndex + 1 : 0;
        float transition = (progress / 0.25f) - currentPhiIndex;
        float phi = Mathf.Lerp(PhiTable[currentPhiIndex], PhiTable[nextPhiIndex], transition);

        float sinPhi = Mathf.Sin(phi);
        float cosPhi = Mathf.Cos(phi);
        float sinTheta = Mathf.Sin(Theta);
        float cosTheta = Mathf.Cos(Theta);

        var dayDir = new Vector3(sinPhi * cosTheta, sinPhi * sinTheta, cosPhi);

        float remaining = 1.0f - progress;
        float nightIntensity = (nightA * remaining) + (nightB * progress);

        return new WowOutdoorLightStats(nightIntensity, dayDir);
    }
}
