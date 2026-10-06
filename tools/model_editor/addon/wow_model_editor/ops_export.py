"""Export operators."""
import bpy
from bpy.props import BoolProperty, StringProperty

from pathlib import Path

from . import client, install, m2_export, wmo_export


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


classes = (WOW_OT_install_model, WOW_OT_uninstall_model, WOW_OT_export_wmo, WOW_OT_new_wmo, WOW_OT_export_m2, WOW_OT_new_m2, WOW_OT_select_finding)
