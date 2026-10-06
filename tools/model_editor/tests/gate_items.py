"""Builds custom items of every slot kind (stock models copied/scaled, new BLPs), writes rows, installs.
Run: blender -b --python gate_items.py -- OUTDIR"""
import sys

sys.path.insert(0, __file__.rsplit("/", 1)[0])
import bpy  # noqa: E402
import common  # noqa: E402

OUT = sys.argv[sys.argv.index("--") + 1]
common.enable(output=OUT)
m2_export, m2_mesh_export = common.submodule("m2_export"), common.submodule("m2_mesh_export")
gd, client, install, fdids, textures = (common.submodule(n) for n in ("game_data", "client", "install", "fdids", "textures"))
files = []
BASE = "item/objectcomponents"


def model(stock, path, scale=1.3):
    top = m2_export.new_m2(path, stock)
    for ob in m2_mesh_export.submesh_objects(top)[0]:
        for v in ob.data.vertices:
            v.co *= scale
    r = m2_export.export(top, options={"force": True})
    assert not r.errors, str(r)
    files.extend(r.files)
    return top.wow_m2.fdid


def blp(path, w, h, a, b):
    name = path.rsplit("/", 1)[1][:-4]
    img = bpy.data.images.new(name, w, h)
    px = [0.0] * (w * h * 4)
    for y in range(h):
        for x in range(w):
            c = a if (x // 8 + y // 8) % 2 else b
            px[(y * w + x) * 4:(y * w + x) * 4 + 4] = (*c, 1.0)
    img.pixels = px
    fdids.allocate(path)
    client.session()
    fdid = textures.export_blp(img, path)
    fdids.adopt_session_ids()
    files.append(path)
    return fdid


RED, YEL, GRN, BLU, WHT = (0.9, 0.1, 0.1), (0.9, 0.7, 0.1), (0.1, 0.8, 0.2), (0.1, 0.2, 0.9), (0.95, 0.95, 0.95)
lay = gd.Layout.discover(client.dirs()[0])
specs = []
# retextured stock shield
shield = "item/objectcomponents/shield/buckler_round_a_01.m2"
specs.append(("Cube Shield", 876, gd.ItemSpec(1680, [gd.ItemModel(fdids.allocate(shield))], [blp(f"{BASE}/shield/custom/cubeshield_01.blp", 128, 128, GRN, WHT)])))
# helm: human male/female copies plus orc and dwarf variants
helm = f"{BASE}/head/helm_eyepatch_a_01_%s.m2"
variants = [("hum", 1, 0), ("huf", 1, 1), ("orm", 2, 0), ("orf", 2, 1), ("dwm", 3, 0), ("dwf", 3, 1)]
models = [gd.ItemModel(model(helm % s, f"{BASE}/head/custom/cubehelm_01_{s}.m2", 1.5), race=r, gender=g, variant=True) for s, r, g in variants]
specs.append(("Cube Helm", 1162, gd.ItemSpec(1166, models, [blp(f"{BASE}/head/custom/cubehelm_01blue.blp", 128, 128, BLU, YEL)])))
# shoulders
l = model(f"{BASE}/shoulder/lshoulder_leather_a_01.m2", f"{BASE}/shoulder/custom/cubeshoulder_l_01.m2", 1.5)
rr = model(f"{BASE}/shoulder/rshoulder_leather_a_01.m2", f"{BASE}/shoulder/custom/cubeshoulder_r_01.m2", 1.5)
specs.append(("Cube Shoulders", 905, gd.ItemSpec(1057, [gd.ItemModel(l, gender=2, position=0), gd.ItemModel(rr, gender=2, position=1)],
                                                  [blp(f"{BASE}/shoulder/custom/cubeshoulder_01.blp", 128, 128, RED, YEL)])))
# chest: new body component textures
TC = "item/texturecomponents"
sec = {0: ("armuppertexture", 128, 64), 3: ("torsouppertexture", 128, 64), 4: ("torsolowertexture", 128, 32)}
sections = {}
for s, (folder, w, h) in sec.items():
    sections[s] = [(blp(f"{TC}/{folder}/custom/cubechest_{g}.blp", w, h, GRN if g == "m" else RED, WHT), 0 if g == "m" else 1) for g in "mf"]
specs.append(("Cube Chest", 193, gd.ItemSpec(7777, [], [], sections)))
report = []
for name, tmpl, spec in specs:
    db = lay.open()
    try:
        ch, display = gd.plan_item_display(db, lay, spec)
    finally:
        db.close()
    gd.apply_changes(lay, ch)
    db = lay.open()
    try:
        ich, item = gd.plan_item(db, lay, tmpl, display, f"{name} (test)")
    finally:
        db.close()
    gd.apply_changes(lay, ich)
    report.append((name, display, item))
install.install(client.dirs()[0], OUT, files)
fdids.export_index()
print("ITEMS", report)
