"""Add menu entries that create correctly tagged model objects, and small property operators."""
import bpy
from bpy.props import BoolProperty, EnumProperty, FloatProperty, IntProperty, StringProperty

from . import m2_camera_bake, m2_export, m2_particles, wmo_export, wmo_lod

M2_KINDS = (("attachment", "Attachment", ""), ("event", "Event", ""), ("light", "Light", ""), ("camera", "Camera", ""),
            ("particle", "Particle emitter", ""), ("ribbon", "Ribbon emitter", ""))
GROUPS = {"attachment": "attachments", "event": "events", "light": "lights", "camera": "cameras",
          "particle": "emitters", "ribbon": "emitters"}


def _child(top, name):
    c = next((c for c in top.children if c.name.split(".")[0].endswith(name)), None)
    if c is None:
        c = bpy.data.collections.new(name)
        top.children.link(c)
    return c


EMITTER_DEFAULTS = {
    "particle": {"emission_rate": 10.0, "lifespan": 1.0, "emission_speed": 1.0, "emission_area_length": 0.5,
                 "emission_area_width": 0.5, "enabled_in": 1.0},
    "ribbon": {"height_above": 0.2, "height_below": 0.2},
}


def new_m2_object(top, kind, location=(0.0, 0.0, 0.0)):
    """Create and link a new `kind` object of M2 model `top`."""
    coll = _child(top, GROUPS[kind])
    if kind == "light":
        ob = bpy.data.objects.new("light_new", bpy.data.lights.new("light_new", "POINT"))
    elif kind == "camera":
        ob = bpy.data.objects.new("camera_new", bpy.data.cameras.new("camera_new"))
    else:
        display, size = {"attachment": ("PLAIN_AXES", 0.1), "event": ("SPHERE", 0.05), "particle": ("CUBE", 0.1),
                         "ribbon": ("CIRCLE", 0.1)}[kind]
        ob = bpy.data.objects.new(f"{kind}_new", None)
        ob.empty_display_type, ob.empty_display_size = display, size
    coll.objects.link(ob)
    ob.location = location
    ob.wow_source = top.wow_m2.template or top.wow_m2.path
    p = ob.wow_m2_object
    p.kind = kind
    if kind in ("particle", "ribbon", "camera"):
        p.id = -1 if kind != "camera" else 0
    if kind == "event":
        p.identifier = "$AH1"
    if kind == "particle":
        m2_particles.set_defaults(ob.wow_m2_particle)
        for name, v in EMITTER_DEFAULTS["particle"].items():
            ob[f"wow_{name}"] = v
    if kind == "ribbon":
        for name, v in EMITTER_DEFAULTS["ribbon"].items():
            ob[f"wow_{name}"] = v
        ob.wow_m2_ribbon.edges_per_second, ob.wow_m2_ribbon.edge_lifetime = 30.0, 0.5
        if len(top.wow_m2.textures):
            ob.wow_m2_ribbon.texture_indices.add().value = 0
        if len(top.wow_m2.materials):
            ob.wow_m2_ribbon.material_indices.add().value = 0
    if kind == "camera":
        tgt = bpy.data.objects.new("camera_new_target", None)
        tgt.empty_display_type, tgt.empty_display_size = "PLAIN_AXES", 0.1
        tgt.location = (location[0], location[1] + 10.0, location[2])
        coll.objects.link(tgt)
        con = ob.constraints.new("TRACK_TO")
        con.target, con.track_axis, con.up_axis = tgt, "TRACK_NEGATIVE_Z", "UP_Y"
    return ob


class WOW_OT_add_m2_object(bpy.types.Operator):
    bl_idname = "wow.add_m2_object"
    bl_label = "WoW M2 object"
    bl_options = {"REGISTER", "UNDO"}
    kind: EnumProperty(items=M2_KINDS)

    @classmethod
    def poll(cls, context):
        ob = context.active_object
        return ob is not None and m2_export.find_root(ob) is not None

    def execute(self, context):
        top = m2_export.find_root(context.active_object)
        ob = new_m2_object(top, self.kind, tuple(context.scene.cursor.location))
        for o in context.selected_objects:
            o.select_set(False)
        ob.select_set(True)
        context.view_layer.objects.active = ob
        return {"FINISHED"}


WMO_KINDS = (("doodad", "Doodad", ""), ("light", "Light", ""), ("fog", "Fog", ""), ("portal", "Portal", ""),
             ("group", "Group", ""), ("antiportal", "Antiportal", ""), ("liquid", "Liquid", ""))
WMO_GROUPS = {"doodad": "doodads", "light": "lights", "fog": "fogs", "portal": "portals", "group": "groups",
              "antiportal": "groups", "liquid": "liquids"}
ANTIPORTAL_FLAGS = 0x4000000 | 0x80 | 0x8


def new_wmo_liquid(top, group_ob, tiles=(8, 8), location=(0.0, 0.0, 0.0)):
    """A flat liquid grid for the group of `group_ob`, anchored at `location`; exported as the group's new liquid."""
    import numpy as np
    from .wmo_import import UNIT_SIZE
    gi = group_ob.wow_wmo_group.index
    xv, yv = tiles[0] + 1, tiles[1] + 1
    gx, gy = np.meshgrid(np.arange(xv), np.arange(yv))
    co = np.stack([location[0] + gx.ravel() * UNIT_SIZE, location[1] + gy.ravel() * UNIT_SIZE,
                   np.full(xv * yv, location[2])], axis=1)
    v0 = (np.arange(tiles[1])[:, None] * xv + np.arange(tiles[0])[None, :]).ravel()
    me = bpy.data.meshes.new(f"liquid_{gi:03d}")
    me.from_pydata(co.tolist(), [], np.stack([v0, v0 + 1, v0 + xv + 1, v0 + xv], axis=1).tolist())
    ob = bpy.data.objects.new(me.name, me)
    ob.display_type = "WIRE"
    _child(top, "liquids").objects.link(ob)
    p = ob.wow_wmo_liquid
    p.group_index, p.verts_dim, p.tiles_dim, p.base_coords = gi, (xv, yv), tiles, location
    return ob


def new_wmo_object(top, kind, location=(0.0, 0.0, 0.0), path="", doodad_set=0):
    """Create and link a new `kind` object of WMO `top`; doodads need a model path."""
    coll = _child(top, WMO_GROUPS[kind])
    if kind == "doodad":
        if coll.children:
            coll = coll.children[min(doodad_set, len(coll.children) - 1)]
        ob = bpy.data.objects.new(path.rsplit("/", 1)[-1].rsplit(".", 1)[0] or "doodad", None)
        ob.empty_display_type, ob.empty_display_size = "ARROWS", 0.5
        ob.wow_wmo_doodad.path = path
        ob.wow_wmo_doodad.doodad_set = doodad_set
    elif kind == "light":
        ob = bpy.data.objects.new("light_new", bpy.data.lights.new("light_new", "POINT"))
        ob.data.use_custom_distance, ob.data.cutoff_distance, ob.data.energy = True, 10.0, 100.0
        ob.wow_wmo_light.atten_end = 10.0
    elif kind == "fog":
        ob = bpy.data.objects.new("fog_new", None)
        ob.empty_display_type, ob.empty_display_size = "SPHERE", 10.0
        p = ob.wow_wmo_fog
        p.larger_radius, p.smaller_radius, p.fog_end, p.fog_start_scalar = 10.0, 5.0, 100.0, 0.1
        p.fog_color = p.uw_color = (128, 128, 128, 255)
        p.uw_end, p.uw_start_scalar = 100.0, 0.1
    elif kind == "portal":
        me = bpy.data.meshes.new("portal_new")
        me.from_pydata([(-1, 0, 0), (1, 0, 0), (1, 0, 2), (-1, 0, 2)], [], [[0, 1, 2, 3]])
        ob = bpy.data.objects.new("portal_new", me)
        ob.display_type = "WIRE"
    elif kind == "antiportal":
        me = bpy.data.meshes.new("antiportal_new")
        me.from_pydata([(-2, 0, 0), (2, 0, 0), (2, 0, 4), (-2, 0, 4)], [], [[0, 1, 2, 3]])
        ob = bpy.data.objects.new("antiportal_new", me)
        ob.display_type = "WIRE"
        ob.wow_wmo_group.flags = ANTIPORTAL_FLAGS
        ob.wow_wmo_group.group_name = "antiportal"
    else:
        me = bpy.data.meshes.new("group_new")
        me.from_pydata([(-5, -5, 0), (5, -5, 0), (5, 5, 0), (-5, 5, 0)], [], [[0, 1, 2, 3]])
        ob = bpy.data.objects.new("group_new", me)
    coll.objects.link(ob)
    ob.location = location
    return ob


class WOW_OT_add_wmo_object(bpy.types.Operator):
    bl_idname = "wow.add_wmo_object"
    bl_label = "WoW WMO object"
    bl_options = {"REGISTER", "UNDO"}
    kind: EnumProperty(items=WMO_KINDS)
    path: StringProperty(name="Model path", description="Doodad M2 path, e.g. world/generic/human/passive doodads/barrel01.m2")

    @classmethod
    def poll(cls, context):
        ob = context.active_object
        return ob is not None and wmo_export.find_root(ob) is not None

    def invoke(self, context, event):
        if self.kind == "doodad":
            return context.window_manager.invoke_props_dialog(self)
        return self.execute(context)

    def execute(self, context):
        top = wmo_export.find_root(context.active_object)
        if self.kind == "doodad" and not self.path:
            self.report({"ERROR"}, "Doodads need a model path")
            return {"CANCELLED"}
        if self.kind == "liquid":
            g = context.active_object
            if g.type != "MESH" or g.wow_wmo_group.index < 0:
                self.report({"ERROR"}, "Select an existing group mesh first")
                return {"CANCELLED"}
            ob = new_wmo_liquid(top, g, location=tuple(context.scene.cursor.location))
        else:
            ob = new_wmo_object(top, self.kind, tuple(context.scene.cursor.location), self.path.replace("\\", "/").lower())
        for o in context.selected_objects:
            o.select_set(False)
        ob.select_set(True)
        context.view_layer.objects.active = ob
        return {"FINISHED"}


class WOW_OT_toggle_flag(bpy.types.Operator):
    bl_idname = "wow.toggle_flag"
    bl_label = "Toggle flag"
    bl_options = {"UNDO", "INTERNAL"}
    owner: StringProperty(description="Path from the active object to the property group")
    attr: StringProperty()
    bit: IntProperty()

    def execute(self, context):
        group = context.active_object.path_resolve(self.owner) if self.owner else context.active_object
        v = int(getattr(group, self.attr)) ^ (1 << self.bit)
        setattr(group, self.attr, v - (1 << 32) if v >= 1 << 31 else v)
        return {"FINISHED"}


class WOW_OT_m2_recompute_bounds(bpy.types.Operator):
    bl_idname = "wow.m2_recompute_bounds"
    bl_label = "Recompute bounds"
    bl_description = "Set the model bounding box and radius from its submesh geometry"
    bl_options = {"UNDO"}

    @classmethod
    def poll(cls, context):
        return context.collection is not None and bool(context.collection.wow_m2.path)

    def execute(self, context):
        import numpy as np
        from . import m2_mesh_export as mx
        top = context.collection
        arm = mx.find_armature(top)
        pts = []
        for ob in sum(mx.submesh_objects(top), []):
            n = len(ob.data.vertices)
            if not n:
                continue
            co = np.empty(n * 3, np.float32)
            ob.data.vertices.foreach_get("co", co)
            mat = np.asarray(ob.matrix_world if arm is None else arm.matrix_world.inverted() @ ob.matrix_world)
            pts.append(co.reshape(-1, 3) @ mat[:3, :3].T + mat[:3, 3])
        if not pts:
            self.report({"WARNING"}, "No submesh geometry")
            return {"CANCELLED"}
        lo, hi, radius = mx.bounds_of(np.concatenate(pts).astype(np.float32))
        p = top.wow_m2
        p.bounding_min, p.bounding_max, p.bounding_radius = lo, hi, radius
        return {"FINISHED"}


class WOW_OT_m2_light_from_blender(bpy.types.Operator):
    bl_idname = "wow.m2_light_from_blender"
    bl_label = "Set M2 light from Blender light"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        ob = context.active_object
        return ob is not None and ob.type == "LIGHT" and ob.wow_m2_object.kind == "light"

    def execute(self, context):
        ob = context.active_object
        d = ob.data
        ob["wow_diffuse_color"] = tuple(d.color)
        ob["wow_diffuse_intensity"] = d.energy / 100.0
        ob["wow_attenuation_end"] = d.cutoff_distance if d.use_custom_distance else 0.0
        return {"FINISHED"}


class WOW_OT_m2_bake_camera(bpy.types.Operator):
    bl_idname = "wow.m2_bake_camera"
    bl_label = "Bake camera path"
    bl_description = "Sample the camera (and its Track To target) over the active sequence into M2 camera spline keys"
    bl_options = {"REGISTER", "UNDO"}
    step: IntProperty(name="Step (ms)", default=100, min=1)

    @classmethod
    def poll(cls, context):
        ob = context.active_object
        top = m2_export.find_root(ob) if ob is not None else None
        return (ob is not None and ob.type == "CAMERA" and ob.wow_m2_object.kind == "camera" and top is not None
                and 0 <= top.wow_m2.active_sequence < len(top.wow_m2.sequences))

    def execute(self, context):
        ob = context.active_object
        top = m2_export.find_root(ob)
        seq = top.wow_m2.sequences[top.wow_m2.active_sequence]
        if seq.action is None:
            self.report({"ERROR"}, "active sequence has no action")
            return {"CANCELLED"}
        m2_camera_bake.bake(context.scene, ob, seq.action, 0, max(int(seq.duration), 1), self.step)
        return {"FINISHED"}


class WOW_OT_wmo_mark_collision(bpy.types.Operator):
    bl_idname = "wow.wmo_mark_collision"
    bl_label = "Mark collision only"
    bl_description = "Assign the selected faces to the collision material: no render batch; poly flags reset to the collision default"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        ob = context.active_object
        return ob is not None and ob.type == "MESH" and context.mode == "EDIT_MESH"

    def execute(self, context):
        import bmesh
        from . import materials
        ob = context.active_object
        me = ob.data
        mat = materials.collision_material()
        if mat.name not in [s.material.name for s in ob.material_slots if s.material]:
            me.materials.append(mat)
        idx = next(i for i, s in enumerate(ob.material_slots) if s.material == mat)
        bm = bmesh.from_edit_mesh(me)
        layer = bm.faces.layers.int.get("wow_poly_flags") or bm.faces.layers.int.new("wow_poly_flags")
        for f in bm.faces:
            if f.select:
                f.material_index = idx
                f[layer] = 0
        bmesh.update_edit_mesh(me)
        return {"FINISHED"}


LIQUID_TYPES = (("0", "Water", ""), ("1", "Ocean", ""), ("2", "Magma", ""), ("3", "Slime", ""))
TILE_TYPE_MASK, TILE_NO_RENDER, TILE_FISHABLE = 0x07, 0x08, 0x40


def apply_tile_type(flags, liquid_type, fishable, no_render):
    """Tile flag byte with the type, fishable and no-render bits replaced (`fishable`/`no_render` None keep them)."""
    f = flags & ~TILE_TYPE_MASK | liquid_type
    if fishable is not None:
        f = f | TILE_FISHABLE if fishable else f & ~TILE_FISHABLE
    if no_render is not None:
        f = f | TILE_NO_RENDER if no_render else f & ~TILE_NO_RENDER
    return f


class WOW_OT_wmo_set_liquid_tile(bpy.types.Operator):
    bl_idname = "wow.wmo_set_liquid_tile"
    bl_label = "Set liquid tile type"
    bl_description = "Set the liquid type of the selected tiles of a WMO liquid"
    bl_options = {"REGISTER", "UNDO"}
    liquid_type: EnumProperty(name="Type", items=LIQUID_TYPES)
    fishable: EnumProperty(name="Fishable", items=(("KEEP", "Keep", ""), ("YES", "Yes", ""), ("NO", "No", "")))
    no_render: EnumProperty(name="No liquid", items=(("KEEP", "Keep", ""), ("YES", "Yes", ""), ("NO", "No", "")))

    @classmethod
    def poll(cls, context):
        ob = context.active_object
        return ob is not None and ob.type == "MESH" and ob.wow_wmo_liquid.verts_dim[0] > 0

    def execute(self, context):
        import bmesh
        ob = context.active_object
        me = ob.data
        fish = {"KEEP": None, "YES": True, "NO": False}[self.fishable]
        hide = {"KEEP": None, "YES": True, "NO": False}[self.no_render]
        edit = ob.mode == "EDIT"
        if edit:
            bm = bmesh.from_edit_mesh(me)
            layer = bm.faces.layers.int.get("wow_tile_flags") or bm.faces.layers.int.new("wow_tile_flags")
            for f in bm.faces:
                if f.select:
                    f[layer] = apply_tile_type(f[layer], int(self.liquid_type), fish, hide)
                    if hide is not None:
                        f.hide_set(hide)
            bmesh.update_edit_mesh(me)
            return {"FINISHED"}
        if "wow_tile_flags" not in me.attributes:
            me.attributes.new("wow_tile_flags", "INT", "FACE")
        data = me.attributes["wow_tile_flags"].data
        for poly in me.polygons:
            if poly.select:
                data[poly.index].value = apply_tile_type(data[poly.index].value, int(self.liquid_type), fish, hide)
                if hide is not None:
                    poly.hide = hide
        return {"FINISHED"}


class WOW_OT_generate_wmo_lods(bpy.types.Operator):
    bl_idname = "wow.generate_wmo_lods"
    bl_label = "Generate LODs"
    bl_description = "Build LOD group objects for the selected WMO groups (all groups when none is selected) by decimating them"
    bl_options = {"REGISTER", "UNDO"}
    levels: IntProperty(name="Levels", min=1, max=3, default=3)
    ratio1: FloatProperty(name="LOD 1", min=0.01, max=1.0, default=0.6)
    ratio2: FloatProperty(name="LOD 2", min=0.01, max=1.0, default=0.35)
    ratio3: FloatProperty(name="LOD 3", min=0.01, max=1.0, default=0.2)
    min_vertices: IntProperty(name="Minimum vertices", min=0, default=200, description="Smaller groups get no LODs")
    merge_materials: BoolProperty(name="Merge materials", description="Give every LOD face the group's most used material", default=False)
    skip_interior: BoolProperty(name="Skip interior groups", default=True)

    @classmethod
    def poll(cls, context):
        ob = context.active_object
        return ob is not None and wmo_export.find_root(ob) is not None

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

    def execute(self, context):
        root = wmo_export.find_root(context.active_object)
        groups = [o for o in context.selected_objects if o.type == "MESH" and o.wow_wmo_group.index >= 0]
        groups = groups or sorted(wmo_export.group_objects(root).values(), key=lambda o: o.wow_wmo_group.index)
        ratios = [self.ratio1, self.ratio2, self.ratio3][:self.levels]
        made = wmo_lod.generate_lods(root, groups, ratios, self.min_vertices, self.merge_materials, self.skip_interior)
        self.report({"INFO"}, f"LODs built for {len(made)} of {len(groups)} groups")
        return {"FINISHED"}


class WOW_MT_add(bpy.types.Menu):
    bl_idname = "WOW_MT_add"
    bl_label = "WoW"

    def draw(self, context):
        ob = context.active_object
        if ob is not None and wmo_export.find_root(ob) is not None:
            for kind, label, _ in WMO_KINDS:
                self.layout.operator("wow.add_wmo_object", text=label).kind = kind
            return
        for kind, label, _ in M2_KINDS:
            self.layout.operator("wow.add_m2_object", text=label).kind = kind


def _menu(self, context):
    self.layout.menu("WOW_MT_add")


classes = (WOW_OT_add_m2_object, WOW_OT_add_wmo_object, WOW_OT_toggle_flag, WOW_OT_m2_recompute_bounds, WOW_OT_m2_light_from_blender, WOW_OT_m2_bake_camera, WOW_OT_wmo_mark_collision, WOW_OT_wmo_set_liquid_tile, WOW_OT_generate_wmo_lods, WOW_MT_add)


def register():
    bpy.types.VIEW3D_MT_add.append(_menu)


def unregister():
    bpy.types.VIEW3D_MT_add.remove(_menu)
