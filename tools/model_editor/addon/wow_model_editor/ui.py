"""Sidebar browser."""
import bpy
from bpy.props import CollectionProperty, EnumProperty, StringProperty

from . import client, index, m2_names


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
        col.operator("wow.import_item_display", icon="IMPORT")
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
        col.separator()
        col.operator("wow.new_wmo", icon="ADD")
        col.operator("wow.new_m2", icon="ADD")
        col.operator("wow.export_wmo", icon="EXPORT")
        col.operator("wow.install_model", icon="IMPORT")
        col.operator("wow.uninstall_model", icon="LOOP_BACK")
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
        col.operator("wow.export_m2", icon="EXPORT")
        col.operator("wow.install_model", icon="IMPORT")
        col.operator("wow.uninstall_model", icon="LOOP_BACK")
        col.operator("wow.write_game_data", icon="DISK_DRIVE")
        col.operator("wow.write_item", icon="DISK_DRIVE")
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


class WOW_PT_m2_report(bpy.types.Panel):
    bl_label = "Export report"
    bl_space_type, bl_region_type, bl_category = "VIEW_3D", "UI", "WoW"

    @classmethod
    def poll(cls, context):
        from . import m2_export
        r = m2_export.last_report
        return r is not None and any(i[0] != "INFO" for i in r.items)

    def draw(self, context):
        from . import m2_export
        r = m2_export.last_report
        col = self.layout.column(align=True)
        for i, (sev, where, msg) in enumerate(r.items):
            if sev == "INFO":
                continue
            op = col.operator("wow.select_finding", text=f"{where}: {msg}", emboss=False,
                              icon="ERROR" if sev == "ERROR" else "INFO")
            op.index = i


class WOW_UL_sequences(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_propname, index):
        alias = bool(item.flags & 0x40)
        layout.label(text=f"{index}: {m2_names.anim_name(item.anim_id)} {item.variation_index}", icon="ACTION" if not alias else "LINKED")
        layout.label(text=f"{item.duration} ms")
        layout.label(text=f"0x{item.flags:x}")


class WOW_PT_m2_sequences(bpy.types.Panel):
    bl_label = "Sequences"
    bl_space_type, bl_region_type, bl_category = "VIEW_3D", "UI", "WoW"

    @classmethod
    def poll(cls, context):
        from . import m2_import
        top = m2_import.root_collection(context.active_object)
        return top is not None

    def draw(self, context):
        from . import m2_import
        p = m2_import.root_collection(context.active_object).wow_m2
        col = self.layout.column()
        col.template_list("WOW_UL_sequences", "", p, "sequences", p, "active_sequence", rows=8)
        col.operator("wow.m2_play_sequence", icon="PLAY")
        row = col.row(align=True)
        row.operator("wow.m2_sequence_add", text="Add", icon="ADD")
        row.operator("wow.m2_sequence_duplicate", text="Duplicate", icon="DUPLICATE")
        row.operator("wow.m2_sequence_remove", text="Remove", icon="REMOVE")
        col.operator("wow.m2_sequence_key", icon="KEY_HLT")
        if 0 <= p.active_sequence < len(p.sequences):
            s = p.sequences[p.active_sequence]
            box = col.box()
            for name in ("anim_id", "variation_index", "alias_next", "flags", "duration", "frequency", "blend_time_in",
                         "blend_time_out", "movespeed", "action"):
                box.prop(s, name)
            box.label(text=f"Bounds radius {s.bounds_radius:.2f}")
        if p.loops:
            col.label(text="Global loops: " + ", ".join(f"{l.timestamp} ms" for l in p.loops))
        row = col.row(align=True)
        row.operator("wow.m2_loop_add", text="Add loop", icon="ADD")
        row.operator("wow.m2_loop_remove", text="Remove loop", icon="REMOVE")


classes = (WowRecent, WOW_PT_browser, WOW_PT_m2, WOW_PT_m2_report, WOW_UL_sequences, WOW_PT_m2_sequences)


def register():
    wm = bpy.types.WindowManager
    wm.wow_query = StringProperty(name="Search", description="Path fragment or FDID", search=_search)
    wm.wow_kind = EnumProperty(items=[("all", "All", ""), ("wmo", "WMO", ""), ("m2", "M2", "")], default="wmo")
    wm.wow_recent = CollectionProperty(type=WowRecent)


def unregister():
    wm = bpy.types.WindowManager
    del wm.wow_query, wm.wow_kind, wm.wow_recent
