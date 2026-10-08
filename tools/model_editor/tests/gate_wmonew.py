"""New two-room WMO from scratch (portal between the rooms, interior light, doorway out), exported over a placed stock WMO and installed.
Run: blender -b --python gate_wmonew.py -- OUTDIR [WMO_PATH] [--no-install]"""
import sys

sys.path.insert(0, __file__.rsplit("/", 1)[0])
import bpy  # noqa: E402
import common  # noqa: E402

OUT, *rest = sys.argv[sys.argv.index("--") + 1:]
OVER = next((a for a in rest if a.endswith(".wmo")), "world/wmo/azeroth/buildings/human_farm/farm.wmo")
common.enable(output=OUT)
wmo_import, wmo_export, ops_add = (common.submodule(n) for n in ("wmo_import", "wmo_export", "ops_add"))
client, install, fdids = (common.submodule(n) for n in ("client", "install", "fdids"))
src = wmo_import.load("world/wmo/cameron.wmo", {"doodads": False})
first = next(o for o in src.all_objects if o.type == "MESH" and o.material_slots)
mat = next(s.material for s in first.material_slots if s.material.wow_wmo_material.material_index >= 0).copy()
mat.wow_wmo_material.material_index = -1
path = "world/wmo/custom/gate_rooms.wmo"
top = wmo_export.new_wmo(path)
groups = next(c for c in top.children if c.name.startswith("groups"))


def room(name, x0, skip):
    v = [(x0 + x, y, z) for x in (0, 6) for y in (0, 6) for z in (0, 4)]
    f = [(0, 1, 3, 2), (4, 6, 7, 5), (0, 4, 5, 1), (2, 3, 7, 6), (0, 2, 6, 4), (1, 5, 7, 3)]
    f = [t for i, t in enumerate(f) if i != skip]
    me = bpy.data.meshes.new(name)
    me.from_pydata(v, [], f)
    me.uv_layers.new(name="uv0")
    me.materials.append(mat)
    ob = bpy.data.objects.new(name, me)
    groups.objects.link(ob)
    ob.wow_wmo_group.group_name = name


room("room_a", 0, 5)
room("room_b", 6, 4)
bpy.context.view_layer.update()
pt = ops_add.new_wmo_object(top, "portal", (6, 3, 0))
pt.rotation_euler.z = 1.5708
pt.scale = (3, 1, 2)
lt = ops_add.new_wmo_object(top, "light", (9, 3, 2.5))
lt.data.color = (1.0, 0.4, 0.1)
dd = next(c for c in top.children if c.name.startswith("doodads"))
st = bpy.data.collections.new("Set_$DefaultGlobal")
dd.children.link(st)
ops_add.new_wmo_object(top, "fog", (6, 3, 2))
ops_add.new_wmo_object(top, "doodad", (3, 3, 0), "world/generic/human/passive doodads/lampposts/duskwoodlamppost.m2")
r = wmo_export.export(top, OVER, options={"force": True})
print("errors", [str(e) for e in r.errors], str(r)[:800])
assert not r.errors
if "--no-install" not in sys.argv:
    install.install(client.dirs()[0], OUT, r.files)
    fdids.export_index()
    print("installed", sorted(r.files))
