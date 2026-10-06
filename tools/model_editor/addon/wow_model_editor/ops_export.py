"""Export operators."""
import bpy
from bpy.props import BoolProperty, StringProperty

from . import m2_export, wmo_export


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


classes = (WOW_OT_export_wmo, WOW_OT_new_wmo, WOW_OT_export_m2)
