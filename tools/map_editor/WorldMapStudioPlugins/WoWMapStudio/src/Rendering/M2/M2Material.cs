using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Typed authoring helper for <see cref="M2MaterialType"/> descriptions — the M2 loader never touches the raw parameter bag directly.</summary>
internal static class M2Material
{
    public const string TypeId = "wow.material.m2";

    public static readonly MeshParameter Texture = MeshParameter.Texture("texture", "Texture", "Albedo texture. Empty for a replaceable (creature skin / item) texture slot.");

    public static readonly MeshParameter Tint = MeshParameter.Color("tint", "Tint", Colors.White, "Fallback color shown where no texture is bound.");

    public static readonly MeshParameter Blend = MeshParameter.Choice("blend", "Blend", WowBlendModes.Options, "opaque", "M2 render-flag blend mode.");

    public static readonly MeshParameter TwoSided = MeshParameter.Bool("two_sided", "Two Sided", false, "M2 render flag 0x04.");

    public static readonly MeshParameter Unlit = MeshParameter.Bool("unlit", "Unlit", false, "M2 render flag 0x01.");

    public static readonly MeshParameter DepthWrite = MeshParameter.Bool("depth_write", "Depth Write", true, "M2 render flag 0x10 (inverted).");

    /// <summary>The M2 texture slot this material was resolved for (see <c>M2TextureDef.Type</c>):
    /// 0 filename-in-model, 1 body/skin, 2 cape/item, 6 hair, 8 fur, 11-13 creature skin 1-3. Purely
    /// descriptive — <see cref="M2MaterialType.Build"/> never reads it — but it is what lets a
    /// <see cref="ModelInstantiateOptions.MaterialOverride"/> tell which replaceable slot a material
    /// came from without the caller re-deriving it from the surface index.</summary>
    public static readonly MeshParameter TextureSlot = MeshParameter.Int("texture_slot", "Texture Slot", 0, 0, int.MaxValue, "M2 replaceable texture slot (0 = not replaceable).");

    public static readonly System.Collections.Generic.IReadOnlyList<MeshParameter> Parameters =
        MeshParameter.List(Texture, Tint, Blend, TwoSided, Unlit, DepthWrite, TextureSlot);

    public static MeshMaterial Describe(
        string texture = "",
        Color? tint = null,
        string blend = "opaque",
        bool twoSided = false,
        bool unlit = false,
        bool depthWrite = true,
        uint textureSlot = 0)
    {
        var values = new MeshParameterValues();
        values.Set(Texture, texture);
        values.Set(Tint, tint ?? Colors.White);
        values.Set(Blend, blend);
        values.Set(TwoSided, twoSided);
        values.Set(Unlit, unlit);
        values.Set(DepthWrite, depthWrite);
        values.Set(TextureSlot, (int)textureSlot);
        return new MeshMaterial(TypeId, values);
    }
}
