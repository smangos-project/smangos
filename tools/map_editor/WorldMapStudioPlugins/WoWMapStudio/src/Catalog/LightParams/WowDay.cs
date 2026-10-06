namespace WorldMapStudio.WoW;

/// <summary>The 2880-unit WoW day (one unit per half-minute from midnight to midnight), shared by
/// <see cref="WowLightBand{T}"/> sampling and <see cref="WowOutdoorLighting"/>.</summary>
public static class WowDay
{
    public const int Duration = 2880;
}
