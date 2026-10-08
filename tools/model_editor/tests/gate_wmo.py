"""Edits a stock WMO (a floating copy of group 0 as a new group, a pillar of removed doodads) and installs it into Content.
Run: blender -b --python gate_wmo.py -- OUTDIR WMO_PATH [--no-install]"""
import sys

sys.path.insert(0, __file__.rsplit("/", 1)[0])
import bpy  # noqa: E402
import common  # noqa: E402

OUT, PATH = sys.argv[sys.argv.index("--") + 1:][:2]
common.enable(output=OUT)
wmo_import, wmo_export = common.submodule("wmo_import"), common.submodule("wmo_export")
client, install, fdids = (common.submodule(n) for n in ("client", "install", "fdids"))
coll = wmo_import.load(PATH, {"doodads": True})
groups = next(c for c in coll.children if c.name.startswith("groups"))
src = next(o for o in groups.objects if o.type == "MESH")
nob = src.copy()
nob.data = src.data.copy()
nob.name = "gate_new_group"
nob.wow_wmo_group.index = -1
nob.wow_wmo_group.group_name = "gate"
nob.location.z += 12
groups.objects.link(nob)
dood = [o for o in coll.all_objects if o.type == "EMPTY" and o.wow_wmo_doodad.index >= 0]
for o in dood[::2]:
    bpy.data.objects.remove(o)
print("removed doodads", len(dood[::2]), "of", len(dood))
r = wmo_export.export(coll, options={"force": True})
assert not r.errors, str(r)
print("exported", sorted(r.files))
if "--no-install" not in sys.argv:
    install.install(client.dirs()[0], OUT, r.files)
    fdids.export_index()
    print("installed")
