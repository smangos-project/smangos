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


def draw_flags(layout, owner_path, group, attr, names=None, bits=32):
    """Toggle buttons for the bits of an int property; `names` maps bit -> label, other bits show their number."""
    names = names or {}
    box = layout.box()
    box.label(text=f"{attr} 0x{getattr(group, attr) & 0xFFFFFFFF:x}")
    grid = box.grid_flow(columns=2, align=True)
    for bit in range(bits):
        op = grid.operator("wow.toggle_flag", text=names.get(bit, f"bit {bit}"),
                           depress=bool((getattr(group, attr) >> bit) & 1))
        op.owner, op.attr, op.bit = owner_path, attr, bit


def draw_group(layout, owner_path, group, skip=(), flags=None):
    """Every property of a PropertyGroup; lists of sub-groups are drawn as boxes. `flags` maps attr -> {bit: label}."""
    flags = flags or {}
    for prop in group.bl_rna.properties:
        name = prop.identifier
        if name in ("rna_type", "name") or name in skip:
            continue
        if name in flags:
            draw_flags(layout, owner_path, group, name, flags[name])
        elif prop.type == "COLLECTION":
            box = layout.box()
            box.label(text=prop.name or name)
            for i, item in enumerate(getattr(group, name)):
                row = box.row(align=True)
                for sub in item.bl_rna.properties:
                    if sub.identifier not in ("rna_type", "name"):
                        row.prop(item, sub.identifier, text="")
        else:
            layout.prop(group, name)


def _in_m2(ob):
    from . import m2_export, m2_mesh_export
    return m2_export.find_root(ob) is not None and not m2_mesh_export.is_collision(ob)


def active_groups(ob):
    """(attribute, kind label) of the wow_* groups that apply to object `ob`."""
    out = []
    m2k = ob.wow_m2_object.kind
    if m2k:
        out.append(("wow_m2_object", m2k))
        if m2k == "particle":
            out.append(("wow_m2_particle", "particle"))
        elif m2k == "ribbon":
            out.append(("wow_m2_ribbon", "ribbon"))
    if ob.wow_wmo_group.index >= 0 and ob.type == "MESH":
        out.append(("wow_wmo_group", "WMO group"))
    if ob.wow_wmo_doodad.index >= 0 or ob.wow_wmo_doodad.fdid:
        out.append(("wow_wmo_doodad", "WMO doodad"))
    if ob.type == "LIGHT" and not m2k and ob.wow_wmo_light.type >= 0 and ob.wow_wmo_light.index >= 0:
        out.append(("wow_wmo_light", "WMO light"))
    if ob.name.startswith("portal_"):
        out.append(("wow_wmo_portal", "WMO portal"))
    if ob.name.startswith("fog_"):
        out.append(("wow_wmo_fog", "WMO fog"))
    if ob.wow_wmo_liquid.verts_dim[0]:
        out.append(("wow_wmo_liquid", "WMO liquid"))
    if ob.type == "MESH" and (ob.wow_m2_submesh.index >= 0 or _in_m2(ob)):
        out.append(("wow_m2_submesh", "M2 submesh"))
    return out


class WOW_PT_object(bpy.types.Panel):
    bl_label = "WoW properties"
    bl_space_type, bl_region_type, bl_category = "VIEW_3D", "UI", "WoW"

    @classmethod
    def poll(cls, context):
        return context.active_object is not None and bool(active_groups(context.active_object))

    def draw(self, context):
        ob = context.active_object
        for attr, label in active_groups(ob):
            col = self.layout.column()
            col.label(text=label)
            skip = {"wow_m2_object": ("kind", "animated", "position", "target", "index"),
                    "wow_m2_submesh": ("index",) if ob.wow_m2_submesh.index < 0 else ()}.get(attr, ())
            if attr == "wow_m2_submesh" and ob.wow_m2_submesh.index < 0:
                skip += ("level", "bone_combo_index", "bone_count", "bone_influences", "center_bone_index",
                         "center_position", "sort_center_position", "sort_radius")
            draw_group(col, attr, getattr(ob, attr), skip, FLAGS.get(attr))
            if attr == "wow_m2_object" and ob.type == "LIGHT":
                col.operator("wow.m2_light_from_blender", icon="LIGHT")


FLAGS = {"wow_m2_particle": {"flags": {}}, "wow_wmo_group": {"flags": {}, "flags2": {}}}


WMO_MATERIAL_FLAGS = {0: "Unlit", 1: "Unfogged", 2: "Two-sided", 3: "Exterior light", 4: "Night glow", 5: "Window", 6: "Clamp S", 7: "Clamp T"}


class WOW_PT_wmo_material(bpy.types.Panel):
    bl_label = "WoW WMO material"
    bl_space_type, bl_region_type, bl_context = "PROPERTIES", "WINDOW", "material"

    @classmethod
    def poll(cls, context):
        m = context.material
        return m is not None and context.active_object is not None and \
            (m.wow_wmo_material.material_index >= 0 or bool(m.wow_wmo_material.texture1))

    def draw(self, context):
        p = context.material.wow_wmo_material
        draw_group(self.layout, "active_material.wow_wmo_material", p, ("material_index", "run_time_data"), {"flags": WMO_MATERIAL_FLAGS, "flags2": {}})


class WOW_PT_collection(bpy.types.Panel):
    bl_label = "WoW model"
    bl_space_type, bl_region_type, bl_context = "PROPERTIES", "WINDOW", "collection"

    @classmethod
    def poll(cls, context):
        c = context.collection
        return c is not None and bool(c.wow_wmo.path or c.wow_m2.path)

    def draw(self, context):
        c = context.collection
        if c.wow_wmo.path:
            draw_group(self.layout, "", c.wow_wmo)
        if c.wow_m2.path:
            draw_group(self.layout, "", c.wow_m2, skip=("sequences", "loops", "active_sequence", "textures", "materials"))
            self.layout.operator("wow.m2_recompute_bounds", icon="MESH_DATA")


classes = (WowRecent, WOW_PT_object, WOW_PT_wmo_material, WOW_PT_collection, WOW_PT_browser, WOW_PT_m2, WOW_PT_m2_report, WOW_UL_sequences, WOW_PT_m2_sequences)


def register():
    wm = bpy.types.WindowManager
    wm.wow_query = StringProperty(name="Search", description="Path fragment or FDID", search=_search)
    wm.wow_kind = EnumProperty(items=[("all", "All", ""), ("wmo", "WMO", ""), ("m2", "M2", "")], default="wmo")
    wm.wow_recent = CollectionProperty(type=WowRecent)


def unregister():
    wm = bpy.types.WindowManager
    del wm.wow_query, wm.wow_kind, wm.wow_recent
