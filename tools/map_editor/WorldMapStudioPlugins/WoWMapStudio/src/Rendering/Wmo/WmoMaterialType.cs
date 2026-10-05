using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Renders <see cref="WmoMaterial"/> descriptions — WMO's own blend/flag vocabulary stays the
/// user-facing parameter set, but building now delegates to the core's generic
/// <see cref="LayeredSurfaceMaterial"/> (via <see cref="MeshMaterialSystem.Build"/>) rather than
/// configuring a <see cref="StandardMaterial3D"/> directly, the same boundary <see cref="M2MaterialType"/>
/// now uses. This is what gets WMO buildings interior/exterior ambient blending: it falls out of
/// <see cref="LayeredSurfaceMaterialType"/> always reading vertex COLOR.a for that, with no change
/// needed here beyond routing through it — a WMO group's real MOCV alpha (wherever the loader already
/// provides it) drives the blend automatically.
///
/// Known simplification: <see cref="WmoMaterial.RepeatTexture"/> is not honored —
/// <see cref="LayeredSurfaceMaterialType"/>'s shader always samples with texture repeat enabled, since
/// making repeat another compile-time shader-variant axis wasn't judged worth it for what should be a
/// rare "clamp this decal-like texture" case. Revisit if that turns out to matter in practice.
/// </summary>
[Subsystem(nameof(MeshMaterialSystem))]
public sealed class WmoMaterialType : IMeshMaterialType
{
    private readonly MeshMaterialSystem _system;

    public WmoMaterialType(MeshMaterialSystem system)
    {
        _system = system;
    }

    public string Id => WmoMaterial.TypeId;

    public string DisplayName => "WMO";

    public string Description => "WoW WMO material: blend mode, two-sided, unlit, texture repeat, vertex color.";

    public int Version => 1;

    public IReadOnlyList<MeshParameter> Parameters => WmoMaterial.Parameters;

    public Material Build(in MeshMaterialBuildContext context)
    {
        (string blend, bool alphaModulatesBlend) = WowBlendModes.ToLayeredSurface(context.Choice(WmoMaterial.Blend));

        MeshMaterial layered = LayeredSurfaceMaterial.Describe(
            texture: context.Texture(WmoMaterial.Texture),
            tint: context.Color(WmoMaterial.Tint),
            blend: blend,
            alphaCutoff: 0.5f,
            alphaModulatesBlend: alphaModulatesBlend,
            twoSided: context.Bool(WmoMaterial.TwoSided),
            unlit: context.Bool(WmoMaterial.Unlit),
            vertexColorTint: context.Bool(WmoMaterial.UseVertexColor));

        return _system.Build(layered);
    }
}
