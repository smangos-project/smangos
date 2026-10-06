"""Generic structural equality over wowlib entities; the implementation lives in the add-on."""
import importlib.util
from pathlib import Path

_spec = importlib.util.spec_from_file_location("deep_compare", Path(__file__).resolve().parents[1] / "addon" / "wow_model_editor" / "deep_compare.py")
_mod = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_mod)
deep_diff = _mod.deep_diff
