"""New lamp post model with a particle emitter, ribbon, light and sound event; writes game object display data and installs it.
Run: blender -b --python gate_m2fx.py -- OUTDIR [--no-install]"""
import sys

sys.path.insert(0, __file__.rsplit("/", 1)[0])
import bpy  # noqa: E402
import common  # noqa: E402

OUT = sys.argv[sys.argv.index("--") + 1]
common.enable(output=OUT)
m2_export, ops_add = common.submodule("m2_export"), common.submodule("ops_add")
gd, client, install, fdids = (common.submodule(n) for n in ("game_data", "client", "install", "fdids"))
top = m2_export.new_m2("world/custom/fxlamp.m2")
print("textures", len(top.wow_m2.textures), "materials", len(top.wow_m2.materials))
ops_add.new_m2_object(top, "particle", (0, 0, 3.0))
ops_add.new_m2_object(top, "ribbon", (0.5, 0, 3.0))
lt = ops_add.new_m2_object(top, "light", (0, 0, 3.5))
lt.data.color = (1.0, 0.3, 0.1)
ops_add.new_m2_object(top, "event", (0, 0, 1.0))
r = m2_export.export(top, options={"force": True})
print("errors", [str(e) for e in r.errors], "warnings", str(r)[:600])
assert not r.errors
p = top.wow_m2
info = gd.ModelInfo(p.fdid, p.path, tuple(p.bounding_min), tuple(p.bounding_max), tuple(p.collision_min), tuple(p.collision_max))
lay = gd.Layout.discover(client.dirs()[0])
db = lay.open()
try:
    gchanges, gdisplay = gd.plan_game_object(db, lay, info)
finally:
    db.close()
print("object display", gdisplay)
gd.apply_changes(lay, gchanges)
if "--no-install" not in sys.argv:
    install.install(client.dirs()[0], OUT, r.files)
    fdids.export_index()
    print("installed")
