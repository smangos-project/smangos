using System.Collections.Generic;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// WoW's liquid material type. Registered as an ordinary <see cref="IMeshMaterialType"/> so a saved
/// preset of this type is possible, but <c>WaterSurfaceMeshFunction</c>/<c>RiverSurfaceMeshFunction</c>
/// don't reach it that way: they declare <see cref="WowLiquidParameters.LiquidTypeIdLookup"/>-shaped
/// parameters directly and construct a <see cref="MeshMaterial"/> via
/// <see cref="WowLiquidParameters.BuildMaterial"/> in their own <c>Build</c>, bypassing material slots
/// (and the <see cref="IModelFormat"/> that material-type resolution otherwise requires).
///
/// Renders an animated texture array (<see cref="LiquidTextureLibrary"/>) through a hand-written shader.
/// Water/ocean (<see cref="LiquidBasicType.Water"/>/<see cref="LiquidBasicType.Ocean"/>) render <b>lit</b>
/// — diffuse N·L against the scene sun plus ambient — while magma/slime stay unlit/self-illuminated,
/// since they are their own light source in the real client. Real WoW water darkens at night and tints
/// toward the sun, which a flat unlit surface cannot show.
///
/// The tint (colour and alpha, from `LightParams`' ocean/river colour bands and
/// `River/OceanShallow/DeepAlpha`) is live: <see cref="WowLiquidTintUpdater"/> refreshes four Godot
/// global shader parameters every frame from <see cref="EnvironmentSystem.Current"/>, so editing a
/// <c>LightParams</c> entry shows up immediately. A per-material uniform set at <see cref="Build"/> time
/// would be stuck: neither `ProceduralSystem`'s nor `MeshMaterialSystem`'s build caches depend on
/// environment state.
///
/// Shallow and deep are blended per-fragment by water depth with `mix(ColorLight, ColorDark, depth)`,
/// reconstructed in the shader from the opaque depth buffer (terrain behind the surface, minus the
/// surface itself), since the rendered mesh has no per-vertex depth. The exported MH2O depth bytes still
/// come from <see cref="WowLiquidRasterizer"/> and are unaffected.
/// </summary>
[Subsystem(nameof(MeshMaterialSystem))]
public sealed class WowLiquidMaterialType : IMeshMaterialType
{
    public const string TypeId = "wow.liquid";

    private readonly MeshMaterialSystem _system;
    private readonly LiquidTextureLibrary _textures = new();

    public WowLiquidMaterialType(MeshMaterialSystem system)
    {
        _system = system;
    }

    public string Id => TypeId;

    public string DisplayName => "WoW Liquid";

    public string Description => "Animated liquid texture from a 3.3.5 client's LiquidType.dbc.";

    public int Version => 1;

    public IReadOnlyList<MeshParameter> Parameters =>
        MeshParameter.List(WowLiquidParameters.LiquidTypeIdChoice(_system.Context), WowLiquidParameters.FrameDuration);

    public Material Build(in MeshMaterialBuildContext context)
    {
        EnsureTintUpdater();

        int liquidTypeId = ParseChoice(context.Choice(WowLiquidParameters.LiquidTypeIdLookup));
        float frameDuration = Mathf.Max(0.01f, context.Float(WowLiquidParameters.FrameDuration));
        LiquidType? row = WowLiquidCatalog.Find(_system.Context, liquidTypeId);
        Texture2DArray? frames = _textures.GetTextureArray(_system.Context, liquidTypeId);
        LiquidBasicType basicType = row?.BasicType ?? LiquidBasicType.Water;
        bool lit = basicType is LiquidBasicType.Water or LiquidBasicType.Ocean;

        var material = new ShaderMaterial { Shader = LiquidShader(lit) };
        if (frames != null)
        {
            material.SetShaderParameter("liquid_frames", frames);
        }

        material.SetShaderParameter("has_frames", frames != null);
        material.SetShaderParameter("anim_uv", new Vector2(row?.AnimationX ?? 1.0f, row?.AnimationY ?? 0.0f));
        material.SetShaderParameter("basic_type", (int)basicType);
        material.SetShaderParameter("frame_duration", frameDuration);
        return material;
    }

    /// <summary>Installs <see cref="WowLiquidTintUpdater"/> into the live scene tree the first time it's
    /// needed. Idempotent across every <see cref="WowLiquidMaterialType"/> instance this editor session
    /// creates (e.g. across a <c>MeshMaterialSystem.DiscoverFrom</c> rerun) via the static instance
    /// field, not just this one.</summary>
    private void EnsureTintUpdater()
    {
        if (GodotObject.IsInstanceValid(_tintUpdater))
        {
            return;
        }

        _tintUpdater = new WowLiquidTintUpdater(_system.Context.Environments);
        _system.Context.Root.AddChild(_tintUpdater);
    }

    private static WowLiquidTintUpdater? _tintUpdater;

    private static int ParseChoice(string value) => int.TryParse(value, out int id) ? id : 1;

    private static Shader? _litShader;
    private static Shader? _unlitShader;

    // lit selects a compile-time render_mode difference (unshaded or not — see BuildShaderCode), so
    // (like LayeredSurfaceMaterialType) it needs two cached Shader instances rather than one shared
    // shader driven purely by uniforms.
    private static Shader LiquidShader(bool lit) => lit
        ? _litShader ??= new Shader { Code = BuildShaderCode(lit: true) }
        : _unlitShader ??= new Shader { Code = BuildShaderCode(lit: false) };

    // Kept in source rather than a .gdshader resource so it needs no Godot import step, matching
    // LandscapeChunkMesh.BuildMaterial's own inline splat shader.
    private static string BuildShaderCode(bool lit)
    {
        // Water/ocean pick up real diffuse N·L + ambient from Godot's own lighting (ROUGHNESS/SPECULAR
        // below feed that); magma/slime stay unshaded/self-illuminated, same as before.
        string shading = lit ? "diffuse_burley" : "unshaded";

        return $$"""
shader_type spatial;
render_mode cull_disabled, {{shading}}, blend_mix;

uniform sampler2DArray liquid_frames : source_color, filter_linear_mipmap, repeat_enable;
uniform sampler2D DEPTH_TEXTURE : hint_depth_texture, filter_linear_mipmap;
uniform vec2 anim_uv = vec2(1.0, 0.0);
uniform int basic_type = 0; // LiquidBasicType: 0 water, 1 ocean, 2 magma, 3 slime
uniform float frame_duration = 0.15;
uniform float depth_scale = 20.0;
uniform bool has_frames = false;

// A fixed sheen strength rather than an exposed setting yet, matching LandscapeChunkMesh's
// height_blend_sharpness convention for "real but not yet author-tunable" constants.
const float fresnel_strength = 0.35;

// Refreshed every frame by WowLiquidTintUpdater, not per-material uniforms: the whole point is that a
// LightParams edit shows up immediately, with nothing to rebuild or invalidate. rgb = colour, a =
// that end's alpha.
global uniform vec4 wow_river_shallow;
global uniform vec4 wow_river_deep;
global uniform vec4 wow_ocean_shallow;
global uniform vec4 wow_ocean_deep;

""" + EnvironmentShaderLibrary.FogFunctionCode + """

varying vec3 world_pos;

void vertex() {
    world_pos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
}

void fragment() {
    // How deep the water is at this fragment, 0 (shore) to 1 (>= depth_scale units), reconstructed from the opaque
    // depth buffer: the distance between the terrain behind the water and the water surface itself. The rendered mesh
    // has no per-vertex MH2O depth channel, and screen-space depth is both a good approximation and finer-grained
    // than MH2O's own 8x8 grid would be. Inlined here rather than a helper function:
    // SCREEN_UV/INV_PROJECTION_MATRIX/VERTEX are only valid inside fragment() itself in Godot's shader language, not
    // in a function declared alongside it.
    float scene_raw = texture(DEPTH_TEXTURE, SCREEN_UV).r;
    vec4 view = INV_PROJECTION_MATRIX * vec4(vec3(SCREEN_UV * 2.0 - 1.0, scene_raw), 1.0);
    float scene_dist = -(view.xyz / view.w).z;
    float water_depth = clamp((scene_dist + VERTEX.z) / max(depth_scale, 0.001), 0.0, 1.0);

    int frame_count = max(textureSize(liquid_frames, 0).z, 1);
    int frame = int(TIME / max(frame_duration, 0.01)) % frame_count;

    vec2 uv;
    if (basic_type == 2 || basic_type == 3) {
        // Magma / slime: the texture scrolls rather than tiling and rotating, at a rate
        // that approximates the client's.
        uv = UV + (anim_uv * TIME * 0.05);
    } else {
        // Water / river / ocean: scale by anim_uv.x, then rotate anim_uv.y degrees about the origin
        // (0,0), not the UV centre.
        float a = radians(anim_uv.y);
        uv = mat2(vec2(cos(a), -sin(a)), vec2(sin(a), cos(a))) * (UV * anim_uv.x);
    }

    // An unbound sampler2DArray reads as opaque white, which clamps texel + tint to white and looks like
    // a broken tint rather than a missing client. Fall back to black so the tint alone shows through.
    vec4 tex = has_frames ? texture(liquid_frames, vec3(uv, float(frame))) : vec4(0.0, 0.0, 0.0, 1.0);

    vec3 color = tex.rgb;
    float alpha = tex.a;
    if (basic_type == 0 || basic_type == 1) {
        // Water/river or ocean only — magma/slime have no "water colour" concept in LightParams.dbc.
        // Mix the shallow and deep ends by depth, then add the texel. Additive, not a multiply.
        vec4 shallow = basic_type == 1 ? wow_ocean_shallow : wow_river_shallow;
        vec4 deep = basic_type == 1 ? wow_ocean_deep : wow_river_deep;
        vec4 tint = mix(shallow, deep, water_depth);
        color = clamp(color + tint.rgb, 0.0, 1.0);
        alpha = tint.a;

        // Fresnel sheen: view-angle brightening toward the sky/sun, so lit water reads as a reflective
        // surface rather than flat tinted plastic. An aesthetic addition, not an accuracy fix.
        float fresnel = pow(1.0 - clamp(dot(NORMAL, VIEW), 0.0, 1.0), 3.0);
        color = clamp(color + wms_sun_glow_color * fresnel * fresnel_strength, 0.0, 1.0);
    }

    ALBEDO = color;
    ALPHA = alpha;
    ROUGHNESS = 0.05;
    SPECULAR = 0.5;

    vec3 view_vec = world_pos - CAMERA_POSITION_WORLD;
    vec3 fog_color;
    float fog_visibility = wms_evaluate_fog(view_vec, world_pos.y, normalize(view_vec), 0, fog_color);
    FOG = vec4(fog_color, 1.0 - fog_visibility);
}
""";
    }
}
