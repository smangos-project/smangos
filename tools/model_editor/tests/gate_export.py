"""Forced exports of stock models into a directory. Run: blender -b --python gate_export.py -- OUTDIR PATH..."""
import sys

sys.path.insert(0, __file__.rsplit("/", 1)[0])
import common  # noqa: E402

OUT, PATHS = sys.argv[sys.argv.index("--") + 1], sys.argv[sys.argv.index("--") + 2:]
common.enable(output=OUT)
m2_import, m2_export = common.submodule("m2_import"), common.submodule("m2_export")
for path in PATHS:
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    r = m2_export.export(top, options={"force": True})
    print(path, "errors" if r.errors else "ok", r.files)
    assert not r.errors, str(r)
