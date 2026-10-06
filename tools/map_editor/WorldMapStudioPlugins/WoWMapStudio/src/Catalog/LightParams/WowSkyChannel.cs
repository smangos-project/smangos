namespace WorldMapStudio.WoW;

/// <summary>The colour channels a <see cref="LightParams"/> band set carries, one per `dbc_lightdata` colour column.</summary>
public enum WowSkyColorChannel
{
    LightGlobalDiffuse,
    LightGlobalAmbient,
    SkyColorTop,
    SkyColorMiddle,
    SkyColorBand1,
    SkyColorBand2,
    SkyColorSmog,
    SkyFogColor,
    SunColor,
    SunCloudColor,
    CloudEmissiveColor,
    CloudLayer1AmbientColor,
    CloudLayer2AmbientColor,
    OceanColorLight,
    OceanColorDark,
    RiverColorLight,
    RiverColorDark,
}

/// <summary>The float channels a <see cref="LightParams"/> band set carries.</summary>
public enum WowSkyFloatChannel
{
    FogDistance,
    FogMultiplier,
    CloudDensity,
}
