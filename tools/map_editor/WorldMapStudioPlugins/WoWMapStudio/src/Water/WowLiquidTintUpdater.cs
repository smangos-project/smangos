using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// The live half of the ocean/river DNC tint. Registers the four <c>wow_*_shallow</c>/<c>wow_*_deep</c>
/// tints as Godot global shader parameters and refreshes them every frame from
/// <see cref="EnvironmentSystem.Current"/>, so every <c>wow.liquid</c> material reads a live value
/// straight in the shader — no per-material rebuild, no cache to go stale, no need for anything to
/// notice a <c>LightParams</c> edit at all.
///
/// Shallow and deep are pushed as separate uniforms rather than pre-blended here because the shader is
/// what knows each fragment's water depth (it reconstructs it from the depth buffer) — this only
/// supplies the two ends of the gradient `mix(ColorLight, ColorDark, depth)` interpolates. Each is
/// packed as <c>rgb = colour, a = that end's alpha</c>, so the lerp blends colour and alpha together.
///
/// One instance lives for the whole editor session, added once as a child of
/// <see cref="EditorContext.Root"/> by <see cref="WowLiquidMaterialType"/> the first time a
/// <c>wow.liquid</c> material is built — a plain Godot <see cref="Node"/>'s own <c>_Process</c> is how
/// this reaches "runs every frame" without any core-file per-frame hook, since nothing in this editor
/// ticks plugin subsystems on its own.
/// </summary>
public sealed partial class WowLiquidTintUpdater : Node
{
    private const string RiverShallow = "wow_river_shallow";
    private const string RiverDeep = "wow_river_deep";
    private const string OceanShallow = "wow_ocean_shallow";
    private const string OceanDeep = "wow_ocean_deep";

    private readonly EnvironmentSystem _environments;

    public WowLiquidTintUpdater(EnvironmentSystem environments)
    {
        _environments = environments;
        Name = "WowLiquidTintUpdater";
        ProcessMode = ProcessModeEnum.Always;
    }

    public override void _Ready()
    {
        foreach (string name in new[] { RiverShallow, RiverDeep, OceanShallow, OceanDeep })
        {
            RenderingServer.GlobalShaderParameterAdd(name, RenderingServer.GlobalShaderParameterType.Vec4, new Vector4(0.0f, 0.0f, 0.0f, 1.0f));
        }
    }

    public override void _Process(double delta)
    {
        EnvironmentValues values = _environments.Current;
        RenderingServer.GlobalShaderParameterSet(RiverShallow, Sample(values, "wow.river.", "shallow"));
        RenderingServer.GlobalShaderParameterSet(RiverDeep, Sample(values, "wow.river.", "deep"));
        RenderingServer.GlobalShaderParameterSet(OceanShallow, Sample(values, "wow.ocean.", "shallow"));
        RenderingServer.GlobalShaderParameterSet(OceanDeep, Sample(values, "wow.ocean.", "deep"));
    }

    /// <summary>
    /// One end of one liquid kind's gradient. Converted sRGB → linear because the shader adds this to a
    /// texel sampled through a <c>source_color</c> sampler (already linearised by Godot) and assigns the
    /// sum to <c>ALBEDO</c>, which is linear — adding raw sRGB numbers there would wash the tint out.
    /// </summary>
    private static Vector4 Sample(EnvironmentValues values, string prefix, string end)
    {
        Color color = values.Colors.GetValueOrDefault(prefix + end, Colors.Black).SrgbToLinear();
        float alpha = values.Floats.GetValueOrDefault(prefix + end + "_alpha", 1.0f);
        return new Vector4(color.R, color.G, color.B, alpha);
    }
}
