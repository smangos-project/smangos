using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Covers <see cref="WowAxisPreset"/> against <see cref="WowCoordinates"/>, the conversion it
/// is meant to mirror.</summary>
public static class WowAxisPresetTests
{
    [EditorTest(Category = "WoW Landscape")]
    public static void Preset_matches_wow_coordinates_conversion()
    {
        AxisConvention convention = new WowAxisPreset(AppSystems.Instance.AxisConventionPresets).CreateConvention();
        var wow = new Vector3(1, 2, 3);

        Assert.IsTrue(convention.ToGodot(wow).IsEqualApprox(WowCoordinates.ToGodot(wow)));
    }

    [EditorTest(Category = "WoW Landscape")]
    public static void Preset_is_registered_on_the_shared_registry()
    {
        bool found = false;
        foreach (IAxisConventionPreset preset in AppSystems.Instance.AxisConventionPresets.Presets)
        {
            found |= preset is WowAxisPreset;
        }

        Assert.IsTrue(found, "WowAxisPreset should self-register with AxisConventionPresets");
    }
}
