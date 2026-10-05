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


class WowImportOptions(bpy.types.PropertyGroup):
    doodads: BoolProperty(name="Doodads", default=True)
    lights: BoolProperty(name="Lights", default=True)
    portals: BoolProperty(name="Portals", default=True)
    liquids: BoolProperty(name="Liquids", default=True)
    group_filter: bpy.props.EnumProperty(
        name="Groups", default="ALL",
        items=[("ALL", "All", ""), ("EXTERIOR", "Exterior", ""), ("INTERIOR", "Interior", "")])


classes = (WowWmoMaterial, WowWmo, WowPortalRef, WowWmoGroup, WowWmoPortal, WowWmoLight, WowWmoDoodad,
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
)


def register():
    for owner, name, cls in _attach:
        setattr(owner, name, PointerProperty(type=cls))


def unregister():
    for owner, name, _ in reversed(_attach):
        delattr(owner, name)
