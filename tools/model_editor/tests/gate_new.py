"""Exports a new cube model, writes its game data to the live world and installs it into Content.
Run: blender -b --python gate_new.py -- OUTDIR [--no-install]"""
import sys

sys.path.insert(0, __file__.rsplit("/", 1)[0])
import bmesh  # noqa: E402
import bpy  # noqa: E402
import common  # noqa: E402

OUT = sys.argv[sys.argv.index("--") + 1]
common.enable(output=OUT)
m2_export, m2_mesh_export = common.submodule("m2_export"), common.submodule("m2_mesh_export")
gd, client, install, fdids = (common.submodule(n) for n in ("game_data", "client", "install", "fdids"))
top = m2_export.new_m2("world/custom/testcube.m2")
ob = m2_mesh_export.submesh_objects(top)[0][0]
for extra in m2_mesh_export.submesh_objects(top)[0][1:]:
    bpy.data.objects.remove(extra)
bm = bmesh.new()
bmesh.ops.create_cube(bm, size=2.0)
bmesh.ops.translate(bm, vec=(0, 0, 1.0), verts=bm.verts)
bm.to_mesh(ob.data)
bm.free()
r = m2_export.export(top, options={"force": True})
assert not r.errors, str(r)
print("exported", r.files, r.items)
p = top.wow_m2
info = gd.ModelInfo(p.fdid, p.path, tuple(p.bounding_min), tuple(p.bounding_max), tuple(p.collision_min), tuple(p.collision_max))
print("info", info)
lay = gd.Layout.discover(client.dirs()[0])
db = lay.open()
try:
    wolf = db.dicts("SELECT ID FROM dbc_creaturedisplayinfo WHERE ModelID = (SELECT ID FROM dbc_creaturemodeldata WHERE FileDataID = (SELECT FileDataID FROM dbc_modelfiledata LIMIT 1)) LIMIT 1")
    cdi = [r for r in db.dicts("SELECT display_id FROM creature_display_info_addon LIMIT 1")][0]["display_id"]
    changes, display = gd.plan_creature(db, lay, info, cdi, scale=1.0, model_file_data=("--no-mfd" not in sys.argv))
    gchanges, gdisplay = gd.plan_game_object(db, lay, info, model_file_data=("--no-mfd" not in sys.argv))
finally:
    db.close()
print("creature display", display, [(c.kind, c.table) for c in changes])
print("object display", gdisplay, [(c.kind, c.table) for c in gchanges])
if "--dry" in sys.argv:
    sys.exit()
gd.apply_changes(lay, changes + gchanges)
if "--no-install" not in sys.argv:
    install.install(client.dirs()[0], OUT, r.files)
    fdids.export_index()
    print("installed")
