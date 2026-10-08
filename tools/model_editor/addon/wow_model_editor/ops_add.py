"""Add menu entries that create correctly tagged model objects, and small property operators."""
import bpy
from bpy.props import BoolProperty, EnumProperty, IntProperty, StringProperty

from . import m2_export, m2_particles

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


class WOW_MT_add(bpy.types.Menu):
    bl_idname = "WOW_MT_add"
    bl_label = "WoW"

    def draw(self, context):
        for kind, label, _ in M2_KINDS:
            self.layout.operator("wow.add_m2_object", text=label).kind = kind


def _menu(self, context):
    self.layout.menu("WOW_MT_add")


classes = (WOW_OT_add_m2_object, WOW_OT_toggle_flag, WOW_OT_m2_light_from_blender, WOW_MT_add)


def register():
    bpy.types.VIEW3D_MT_add.append(_menu)


def unregister():
    bpy.types.VIEW3D_MT_add.remove(_menu)
