"""Sidebar browser."""
import bpy
from bpy.props import CollectionProperty, EnumProperty, StringProperty

from . import client, index


def _search(self, context, edit_text):
    try:
        s = client.session()
        index.load(s.content, client.prefs().cache_dir)
    except Exception:  # noqa: BLE001
        return []
    return [e[1] for e in index.search(edit_text, context.window_manager.wow_kind, 200)]


class WowRecent(bpy.types.PropertyGroup):
    pass


class WOW_PT_browser(bpy.types.Panel):
    bl_label = "WoW models"
    bl_space_type, bl_region_type, bl_category = "VIEW_3D", "UI", "WoW"

    def draw(self, context):
        wm = context.window_manager
        col = self.layout.column()
        if not client.prefs().content_dir:
            col.label(text="Set the Content directory in preferences", icon="ERROR")
            col.operator("preferences.addon_show").module = __package__
            return
        col.prop(wm, "wow_kind", expand=True)
        col.prop(wm, "wow_query", text="")
        fdid = index.fdid_of(wm.wow_query) if index._state["key"] else None
        col.label(text=f"FDID {fdid}" if fdid else "")
        col.operator("wow.import_selected", icon="IMPORT")
        o = wm.wow_import
        box = col.box()
        box.prop(o, "doodads")
        box.prop(o, "lights")
        box.prop(o, "portals")
        box.prop(o, "liquids")
        box.prop(o, "group_filter")
        box.prop(o, "doodad_meshes")
        if wm.wow_kind == "m2" or wm.wow_query.endswith(".m2"):
            box = col.box()
            box.label(text="M2")
            box.prop(o, "skin")
            box.prop(o, "display")
            if o.display == "ID":
                box.prop(o, "display_id")
            for name in ("attachments", "events", "cameras", "m2_lights", "emitters", "collision"):
                box.prop(o, name)
        if wm.wow_recent:
            col.label(text="Recent")
            for r in wm.wow_recent:
                op = col.operator("wow.import_selected", text=r.name, emboss=False)
                op.path = r.name


class WOW_PT_m2(bpy.types.Panel):
    bl_label = "WoW M2"
    bl_space_type, bl_region_type, bl_category = "VIEW_3D", "UI", "WoW"

    @classmethod
    def poll(cls, context):
        from . import m2_import
        return m2_import.root_collection(context.active_object) is not None

    def draw(self, context):
        from . import displays, m2_import
        top = m2_import.root_collection(context.active_object)
        p = top.wow_m2
        col = self.layout.column()
        col.label(text=f"{p.path} (FDID {p.fdid})")
        col.label(text=f"Skin {p.skin_index + 1}/{p.skin_count}, {p.sequence_count} sequences")
        try:
            listed = displays.for_model(p.fdid)
        except Exception:  # noqa: BLE001
            listed = []
        if listed:
            col.label(text="Display")
            op = col.operator("wow.m2_set_display", text="None", depress=p.display_id < 0)
            op.display_id = -1
            for d in listed[:24]:
                op = col.operator("wow.m2_set_display", text=str(d.id), depress=d.id == p.display_id)
                op.display_id = d.id


classes = (WowRecent, WOW_PT_browser, WOW_PT_m2)


def register():
    wm = bpy.types.WindowManager
    wm.wow_query = StringProperty(name="Search", description="Path fragment or FDID", search=_search)
    wm.wow_kind = EnumProperty(items=[("all", "All", ""), ("wmo", "WMO", ""), ("m2", "M2", "")], default="wmo")
    wm.wow_recent = CollectionProperty(type=WowRecent)


def unregister():
    wm = bpy.types.WindowManager
    del wm.wow_query, wm.wow_kind, wm.wow_recent
