using Godot;

namespace WorldMapStudio.WoW;

/// <summary>Typed authoring helper for <see cref="WmoMaterialType"/> descriptions — the WMO loader never touches the raw parameter bag directly.</summary>
internal static class WmoMaterial
{
    public const string TypeId = "wow.material.wmo";

    public static readonly MeshParameter Texture = MeshParameter.Texture("texture", "Texture", "Albedo texture.");

    public static readonly MeshParameter Tint = MeshParameter.Color("tint", "Tint", Colors.White, "Fallback color shown where no texture is bound.");

    public static readonly MeshParameter Blend = MeshParameter.Choice("blend", "Blend", WowBlendModes.Options, "opaque", "MOMT blend mode.");

    public static readonly MeshParameter TwoSided = MeshParameter.Bool("two_sided", "Two Sided", false, "MOMT two-sided flag.");

    public static readonly MeshParameter Unlit = MeshParameter.Bool("unlit", "Unlit", false, "MOMT unlit flag.");

    public static readonly MeshParameter RepeatTexture = MeshParameter.Bool("repeat_texture", "Repeat Texture", true, "MOMT texture-repeat flag.");

    public static readonly MeshParameter UseVertexColor = MeshParameter.Bool("vertex_color", "Use Vertex Color", false, "Whether the group carries per-vertex MOCV colors.");

    public static readonly System.Collections.Generic.IReadOnlyList<MeshParameter> Parameters =
        MeshParameter.List(Texture, Tint, Blend, TwoSided, Unlit, RepeatTexture, UseVertexColor);

    public static MeshMaterial Describe(
        string texture = "",
        Color? tint = null,
        string blend = "opaque",
        bool twoSided = false,
        bool unlit = false,
        bool repeatTexture = true,
        bool vertexColor = false)
    {
        var values = new MeshParameterValues();
        values.Set(Texture, texture);
        values.Set(Tint, tint ?? Colors.White);
        values.Set(Blend, blend);
        values.Set(TwoSided, twoSided);
        values.Set(Unlit, unlit);
        values.Set(RepeatTexture, repeatTexture);
        values.Set(UseVertexColor, vertexColor);
        return new MeshMaterial(TypeId, values);
    }
}
