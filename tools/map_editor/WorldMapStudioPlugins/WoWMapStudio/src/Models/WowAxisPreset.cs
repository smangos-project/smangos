namespace WorldMapStudio.WoW;

/// <summary>
/// The axis convention matching WoW's own client-model space: X forward, Y left, Z up. Registers as
/// an axis preset so a project mapping for WoW starts from the exact convention <see cref="WowCoordinates"/>
/// already converts through, instead of the user re-deriving it by hand.
/// </summary>
[Subsystem(nameof(AxisConventionPresets))]
public sealed class WowAxisPreset : IAxisConventionPreset
{
    public WowAxisPreset(AxisConventionPresets presets)
    {
    }

    public string Name => "WoW";

    public string Description => "WoW's client-model axes: X forward, Y left, Z up.";

    // Matches WowCoordinates.ToGodot's (x, z, -y) mapping: user X -> Godot +X, user Y -> Godot -Z
    // (a positive WoW Y is west, which is Godot -Z), user Z (up) -> Godot +Y.
    public AxisConvention CreateConvention() =>
        AxisConvention.Create(SignedAxis.PosX, SignedAxis.NegZ, SignedAxis.PosY);
}
