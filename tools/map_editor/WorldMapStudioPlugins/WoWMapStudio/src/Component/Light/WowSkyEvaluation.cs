using System.Collections.Generic;
using System.Linq;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Turns one resolved <see cref="LightParams"/> into the <see cref="EnvironmentValues"/> a light contributes at a time of day.</summary>
internal static class WowSkyEvaluation
{
    /// <summary>WoW asset path of the stars model, drawn whenever a param has no skybox or sets the combine flag.</summary>
    public const string StarsModelPath = "Environments/Stars/Stars.m2";

    public static int WowTimeOf(in EnvironmentTime time) =>
        (((int)(time.DayFraction * WowDay.Duration) % WowDay.Duration) + WowDay.Duration) % WowDay.Duration;

    public static EnvironmentValues Evaluate(LightParams p, CatalogEntityRegistry catalog, int wowTime)
    {
        WowOutdoorLightStats stats = WowOutdoorLighting.Sample(wowTime);
        LightSkybox? skybox = p.LightSkyboxId is int id
            ? catalog.OfType<LightSkybox>().FirstOrDefault(candidate => candidate.SkyboxId == id)
            : null;

        SkyGradientStop[] gradient =
        [
            new SkyGradientStop(90.0f, p.SampleColor(WowSkyColorChannel.SkyColorTop, wowTime)),
            new SkyGradientStop(18.0f, p.SampleColor(WowSkyColorChannel.SkyColorMiddle, wowTime)),
            new SkyGradientStop(10.0f, p.SampleColor(WowSkyColorChannel.SkyColorBand1, wowTime)),
            new SkyGradientStop(3.0f, p.SampleColor(WowSkyColorChannel.SkyColorBand2, wowTime)),
            new SkyGradientStop(0.0f, p.SampleColor(WowSkyColorChannel.SkyColorSmog, wowTime)),
            new SkyGradientStop(-30.0f, p.SampleColor(WowSkyColorChannel.SkyFogColor, wowTime)),
            new SkyGradientStop(-90.0f, p.SampleColor(WowSkyColorChannel.SkyFogColor, wowTime)),
        ];

        float fogEnd = p.SampleFloat(WowSkyFloatChannel.FogDistance, wowTime) / 36.0f;
        if (fogEnd <= 0.0f)
        {
            fogEnd = 6500.0f / 36.0f;
        }

        // Taken as authored: a negative multiplier starts the fog behind the camera.
        float fogStart = p.SampleFloat(WowSkyFloatChannel.FogMultiplier, wowTime) * fogEnd;

        return new EnvironmentValues
        {
            SunColor = p.SampleColor(WowSkyColorChannel.LightGlobalDiffuse, wowTime),
            SunDirection = WowCoordinates.ToGodot(stats.DayDir),
            SunEnergy = 1.0f,
            AmbientColor = p.SampleColor(WowSkyColorChannel.LightGlobalAmbient, wowTime),
            AmbientEnergy = 1.0f,
            SkyGradient = gradient,
            FogEnabled = true,
            FogColor = p.SampleColor(WowSkyColorChannel.SkyFogColor, wowTime),
            FogStart = fogStart,
            FogEnd = fogEnd,
            FogCurve = FogRate(fogEnd - fogStart),
            SkyLayers = BuildSkyLayers(skybox, stats),
            Floats = new Dictionary<string, float>
            {
                ["wow.glow"] = p.Glow,
                ["wow.cloud_density"] = p.SampleFloat(WowSkyFloatChannel.CloudDensity, wowTime),
                ["wow.river.shallow_alpha"] = p.RiverShallowAlpha,
                ["wow.river.deep_alpha"] = p.RiverDeepAlpha,
                ["wow.ocean.shallow_alpha"] = p.OceanShallowAlpha,
                ["wow.ocean.deep_alpha"] = p.OceanDeepAlpha,
            },
            Colors = new Dictionary<string, Color>
            {
                ["wow.ocean.shallow"] = p.SampleColor(WowSkyColorChannel.OceanColorLight, wowTime),
                ["wow.ocean.deep"] = p.SampleColor(WowSkyColorChannel.OceanColorDark, wowTime),
                ["wow.river.shallow"] = p.SampleColor(WowSkyColorChannel.RiverColorLight, wowTime),
                ["wow.river.deep"] = p.SampleColor(WowSkyColorChannel.RiverColorDark, wowTime),
            },
        };
    }

    private static List<SkyLayer> BuildSkyLayers(LightSkybox? skybox, WowOutdoorLightStats stats)
    {
        var layers = new List<SkyLayer>();
        bool hasSkybox = skybox is { ModelPath.Length: > 0 };

        if (hasSkybox)
        {
            layers.Add(new SkyLayer(skybox!.ModelPath, 1.0f, 1.0f));
        }

        // Slightly inside the skybox so the two never z-fight.
        if (!hasSkybox || (skybox!.Flags & WowSkyboxFlags.Combine) != 0)
        {
            layers.Add(new SkyLayer(StarsModelPath, stats.NightIntensity, 0.98f));
        }

        return layers;
    }

    private static float FogRate(float fogRange)
    {
        const float fogFarClip = 1583.333374f;
        return fogRange <= fogFarClip
            ? ((1.0f - (fogRange / fogFarClip)) * 5.5f) + 1.5f
            : 1.5f;
    }
}
