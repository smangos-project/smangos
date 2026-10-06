"""Export operators."""
import bpy
from bpy.props import BoolProperty, StringProperty

from . import wmo_export


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


classes = (WOW_OT_export_wmo, WOW_OT_new_wmo)
