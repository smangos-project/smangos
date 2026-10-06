"""Builds a custom sword (copy of a stock one, scaled, new BLP), its item display and a test item, installs it.
Run: blender -b --python gate_item.py -- OUTDIR [--dry]"""
import sys

sys.path.insert(0, __file__.rsplit("/", 1)[0])
import bpy  # noqa: E402
import common  # noqa: E402

OUT = sys.argv[sys.argv.index("--") + 1]
common.enable(output=OUT)
m2_export, m2_mesh_export = common.submodule("m2_export"), common.submodule("m2_mesh_export")
gd, client, install, fdids, textures = (common.submodule(n) for n in ("game_data", "client", "install", "fdids", "textures"))
STOCK = "item/objectcomponents/weapon/sword_1h_short_a_02.m2"
PATH = "item/objectcomponents/weapon/custom/cubesword_01.m2"
BLP = "item/objectcomponents/weapon/custom/cubesword_01red.blp"
top = m2_export.new_m2(PATH, STOCK)
for ob in m2_mesh_export.submesh_objects(top)[0]:
    for v in ob.data.vertices:
        v.co *= 1.6
r = m2_export.export(top, options={"force": True})
assert not r.errors, str(r)
img = bpy.data.images.new("cubesword_01red", 128, 128)
px = [0.0] * (128 * 128 * 4)
for y in range(128):
    for x in range(128):
        c = (0.9, 0.1, 0.1) if (x // 16 + y // 16) % 2 else (0.9, 0.7, 0.1)
        px[(y * 128 + x) * 4:(y * 128 + x) * 4 + 4] = (*c, 1.0)
img.pixels = px
fdids.allocate(BLP)
client.session()
blp_fdid = textures.export_blp(img, BLP)
fdids.adopt_session_ids()
files = [*r.files, BLP]
print("files", files, "blp", blp_fdid, "model", top.wow_m2.fdid)
spec = gd.ItemSpec(26577, [gd.ItemModel(top.wow_m2.fdid)], [blp_fdid])
lay = gd.Layout.discover(client.dirs()[0])
db = lay.open()
try:
    changes, display = gd.plan_item_display(db, lay, spec)
    ichanges, item = gd.plan_item(db, lay, 727, display, "Cube Sword (test)")
finally:
    db.close()
print("display", display, "item", item, len(changes) + len(ichanges), "changes")
if "--dry" not in sys.argv:
    gd.apply_changes(lay, changes + ichanges)
    install.install(client.dirs()[0], OUT, files)
    fdids.export_index()
    print("installed")
