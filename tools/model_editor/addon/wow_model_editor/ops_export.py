"""Export operators."""
import bpy
from bpy.props import BoolProperty, EnumProperty, FloatProperty, IntProperty, StringProperty

from pathlib import Path

from . import client, fdids, game_data, install, m2_export, wmo_export


class WOW_OT_export_wmo(bpy.types.Operator):
    bl_idname = "wow.export_wmo"
    bl_label = "Export WMO"
    bl_description = "Write the active WMO collection to the output directory"
    force: BoolProperty(name="Rebuild all groups", default=False)
    dry_run: BoolProperty(name="Validate only", default=False)

    @classmethod
    def poll(cls, context):
        return wmo_export.find_root(context.active_object or context.collection) is not None

    def execute(self, context):
        root = wmo_export.find_root(context.active_object or context.collection)
        try:
            r = wmo_export.export(root, options={"force": self.force, "dry_run": self.dry_run})
        except Exception as e:  # noqa: BLE001
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        for sev, where, msg in r.items:
            if sev != "INFO":
                self.report({sev}, f"{where}: {msg}")
        if r.errors:
            return {"CANCELLED"}
        s = r.stats
        self.report({"INFO"}, f"{root.wow_wmo.path}: {s['groups_rebuilt']} groups rebuilt, "
                              f"{s['files_written']} files written, {s['files_removed']} removed")
        return {"FINISHED"}


class WOW_OT_new_wmo(bpy.types.Operator):
    bl_idname = "wow.new_wmo"
    bl_label = "New WMO"
    bl_description = "Create an empty WMO collection; mesh objects in its groups collection become groups"
    path: StringProperty(name="Client path", default="world/wmo/custom/new.wmo")

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

    def execute(self, context):
        if not self.path.lower().endswith(".wmo"):
            self.report({"ERROR"}, "Path must end in .wmo")
            return {"CANCELLED"}
        wmo_export.new_wmo(self.path)
        return {"FINISHED"}


class WOW_OT_export_m2(bpy.types.Operator):
    bl_idname = "wow.export_m2"
    bl_label = "Export M2"
    bl_description = "Write the active M2 collection to the output directory"
    force: BoolProperty(name="Force write", description="Write even when nothing changed", default=False)
    dry_run: BoolProperty(name="Validate only", default=False)
    write_anyway: BoolProperty(name="Write anyway", description="Write although validation reports errors", default=False)

    @classmethod
    def poll(cls, context):
        return m2_export.find_root(context.active_object or context.collection) is not None

    def execute(self, context):
        root = m2_export.find_root(context.active_object or context.collection)
        try:
            r = m2_export.export(root, options={"force": self.force, "dry_run": self.dry_run, "write_anyway": self.write_anyway})
        except Exception as e:  # noqa: BLE001
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        for sev, where, msg in r.items:
            if sev != "INFO":
                self.report({sev}, f"{where}: {msg}")
        if r.errors:
            return {"CANCELLED"}
        self.report({"INFO"}, f"{root.wow_m2.path}: {'changed' if r.dirty else 'unchanged'}, {r.stats['files_written']} files written")
        return {"FINISHED"}


class WOW_OT_select_finding(bpy.types.Operator):
    bl_idname = "wow.select_finding"
    bl_label = "Select"
    bl_description = "Select the object a finding refers to"
    index: bpy.props.IntProperty()

    def execute(self, context):
        r = m2_export.last_report
        ob = r.objects[self.index] if r and self.index < len(r.objects) else None
        if ob is None or ob.name not in context.view_layer.objects:
            return {"CANCELLED"}
        for o in context.selected_objects:
            o.select_set(False)
        ob.select_set(True)
        context.view_layer.objects.active = ob
        return {"FINISHED"}


class WOW_OT_new_m2(bpy.types.Operator):
    bl_idname = "wow.new_m2"
    bl_label = "New M2"
    bl_description = "Start a new model from a small template; edit it and export to a new path"
    path: StringProperty(name="Client path", default="world/custom/new.m2")
    template: StringProperty(name="Template", description="Client path of the model to start from; empty for a lamp post")

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

    def execute(self, context):
        if not self.path.lower().endswith(".m2"):
            self.report({"ERROR"}, "Path must end in .m2")
            return {"CANCELLED"}
        m2_export.new_m2(self.path, self.template or None)
        return {"FINISHED"}


def _last_files(context):
    """Client paths the last export of the active model wrote (and that exist in the Output directory)."""
    ob = context.active_object or context.collection
    if m2_export.find_root(ob) is not None:
        files = list(getattr(m2_export.last_report, "files", None) or [])
    elif wmo_export.find_root(ob) is not None:
        files = list(getattr(wmo_export.last_report, "files", None) or [])
    else:
        files = []
    out = Path(client.dirs()[1])
    return [f for f in files if (out / f).is_file()]


class WOW_OT_install_model(bpy.types.Operator):
    bl_idname = "wow.install_model"
    bl_label = "Install to Content"
    bl_description = "Copy the last export of the active model into the client Content directory (stock files are backed up); restart the client afterwards"
    all_output: BoolProperty(name="Everything in Output", description="Install every file in the output directory", default=False)

    def execute(self, context):
        content, out = client.dirs()
        try:
            rels = install.output_files(out) if self.all_output else _last_files(context)
            if not rels:
                self.report({"ERROR"}, "Nothing to install: export the model first")
                return {"CANCELLED"}
            install.install(content, out, rels)
            fdids.export_index()
        except (install.InstallError, OSError) as e:
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Installed {len(rels)} files; restart the client to load them")
        return {"FINISHED"}


class WOW_OT_uninstall_model(bpy.types.Operator):
    bl_idname = "wow.uninstall_model"
    bl_label = "Uninstall"
    bl_description = "Restore the stock Content files replaced by installed models and remove installed new files"

    def execute(self, context):
        try:
            done = install.uninstall(client.dirs()[0])
        except (install.InstallError, OSError) as e:
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Restored {len(done)} files; restart the client")
        return {"FINISHED"}


class WOW_OT_write_game_data(bpy.types.Operator):
    bl_idname = "wow.write_game_data"
    bl_label = "Write game data"
    bl_description = "Create or update the creature or game object display rows of the active M2 (written to the world database, client db2 export)"
    kind: EnumProperty(name="Kind", items=[("creature", "Creature", ""), ("object", "Game object", "")], default="creature")
    template_display: IntProperty(name="Template display", description="CreatureDisplayInfo id whose other fields are copied", min=0)
    scale: FloatProperty(name="Scale", default=1.0, min=0.01)
    textures: StringProperty(name="Texture FDIDs", description="Up to four comma separated file data ids")
    update_stock: BoolProperty(name="Update stock rows", description="Also rewrite rows that exist in the stock data", default=False)
    dry_run: BoolProperty(name="Preview only", default=True)

    @classmethod
    def poll(cls, context):
        return m2_export.find_root(context.active_object or context.collection) is not None

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

    def execute(self, context):
        root = m2_export.find_root(context.active_object or context.collection)
        p = root.wow_m2
        info = game_data.ModelInfo(p.fdid, p.path, tuple(p.bounding_min), tuple(p.bounding_max),
                                   tuple(p.collision_min), tuple(p.collision_max))
        try:
            layout = game_data.Layout.discover(client.dirs()[0])
            db = layout.open()
            try:
                if self.kind == "creature":
                    tex = [int(t) for t in self.textures.replace(",", " ").split()]
                    changes, display = game_data.plan_creature(db, layout, info, self.template_display, self.scale, tex,
                                                               update_stock_model=self.update_stock)
                else:
                    changes, display = game_data.plan_game_object(db, layout, info, update_stock=self.update_stock)
            finally:
                db.close()
            for c in changes:
                self.report({"INFO"}, f"{c.kind} {c.table} {c.values[game_data.KEYS[c.table]]}")
            if self.dry_run or not changes:
                self.report({"INFO"}, f"Display {display}: {len(changes)} changes" + (" (preview)" if self.dry_run else ""))
                return {"FINISHED"}
            game_data.apply_changes(layout, changes)
        except (game_data.GameDataError, OSError, ValueError) as e:
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Display {display} written to the world database (uncommitted)")
        return {"FINISHED"}


class WOW_OT_write_item(bpy.types.Operator):
    bl_idname = "wow.write_item"
    bl_label = "Write item display"
    bl_description = "Create or update the item display rows of the active M2 (and optionally a test item) in the world database"
    template_display: IntProperty(name="Template display", description="ItemDisplayInfo id whose other fields are copied", min=1)
    textures: StringProperty(name="Texture FDIDs", description="Up to two comma separated BLP file data ids (one per model)")
    variants: StringProperty(name="Other models", description="Semicolon separated fdid:race:gender:position (helm race variants, shoulder sides); the active model is the first")
    race: IntProperty(name="Race", description="Race of the active model when it is a helm variant (1 human .. 8 troll); 0 for none", min=0, max=8)
    gender: IntProperty(name="Gender", description="0 male, 1 female, 2 any", default=2, min=0, max=2)
    position: IntProperty(name="Position", description="0 left, 1 right shoulder; -1 for other items", default=-1, min=-1, max=1)
    sections: StringProperty(name="Body textures", description="Armor without a model: semicolon separated section:fdid:gender (section 0..7, gender -1 for the only texture)")
    template_item: IntProperty(name="Template item", description="Create a test item copied from this item id (0: none)", min=0)
    item_name: StringProperty(name="Item name", default="Test item")
    dry_run: BoolProperty(name="Preview only", default=True)

    @classmethod
    def poll(cls, context):
        return True

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

    def _spec(self, root):
        spec = game_data.ItemSpec(self.template_display)
        if root is not None and root.wow_m2.fdid:
            first = game_data.ItemModel(root.wow_m2.fdid, race=self.race, gender=self.gender, position=self.position, variant=bool(self.race))
            spec.models.append(first)
        for part in filter(None, (v.strip() for v in self.variants.split(";"))):
            f = [int(x) for x in part.split(":")]
            spec.models.append(game_data.ItemModel(f[0], race=f[1], gender=f[2], position=f[3] if len(f) > 3 else -1, variant=f[1] > 0))
        spec.textures = [int(t) for t in self.textures.replace(",", " ").split()]
        for part in filter(None, (v.strip() for v in self.sections.split(";"))):
            sec, fdid, gender = (int(x) for x in part.split(":"))
            spec.sections.setdefault(sec, []).append((fdid, gender))
        return spec

    def execute(self, context):
        root = m2_export.find_root(context.active_object or context.collection)
        try:
            spec = self._spec(root)
            layout = game_data.Layout.discover(client.dirs()[0])
            db = layout.open()
            try:
                changes, display = game_data.plan_item_display(db, layout, spec)
                item = 0
            finally:
                db.close()
            if not self.dry_run:
                game_data.apply_changes(layout, changes)
                if self.template_item:
                    db = layout.open()
                    try:
                        more, item = game_data.plan_item(db, layout, self.template_item, display, self.item_name)
                    finally:
                        db.close()
                    game_data.apply_changes(layout, more)
        except (game_data.GameDataError, OSError, ValueError) as e:
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        for c in changes:
            self.report({"INFO"}, f"{c.kind} {c.table} {c.values[game_data.KEYS[c.table]]}")
        if self.dry_run:
            self.report({"INFO"}, f"Display {display}: {len(changes)} changes (preview)")
        else:
            self.report({"INFO"}, f"Display {display}" + (f", item {item}" if item else "") + " written (uncommitted); restart the server and clear Cache/")
        return {"FINISHED"}


classes = (WOW_OT_write_item, WOW_OT_write_game_data, WOW_OT_install_model, WOW_OT_uninstall_model, WOW_OT_export_wmo, WOW_OT_new_wmo, WOW_OT_export_m2, WOW_OT_new_m2, WOW_OT_select_finding)
