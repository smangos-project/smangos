import os
from pathlib import Path

import bpy
from bpy.props import StringProperty


def _cache_default():
    return str(Path.home() / ".cache" / "wow_model_editor")


class WowPreferences(bpy.types.AddonPreferences):
    bl_idname = __package__

    content_dir: StringProperty(
        name="Content directory", subtype="DIR_PATH",
        description="Extracted client tree containing fdid.index",
        default=os.environ.get("MODEL_EDITOR_CONTENT", ""))
    output_dir: StringProperty(
        name="Output directory", subtype="DIR_PATH",
        description="Where edited files are written; overrides Content on later reads")
    custom_listfile: StringProperty(
        name="World database", subtype="FILE_PATH",
        description="world database holding the file_data_id table; defaults to .world/world.doltlite")
    cache_dir: StringProperty(
        name="Cache directory", subtype="DIR_PATH", default=_cache_default())

    def draw(self, context):
        col = self.layout.column()
        col.prop(self, "content_dir")
        col.prop(self, "output_dir")
        col.prop(self, "custom_listfile")
        col.prop(self, "cache_dir")
        if not self.content_dir:
            col.label(text="Set the Content directory to browse client files", icon="ERROR")


classes = (WowPreferences,)
