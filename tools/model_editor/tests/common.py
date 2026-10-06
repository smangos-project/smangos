"""Shared helpers for the headless tests: enable the installed extension with the local Content tree."""
import importlib
import os
from pathlib import Path

import addon_utils
import bpy

NAME = "bl_ext.user_default.wow_model_editor"
ROOT = Path(__file__).resolve().parents[3]
CONTENT = os.environ.get("MODEL_EDITOR_CONTENT", str(ROOT / ".client" / "Content"))


def enable(cache="/tmp/me-cache", output="/tmp/me-out"):
    def fail(e):
        raise e
    addon_utils.enable(NAME, default_set=True, persistent=True, handle_error=fail)
    p = bpy.context.preferences.addons[NAME].preferences
    p.content_dir, p.output_dir, p.cache_dir = CONTENT, output, cache
    return importlib.import_module(NAME)


def submodule(name):
    return importlib.import_module(f"{NAME}.{name}")


def scratch_world(name):
    """Points the addon at a scratch world database holding the live stock fdid rows; Content/fdid.index is never rewritten."""
    mod = submodule("doltlite")
    fdids = submodule("fdids")
    bpy.context.preferences.addons[NAME].preferences.custom_listfile = ""
    live = fdids.with_db(submodule("registry").read_all)
    path = Path(f"/tmp/me-world-{name}.doltlite")
    for f in path.parent.glob(path.name + "*"):
        f.unlink()
    db = mod.DB(path, mod.find_library(ROOT), create=True)
    db.execute("CREATE TABLE file_data_id (id INTEGER PRIMARY KEY, path TEXT NOT NULL UNIQUE, source TEXT NOT NULL, retired INTEGER NOT NULL DEFAULT 0)")
    with db.transaction():
        for i, p in live.items():
            if i < 2_000_000_000:
                db.execute("INSERT INTO file_data_id (id, path, source) VALUES (?, ?, 'listfile')", (i, p))
    db.close()
    bpy.context.preferences.addons[NAME].preferences.custom_listfile = str(path)
    fdids.export_index = lambda: False
    submodule("client").close()
    return path
