using System.Collections.Generic;

namespace WorldMapStudio.WoW;

/// <summary>
/// The client's M2Blend vocabulary, shared by M2 materials and WMO materials (WMO's <c>MOMT.blendMode</c>
/// uses the same enum — see the verification note in the pre-split <c>M2ModelLoader.ToBlendMode</c>).
/// Kept as its own distinct-value <see cref="MeshParameterOption"/> set rather than collapsing onto the
/// core's generic <c>StandardMeshMaterial.Blend</c>, so "no add alpha" and "add" stay two different
/// choices even though Godot renders them the same way today.
/// </summary>
internal static class WowBlendModes
{
    public static readonly IReadOnlyList<MeshParameterOption> Options =
    [
        new("opaque", "Opaque"),
        new("alpha_key", "Alpha Key"),
        new("alpha", "Alpha"),
        new("no_add_alpha", "No Add Alpha"),
        new("add", "Add"),
        new("mod", "Mod"),
        new("mod2x", "Mod2x"),
    ];

    /// <summary>Raw M2Blend value (0-6, as read from an M2 material or a WMO's MOMT) to its choice id.</summary>
    public static string FromRaw(uint blend) => blend switch
    {
        1 => "alpha_key",
        2 => "alpha",
        3 => "no_add_alpha",
        4 => "add",
        5 => "mod",
        6 => "mod2x",
        _ => "opaque",
    };

    /// <summary>
    /// This vocabulary onto <see cref="LayeredSurfaceMaterial"/>'s generic one, shared by
    /// <see cref="WoW.M2MaterialType"/> and <see cref="WoW.WmoMaterialType"/> since both render through
    /// it now. <c>no_add_alpha</c> and <c>add</c> both mean "additive" but differ in whether the
    /// texture's alpha channel scales the effect — a distinction Godot's <see cref="StandardMaterial3D"/>
    /// had no shader parameter for, but <see cref="LayeredSurfaceMaterialType"/>'s custom shader does.
    /// </summary>
    public static (string Blend, bool AlphaModulatesBlend) ToLayeredSurface(string blend) => blend switch
    {
        "alpha_key" => ("alpha_cutout", true),
        "alpha" => ("alpha_blend", true),
        "no_add_alpha" => ("additive", false),
        "add" => ("additive", true),
        "mod" => ("modulate", true),
        "mod2x" => ("modulate2x", true),
        _ => ("opaque", true),
    };
}
