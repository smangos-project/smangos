using Godot;

namespace WorldMapStudio.WoW;

public static class LightParamsTests
{
    [EditorTest(Category = "WoW Lighting")]
    public static void Color_bands_interpolate_between_keys()
    {
        var p = new LightParams { ParamsId = 1 };
        p.ColorBand(WowSkyColorChannel.SkyFogColor).Add(0, new Color(0, 0, 0));
        p.ColorBand(WowSkyColorChannel.SkyFogColor).Add(1000, new Color(1, 1, 1));

        Color mid = p.SampleColor(WowSkyColorChannel.SkyFogColor, 500);

        Assert.IsTrue(Mathf.IsEqualApprox(mid.R, 0.5f));
    }
}
