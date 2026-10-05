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
        if wm.wow_recent:
            col.label(text="Recent")
            for r in wm.wow_recent:
                op = col.operator("wow.import_selected", text=r.name, emboss=False)
                op.path = r.name


classes = (WowRecent, WOW_PT_browser)


def register():
    wm = bpy.types.WindowManager
    wm.wow_query = StringProperty(name="Search", description="Path fragment or FDID", search=_search)
    wm.wow_kind = EnumProperty(items=[("all", "All", ""), ("wmo", "WMO", ""), ("m2", "M2", "")], default="wmo")
    wm.wow_recent = CollectionProperty(type=WowRecent)


def unregister():
    wm = bpy.types.WindowManager
    del wm.wow_query, wm.wow_kind, wm.wow_recent
