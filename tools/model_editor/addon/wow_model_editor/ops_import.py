"""Import operators."""
from pathlib import Path

import bpy
from bpy.props import IntProperty, StringProperty
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


def import_path(op, context, path, extra=None):
    o = context.window_manager.wow_import
    try:
        if path.endswith(".m2"):
            from . import m2_import
            m2_import.load(path, {"skin": o.skin, "display": o.display, "display_id": o.display_id,
                                  "attachments": o.attachments, "events": o.events, "cameras": o.cameras,
                                  "lights": o.m2_lights, "emitters": o.emitters, "collision": o.collision,
                                  **(extra or {})})
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


class WOW_OT_import_item_display(bpy.types.Operator):
    bl_idname = "wow.import_item_display"
    bl_label = "Import item display"
    bl_description = "Import the model of an ItemDisplayInfo (human male variant by default)"

    display_id: IntProperty(name="ItemDisplayInfo id", min=1)
    race: IntProperty(name="Race", description="Variant race (0: default)", min=0, max=8)

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

    def execute(self, context):
        from . import game_data
        try:
            layout = game_data.Layout.discover(client.dirs()[0])
            db = layout.open()
            try:
                models, textures = game_data.item_display_models(db, self.display_id)
            finally:
                db.close()
            pick = next((m for m in models if m.race == self.race and m.gender in (0, 2)), None) if self.race else game_data.pick_default(models)
            if pick is None:
                self.report({"ERROR"}, f"display {self.display_id} has no model")
                return {"CANCELLED"}
            path = resolve(str(pick.fdid))
        except (game_data.GameDataError, ValueError, RuntimeError, OSError) as e:
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        skin = next((t for t in textures if t), 0)
        self.report({"INFO"}, f"{len(models)} models, textures {textures}; importing {path}")
        return import_path(self, context, path, {"object_skin": skin})


def menu_import(self, context):
    self.layout.operator(WOW_OT_import_file.bl_idname, text="WoW model (.m2/.wmo)")
    self.layout.operator(WOW_OT_import_path.bl_idname, text="WoW model by path or FDID")
    self.layout.operator(WOW_OT_import_item_display.bl_idname, text="WoW item display by id")


classes = (WOW_OT_import_path, WOW_OT_import_file, WOW_OT_import_selected, WOW_OT_import_item_display)


def register():
    bpy.types.TOPBAR_MT_file_import.append(menu_import)


def unregister():
    bpy.types.TOPBAR_MT_file_import.remove(menu_import)
