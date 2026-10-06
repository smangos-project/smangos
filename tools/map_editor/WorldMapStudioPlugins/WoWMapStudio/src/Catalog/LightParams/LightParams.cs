using System;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// A `dbc_lightparams` row with its time-keyed `dbc_lightdata` rows turned into one
/// <see cref="WowLightBand{T}"/> per channel. <see cref="ParamsId"/> is the game's own id, named
/// so it doesn't shadow <see cref="Entity.Id"/>. Preview-only: the bands are read from the table, not edited.
/// </summary>
public sealed class LightParams : CatalogEntity, ISavedCatalogEntity
{
    public int ParamsId { get; set; }

    public float Glow { get; set; }

    public float RiverShallowAlpha { get; set; }

    public float RiverDeepAlpha { get; set; }

    public float OceanShallowAlpha { get; set; }

    public float OceanDeepAlpha { get; set; }

    public int? LightSkyboxId { get; set; }

    public WowLightBand<Color>[] ColorBands { get; } = CreateBands<Color>(Enum.GetValues<WowSkyColorChannel>().Length);

    public WowLightBand<float>[] FloatBands { get; } = CreateBands<float>(Enum.GetValues<WowSkyFloatChannel>().Length);

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Light Params {ParamsId}";

    public WowLightBand<Color> ColorBand(WowSkyColorChannel channel) => ColorBands[(int)channel];

    public WowLightBand<float> FloatBand(WowSkyFloatChannel channel) => FloatBands[(int)channel];

    public Color SampleColor(WowSkyColorChannel channel, int time) =>
        ColorBand(channel).Sample(time, Colors.Black, static (a, b, t) => a.Lerp(b, t));

    public float SampleFloat(WowSkyFloatChannel channel, int time) =>
        FloatBand(channel).Sample(time, 0.0f, Mathf.Lerp);

    private static WowLightBand<T>[] CreateBands<T>(int count)
    {
        var bands = new WowLightBand<T>[count];
        for (int i = 0; i < bands.Length; i++)
        {
            bands[i] = new WowLightBand<T>();
        }

        return bands;
    }
}
