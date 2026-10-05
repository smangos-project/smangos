"""Import operators."""
from pathlib import Path

import bpy
from bpy.props import StringProperty
from bpy_extras.io_utils import ImportHelper

from . import client, index, textures


def resolve(text):
    """Client path for a path or FDID string; raises ValueError."""
    s = client.session()
    index.load(s.content, client.prefs().cache_dir)
    text = text.strip().replace("\\", "/")
    if text.isdigit():
        path = index.path_of(int(text))
    else:
        path = text.lower() if index.fdid_of(text) is not None else None
        if not path:
            hits = index.search(text, "all", 1)
            path = hits[0][1] if hits else None
    if not path:
        raise ValueError(f"{text} is not in fdid.index")
    return path


def import_path(op, context, path):
    o = context.window_manager.wow_import
    try:
        if path.endswith(".m2"):
            from . import m2_import
            m2_import.load(path, {"skin": o.skin, "display": o.display, "display_id": o.display_id,
                                  "attachments": o.attachments, "events": o.events, "cameras": o.cameras,
                                  "lights": o.m2_lights, "emitters": o.emitters, "collision": o.collision})
            for w in m2_import.take_warnings():
                op.report({"WARNING"}, w)
        elif path.endswith(".wmo"):
            from . import wmo_import
            wmo_import.load(path, {"doodads": o.doodads, "lights": o.lights, "portals": o.portals,
                                   "liquids": o.liquids, "group_filter": o.group_filter,
                                   "doodad_meshes": o.doodad_meshes})
        else:
            op.report({"ERROR"}, f"{path} is not a .wmo/.m2")
            return {"CANCELLED"}
    except Exception as e:  # noqa: BLE001
        op.report({"ERROR"}, f"{path}: {e}")
        return {"CANCELLED"}
    miss = textures.take_missing()
    if miss:
        op.report({"WARNING"}, f"{len(miss)} textures could not be decoded: " + "; ".join(list(miss.values())[:5]))
    recent = context.window_manager.wow_recent
    for i, r in enumerate(recent):
        if r.name == path:
            recent.remove(i)
            break
    recent.add().name = path
    recent.move(len(recent) - 1, 0)
    while len(recent) > 10:
        recent.remove(len(recent) - 1)
    return {"FINISHED"}


class WOW_OT_import_path(bpy.types.Operator):
    bl_idname = "wow.import_path"
    bl_label = "Import WoW model"
    bl_description = "Import a model by client path or FDID"

    path: StringProperty(name="Path or FDID")

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

    def execute(self, context):
        try:
            path = resolve(self.path)
        except (ValueError, RuntimeError) as e:
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        return import_path(self, context, path)


class WOW_OT_import_file(bpy.types.Operator, ImportHelper):
    bl_idname = "wow.import_file"
    bl_label = "Import WoW model"
    filename_ext = ".wmo"
    filter_glob: StringProperty(default="*.wmo;*.m2", options={"HIDDEN"})

    def invoke(self, context, event):
        try:
            self.filepath = str(Path(bpy.path.abspath(client.prefs().content_dir)) / "")
        except Exception:  # noqa: BLE001
            pass
        return super().invoke(context, event)

    def execute(self, context):
        try:
            content = Path(client.session().content).resolve()
            rel = Path(self.filepath).resolve().relative_to(content).as_posix()
        except ValueError:
            self.report({"ERROR"}, "File is outside the Content directory")
            return {"CANCELLED"}
        except RuntimeError as e:
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        return import_path(self, context, rel.lower())


class WOW_OT_import_selected(bpy.types.Operator):
    bl_idname = "wow.import_selected"
    bl_label = "Import"

    path: StringProperty()

    def execute(self, context):
        try:
            path = resolve(self.path or context.window_manager.wow_query)
        except (ValueError, RuntimeError) as e:
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        return import_path(self, context, path)


def menu_import(self, context):
    self.layout.operator(WOW_OT_import_file.bl_idname, text="WoW model (.m2/.wmo)")
    self.layout.operator(WOW_OT_import_path.bl_idname, text="WoW model by path or FDID")


classes = (WOW_OT_import_path, WOW_OT_import_file, WOW_OT_import_selected)


def register():
    bpy.types.TOPBAR_MT_file_import.append(menu_import)


def unregister():
    bpy.types.TOPBAR_MT_file_import.remove(menu_import)
