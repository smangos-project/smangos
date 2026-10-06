"""Headless addon tests. Run: blender -b --factory-startup --python test_addon.py -- [--all] [--filter SUBSTR]"""
import os
import sys
import time
import traceback

import bpy
import numpy as np

sys.path.insert(0, __file__.rsplit("/", 1)[0])
sys.path.insert(0, __file__.rsplit("/", 2)[0] + "/spike")
import common  # noqa: E402
from deep import deep_diff  # noqa: E402

common.enable()
index = common.submodule("index")
textures = common.submodule("textures")
wmo_import = common.submodule("wmo_import")
client = common.submodule("client")
props = common.submodule("props")
m2_import = common.submodule("m2_import")
m2_names = common.submodule("m2_names")
displays = common.submodule("displays")
m2_anim = common.submodule("m2_anim")
m2_anim_export = common.submodule("m2_anim_export")
m2_seq = common.submodule("m2_seq")

ARGS = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
REFS = ["world/wmo/cameron.wmo", "world/wmo/kalimdor/ogrimmar/kl_pvpbarracks.wmo",
        "world/wmo/azeroth/buildings/stormwind/stormwind.wmo"]
KNOWN_BAD = {"world/expansion11/doodads/elwynn/12elw_elwynn_main_tree_apple_d02.m2", "world/wmo/expansion11/troll/12tr_amani_bridgewood_plank01.wmo"}  # truncated in the shipped data
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


M2S = ["creature/wolf/wolf.m2", "character/human/male/humanmale.m2", "spells/fireball_missile_high.m2"]


def find_m2(stem):
    index.load(client.session().content, client.prefs().cache_dir)
    return next(e[1] for e in index.entries("m2") if e[1].endswith("/" + stem))


@test
def m2_names_and_displays():
    assert m2_names.event(int.from_bytes(b"$CAH", "little")) == "$CAH"
    assert m2_names.bone_name(3, 26) == "Root_3" and m2_names.bone_name(1, -1) == "bone_1"
    ds = displays.for_model(126487)
    assert len(ds) >= 12 and all(isinstance(d.textures, tuple) and len(d.textures) == 4 for d in ds)
    assert ds[0].scale > 0 and any(d.textures[0] or d.textures[1] for d in ds)
    assert displays.for_model(1) == []


@test
def import_m2_geometry():
    for path in M2S:
        clear()
        t = time.time()
        top = m2_import.load(path, {"display": "NONE"})
        secs = time.time() - t
        m = m2_import.read(path)
        prof = m.skins[0].profile
        root = m.root
        subs = {o.wow_m2_submesh.index: o for o in top.all_objects if o.type == "MESH" and o.wow_m2_submesh.index >= 0}
        assert len(subs) == len(prof.submeshes), path
        skin_verts = np.asarray(prof.vertices)
        for si, sm in enumerate(prof.submeshes):
            ob = subs[si]
            me = ob.data
            assert len(me.polygons) == sm.index_count // 3 and len(me.vertices) == sm.vertex_count, (path, si)
            wv = np.empty(len(me.vertices), np.int32)
            me.attributes["wow_vertex"].data.foreach_get("value", wv)
            assert (wv == skin_verts[sm.vertex_start:sm.vertex_start + sm.vertex_count]).all()
            co = np.empty(len(me.vertices) * 3, np.float32)
            me.vertices.foreach_get("co", co)
            for k in (0, len(wv) - 1):
                v = root.vertices[int(wv[k])]
                assert np.allclose(co.reshape(-1, 3)[k], (v.pos.x, v.pos.y, v.pos.z), atol=1e-5)
            if len(wv):
                v = root.vertices[int(wv[0])]
                lu = np.empty(len(me.loops) * 2, np.float32)
                me.uv_layers["uv_0"].data.foreach_get("uv", lu)
                vi = np.empty(len(me.loops), np.int32)
                me.loops.foreach_get("vertex_index", vi)
                j = int(np.nonzero(vi == 0)[0][0])
                assert np.allclose(lu.reshape(-1, 2)[j], (v.tex_coords[0].x, 1 - v.tex_coords[0].y), atol=1e-5)
            p = ob.wow_m2_submesh
            assert p.skin_section_id == sm.skin_section_id and p.bone_combo_index == sm.bone_combo_index
        arm = bpy.data.objects[f"{top.wow_m2.name and posix_stem(path)}_rig"]
        bones = arm.data.bones
        assert len(bones) == len(root.bones)
        for i, b in enumerate(root.bones):
            eb = next(x for x in bones if x.wow_m2_bone.index == i)
            assert np.allclose(eb.head_local, (b.pivot.x, b.pivot.y, b.pivot.z), atol=1e-5)
            assert (eb.parent.wow_m2_bone.index if eb.parent else -1) == (b.parent_bone if 0 <= b.parent_bone < len(root.bones) else -1)
        for si, ob in list(subs.items())[:6]:
            sums = np.zeros(len(ob.data.vertices))
            for v in ob.data.vertices:
                sums[v.index] = sum(g.weight for g in v.groups)
            assert (np.abs(sums - 1) < 3 / 255).all() or not len(ob.vertex_groups), (path, si)
        p = top.wow_m2
        assert p.path == path and p.format_version == root.format_version and len(p.textures) == len(root.textures)
        assert len(p.materials) == len(root.materials)
        print(f"  {path}: {secs:.2f}s {len(subs)} submeshes {len(bones)} bones")


def posix_stem(path):
    return path.rsplit("/", 1)[1].rsplit(".", 1)[0]


@test
def import_m2_materials_and_extras():
    clear()
    path = M2S[0]
    top = m2_import.load(path)
    root = m2_import.read(path).root
    prof = m2_import.read(path).skins[0].profile
    batches = {}
    for mat in bpy.data.materials:
        for b in mat.wow_m2_batches:
            batches[b.batch_index] = b
    assert len(batches) == len(prof.batches)
    for bi, b in enumerate(prof.batches):
        p = batches[bi]
        assert p.shader_id == b.shader_id and p.material_layer == b.material_layer and p.texture_count == b.texture_count
        assert p.blending_mode == root.materials[b.material_index].blending_mode
    objs = top.all_objects
    assert len([o for o in objs if o.wow_m2_object.kind == "attachment"]) == len(root.attachments)
    assert len([o for o in objs if o.wow_m2_object.kind == "event"]) == len(root.events)
    assert len([o for o in objs if o.type == "CAMERA"]) == len(root.cameras)
    bpy.context.view_layer.update()
    a = next(o for o in objs if o.wow_m2_object.kind == "attachment")
    att = root.attachments[a.wow_m2_object.index]
    assert np.allclose(a.matrix_world.translation, (att.position.x, att.position.y, att.position.z), atol=1e-4)
    assert any(o.name.endswith("_collision") and o.hide_viewport for o in objs)
    # display swap changes the skin image and keeps one mesh visible
    imgs = lambda: {n.image.name for m in bpy.data.materials for n in m.node_tree.nodes if n.bl_idname == "ShaderNodeTexImage"}
    before = imgs()
    ds = displays.for_model(m2_import.index.fdid_of(path))
    other = next(d for d in ds if d.textures[0] and d.textures[1] and d.id != top.wow_m2.display_id)
    m2_import.set_display(top, other.id)
    assert top.wow_m2.display_id == other.id
    assert imgs() != before or any(not d.textures[1] for d in ds[:1])
    clear()
    top = m2_import.load("creature/wolf/wolf.m2", {"display": "NONE"})
    assert top.wow_m2.display_id == -1
    assert any(n.image.name == textures.PLACEHOLDER for m in bpy.data.materials for n in m.node_tree.nodes if n.bl_idname == "ShaderNodeTexImage")


@test
def import_m2_hardcoded_and_particles():
    clear()
    top = m2_import.load(find_m2("duskwoodlamppost.m2"))
    mats = [m for m in bpy.data.materials if len(m.wow_m2_batches)]
    assert mats and all(b.texture_fdids[0] for m in mats for b in m.wow_m2_batches)
    clear()
    top = m2_import.load(find_m2("sc_brazier1.m2"))
    assert any(o.wow_m2_object.kind == "particle" for o in top.all_objects)
    clear()
    top = m2_import.load(M2S[2])
    assert any(o.type == "LIGHT" for o in top.all_objects)


@test
def import_doodad_meshes():
    clear()
    path = "world/wmo/kalimdor/ogrimmar/kl_pvpbarracks.wmo"
    t = time.time()
    coll = wmo_import.load(path, {"doodad_meshes": True})
    root = wmo_expect(path)[1]
    inst = [o for o in coll.all_objects if o.instance_type == "COLLECTION"]
    assert 0 < len(inst) <= len(root.doodad_defs)
    lib = bpy.data.collections["wmo_doodad_lib"]
    assert len(lib.children) == len({o.instance_collection for o in inst})
    assert all(o.wow_wmo_doodad.index >= 0 for o in inst)
    print(f"  {len(inst)} instances, {len(lib.children)} models, {time.time() - t:.1f}s")
    n = len(lib.children)
    wmo_import.load(path, {"doodad_meshes": True})
    assert len(bpy.data.collections["wmo_doodad_lib"].children) == n


@test
def operators_m2():
    clear()
    wm = bpy.context.window_manager
    assert bpy.ops.wow.import_path(path="creature/wolf/wolf.m2") == {"FINISHED"}
    assert bpy.data.collections["wolf.m2"]
    bpy.context.view_layer.objects.active = bpy.data.objects["wolf_0_0"]
    assert bpy.ops.wow.m2_set_display(display_id=-1) == {"FINISHED"}
    wm.wow_kind = "m2"


ANIM_MODELS = ["creature/wolf/wolf.m2", "character/human/male/humanmale.m2", "spells/fireball_missile_high.m2"]


def anim_diff(path):
    """First difference between the source M2 root and the one rebuilt from the imported Blender animation, or None."""
    clear()
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    fresh = m2_import.read(path)
    m2_anim_export.build(top, fresh.root)
    return deep_diff(m2_import.read(path).root, fresh.root, "root")


@test
def m2_anim_import():
    clear()
    t = time.time()
    top = m2_import.load("creature/wolf/wolf.m2", {"display": "NONE"})
    p = top.wow_m2
    assert len(p.sequences) == 32 and sum(1 for s in p.sequences if s.action) == 30
    assert sum(1 for s in p.sequences if s.flags & 0x40) == 2
    assert bpy.context.scene.render.fps == 1000
    arm = next(o for o in top.all_objects if o.type == "ARMATURE")
    assert all(b.bone.wow_m2_bone.index >= 0 and b.rotation_mode == "QUATERNION" for b in arm.pose.bones)
    assert m2_seq.play(top, 0)
    assert arm.animation_data.action == p.sequences[0].action and arm.animation_data.action_slot is not None
    assert bpy.context.scene.frame_end == p.sequences[0].duration
    clear()
    t = time.time()
    top = m2_import.load("character/human/male/humanmale.m2", {"display": "NONE"})
    secs = time.time() - t
    assert len(top.wow_m2.sequences) == 143 and len(top.wow_m2.loops) > 0 and secs < 10, secs
    m2_seq.play(top, 1)
    arm = next(o for o in top.all_objects if o.type == "ARMATURE")
    assert "wow_loops" in arm.animation_data.nla_tracks


@test
def m2_anim_cquat_codec():
    raw = np.array([(-32768, -1, 0, 32767), (5, -5, 32766, -32767)], dtype=[(n, "<i2") for n in "xyzw"])
    dec = m2_anim._cquat(raw)
    back = m2_anim._enc_cquat(dec)
    assert np.array_equal(np.asarray(back)[["x", "y", "z", "w"]].tolist(), raw.tolist()), (back, raw)
    every = np.arange(-32768, 32768, dtype=np.int16)
    arr = np.zeros(len(every), dtype=raw.dtype)
    arr["x"] = arr["y"] = arr["z"] = arr["w"] = every
    assert np.array_equal(np.asarray(m2_anim._enc_cquat(m2_anim._cquat(arr)))["x"], every)


@test
def m2_anim_round_trip():
    for path in ANIM_MODELS:
        d = anim_diff(path)
        assert d is None, (path, d)


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


def sweep_m2():
    index.load(client.session().content, client.prefs().cache_dir)
    fails, t0 = [], time.time()
    entries = index.entries("m2")
    only = os.environ.get("SWEEP_ONLY")
    if only:
        entries = [e for e in entries if only in e[1]]
    for fdid, path, _ in entries:
        clear()
        if os.environ.get("SWEEP_VERBOSE"):
            print("M2", path, flush=True)
        try:
            m2_import.load(path, {"display": "NONE"})
        except Exception as e:  # noqa: BLE001
            fails.append((path, f"{type(e).__name__}: {e}"[:200]))
    fails = [f for f in fails if f[0] not in KNOWN_BAD]
    print(f"m2 sweep: {len(entries)} M2s, {len(fails)} failures, {time.time() - t0:.0f}s")
    for f in fails[:400]:
        print("  FAIL", *f)
    return fails


def sweep_m2_anim():
    index.load(client.session().content, client.prefs().cache_dir)
    fails, t0 = [], time.time()
    entries = index.entries("m2")
    only, step = os.environ.get("SWEEP_ONLY"), int(os.environ.get("SWEEP_STEP", "1"))
    if only:
        entries = [e for e in entries if only in e[1]]
    entries = entries[::step]
    for fdid, path, _ in entries:
        if path in KNOWN_BAD:
            continue
        try:
            d = anim_diff(path)
            if d:
                fails.append((path, d))
        except Exception as e:  # noqa: BLE001
            fails.append((path, f"{type(e).__name__}: {e}"[:200]))
    print(f"m2 anim sweep: {len(entries)} M2s, {len(fails)} differences, {time.time() - t0:.0f}s")
    for f in fails[:400]:
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
if "--all-m2" in ARGS:
    failed += len(sweep_m2())
if "--all-m2-anim" in ARGS:
    failed += len(sweep_m2_anim())
print("RESULT", "FAIL" if failed else "OK")
sys.exit(1 if failed else 0)
