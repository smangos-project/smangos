"""Headless addon tests. Run: blender -b --factory-startup --python test_addon.py -- [--all] [--filter SUBSTR]"""
import sys
import time
import traceback

import bpy
import numpy as np

sys.path.insert(0, __file__.rsplit("/", 1)[0])
import common  # noqa: E402

common.enable()
index = common.submodule("index")
textures = common.submodule("textures")
wmo_import = common.submodule("wmo_import")
client = common.submodule("client")
props = common.submodule("props")

ARGS = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
REFS = ["world/wmo/cameron.wmo", "world/wmo/kalimdor/ogrimmar/kl_pvpbarracks.wmo",
        "world/wmo/azeroth/buildings/stormwind/stormwind.wmo"]
KNOWN_BAD = {"world/wmo/expansion11/troll/12tr_amani_bridgewood_plank01.wmo"}  # truncated in the shipped data
tests = []


def test(f):
    tests.append(f)
    return f


def clear():
    for coll in (bpy.data.objects, bpy.data.meshes, bpy.data.materials, bpy.data.images, bpy.data.lights,
                 bpy.data.collections, bpy.data.node_groups):
        for d in list(coll):
            coll.remove(d)


@test
def catalogue():
    index.load(client.session().content, client.prefs().cache_dir)
    m2, wmo = index.entries("m2"), index.entries("wmo")
    assert len(m2) > 9000 and len(wmo) > 800, (len(m2), len(wmo))
    assert not any(e[1].endswith("_000.wmo") for e in wmo)
    assert index.fdid_of("World/WMO/Cameron.wmo") == 108237 or index.path_of(index.fdid_of("world/wmo/cameron.wmo")) == "world/wmo/cameron.wmo"
    assert index.search("cameron", "wmo")[0][1] == "world/wmo/cameron.wmo"
    fd = index.fdid_of("world/wmo/cameron.wmo")
    assert index.search(str(fd))[0][0] == fd
    assert len(index.search("wmo", "all", 50)) == 50
    assert index.search("zzzzqqq") == []


@test
def texture_cache_and_placeholder():
    img = textures.get_image(129995)
    assert img.size[0] > 0 and img["wow_fdid"] == 129995 and img.alpha_mode == "CHANNEL_PACKED"
    assert textures.get_image(129995) is img
    assert textures.get_image(0).name == textures.PLACEHOLDER
    assert textures.get_image(1).name == textures.PLACEHOLDER and 1 in textures.take_missing()


@test
def blp_export_round_trip():
    img = textures.get_image(129995)
    ref = textures.image_to_rgba(img)
    fdid = textures.export_blp(img, "test/me_roundtrip.blp")
    out = textures.decode(fdid)
    assert out.shape == ref.shape
    diff = np.abs(out.astype(int) - ref.astype(int)).mean()
    assert diff < 8, diff


def wmo_expect(path):
    w = wmo_import.read(path)
    return w, w.root


def mesh_groups(coll):
    return {o.wow_wmo_group.index: o for o in coll.all_objects if o.type == "MESH" and o.wow_wmo_group.index >= 0}


@test
def import_reference_wmos():
    for path in REFS:
        clear()
        t = time.time()
        coll = wmo_import.load(path)
        secs = time.time() - t
        w, root = wmo_expect(path)
        groups = mesh_groups(coll)
        assert len(groups) == len(w.groups), path
        for gi, g in enumerate(w.groups):
            ob = groups[gi]
            me, b = ob.data, g.body
            assert len(me.vertices) == len(b.vertices)
            assert len(me.polygons) == len(b.indices) // 3 or len(me.polygons) == len(b.large_indices) // 3
            assert len(me.uv_layers) == len(b.texcoords)
            assert len(me.attributes["wow_poly_flags"].data) == len(me.polygons)
            if gi < 5 and len(b.vertices):
                co = np.asarray(b.vertices).view(np.float32).reshape(-1, 3)
                got = np.empty(len(co) * 3, np.float32)
                me.vertices.foreach_get("co", got)
                assert np.allclose(got.reshape(-1, 3), co)
                uv = np.asarray(b.texcoords[0])
                lu = np.empty(len(me.loops) * 2, np.float32)
                me.uv_layers[0].data.foreach_get("uv", lu)
                vi = np.empty(len(me.loops), np.int32)
                me.loops.foreach_get("vertex_index", vi)
                assert np.allclose(lu.reshape(-1, 2)[:, 0], uv["x"][vi]) and np.allclose(lu.reshape(-1, 2)[:, 1], 1 - uv["y"][vi])
            n_batch_faces = {}
            if len(me.polygons):
                mi = np.empty(len(me.polygons), np.int32)
                me.polygons.foreach_get("material_index", mi)
                bt = np.empty(len(me.polygons), np.int32)
                me.attributes["wow_batch"].data.foreach_get("value", bt)
                assert ((bt >= 0) | (np.array([me.materials[i].name for i in mi]) == "wow_collision")).all()
        doodads = [o for o in coll.all_objects if o.type == "EMPTY" and o.wow_wmo_doodad.index >= 0]
        assert len(doodads) == len(root.doodad_defs), (len(doodads), len(root.doodad_defs))
        assert len([o for o in coll.all_objects if o.name.startswith("portal_")]) == len(root.portals)
        assert len([o for o in coll.all_objects if o.type == "LIGHT"]) == len(root.lights)
        assert len([o for o in coll.all_objects if o.name.startswith("fog_")]) == len(root.fogs)
        if len(root.doodad_defs):
            d = root.doodad_defs[0]
            ob = next(o for o in doodads if o.wow_wmo_doodad.index == 0)
            assert np.allclose(ob.location, (d.position.x, d.position.y, d.position.z), atol=1e-4)
            assert ob.wow_wmo_doodad.fdid == root.doodad_fdids[d.name_index]
        liq = sum(1 for g in w.groups if not g.body.liquid.empty)
        assert len([o for o in coll.all_objects if o.name.startswith("liquid_")]) == liq
        print(f"  {path}: {secs:.1f}s {len(groups)} groups {len(root.doodad_defs)} doodads {liq} liquids")
        if path.endswith("stormwind.wmo"):
            assert secs < 20, secs


@test
def material_props_round_trip():
    clear()
    path = REFS[1]
    coll = wmo_import.load(path, {"doodads": False})
    root = wmo_expect(path)[1]
    mats = {m.wow_wmo_material.material_index: m for m in bpy.data.materials if m.wow_wmo_material.material_index >= 0}
    assert len(mats) == len(root.materials)
    for i, sm in enumerate(root.materials):
        p = mats[i].wow_wmo_material
        assert props.u32(p.flags) == int(sm.flags) and p.shader == sm.shader and p.blend_mode == sm.blend_mode
        assert (props.u32(p.texture1), props.u32(p.texture2), props.u32(p.texture3)) == (sm.texture1, sm.texture2, sm.texture3)
        assert tuple(p.diff_color) == (sm.diff_color.r, sm.diff_color.g, sm.diff_color.b, sm.diff_color.a)
        assert tuple(p.sidn_color) == (sm.sidn_color.r, sm.sidn_color.g, sm.sidn_color.b, sm.sidn_color.a)
        assert p.ground_type == sm.ground_type and props.u32(p.color2) == sm.color2 and props.u32(p.flags2) == sm.flags2
        assert [props.u32(v) for v in p.run_time_data] == list(sm.run_time_data)
    wp = coll.wow_wmo
    assert wp.path == path and wp.wmo_id == root.header.wmo_id and wp.mver == root.mver
    g0 = mesh_groups(coll)[0].wow_wmo_group
    hdr = wmo_expect(path)[0].groups[0].body.header
    assert props.u32(g0.flags) == int(hdr.flags) and g0.group_liquid == hdr.group_liquid and list(g0.fog_ids) == list(hdr.fog_ids)


@test
def operators():
    clear()
    r = bpy.ops.wow.import_path(path="world/wmo/cameron.wmo")
    assert r == {"FINISHED"} and bpy.data.collections["cameron.wmo"]
    wm = bpy.context.window_manager
    assert wm.wow_recent[0].name == "world/wmo/cameron.wmo"
    wm.wow_query = "kl_pvpbarracks"
    assert bpy.ops.wow.import_selected() == {"FINISHED"}
    assert bpy.ops.wow.import_path(path=str(index.fdid_of("world/wmo/cameron.wmo"))) == {"FINISHED"}
    try:
        bpy.ops.wow.import_path(path="no/such/file.wmo")
        assert False
    except RuntimeError:
        pass
    assert wm.wow_import.doodads


def sweep():
    index.load(client.session().content, client.prefs().cache_dir)
    fails, t0 = [], time.time()
    wmos = index.entries("wmo")
    for i, (fdid, path, _) in enumerate(wmos):
        clear()
        try:
            wmo_import.load(path)
        except Exception as e:  # noqa: BLE001
            fails.append((path, f"{type(e).__name__}: {e}"[:200]))
    fails = [f for f in fails if f[0] not in KNOWN_BAD]
    print(f"sweep: {len(wmos)} WMOs, {len(fails)} failures, {time.time() - t0:.0f}s")
    for f in fails:
        print("  FAIL", *f)
    return fails


failed = 0
sel = [a for a in ARGS if not a.startswith("--")]
if "--filter" in ARGS:
    sel = [ARGS[ARGS.index("--filter") + 1]]
for f in tests:
    if sel and not any(s in f.__name__ for s in sel):
        continue
    try:
        f()
        print("PASS", f.__name__)
    except Exception:  # noqa: BLE001
        failed += 1
        print("FAIL", f.__name__)
        traceback.print_exc()
if "--all" in ARGS:
    failed += len(sweep())
print("RESULT", "FAIL" if failed else "OK")
sys.exit(1 if failed else 0)
