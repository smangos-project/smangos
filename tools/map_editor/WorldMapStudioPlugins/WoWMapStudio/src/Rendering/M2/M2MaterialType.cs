using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Renders <see cref="M2Material"/> descriptions — M2's own blend/flag vocabulary stays the
/// user-facing parameter set, but building now delegates to the core's generic
/// <see cref="LayeredSurfaceMaterial"/> (via <see cref="MeshMaterialSystem.Build"/>, so it shares that
/// type's build cache and shader-variant cache too) rather than configuring a
/// <see cref="StandardMaterial3D"/> directly — this is the "adapt the data side" boundary the shading
/// engine work described: only the mapping from M2's blend values onto the generic material's
/// parameters lives here now, not any rendering capability itself.
/// </summary>
[Subsystem(nameof(MeshMaterialSystem))]
public sealed class M2MaterialType : IMeshMaterialType
{
    private readonly MeshMaterialSystem _system;

    public M2MaterialType(MeshMaterialSystem system)
    {
        _system = system;
    }

    public string Id => M2Material.TypeId;

    public string DisplayName => "M2";

    public string Description => "WoW M2 render-flag material: blend mode, two-sided, unlit, depth write.";

    public int Version => 1;

    public IReadOnlyList<MeshParameter> Parameters => M2Material.Parameters;

    public Material Build(in MeshMaterialBuildContext context)
    {
        (string blend, bool alphaModulatesBlend) = WowBlendModes.ToLayeredSurface(context.Choice(M2Material.Blend));

        MeshMaterial layered = LayeredSurfaceMaterial.Describe(
            texture: context.Texture(M2Material.Texture),
            tint: context.Color(M2Material.Tint),
            blend: blend,
            alphaCutoff: 0.5f,
            alphaModulatesBlend: alphaModulatesBlend,
            twoSided: context.Bool(M2Material.TwoSided),
            unlit: context.Bool(M2Material.Unlit),
            depthWrite: context.Bool(M2Material.DepthWrite));

        return _system.Build(layered);
    }
}
