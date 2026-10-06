"""blender -b --python render_edited.py -- OUT.png WMO_PATH: edits a WMO (lifts a vertex band), exports it to the output directory, then renders the re-imported result."""
import runpy
import sys

import bpy

sys.path.insert(0, __file__.rsplit("/", 1)[0])
import common  # noqa: E402

out, path = sys.argv[sys.argv.index("--") + 1:][:2]
common.enable()
wmo_import, wmo_export = common.submodule("wmo_import"), common.submodule("wmo_export")
coll = wmo_import.load(path, {"lights": False, "doodads": False})
ob = next(o for o in coll.all_objects if o.type == "MESH" and o.name.startswith(coll.name.split(".")[0]) or o.type == "MESH")
top = max(v.co.z for v in ob.data.vertices)
for v in ob.data.vertices:
    if v.co.z > top - 3:
        v.co.z += 6
ob.data.update()
r = wmo_export.export(coll)
print("export", r.stats, [i for i in r.items if i[0] != "INFO"])
assert not r.errors
runpy.run_path(__file__.rsplit("/", 1)[0] + "/render_model.py", run_name="__main__")
