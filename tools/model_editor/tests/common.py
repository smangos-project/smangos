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
