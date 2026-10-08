"""PropertyGroups holding the raw WoW fields that export rebuilds from."""
import bpy
from bpy.props import (BoolProperty, CollectionProperty, FloatProperty, FloatVectorProperty, IntProperty,
                       IntVectorProperty, PointerProperty, StringProperty)

U32 = 1 << 32


def s32(v):
    """Store an unsigned 32-bit value in a signed Blender int."""
    v = int(v)
    return v - U32 if v >= 1 << 31 else v


def u32(v):
    return int(v) % U32


class WowWmoMaterial(bpy.types.PropertyGroup):
    flags: IntProperty()
    shader: IntProperty()
    blend_mode: IntProperty()
    texture1: IntProperty(description="FileDataID")
    texture2: IntProperty(description="FileDataID")
    texture3: IntProperty(description="FileDataID")
    sidn_color: IntVectorProperty(size=4)
    frame_sidn_color: IntVectorProperty(size=4)
    diff_color: IntVectorProperty(size=4)
    ground_type: IntProperty()
    color2: IntProperty()
    flags2: IntProperty()
    run_time_data: IntVectorProperty(size=4)
    material_index: IntProperty(default=-1, description="Index in MOMT")


class WowWmo(bpy.types.PropertyGroup):
    path: StringProperty()
    fdid: IntProperty()
    mver: IntProperty()
    flags: IntProperty()
    wmo_id: IntProperty()
    num_lod: IntProperty()
    is_new: BoolProperty()
    ambient_color: IntVectorProperty(size=4)


class WowPortalRef(bpy.types.PropertyGroup):
    group_index: IntProperty()
    portal_index: IntProperty()
    side: IntProperty()


class WowWmoGroup(bpy.types.PropertyGroup):
    index: IntProperty(default=-1)
    flags: IntProperty()
    flags2: IntProperty()
    group_liquid: IntProperty()
    unique_id: IntProperty()
    fog_ids: IntVectorProperty(size=4)
    group_name: StringProperty()
    descriptive_name: StringProperty()
    name_offset: IntProperty()
    group_name_offset: IntProperty()
    descriptive_name_offset: IntProperty()
    int_batch_count: IntProperty()
    trans_batch_count: IntProperty()
    ext_batch_count: IntProperty()
    batch_type_d: IntProperty()
    parent_split: IntProperty()
    next_split: IntProperty()
    fdid: IntProperty()


class WowWmoPortal(bpy.types.PropertyGroup):
    index: IntProperty()
    plane_normal: FloatVectorProperty(size=3)
    plane_distance: FloatProperty()
    refs: CollectionProperty(type=WowPortalRef)


class WowWmoLight(bpy.types.PropertyGroup):
    index: IntProperty()
    type: IntProperty()
    use_atten: IntProperty()
    color: IntVectorProperty(size=4)
    intensity: FloatProperty()
    atten_start: FloatProperty()
    atten_end: FloatProperty()
    rotation: FloatVectorProperty(size=4, description="x y z w")


class WowWmoDoodad(bpy.types.PropertyGroup):
    index: IntProperty(default=-1)
    doodad_set: IntProperty()
    fdid: IntProperty()
    path: StringProperty()
    name_index: IntProperty()
    flags: IntProperty()
    color: IntVectorProperty(size=4)
    scale: FloatProperty(default=1.0)


class WowWmoLiquid(bpy.types.PropertyGroup):
    group_index: IntProperty()
    material_id: IntProperty()
    verts_dim: IntVectorProperty(size=2)
    tiles_dim: IntVectorProperty(size=2)
    base_coords: FloatVectorProperty(size=3)


class WowWmoFog(bpy.types.PropertyGroup):
    index: IntProperty()
    flags: IntProperty()
    larger_radius: FloatProperty()
    smaller_radius: FloatProperty()
    fog_end: FloatProperty()
    fog_start_scalar: FloatProperty()
    fog_color: IntVectorProperty(size=4)
    uw_end: FloatProperty()
    uw_start_scalar: FloatProperty()
    uw_color: IntVectorProperty(size=4)


class WowM2Texture(bpy.types.PropertyGroup):
    type: IntProperty()
    flags: IntProperty()
    fdid: IntProperty()


class WowM2Material(bpy.types.PropertyGroup):
    flags: IntProperty()
    blending_mode: IntProperty()


class WowM2Sequence(bpy.types.PropertyGroup):
    anim_id: IntProperty()
    variation_index: IntProperty()
    variation_next: IntProperty(default=-1)
    alias_next: IntProperty()
    flags: IntProperty()
    duration: IntProperty(description="Milliseconds")
    frequency: IntProperty()
    blend_time_in: IntProperty()
    blend_time_out: IntProperty()
    movespeed: FloatProperty()
    replay_min: IntProperty()
    replay_max: IntProperty()
    bounds_min: FloatVectorProperty(size=3)
    bounds_max: FloatVectorProperty(size=3)
    bounds_radius: FloatProperty()
    padding: IntProperty()
    action: PointerProperty(type=bpy.types.Action)


class WowM2Loop(bpy.types.PropertyGroup):
    timestamp: IntProperty(description="Milliseconds")
    action: PointerProperty(type=bpy.types.Action)


def _seq_changed(props):
    from . import m2_seq
    m2_seq.on_active(props)


class WowM2(bpy.types.PropertyGroup):
    path: StringProperty()
    template: StringProperty(description="Model the unedited data is read from; empty for imported models")
    is_new: BoolProperty()
    fdid: IntProperty()
    name: StringProperty()
    format_version: IntProperty()
    global_flags: IntProperty()
    skin_index: IntProperty()
    skin_fdids: IntVectorProperty(size=4)
    skin_count: IntProperty()
    display_id: IntProperty(default=-1, description="-1 = none")
    sequence_count: IntProperty()
    bounding_min: FloatVectorProperty(size=3)
    bounding_max: FloatVectorProperty(size=3)
    bounding_radius: FloatProperty()
    collision_min: FloatVectorProperty(size=3)
    collision_max: FloatVectorProperty(size=3)
    collision_radius: FloatProperty()
    textures: CollectionProperty(type=WowM2Texture)
    materials: CollectionProperty(type=WowM2Material)
    sequences: CollectionProperty(type=WowM2Sequence)
    loops: CollectionProperty(type=WowM2Loop)
    active_sequence: IntProperty(default=-1, update=lambda self, ctx: _seq_changed(self))


class WowM2Submesh(bpy.types.PropertyGroup):
    index: IntProperty(default=-1)
    skin_section_id: IntProperty()
    level: IntProperty()
    bone_combo_index: IntProperty()
    bone_count: IntProperty()
    bone_influences: IntProperty()
    center_bone_index: IntProperty()
    center_position: FloatVectorProperty(size=3)
    sort_center_position: FloatVectorProperty(size=3)
    sort_radius: FloatProperty()


class WowM2Bone(bpy.types.PropertyGroup):
    index: IntProperty(default=-1)
    key_bone_id: IntProperty()
    flags: IntProperty()
    parent_bone: IntProperty()
    submesh_id: IntProperty()
    bone_name_crc: IntProperty()
    pivot: FloatVectorProperty(size=3)


class WowM2Batch(bpy.types.PropertyGroup):
    flags: IntProperty()
    priority_plane: IntProperty()
    shader_id: IntProperty()
    skin_section_index: IntProperty()
    geoset_index: IntProperty()
    color_index: IntProperty()
    material_index: IntProperty()
    material_layer: IntProperty()
    texture_count: IntProperty()
    texture_combo_index: IntProperty()
    texture_coord_combo_index: IntProperty()
    texture_weight_combo_index: IntProperty()
    texture_transform_combo_index: IntProperty()
    batch_index: IntProperty()
    render_flags: IntProperty()
    blending_mode: IntProperty()
    texture_indices: IntVectorProperty(size=2, default=(-1, -1))
    texture_types: IntVectorProperty(size=2)
    texture_fdids: IntVectorProperty(size=2)
    uv_sets: IntVectorProperty(size=2, description="-1 = environment map")
    preview_color: FloatVectorProperty(size=4, default=(1.0, 1.0, 1.0, 1.0))
    preview_weight: FloatProperty(default=1.0)
    preview_uv_offset: FloatVectorProperty(size=2)
    preview_uv_scale: FloatVectorProperty(size=2, default=(1.0, 1.0))


class WowM2Object(bpy.types.PropertyGroup):
    kind: StringProperty()
    index: IntProperty(default=-1)
    id: IntProperty()
    identifier: StringProperty()
    data: IntProperty()
    bone: IntProperty()
    flags: IntProperty()
    position: FloatVectorProperty(size=3)
    target: FloatVectorProperty(size=3)
    animated: BoolProperty()


class WowM2RampKey(bpy.types.PropertyGroup):
    time: IntProperty(description="Lifetime position, 0..32767")
    value: FloatVectorProperty(size=3)


class WowM2Particle(bpy.types.PropertyGroup):
    flags: IntProperty()
    emitter_type: IntProperty(default=1, description="1 plane, 2 sphere, 3 spline")
    blending_type: IntProperty(default=4)
    texture_slots: IntVectorProperty(size=3, description="M2 texture indices (packed 5 bits each)")
    rows: IntProperty(default=1, min=1)
    columns: IntProperty(default=1, min=1)
    priority_plane: IntProperty()
    particle_color_index: IntProperty()
    tail_length: FloatProperty()
    drag: FloatProperty()
    base_spin: FloatProperty()
    base_spin_variation: FloatProperty()
    spin_speed: FloatProperty()
    spin_speed_variation: FloatProperty()
    tumble_min: FloatVectorProperty(size=3)
    tumble_max: FloatVectorProperty(size=3)
    wind_vector: FloatVectorProperty(size=3)
    wind_time: FloatProperty()
    follow_speed1: FloatProperty()
    follow_speed2: FloatProperty()
    follow_scale1: FloatProperty()
    follow_scale2: FloatProperty()
    twinkle_speed: FloatProperty()
    twinkle_percent: FloatProperty(default=1.0)
    twinkle_scale: FloatVectorProperty(size=2, default=(1.0, 1.0), description="min, max")
    inherit_velocity_scale: FloatProperty()
    emission_rate_variation: FloatProperty()
    lifespan_variation: FloatProperty()
    scale_vary: FloatVectorProperty(size=2)
    color_keys: CollectionProperty(type=WowM2RampKey)
    alpha_keys: CollectionProperty(type=WowM2RampKey)
    scale_keys: CollectionProperty(type=WowM2RampKey)


class WowM2Index(bpy.types.PropertyGroup):
    value: IntProperty()


class WowM2Ribbon(bpy.types.PropertyGroup):
    texture_indices: CollectionProperty(type=WowM2Index)
    material_indices: CollectionProperty(type=WowM2Index)
    edges_per_second: FloatProperty(default=20.0)
    edge_lifetime: FloatProperty(default=1.0)
    gravity: FloatProperty()
    texture_rows: IntProperty(default=1, min=1)
    texture_cols: IntProperty(default=1, min=1)
    priority_plane: IntProperty()
    ribbon_color_index: IntProperty()
    texture_transform_lookup_index: IntProperty()


class WowImportOptions(bpy.types.PropertyGroup):
    doodads: BoolProperty(name="Doodads", default=True)
    lights: BoolProperty(name="Lights", default=True)
    portals: BoolProperty(name="Portals", default=True)
    liquids: BoolProperty(name="Liquids", default=True)
    group_filter: bpy.props.EnumProperty(
        name="Groups", default="ALL",
        items=[("ALL", "All", ""), ("EXTERIOR", "Exterior", ""), ("INTERIOR", "Interior", "")])
    doodad_meshes: BoolProperty(name="Doodad meshes", description="Show doodads as M2 instances", default=False)
    skin: IntProperty(name="Skin", min=0, default=0)
    display: bpy.props.EnumProperty(
        name="Display", default="AUTO",
        items=[("AUTO", "Auto", "First creature display using the model"), ("NONE", "None", ""),
               ("ID", "Display ID", "")])
    display_id: IntProperty(name="Display ID", min=0, default=0)
    attachments: BoolProperty(name="Attachments", default=True)
    events: BoolProperty(name="Events", default=True)
    cameras: BoolProperty(name="Cameras", default=True)
    m2_lights: BoolProperty(name="Lights", default=True)
    emitters: BoolProperty(name="Emitters", default=True)
    collision: BoolProperty(name="Collision", default=True)


classes = (WowM2Texture, WowM2Material, WowM2Sequence, WowM2Loop, WowM2, WowM2Submesh, WowM2Bone, WowM2Batch, WowM2Object, WowM2RampKey, WowM2Particle, WowM2Index, WowM2Ribbon, WowWmoMaterial, WowWmo, WowPortalRef, WowWmoGroup, WowWmoPortal, WowWmoLight, WowWmoDoodad,
           WowWmoLiquid, WowWmoFog, WowImportOptions)

_attach = (
    (bpy.types.Material, "wow_wmo_material", WowWmoMaterial),
    (bpy.types.Collection, "wow_wmo", WowWmo),
    (bpy.types.Object, "wow_wmo_group", WowWmoGroup),
    (bpy.types.Object, "wow_wmo_portal", WowWmoPortal),
    (bpy.types.Object, "wow_wmo_light", WowWmoLight),
    (bpy.types.Object, "wow_wmo_doodad", WowWmoDoodad),
    (bpy.types.Object, "wow_wmo_liquid", WowWmoLiquid),
    (bpy.types.Object, "wow_wmo_fog", WowWmoFog),
    (bpy.types.WindowManager, "wow_import", WowImportOptions),
    (bpy.types.Collection, "wow_m2", WowM2),
    (bpy.types.Object, "wow_m2_submesh", WowM2Submesh),
    (bpy.types.Bone, "wow_m2_bone", WowM2Bone),
    (bpy.types.Object, "wow_m2_object", WowM2Object),
    (bpy.types.Object, "wow_m2_particle", WowM2Particle),
    (bpy.types.Object, "wow_m2_ribbon", WowM2Ribbon),
)


def register():
    bpy.types.Material.wow_m2_batches = CollectionProperty(type=WowM2Batch)
    bpy.types.Object.wow_source = StringProperty()
    for owner, name, cls in _attach:
        setattr(owner, name, PointerProperty(type=cls))


def unregister():
    del bpy.types.Material.wow_m2_batches, bpy.types.Object.wow_source
    for owner, name, _ in reversed(_attach):
        delattr(owner, name)
