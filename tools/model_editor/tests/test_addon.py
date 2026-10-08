"""Headless addon tests. Run: blender -b --factory-startup --python test_addon.py -- [--all] [--filter SUBSTR]"""
import json
import os
import sys
import time
import types
import traceback

import bpy
import mathutils
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
m2_export = common.submodule("m2_export")
fdids = common.submodule("fdids")
m2_seq = common.submodule("m2_seq")
wmo_export = common.submodule("wmo_export")
wmo_bsp = common.submodule("wmo_bsp")
wmo_geometry = common.submodule("wmo_geometry")

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


M2S = ["creature/wolf/wolf.m2", "character/human/male/humanmale.m2", "spells/fireball_missile_high.m2", "spells/arcaneexplosion_base.m2", "spells/assassinate_missile.m2",
              "character/dwarf/female/dwarffemale.m2", "creature/cryptscarab/crypt_scarab.m2"]


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
        arm = bpy.data.objects[f"{posix_stem(path)}_rig"]
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
def m2_camera_fov_driven_by_prop():
    clear()
    top = m2_import.load("cameras/flybydwarf.m2", {"display": "NONE"})
    cam = next(o for o in top.all_objects if o.type == "CAMERA")
    cam["wow_fov"] = (0.5, 0.0, 0.0)
    bpy.context.view_layer.update()
    assert abs(cam.data.angle_y - 0.5) < 1e-4, cam.data.angle_y
    clear()


@test
def m2_light_driven_by_props():
    clear()
    ob = bpy.data.objects.new("l", bpy.data.lights.new("l", "POINT"))
    bpy.context.scene.collection.objects.link(ob)
    ob.data.use_custom_distance = True
    ob["wow_diffuse_color"], ob["wow_diffuse_intensity"], ob["wow_attenuation_end"] = (0.2, 0.4, 0.6), 2.0, 30.0
    m2_anim.drive_object_data(types.SimpleNamespace(objects={("light", 0): ob}))
    bpy.context.view_layer.update()
    assert np.allclose(ob.data.color, (0.2, 0.4, 0.6), atol=1e-5) and abs(ob.data.energy - 200) < 1e-3
    assert abs(ob.data.cutoff_distance - 30) < 1e-4
    clear()


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


MPY2_WMO = "world/wmo/dungeon/md_crypt/md_crypt_d.wmo"


def export_files(path, force=False, import_opts=None):
    clear()
    coll = wmo_import.load(path, import_opts or {"doodads": False})
    r = wmo_export.export(coll, options={"dry_run": True, "force": force})
    return coll, r


def differing(files):
    content = client.session().content
    return [k for k, v in files.items() if not (content / k).is_file() or (content / k).read_bytes() != v]


@test
def bsp_matches_brute_force():
    rng = np.random.default_rng(7)
    for path, gis in ((REFS[1], (1, 3, 5)), (REFS[0], (0,))):
        w = wmo_expect(path)[0]
        for gi in gis:
            b = w.groups[gi].body
            v = np.asarray(b.vertices).view(np.float32).reshape(-1, 3)
            tris = np.asarray(b.indices).reshape(-1, 3).astype(int)
            nodes, faces = wmo_bsp.build(v, tris)
            assert wmo_bsp.covers_all(nodes, faces, len(tris))
            lo, hi = v.min(0), v.max(0)
            segs = [(rng.uniform(lo, hi), rng.uniform(lo, hi)) for _ in range(150)]
            assert wmo_bsp.check(nodes, faces, v, tris, segs) == 0
            assert nodes["flags"].max() <= 4 and len(faces) >= len(tris)
    nodes, faces = wmo_bsp.build(np.zeros((3, 3)), np.zeros((0, 3), int))
    assert len(nodes) == 0 and len(faces) == 0


@test
def geometry_vertex_split_and_batches():
    co = np.array([[0, 0, 0], [1, 0, 0], [0, 1, 0], [1, 1, 0], [5, 5, 5]], np.float32)
    tri_vi = np.array([[0, 1, 2], [1, 3, 2], [0, 1, 2]])
    n = np.tile(np.array([0, 0, 1], np.float32), (3, 3, 1))
    n[2, 0] = (0, 1, 0)
    uv = [np.zeros((3, 3, 2), np.float32)]
    c = {"co": co, "tri_vi": tri_vi, "tri_normal": n, "tri_uv": uv, "vcol": [np.full((5, 4), 255, np.uint8)],
         "face_mat": np.array([3, 3, -1]), "face_flags": np.array([0x24, 0, 0]), "face_batch": np.array([-1, -1, -1]),
         "default_class": wmo_geometry.INT}
    a = wmo_geometry.build_group(c)
    assert len(a.vertices) == 6, len(a.vertices)
    assert list(a.batches["material"]) == [3] and a.batches["count"][0] == 6
    assert list(a.poly_mat) == [3, 3, 255] and list(a.poly_flags) == [0x24, 0x24, 0x08]
    assert a.batches["start"][0] == 0 and a.indices.max() < 6
    assert (a.vertices[:, 0] <= 5).all() and (a.vertices == 5).any()
    src = dict(c, src_class=[wmo_geometry.TRANS, wmo_geometry.EXT], src_mat=[3, 9])
    c2 = dict(src, face_mat=np.array([9, 3, 3]), face_batch=np.array([1, 1, 0]))
    a = wmo_geometry.build_group(c2)
    assert list(a.batch_class) == [wmo_geometry.TRANS, wmo_geometry.EXT, wmo_geometry.EXT]
    assert list(a.batches["material"]) == [3, 9, 3] and list(a.face_order) == [2, 0, 1]


@test
def wmo_export_unedited_is_identical():
    for path in REFS + [MPY2_WMO]:
        t = time.time()
        coll, r = export_files(path)
        assert not r.errors, str(r)
        assert r.stats["groups_rebuilt"] == 0, (path, str(r))
        assert differing(r.files) == [], (path, str(r))
        print(f"  {path}: {len(r.files)} files {time.time() - t:.1f}s")


@test
def wmo_export_large_uvs_unedited_clean():
    path = "world/wmo/dungeon/kl_wailing/wailingcaverns.wmo"
    coll, r = export_files(path)
    assert not r.errors and r.stats["groups_rebuilt"] == 0, str(r)
    ob = mesh_groups(coll)[0]
    ob.data.uv_layers[0].data[0].uv.x += 0.01
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors and r.stats["groups_rebuilt"] == 1, str(r)


@test
def wmo_export_forced_rebuild_is_equivalent():
    for path in (REFS[0], REFS[1], MPY2_WMO):
        coll, r = export_files(path, force=True)
        assert not r.errors, str(r)
        assert r.stats["groups_rebuilt"] == len(group_files(r, path)), path
        d = rebuild_diff(path, r.files)
        assert d == [], (path, d[:5])


def group_files(r, path):
    return [k for k in r.files if k != path]


def reread(path, files):
    from wowlib.formats import wmo as wwmo
    w = wwmo.WMO.for_version(client.VERSION)
    gp = [k for k in sorted(files) if k != path and k.endswith(".wmo")]
    w.read(files[path], [files[k] for k in gp])
    return w


def tri_rows(body):
    v = np.asarray(body.vertices).view(np.float32).reshape(-1, 3)
    n = np.asarray(body.normals).view(np.float32).reshape(-1, 3)
    uv = np.asarray(body.texcoords[0]) if len(body.texcoords) else np.zeros(len(v), [("x", "f4"), ("y", "f4")])
    uv = np.stack([uv["x"], uv["y"]], 1)
    idx = np.asarray(body.indices).astype(int).reshape(-1, 3)
    polys = np.asarray(body.polys if len(body.polys) else body.polys2)
    mat = np.full(len(idx), -1, np.int64)
    for b in body.batches:
        m = b.material_id_large if b.flags & 2 else b.material_id
        mat[b.start_index // 3:(b.start_index + b.count) // 3] = m
    corner = np.rint(np.nan_to_num(np.concatenate([v, uv, n], 1) * 1000)).astype(np.int64)
    return idx, corner, polys["flags"].astype(np.int64)[:len(idx)], mat


def tri_multiset(body, corner_ids):
    idx, corner, flags, mat = tri_rows(body)
    ids = np.sort(corner_ids[idx], axis=1)
    rows = np.concatenate([ids, flags[:, None], mat[:, None]], 1)
    return rows[np.lexsort(rows.T[::-1])]


def rebuild_diff(path, files):
    src, new = wmo_expect(path)[0], reread(path, files)
    out = []
    if len(src.groups) != len(new.groups):
        return ["group count"]
    for gi, (a, b) in enumerate(zip(src.groups, new.groups)):
        ia, ca, _, _ = tri_rows(a.body)
        ib, cb, _, _ = tri_rows(b.body)
        allc = np.concatenate([ca, cb])
        _, inv = np.unique(allc, axis=0, return_inverse=True)
        inv = inv.reshape(-1)
        ra, rb = tri_multiset(a.body, inv[:len(ca)]), tri_multiset(b.body, inv[len(ca):])
        if ra.shape != rb.shape or not (ra == rb).all():
            out.append(f"group {gi} triangles")
        nb = b.body
        v = np.asarray(nb.vertices).view(np.float32).reshape(-1, 3)
        tris = np.asarray(nb.indices).astype(int).reshape(-1, 3)
        if len(tris):
            nodes, faces = np.asarray(nb.bsp_nodes), np.asarray(nb.bsp_face_indices)
            if not wmo_bsp.covers_all(nodes, faces, len(tris)):
                out.append(f"group {gi} bsp")
    if not new.validate().ok and src.validate().ok:
        out.append("validate")
    return out


@test
def wmo_export_vertex_edit():
    path = REFS[0]
    clear()
    coll = wmo_import.load(path, {"doodads": False})
    ob = mesh_groups(coll)[0]
    ob.data.vertices[5].co.z += 2.0
    ob.data.update()
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors and r.stats["groups_rebuilt"] == 1, str(r)
    d = differing(r.files)
    assert sorted(d) == sorted([path, wmo_export.group_path(path, 0)]), d
    new = reread(path, r.files)
    src = wmo_expect(path)[0]
    va = np.asarray(src.groups[0].body.vertices).view(np.float32).reshape(-1, 3)
    vb = np.asarray(new.groups[0].body.vertices).view(np.float32).reshape(-1, 3)
    assert len(va) == len(vb) and np.allclose(vb[5] - va[5], (0, 0, 2.0), atol=1e-5)
    assert (np.abs(vb - va).sum(1) > 1e-6).sum() == 1
    hb = new.groups[0].body.header.bounding_box
    assert hb.max.z >= vb[:, 2].max() and new.validate().ok


@test
def wmo_export_materials():
    path = REFS[0]
    clear()
    common.scratch_world("wmo1")
    coll = wmo_import.load(path, {"doodads": False})
    ob = mesh_groups(coll)[0]
    mat = next(s.material for s in ob.material_slots if s.material.wow_wmo_material.material_index >= 0)
    idx = mat.wow_wmo_material.material_index
    mat.wow_wmo_material.blend_mode = 2
    nm = mat.copy()
    nm.wow_wmo_material.material_index = -1
    img = bpy.data.images.new("My Tex", 16, 16, alpha=True)
    img.pixels.foreach_set(np.tile(np.array([1, 0, 0, 1], np.float32), 256))
    for n in nm.node_tree.nodes:
        if n.label == "texture1":
            n.image = img
    ob.data.materials.append(nm)
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    new = reread(path, r.files)
    src = wmo_expect(path)[1]
    assert len(new.root.materials) == len(src.materials) + 1
    assert new.root.materials[idx].blend_mode == 2 and src.materials[idx].blend_mode != 2
    fdid = new.root.materials[len(src.materials)].texture1
    assert fdid >= 2_000_000_000
    assert r.files["world/wmo/custom/my_tex.blp"][:4] == b"BLP2"
    assert fdids.read() == {fdid: "world/wmo/custom/my_tex.blp"}
    r2 = wmo_export.export(coll, options={"dry_run": True})
    assert len(fdids.read()) == 1 and not r2.errors


@test
def wmo_export_validation():
    path = REFS[0]
    clear()
    common.scratch_world("wmo2")
    coll = wmo_import.load(path, {"doodads": False})
    ob = mesh_groups(coll)[0]
    mat = next(s.material for s in ob.material_slots if s.material.wow_wmo_material.material_index >= 0)
    nm = mat.copy()
    nm.wow_wmo_material.material_index = -1
    img = bpy.data.images.new("Odd Tex", 12, 8, alpha=True)
    for n in nm.node_tree.nodes:
        if n.label == "texture1":
            n.image = img
    ob.data.materials.append(nm)
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    assert any("power of two" in i[2] for i in r.items), str(r)


@test
def wmo_export_new_group():
    path = REFS[0]
    clear()
    common.scratch_world("wmo3")
    coll = wmo_import.load(path, {"doodads": False})
    src = mesh_groups(coll)[0]
    nob = src.copy()
    nob.data = src.data.copy()
    nob.name = "my_new_group"
    nob.wow_wmo_group.index = -1
    nob.wow_wmo_group.group_name = "extra"
    nob.location.z += 5
    next(c for c in coll.children if c.name.startswith("groups")).objects.link(nob)
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    new = reread(path, r.files)
    old = wmo_expect(path)[0]
    n = len(old.groups)
    assert len(new.groups) == n + 1 and r.new_groups["my_new_group"] == n
    a = np.asarray(old.groups[0].body.vertices).view(np.float32).reshape(-1, 3)
    b = np.asarray(new.groups[n].body.vertices).view(np.float32).reshape(-1, 3)
    assert len(a) == len(b) and np.allclose(np.sort(a[:, 2]) + 5, np.sort(b[:, 2]), atol=1e-3)
    assert len(new.root.group_infos) == n + 1 and new.validate().ok, "; ".join(str(i.message) + str(i.path) for i in new.validate().issues)
    assert path.replace(".wmo", f"_{n:03d}.wmo") in r.files


@test
def wmo_export_new_wmo():
    clear()
    common.scratch_world("wmo4")
    src = wmo_import.load(REFS[0], {"doodads": False})
    mat = next(s.material for s in mesh_groups(src)[0].material_slots if s.material.wow_wmo_material.material_index >= 0).copy()
    mat.wow_wmo_material.material_index = -1
    path = "world/wmo/custom/test_box.wmo"
    top = wmo_export.new_wmo(path)
    me = bpy.data.meshes.new("box")
    v = [(x, y, z) for x in (0, 4) for y in (0, 4) for z in (0, 4)]
    f = [(0, 1, 3, 2), (4, 6, 7, 5), (0, 4, 5, 1), (2, 3, 7, 6), (0, 2, 6, 4), (1, 5, 7, 3)]
    me.from_pydata(v, [], f)
    me.uv_layers.new(name="uv0")
    me.materials.append(mat)
    ob = bpy.data.objects.new("box", me)
    next(c for c in top.children if c.name.startswith("groups")).objects.link(ob)
    r = wmo_export.export(top, options={"dry_run": True})
    assert not r.errors, str(r)
    new = reread(path, r.files)
    assert len(new.groups) == 1 and len(new.root.materials) == 1 and len(new.groups[0].body.vertices) >= 8
    assert new.validate().ok, "; ".join(str(i.message) + str(i.path) for i in new.validate().issues)
    assert path in r.files and path.replace(".wmo", "_000.wmo") in r.files


@test
def wmo_export_lod_warning():
    path = "world/wmo/dungeon/md_crypt/classic_md_crypt_d.wmo"
    clear()
    coll = wmo_import.load(path, {"doodads": False})
    ob = mesh_groups(coll)[0]
    ob.data.vertices[0].co.z += 0.5
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    assert any("_lod1.wmo" in i[2] for i in r.items), str(r)


@test
def wmo_export_root_objects():
    path = REFS[1]
    coll, r = export_files(path, import_opts={"doodads": True})
    assert not r.errors and differing(r.files) == [], str(r)
    doodads = [o for o in coll.all_objects if o.type == "EMPTY" and o.wow_wmo_doodad.index >= 0]
    assert len(doodads) > 50
    d0 = next(o for o in doodads if o.wow_wmo_doodad.index == 3)
    d0.location.z += 1.5
    extra = bpy.data.objects.new("new_doodad", None)
    d0.users_collection[0].objects.link(extra)
    extra.location = d0.location
    p = extra.wow_wmo_doodad
    p.index, p.doodad_set, p.fdid, p.scale, p.color = -1, 0, d0.wow_wmo_doodad.fdid, 1.0, (255, 255, 255, 255)
    p.name_index = d0.wow_wmo_doodad.name_index
    fog = next(o for o in coll.all_objects if o.name.startswith("fog_"))
    fog.wow_wmo_fog.fog_end += 10
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    new = reread(path, r.files)
    src = wmo_expect(path)[1]
    n = len(src.doodad_defs)
    assert len(new.root.doodad_defs) == n + 1 and new.validate().ok, str(new.validate())
    assert abs(new.root.doodad_defs[3].position.z - src.doodad_defs[3].position.z - 1.5) < 1e-4
    assert abs(new.root.fogs[fog.wow_wmo_fog.index].fog.end - src.fogs[fog.wow_wmo_fog.index].fog.end - 10) < 1e-3
    assert sum(s.count for s in new.root.doodad_sets) >= sum(s.count for s in src.doodad_sets) + 1
    refs = sum(len(g.body.doodad_refs) for g in new.groups) - sum(len(g.body.doodad_refs) for g in wmo_expect(path)[0].groups)
    assert refs >= 1


@test
def wmo_doodad_delete():
    path = REFS[1]
    coll, r = export_files(path, import_opts={"doodads": True})
    src = wmo_expect(path)[1]
    n = len(src.doodad_defs)
    obs = {o.wow_wmo_doodad.index: o for o in coll.all_objects if o.type == "EMPTY" and o.wow_wmo_doodad.index >= 0}
    name3 = int(src.doodad_defs[3].name_index)
    gone = [i for i in range(n) if int(src.doodad_defs[i].name_index) == name3] + [10]
    for i in set(gone):
        bpy.data.objects.remove(obs[i])
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    new = reread(path, r.files)
    root = new.root
    drop = set(gone)
    assert len(root.doodad_defs) == n - len(drop) and new.validate().ok, str(new.validate())
    assert sum(s.count for s in root.doodad_sets) == sum(s.count for s in src.doodad_sets) - len(drop)
    assert len(root.doodad_fdids) == len({int(src.doodad_defs[i].name_index) for i in range(n) if i not in drop})
    assert all((int(d.name_index)) < len(root.doodad_fdids) for d in root.doodad_defs)
    keep = [i for i in range(n) if i not in drop]
    for j, i in enumerate(keep[:40]):
        assert abs(root.doodad_defs[j].position.x - src.doodad_defs[i].position.x) < 1e-4
        assert root.doodad_fdids[int(root.doodad_defs[j].name_index)] == src.doodad_fdids[int(src.doodad_defs[i].name_index)]
    for g in new.groups:
        assert all(int(x) < len(root.doodad_defs) for x in g.body.doodad_refs)
    old_total = sum(len(g.body.doodad_refs) for g in wmo_expect(path)[0].groups)
    assert sum(len(g.body.doodad_refs) for g in new.groups) < old_total


@test
def wmo_light_fog_delete():
    path = REFS[2]
    coll, r = export_files(path, import_opts={"lights": True})
    src = wmo_expect(path)[1]
    nl, nf = len(src.lights), len(src.fogs)
    lights = sorted((o for o in coll.all_objects if o.type == "LIGHT"), key=lambda o: o.wow_wmo_light.index)
    fogs = sorted((o for o in coll.all_objects if o.name.startswith("fog_")), key=lambda o: o.wow_wmo_fog.index)
    assert nl > 5 and nf >= 3
    keep_pos = (src.lights[5].position.x, src.lights[5].position.y)
    bpy.data.objects.remove(lights[0])
    bpy.data.objects.remove(lights[4])
    bpy.data.objects.remove(fogs[0])
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    new = reread(path, r.files)
    assert len(new.root.lights) == nl - 2 and len(new.root.fogs) == nf - 1 and new.validate().ok, str(new.validate())
    assert int(new.root.header.n_lights) == nl - 2
    assert abs(new.root.lights[3].position.x - keep_pos[0]) < 1e-4
    assert all(int(x) < nf - 1 for g in new.groups for x in g.body.header.fog_ids)


@test
def wmo_portal_create_delete():
    path = REFS[1]
    coll, r = export_files(path)
    src = wmo_expect(path)[1]
    n = len(src.portals)
    portals = sorted((o for o in coll.all_objects if o.name.startswith("portal_")), key=lambda o: o.wow_wmo_portal.index)
    assert n > 3
    keep_v = [(v.x, v.y, v.z) for v in src.portal_vertices[src.portals[3].start_vertex:src.portals[3].start_vertex + src.portals[3].count]]
    keep_refs = sorted((int(x.group_index), int(x.side)) for x in src.portal_refs if int(x.portal_index) == 3)
    bpy.data.objects.remove(portals[1])
    old = portals[0]
    ob = bpy.data.objects.new("portal_new", bpy.data.meshes.new("portal_new"))
    c = old.data.vertices[0].co
    ob.data.from_pydata([c, c + mathutils.Vector((0, 0, 3)), c + mathutils.Vector((3, 0, 3)), c + mathutils.Vector((3, 0, 0))], [], [[0, 1, 2, 3]])
    ob.location = old.matrix_world.translation
    next(c for c in coll.children_recursive if c.name.startswith("portals")).objects.link(ob)
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    new = reread(path, r.files)
    assert len(new.root.portals) == n and new.validate().ok, str(new.validate())
    assert sum(int(g.body.header.portal_count) for g in new.groups) == len(new.root.portal_refs)
    got_refs = sorted((int(x.group_index), int(x.side)) for x in new.root.portal_refs if int(x.portal_index) == 2)
    assert got_refs == keep_refs, (got_refs, keep_refs, [(int(x.portal_index), int(x.group_index)) for x in new.root.portal_refs])
    last = new.root.portals[n - 1]
    assert last.count == 4
    assert any(int(x.portal_index) == n - 1 for x in new.root.portal_refs)
    p2 = new.root.portals[2]
    got = [(v.x, v.y, v.z) for v in new.root.portal_vertices[p2.start_vertex:p2.start_vertex + p2.count]]
    assert np.allclose(got, keep_v, atol=1e-4)


@test
def wmo_group_delete():
    path = REFS[1]
    coll, r = export_files(path)
    w0 = wmo_expect(path)[0]
    src = w0.root
    n = len(w0.groups)
    assert n > 3 and coll.wow_wmo.all_groups
    victim = 1
    ob = next(o for o in coll.all_objects if o.type == "MESH" and o.wow_wmo_group.index == victim)
    keep_verts = len(w0.groups[2].body.vertices)
    bpy.data.objects.remove(ob)
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    new = reread(path, r.files)
    assert len(new.groups) == n - 1 and len(new.root.group_infos) == n - 1 and len(new.root.group_fdids) == n - 1
    assert len(new.groups[1].body.vertices) == keep_verts
    assert new.validate().ok, "; ".join(str(i.message) + str(i.path) for i in new.validate().issues)
    assert all(int(x.group_index) < n - 1 for x in new.root.portal_refs)
    assert sum(int(g.body.header.portal_count) for g in new.groups) == len(new.root.portal_refs)
    assert len(new.root.portals) < len(src.portals)


@test
def wmo_group_reorder():
    path = REFS[1]
    coll, r = export_files(path)
    w0 = wmo_expect(path)[0]
    n = len(w0.groups)
    objs = {o.wow_wmo_group.index: o for o in coll.all_objects if o.type == "MESH" and o.wow_wmo_group.index >= 0}
    objs[0].wow_wmo_group.position = 2
    objs[2].wow_wmo_group.position = 0
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    new = reread(path, r.files)
    assert len(new.groups) == n
    for a, b in ((0, 2), (2, 0), (1, 1)):
        assert len(new.groups[a].body.vertices) == len(w0.groups[b].body.vertices)
    assert new.validate().ok, "; ".join(str(i.message) + str(i.path) for i in new.validate().issues)
    assert len(new.root.portals) == len(w0.root.portals)
    assert sum(int(g.body.header.portal_count) for g in new.groups) == len(new.root.portal_refs)
    old_pairs = sorted((int(x.group_index) for x in w0.root.portal_refs))
    swap = {0: 2, 2: 0}
    assert sorted(int(x.group_index) for x in new.root.portal_refs) == sorted(swap.get(g, g) for g in old_pairs)


@test
def wmo_material_flag_toggle():
    path = REFS[0]
    coll, _ = export_files(path)
    ob = mesh_groups(coll)[0]
    mat = next(s.material for s in ob.material_slots if s.material.wow_wmo_material.material_index >= 0)
    i = mat.wow_wmo_material.material_index
    before = int(wmo_expect(path)[0].root.materials[i].flags)
    assert bpy.types.WOW_PT_wmo_material.poll(types.SimpleNamespace(material=mat, active_object=ob))
    ob.active_material_index = [s.material for s in ob.material_slots].index(mat)
    with bpy.context.temp_override(active_object=ob):
        bpy.ops.wow.toggle_flag(owner="active_material.wow_wmo_material", attr="flags", bit=2)
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    assert int(reread(path, r.files).root.materials[i].flags) == before ^ 4


@test
def wmo_light_type_edit():
    path = next(p for p in REFS if any(l for l in wmo_expect(p)[0].root.lights))
    coll, _ = export_files(path, import_opts={"doodads": False})
    ob = next(o for o in coll.all_objects if o.type == "LIGHT" and o.wow_wmo_light.index >= 0)
    i = ob.wow_wmo_light.index
    ob.data.type = "SUN" if ob.data.type != "SUN" else "POINT"
    want = 2 if ob.data.type == "SUN" else 0
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    assert int(reread(path, r.files).root.lights[i].type) == want


@test
def wmo_group_meta_edit():
    path = REFS[1]
    coll, r = export_files(path)
    ob = mesh_groups(coll)[2]
    p = ob.wow_wmo_group
    src = wmo_expect(path)[0].groups[2].body.header
    p.group_name, p.descriptive_name = "renamed_group", "A fresh description"
    p.flags = int(src.flags) ^ 0x1000
    p.fog_ids = (1, 0, 0, 0)
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    new = reread(path, r.files)
    h = new.groups[2].body.header
    names = new.root.group_names
    assert names.at(h.group_name) == "renamed_group" and names.at(h.descriptive_group_name) == "A fresh description"
    assert names.at(new.root.group_infos[2].name_offset) == "renamed_group"
    assert (int(h.flags) ^ int(src.flags)) == 0x1000 and int(new.root.group_infos[2].flags) == int(h.flags)
    assert int(h.fog_ids[0]) == 1 and new.validate().ok, str(new.validate())


@test
def wmo_header_edit():
    path = REFS[0]
    coll, r = export_files(path)
    p = coll.wow_wmo
    src = wmo_expect(path)[1].header
    p.wmo_id = 4321
    p.flags = int(src.flags) ^ 0x2
    p.ambient_color = (10, 20, 30, 255)
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    h = reread(path, r.files).root.header
    assert h.wmo_id == 4321 and int(h.flags) ^ int(src.flags) == 0x2
    assert (h.ambient_color.r, h.ambient_color.g, h.ambient_color.b) == (10, 20, 30)


@test
def wmo_add_menu_objects_export():
    path = REFS[1]
    clear()
    coll = wmo_import.load(path, {"doodads": True, "lights": True})
    src = wmo_expect(path)[1]
    bpy.context.view_layer.objects.active = next(iter(mesh_groups(coll).values()))
    model = index.path_of(src.doodad_fdids[0])
    for kind in ("doodad", "light", "fog", "portal"):
        assert bpy.ops.wow.add_wmo_object(kind=kind, path=model) == {"FINISHED"}, kind
        bpy.context.view_layer.objects.active = next(iter(mesh_groups(coll).values()))
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    new = reread(path, r.files)
    root = new.root
    assert len(root.doodad_defs) == len(src.doodad_defs) + 1, (len(root.doodad_defs), len(src.doodad_defs))
    assert len(root.lights) == len(src.lights) + 1, (len(root.lights), len(src.lights))
    assert len(root.fogs) == len(src.fogs) + 1 and len(root.portals) == len(src.portals) + 1
    assert new.validate().ok, str(new.validate())


@test
def wmo_liquid_create():
    path = REFS[1]
    clear()
    coll = wmo_import.load(path, {"doodads": False, "liquids": False})
    src = wmo_expect(path)[0]
    gi = next(i for i, g in enumerate(src.groups) if g.body.liquid.empty)
    ob = mesh_groups(coll)[gi]
    bpy.context.view_layer.objects.active = ob
    assert bpy.ops.wow.add_wmo_object(kind="liquid") == {"FINISHED"}
    liq = bpy.context.view_layer.objects.active
    assert liq.name.startswith("liquid_") and liq.wow_wmo_liquid.group_index == gi
    for v in liq.data.vertices:
        v.co.z = 3.0
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    new = reread(path, r.files)
    L = new.groups[gi].body.liquid
    assert not L.empty and (L.verts_dim.x, L.verts_dim.y) == (9, 9) and (L.tiles_dim.x, L.tiles_dim.y) == (8, 8)
    assert all(abs(v.height - 3.0) < 1e-4 for v in L.vertices)
    assert int(new.groups[gi].body.header.flags) & 0x1000
    assert new.validate().ok, str(new.validate())


@test
def wmo_doodad_sets_edit():
    path = REFS[1]
    clear()
    coll = wmo_import.load(path, {"doodads": True})
    src = wmo_expect(path)[1]
    sets = [(s.name, s.start_index, s.count) for s in src.doodad_sets]
    dc = next(c for c in coll.children if c.name.startswith("doodads"))
    colls = [c for c in dc.children if "wow_set" in c]
    assert len(colls) == len(sets) == 1, sets
    movers = sorted(colls[0].objects, key=lambda o: o.name)[:2]
    fresh = bpy.data.collections.new("fresh_set")
    dc.children.link(fresh)
    for ob in movers:
        colls[0].objects.unlink(ob)
        fresh.objects.link(ob)
    colls[0].name = "renamed_set"
    r = wmo_export.export(coll, options={"dry_run": True})
    assert not r.errors, str(r)
    new = reread(path, r.files)
    got = [(s.name, s.count) for s in new.root.doodad_sets]
    assert len(got) == len(sets) + 1 and got[-1] == ("fresh_set", 2), got
    assert got[0] == ("renamed_set", sets[0][2] - 2), got
    assert len(new.root.doodad_defs) == len(src.doodad_defs) and new.validate().ok, str(new.validate())


@test
def wmo_export_operator():
    clear()
    coll = wmo_import.load(REFS[0], {"doodads": False})
    ob = next(iter(mesh_groups(coll).values()))
    with bpy.context.temp_override(active_object=ob):
        assert bpy.ops.wow.export_wmo.poll()
        assert bpy.ops.wow.export_wmo(dry_run=True) == {"FINISHED"}


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


ANIM_MODELS = ["creature/wolf/wolf.m2", "character/human/male/humanmale.m2", "spells/fireball_missile_high.m2", "spells/arcaneexplosion_base.m2", "spells/assassinate_missile.m2"]


def anim_diff(path):
    """First difference between the source M2 root and the one rebuilt from the imported Blender animation, or None."""
    clear()
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    fresh = m2_import.read(path)
    m2_anim_export.build(top, fresh.root)
    return deep_diff(m2_import.read(path).root, fresh.root, "root")


def m2_out_files():
    out = common.Path(client.session().project)
    return sorted(p.relative_to(out).as_posix() for p in out.rglob("*") if p.is_file() and "creature" in p.parts or p.suffix in (".m2", ".skin"))


@test
def m2_export_unchanged_and_forced():
    clear()
    top = m2_import.load("creature/wolf/wolf.m2", {"display": "NONE", "collision": False})
    r = m2_export.export(top)
    assert not r.errors and not r.dirty and r.stats["files_written"] == 0, (r.diff, str(r))
    r = m2_export.export(top, options={"force": True})
    assert not r.errors and r.stats["files_written"] == 3, str(r)
    out = common.Path(client.session().project)
    assert (out / "creature/wolf/wolf.m2").is_file() and (out / "creature/wolf/wolf00.skin").is_file()
    again = m2_import.read("creature/wolf/wolf.m2")
    assert m2_export.differs(r.model, again) is None
    r = m2_export.export(top)
    assert r.stats["files_removed"] == 3 and not (out / "creature/wolf/wolf.m2").exists()


@test
def m2_export_new_path_allocates_ids():
    clear()
    common.scratch_world("m2new")
    try:
        top = m2_import.load("creature/wolf/wolf.m2", {"display": "NONE", "collision": False})
        r = m2_export.export(top, "creature/zzme/wolfcopy.m2", {"force": True})
        assert not r.errors, str(r)
        got = fdids.read()
        assert sorted(got.values()) == ["creature/zzme/wolfcopy.m2", "creature/zzme/wolfcopy00.skin", "creature/zzme/wolfcopy01.skin"], got
        m = m2_import.read("creature/zzme/wolfcopy.m2")
        skin_ids = [int(x) for x in m.chunks.skin_fdids]
        assert skin_ids == [k for k, v in sorted(got.items()) if v.endswith(".skin")], (skin_ids, got)
        assert "wolfcopy" not in common.Path(client.session().content, "fdid.index").read_text()
    finally:
        client.close()


@test
def m2_geometry_unedited_clean():
    for path in ("creature/wolf/wolf.m2", "character/human/male/humanmale.m2"):
        clear()
        top = m2_import.load(path, {"display": "NONE", "collision": False})
        r = m2_export.export(top, options={"dry_run": True})
        assert not r.errors and not r.dirty, (path, r.diff, str(r))


@test
def m2_normals_snap_to_source():
    for stem in ("cat.m2", "fishdeadblue.m2", "energytrail03.m2", "sword_2h_blackwing_a_02.m2"):
        path = find_m2(stem)
        clear()
        top = m2_import.load(path, {"display": "NONE", "collision": False})
        r = m2_export.export(top, options={"dry_run": True})
        assert not r.errors and not r.dirty, (stem, r.diff, str(r))
    geo = common.submodule("m2_geometry")
    z = np.zeros((3, 2), np.float32)
    src = (np.zeros((3, 3), np.float32), np.array([[0, 0, 1], [0, 0, 1], [1e-8, 0, 0]], np.float32),
           np.zeros((3, 2), np.float32), np.zeros((3, 2), np.float32), np.zeros((3, 4), np.uint8), np.zeros((3, 4), np.uint8))
    nrm = np.array([[[0, 0, 1], [0, 1, 0], [0, 0, 1]]], np.float32)
    sub = geo.SubmeshIn(src[0], np.array([[0, 1, 2]], np.int32), nrm, [np.zeros((1, 3, 2), np.float32)] * 2,
                        src[4], src[5], np.arange(3, dtype=np.int32))
    out = geo.split(sub, src)[1]
    assert np.allclose(out[0], (0, 0, 1)) and np.allclose(out[1], (0, 1, 0)), out
    assert np.array_equal(out[2], src[1][2]), out


@test
def m2_index_level_forced_rebuild():
    path = find_m2("10fx_custom_shadow_ring.m2")
    clear()
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    src = m2_import.read(path)
    r = m2_export.export(top, options={"force": True})
    try:
        assert not r.errors, str(r)
        again = m2_import.read(path)
        assert len(again.root.vertices) == len(src.root.vertices)
        sm = again.skins[0].profile.submeshes
        assert max(int(s.level) for s in sm) >= 1 and again.validate().ok
        clear()
        top = m2_import.load(path, {"display": "NONE", "collision": False})
        assert not m2_export.export(top, options={"dry_run": True}).dirty
    finally:
        _cleanup_out(path, len(src.skins))


@test
def m2_geometry_edit_round_trip():
    path = "creature/wolf/wolf.m2"
    clear()
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    subs = sorted((o for o in top.all_objects if o.type == "MESH"), key=lambda o: o.wow_m2_submesh.index)
    n_sub = len(subs)
    ob = subs[0]
    ob.data.vertices[0].co.x += 0.25
    moved = tuple(ob.data.vertices[0].co)
    r = m2_export.export(top, options={"force": True})
    assert not r.errors and r.dirty, str(r)
    again = m2_import.read(path)
    assert len(again.skins) == len(m2_import.read(path).skins) and len(again.skins[0].profile.submeshes) == n_sub
    assert all(len(s.profile.submeshes) == n_sub and len(s.profile.indices) == len(again.skins[0].profile.indices) for s in again.skins)
    assert again.validate().ok, [str(i) for i in again.validate().issues]
    v = again.root.vertices[0]
    assert abs(v.pos.x - moved[0]) < 1e-5, (v.pos.x, moved)
    bpy.data.objects.remove(subs[-1])
    r = m2_export.export(top, options={"force": True})
    assert not r.errors, str(r)
    again = m2_import.read(path)
    assert len(again.skins[0].profile.submeshes) == n_sub - 1
    assert all(b.skin_section_index < n_sub - 1 for b in again.skins[0].profile.batches)
    out = common.Path(client.session().project)
    for f in (path, "creature/wolf/wolf00.skin", "creature/wolf/wolf01.skin"):
        (out / f).unlink(missing_ok=True)
    client.close()


@test
def m2_geoset_id_edit():
    path = "creature/wolf/wolf.m2"
    clear()
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    subs = sorted((o for o in top.all_objects if o.type == "MESH"), key=lambda o: o.wow_m2_submesh.index)
    ob = subs[0]
    assert any(a == "wow_m2_submesh" for a, _ in common.submodule("ui").active_groups(ob))
    new_id = ob.wow_m2_submesh.skin_section_id + 7
    ob.wow_m2_submesh.skin_section_id = new_id
    try:
        r = m2_export.export(top, options={"force": True})
        assert not r.errors and r.dirty, str(r)
        again = m2_import.read(path)
        assert int(again.skins[0].profile.submeshes[0].skin_section_id) == new_id
    finally:
        _cleanup_out(path, 2)


@test
def m2_new_mesh_shadow_batch():
    path = "creature/wolf/wolf.m2"
    clear()
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    subs = sorted((o for o in top.all_objects if o.type == "MESH"), key=lambda o: o.wow_m2_submesh.index)
    n_sub = len(subs)
    nob = subs[0].copy()
    nob.data = subs[0].data.copy()
    nob.name = "extra"
    nob.wow_m2_submesh.index = -1
    for c in subs[0].users_collection:
        c.objects.link(nob)
    nob.parent = subs[0].parent
    nob.data.transform(__import__("mathutils").Matrix.Translation((0, 0, 1)))
    try:
        r = m2_export.export(top, options={"force": True})
        assert not r.errors and r.dirty, str(r)
        again = m2_import.read(path)
        prof = again.skins[0].profile
        assert len(prof.submeshes) == n_sub + 1 and again.validate().ok
        assert {int(sb.submesh_id) for sb in prof.shadow_batches} >= {int(b.skin_section_index) for b in prof.batches}
    finally:
        _cleanup_out(path, 2)


def _edit_bones(top, fn):
    arm = next(o for o in top.all_objects if o.type == "ARMATURE")
    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.mode_set(mode="EDIT")
    try:
        fn(arm.data.edit_bones)
    finally:
        bpy.ops.object.mode_set(mode="OBJECT")
    return arm


def _vec3(v):
    return (v.x, v.y, v.z)


def _cleanup_out(path, n_skins=1):
    out = common.Path(client.session().project)
    base = path[:-3]
    for f in (path, *(f"{base}{i:02d}.skin" for i in range(n_skins + 1))):
        (out / f).unlink(missing_ok=True)
    client.close()


@test
def m2_rig_edit_round_trip():
    path = "character/human/male/humanmale.m2"
    clear()
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    src = m2_import.read(path)
    n = len(src.root.bones)
    r = m2_export.export(top, options={"dry_run": True})
    assert not r.dirty, r.diff

    by_index = {b.wow_m2_bone.index: b.name for b in next(o for o in top.all_objects if o.type == "ARMATURE").data.bones}

    def edit(eb):
        eb[by_index[3]].head.x += 0.1
        p = eb[by_index[5]]
        e = eb.new("finger_new")
        e.head, e.tail, e.parent = p.head, (p.head.x, p.head.y + 0.05, p.head.z), p
    arm = _edit_bones(top, edit)
    old_name3 = [b.name for b in arm.data.bones if b.wow_m2_bone.index == 3][0]
    new_pivot = tuple(arm.data.bones[old_name3].head_local)
    try:
        r = m2_export.export(top, options={"force": True})
        assert not r.errors, str(r)
        m = m2_import.read(path)
        assert len(m.root.bones) == n + 1
        assert abs(m.root.bones[3].pivot.x - new_pivot[0]) < 1e-5
        assert m.root.bones[n].parent_bone == 5, (m.root.bones[n].parent_bone, [b.name for b in arm.data.bones][-3:])
        assert [int(x) for x in m.root.bone_lookup_table] == [int(x) for x in src.root.bone_lookup_table]
        assert m.validate().ok, [str(i) for i in m.validate().issues]
        top2 = m2_import.load(path, {"display": "NONE", "collision": False})
        assert len(next(o for o in top2.all_objects if o.type == "ARMATURE").data.bones) == n + 1
    finally:
        _cleanup_out(path, len(src.skins))


@test
def m2_rig_delete_remaps_references():
    path = "character/human/male/humanmale.m2"
    clear()
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    src = m2_import.read(path)
    n = len(src.root.bones)
    used = {int(i) for v in src.root.vertices for i, w in zip(v.bone_indices, v.bone_weights) if w}
    parents = {int(b.parent_bone) for b in src.root.bones}
    victim = next(i for i in range(n) if i not in used and i not in parents)
    arm = next(o for o in top.all_objects if o.type == "ARMATURE")
    name = next(b.name for b in arm.data.bones if b.wow_m2_bone.index == victim)
    _edit_bones(top, lambda eb: eb.remove(eb[name]))
    try:
        r = m2_export.export(top, options={"force": True})
        assert not r.errors, str(r)
        m = m2_import.read(path)
        assert len(m.root.bones) == n - 1
        for v_new, v_old in zip(m.root.vertices, src.root.vertices):
            for a, b, w in zip(v_new.bone_indices, v_old.bone_indices, v_old.bone_weights):
                if w:
                    assert int(a) == int(b) - (int(b) > victim)
        for a_new, a_old in zip(m.root.attachments, src.root.attachments):
            want = int(a_old.bone) - (int(a_old.bone) > victim) if int(a_old.bone) != victim else 0
            assert int(a_new.bone) == want
        assert [int(b.parent_bone) for b in m.root.bones] == [
            (p - (p > victim) if p >= 0 else p) for i, p in enumerate(int(b.parent_bone) for b in src.root.bones) if i != victim]
        assert m.validate().ok, [str(i) for i in m.validate().issues]
    finally:
        _cleanup_out(path, len(src.skins))


@test
def m2_material_edit_round_trip():
    path = find_m2("duskwoodlamppost.m2")
    clear()
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    src = m2_import.read(path)
    r = m2_export.export(top, options={"dry_run": True})
    assert not r.dirty, r.diff
    mat = next(m for m in bpy.data.materials if len(m.wow_m2_batches))
    p0 = mat.wow_m2_batches[0]
    old_fdid = int(p0.texture_fdids[0])
    img = bpy.data.images.new("lamp_retex", 16, 16, alpha=True)
    node = next(n for n in mat.node_tree.nodes if n.bl_idname == "ShaderNodeTexImage"
                and n.get("wow_batch") == p0.batch_index and n.get("wow_slot") == 0)
    node.image = img
    p0.blending_mode = 2
    nt, nm = len(src.root.textures), len(src.root.materials)
    try:
        r = m2_export.export(top, options={})
        assert not r.errors, str(r)
        assert r.dirty and any(f.endswith("lamp_retex.blp") for f in r.files), r.files
        blp = next(f for f in r.files if f.endswith("lamp_retex.blp"))
        assert (common.Path(client.session().project) / blp).is_file()
        m = m2_import.read(path)
        b = m.skins[0].profile.batches[p0.batch_index]
        fd = int(m.chunks.texture_fdids[m.root.texture_lookup_table[b.texture_combo_index]])
        assert fd != old_fdid and fd == img["wow_fdid"], (fd, old_fdid)
        assert int(m.root.materials[b.material_index].blending_mode) == 2
        assert len(m.root.textures) >= nt and len(m.root.materials) >= nm
        assert m.validate().ok, [str(i) for i in m.validate().issues]
        top2 = m2_import.load(path, {"display": "NONE", "collision": False})
        assert any(b.texture_fdids[0] == fd for mt in bpy.data.materials for b in mt.wow_m2_batches)
    finally:
        _cleanup_out(path, len(src.skins))
        for f in r.files:
            (common.Path(client.session().project) / f).unlink(missing_ok=True)
        client.close()


@test
def m2_material_layer_add_remove():
    path = find_m2("duskwoodlamppost.m2")
    clear()
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    src = m2_import.read(path)
    nb = len(src.skins[0].profile.batches)
    mat = next(m for m in bpy.data.materials if len(m.wow_m2_batches))
    n0 = len(mat.wow_m2_batches)
    with bpy.context.temp_override(material=mat):
        bpy.ops.wow.add_m2_batch()
    assert len(mat.wow_m2_batches) == n0 + 1 and mat.wow_m2_batches[n0].slot == -1
    mat.wow_m2_batches[n0].blending_mode = 3
    mat.wow_m2_batches[n0].texture_count = 2
    mat.wow_m2_batches[n0].texture_types[1] = 0
    mat.wow_m2_batches[n0].texture_fdids[1] = mat.wow_m2_batches[0].texture_fdids[0]
    users = sum(1 for o in bpy.data.objects if o.material_slots and o.material_slots[0].material == mat)
    r = None
    try:
        r = m2_export.export(top, options={})
        assert not r.errors, str(r)
        m = m2_import.read(path)
        nbn = len(m.skins[0].profile.batches)
        assert nbn == nb + users, (nb, nbn, users)
        assert m.validate().ok, [str(i) for i in m.validate().issues]
        nbat = [b for b in m.skins[0].profile.batches if int(b.texture_count) == 2]
        assert nbat
        top2 = m2_import.load(path, {"display": "NONE", "collision": False})
        mat2 = next(x for x in bpy.data.materials if len(x.wow_m2_batches) == n0 + 1)
        with bpy.context.temp_override(material=mat2):
            bpy.ops.wow.remove_m2_batch(index=n0)
        r = m2_export.export(top2, options={})
        assert not r.errors, str(r)
        assert len(m2_import.read(path).skins[0].profile.batches) == nb
    finally:
        _cleanup_out(path, len(src.skins))
        for f in r.files if r else ():
            (common.Path(client.session().project) / f).unlink(missing_ok=True)
        client.close()


@test
def m2_texture_entries_edit_add():
    path = find_m2("duskwoodlamppost.m2")
    clear()
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    src = m2_import.read(path)
    n0 = len(src.root.textures)
    top.wow_m2.textures[0].flags = 0
    with bpy.context.temp_override(collection=top):
        bpy.ops.wow.add_m2_texture()
    top.wow_m2.textures[n0].fdid = 189514
    top.wow_m2.textures[n0].flags = 1
    r = None
    try:
        r = m2_export.export(top, options={})
        assert not r.errors, str(r)
        m = m2_import.read(path)
        assert len(m.root.textures) == n0 + 1, len(m.root.textures)
        assert int(m.root.textures[0].flags) == 0 and int(m.root.textures[n0].flags) == 1
        assert int(m.chunks.texture_fdids[n0]) == 189514
        assert m.validate().ok, [str(i) for i in m.validate().issues]
    finally:
        _cleanup_out(path, len(src.skins))
        for f in r.files if r else ():
            (common.Path(client.session().project) / f).unlink(missing_ok=True)
        client.close()


@test
def m2_header_flags_and_bounds():
    path = find_m2("duskwoodlamppost.m2")
    clear()
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    src = m2_import.read(path)
    p = top.wow_m2
    with bpy.context.temp_override(collection=top):
        bpy.ops.wow.m2_recompute_bounds()
    sm = src.root.bounding_box.max
    assert all(a <= b + 1e-3 for a, b in zip(p.bounding_max, (sm.x, sm.y, sm.z))) and p.bounding_radius > 0
    p.global_flags = int(src.root.global_flags) ^ 0x10
    p.bounding_radius = p.bounding_radius + 5
    want = p.bounding_radius
    r = None
    try:
        r = m2_export.export(top, options={})
        assert not r.errors, str(r)
        m = m2_import.read(path)
        assert int(m.root.global_flags) == int(src.root.global_flags) ^ 0x10
        assert abs(m.root.bounding_sphere_radius - want) < 1e-3
    finally:
        _cleanup_out(path, len(src.skins))
        for f in r.files if r else ():
            (common.Path(client.session().project) / f).unlink(missing_ok=True)
        client.close()


@test
def m2_batch_static_tracks_new_records():
    path = find_m2("duskwoodlamppost.m2")
    clear()
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    src = m2_import.read(path)
    nc, nw, nt = len(src.root.colors), len(src.root.texture_weights), len(src.root.texture_transforms)
    mat = next(m for m in bpy.data.materials if len(m.wow_m2_batches))
    p = mat.wow_m2_batches[0]
    p.preview_color = (0.25, 0.5, 0.75, 0.5)
    p.preview_weight = 0.5
    p.preview_uv_offset = (0.125, 0.25)
    r = None
    try:
        r = m2_export.export(top, options={})
        assert not r.errors, str(r)
        m = m2_import.read(path)
        root = m.root
        assert m.validate().ok, [str(i) for i in m.validate().issues]
        assert len(root.colors) == nc + 1 and len(root.texture_weights) == nw + 1 and len(root.texture_transforms) == nt + 1
        users = [b for b in m.skins[0].profile.batches if int(b.color_index) == nc]
        assert users
        vals = m2_import._preview_values(root, users[0])
        assert np.allclose(vals[0], (0.25, 0.5, 0.75, 0.5), atol=1e-3) and abs(vals[1] - 0.5) < 1e-3
        assert np.allclose(vals[2], (0.125, 0.25), atol=1e-4)
    finally:
        _cleanup_out(path, len(src.skins))
        for f in r.files if r else ():
            (common.Path(client.session().project) / f).unlink(missing_ok=True)
        client.close()


@test
def m2_filename_textures():
    for stem in ("talktome_chat.m2", "bellow.m2"):
        path = find_m2(stem)
        clear()
        top = m2_import.load(path, {"display": "NONE"})
        r = m2_export.export(top, options={"dry_run": True})
        assert not r.errors and not r.dirty, (stem, str(r), getattr(r, "diff", None))
    src = m2_import.read(path)
    assert not len(src.chunks.texture_fdids) and src.root.textures[0].filename
    mat = next(m for m in bpy.data.materials if len(m.wow_m2_batches))
    p0 = mat.wow_m2_batches[0]
    img = bpy.data.images.new("bellow_retex", 16, 16, alpha=True)
    next(n for n in mat.node_tree.nodes if n.bl_idname == "ShaderNodeTexImage"
         and n.get("wow_batch") == p0.batch_index and n.get("wow_slot") == 0).image = img
    try:
        r = m2_export.export(top, options={})
        assert not r.errors, str(r)
        m = m2_import.read(path)
        fd = list(m.chunks.texture_fdids)
        assert len(fd) == len(m.root.textures) and img["wow_fdid"] in fd and all(fd), fd
        assert not any(t.filename for t in m.root.textures)
        for i, t in enumerate(src.root.textures):
            assert index.fdid_of(t.filename) in fd, t.filename
    finally:
        _cleanup_out(path, len(src.skins))
        for f in r.files:
            (common.Path(client.session().project) / f).unlink(missing_ok=True)
        client.close()


@test
def m2_attachment_lookup_duplicates():
    for stem in ("orcfemale.m2", "axe_1h_horde_c_02.m2"):
        path = find_m2(stem)
        clear()
        top = m2_import.load(path, {"display": "NONE"})
        r = m2_export.export(top, options={"dry_run": True})
        assert not r.errors and not r.dirty, (stem, str(r), getattr(r, "diff", None))


@test
def m2_objects_unedited_clean():
    for path in ("creature/wolf/wolf.m2", "character/human/male/humanmale.m2", find_m2("duskwoodlamppost.m2")):
        clear()
        top = m2_import.load(path, {"display": "NONE"})
        r = m2_export.export(top, options={"dry_run": True})
        assert not r.errors and not r.dirty, (path, r.diff, str(r))


@test
def m2_object_edit_round_trip():
    path = "character/human/male/humanmale.m2"
    clear()
    top = m2_import.load(path, {"display": "NONE"})
    src = m2_import.read(path)
    na, ne = len(src.root.attachments), len(src.root.events)
    assert na > 3 and ne > 1
    atts = sorted((o for o in top.all_objects if o.wow_m2_object.kind == "attachment"), key=lambda o: o.wow_m2_object.index)
    evs = sorted((o for o in top.all_objects if o.wow_m2_object.kind == "event"), key=lambda o: o.wow_m2_object.index)
    gone = atts[2]
    gone_id = gone.wow_m2_object.id
    keep_id = atts[3].wow_m2_object.id
    old_pos = _vec3(src.root.attachments[0].position)
    atts[0].location.z += 0.25
    evs[1].wow_m2_object.data = 77
    bpy.data.objects.remove(gone)
    try:
        r = m2_export.export(top, options={})
        assert not r.errors, str(r)
        assert r.dirty
        m = m2_import.read(path)
        assert len(m.root.attachments) == na - 1 and len(m.root.events) == ne
        assert abs(m.root.attachments[0].position.z - old_pos[2] - 0.25) < 1e-3
        ids = [int(a.id) for a in m.root.attachments]
        assert gone_id not in ids and keep_id in ids
        lut = list(m.root.attachment_lookup_table)
        assert all(ids[lut[i]] == i for i in ids if i < len(lut) and lut[i] != 0xFFFF)
        assert int(m.root.events[1].data) == 77
        assert m.validate().ok, [str(i) for i in m.validate().issues]
    finally:
        _cleanup_out(path, len(src.skins))


LIGHT_MODEL = "interface/glues/models/ui_characterselect/ui_characterselect.m2"


def _kind_obs(top, kind):
    return sorted((o for o in top.all_objects if o.wow_m2_object.kind == kind), key=lambda o: o.wow_m2_object.index)


@test
def m2_light_create_delete():
    clear()
    top = m2_import.load(LIGHT_MODEL, {"display": "NONE"})
    r = m2_export.export(top, options={"dry_run": True})
    assert not r.errors and not r.dirty, (r.diff, str(r))
    src = m2_import.read(LIGHT_MODEL)
    n = len(src.root.lights)
    lights = _kind_obs(top, "light")
    assert n == 3 and len(lights) == 3
    keep = lights[2]
    keep_col = tuple(keep["wow_diffuse_color"]) if "wow_diffuse_color" in keep else None
    bpy.data.objects.remove(lights[1])
    ob = bpy.data.objects.new("new_light", bpy.data.lights.new("new_light", "POINT"))
    top.objects.link(ob)
    ob.wow_m2_object.kind = "light"
    ob.location = (1.0, 2.0, 3.0)
    ob["wow_diffuse_color"] = (0.25, 0.5, 0.75)
    ob["wow_diffuse_intensity"] = 2.0
    ob["wow_attenuation_end"] = 7.0
    r = m2_export.export(top, options={"dry_run": True})
    assert not r.errors, str(r)
    m = r.model
    assert len(m.root.lights) == 3
    old2, new = m.root.lights[1], m.root.lights[2]
    assert _vec3(old2.position) == _vec3(src.root.lights[2].position)
    assert new.type == 1 and np.allclose(_vec3(new.position), (1, 2, 3), atol=1e-4)
    assert new.diffuse_color.timeline_count() == 1 and new.attenuation_end.timeline_count() == 1
    assert np.allclose(np.asarray(new.diffuse_intensity.timeline_values(0), np.float32), 2.0)
    d = new.diffuse_color.timeline_values(0)[0]
    assert np.allclose((d.x, d.y, d.z), (0.25, 0.5, 0.75))
    if keep_col is not None:
        v = old2.diffuse_color.timeline_values(0)[0]
        assert np.allclose((v.x, v.y, v.z), keep_col, atol=1e-4)
    assert m.validate().ok, [str(i) for i in m.validate().issues]


@test
def m2_camera_create_delete():
    path = "character/human/male/humanmale.m2"
    clear()
    top = m2_import.load(path, {"display": "NONE"})
    src = m2_import.read(path)
    cams = _kind_obs(top, "camera")
    assert len(cams) == len(src.root.cameras) == 2
    keep_pos = _vec3(src.root.cameras[1].position_base)
    bpy.data.objects.remove(cams[0])
    ob = bpy.data.objects.new("flyby", bpy.data.cameras.new("flyby"))
    top.objects.link(ob)
    ob.wow_m2_object.kind, ob.wow_m2_object.id = "camera", 5
    ob.location = (4.0, 5.0, 6.0)
    ob.rotation_euler = (np.pi / 2, 0, 0)
    ob.data.clip_start, ob.data.clip_end = 0.5, 90.0
    r = m2_export.export(top, options={"dry_run": True})
    assert not r.errors, str(r)
    m = r.model
    assert len(m.root.cameras) == 2
    old, new = m.root.cameras[0], m.root.cameras[1]
    assert np.allclose(_vec3(old.position_base), keep_pos, atol=1e-4)
    assert int(new.type) == 5 and np.allclose(_vec3(new.position_base), (4, 5, 6), atol=1e-4)
    assert np.allclose(_vec3(new.target_position_base), (4, 5 + 10, 6), atol=1e-3), _vec3(new.target_position_base)
    assert abs(new.far_clip - 90.0) < 1e-4 and abs(new.near_clip - 0.5) < 1e-4
    assert new.fov.timeline_count() == 1 and new.positions.timeline_count() == 1
    lut = list(m.root.camera_lookup_table)
    assert lut[int(old.type)] == 0 and len(lut) >= 6 and lut[5] == 1, lut
    assert m.validate().ok, [str(i) for i in m.validate().issues]


@test
def m2_particle_create_delete_edit():
    path = "interface/glues/models/ui_deathknight/ui_deathknight.m2"
    clear()
    top = m2_import.load(path, {"display": "NONE"})
    r = m2_export.export(top, options={"dry_run": True})
    assert not r.errors and not r.dirty, (r.diff, str(r))
    src = m2_import.read(path)
    n = len(src.root.particle_emitters)
    ps = _kind_obs(top, "particle")
    assert n == len(ps) > 5
    edit = ps[4]
    edit.wow_m2_particle.drag = 0.75
    edit.wow_m2_particle.rows = 2
    edit.wow_m2_particle.alpha_keys[1].value = (0.5, 0, 0)
    edit.wow_m2_particle.color_keys.add().time = 32767
    bpy.data.objects.remove(ps[1])
    ob = bpy.data.objects.new("spark", None)
    top.objects.link(ob)
    ob.wow_m2_object.kind, ob.wow_m2_object.id = "particle", -1
    ob.location = (1.0, 2.0, 3.0)
    ob.wow_m2_particle.tail_length = 3.0
    ob.wow_m2_particle.rows = ob.wow_m2_particle.columns = 1
    m2_particles = common.submodule("m2_particles")
    m2_particles.set_defaults(ob.wow_m2_particle)
    ob["wow_emission_rate"] = 12.0
    r = m2_export.export(top, options={"dry_run": True})
    assert not r.errors, str(r)
    ps_out = list(r.model.root.particle_emitters)
    assert len(ps_out) == n
    e = ps_out[3]
    assert abs(e.drag - 0.75) < 1e-6 and e.rows == 2
    assert abs(e.alpha_track.keys[1].value / 32767.0 - 0.5) < 1e-3 and len(e.color_track.keys) == len(e.color_track.timestamps)
    new = ps_out[-1]
    assert np.allclose(_vec3(new.position), (1, 2, 3), atol=1e-4) and abs(new.tail_length - 3.0) < 1e-6
    assert len(new.color_track.keys) == 3 and list(new.alpha_track.timestamps) == [0, 16384, 32767]
    assert new.emission_rate.timeline_count() == 1 and new.emission_rate.key_count(0) == 1
    assert ps_out[1].position.x == src.root.particle_emitters[2].position.x
    assert r.model.validate().ok, [str(i) for i in r.model.validate().issues]


@test
def m2_ribbon_create_delete_edit():
    path = "creature/spells/fireelementaltotem.m2"
    clear()
    top = m2_import.load(path, {"display": "NONE"})
    r = m2_export.export(top, options={"dry_run": True})
    assert not r.errors and not r.dirty, (r.diff, str(r))
    src = m2_import.read(path)
    n = len(src.root.ribbon_emitters)
    rs = _kind_obs(top, "ribbon")
    assert n == len(rs) == 3
    rs[2].wow_m2_ribbon.edges_per_second = 33.0
    rs[2].wow_m2_ribbon.texture_indices.add().value = 0
    bpy.data.objects.remove(rs[0])
    ob = bpy.data.objects.new("trail", None)
    top.objects.link(ob)
    ob.wow_m2_object.kind, ob.wow_m2_object.id = "ribbon", -1
    ob.location = (0.0, 1.0, 2.0)
    ob.wow_m2_ribbon.texture_indices.add().value = 1
    ob.wow_m2_ribbon.material_indices.add().value = 0
    ob["wow_height_above"] = 0.5
    r = m2_export.export(top, options={"dry_run": True})
    assert not r.errors, str(r)
    out = list(r.model.root.ribbon_emitters)
    assert len(out) == 3
    assert abs(out[1].edges_per_second - 33.0) < 1e-5 and len(out[1].texture_indices) == len(src.root.ribbon_emitters[2].texture_indices) + 1
    new = out[2]
    assert np.allclose(_vec3(new.position), (0, 1, 2), atol=1e-4) and list(new.texture_indices) == [1]
    assert new.height_above.timeline_count() == 1
    ob.wow_m2_ribbon.texture_indices[0].value = 99
    r = m2_export.export(top, options={"dry_run": True})
    assert r.errors


@test
def m2_add_menu_objects_export():
    path = "interface/glues/models/ui_characterselect/ui_characterselect.m2"
    clear()
    top = m2_import.load(path, {"display": "NONE"})
    src = m2_import.read(path)
    bpy.context.view_layer.objects.active = _kind_obs(top, "light")[0]
    for kind in ("attachment", "event", "light", "camera", "particle", "ribbon"):
        assert bpy.ops.wow.add_m2_object(kind=kind) == {"FINISHED"}
        assert bpy.context.active_object.wow_m2_object.kind == kind
        bpy.context.view_layer.objects.active = _kind_obs(top, "light")[0]
    ob = next(o for o in top.all_objects if o.wow_m2_object.kind == "particle")
    bpy.context.view_layer.objects.active = ob
    assert bpy.ops.wow.toggle_flag(owner="wow_m2_particle", attr="flags", bit=3) == {"FINISHED"}
    assert ob.wow_m2_particle.flags == 8
    ob.wow_m2_particle.texture_slots[0] = 0
    r = m2_export.export(top, options={"dry_run": True})
    assert not r.errors, str(r)
    root = r.model.root
    assert len(root.lights) == len(src.root.lights) + 1 and len(root.cameras) == len(src.root.cameras) + 1
    assert len(root.particle_emitters) == len(src.root.particle_emitters) + 1
    assert len(root.ribbon_emitters) == len(src.root.ribbon_emitters) + 1
    assert len(root.attachments) == len(src.root.attachments) + 1 and len(root.events) == len(src.root.events) + 1
    assert int(root.particle_emitters[-1].flags) == 8


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
def m2_anim_model_tracks():
    clear()
    top = m2_import.load("spells/arcaneexplosion_base.m2", {"display": "NONE"})
    paths = {fc.data_path for s in top.wow_m2.sequences if s.action for cb in s.action.layers[0].strips[0].channelbags for fc in cb.fcurves}
    paths |= {fc.data_path for l in top.wow_m2.loops if l.action for cb in l.action.layers[0].strips[0].channelbags for fc in cb.fcurves}
    assert any(p.startswith('["wow_tt_translation') for p in paths), paths


@test
def m2_anim_object_tracks():
    clear()
    top = m2_import.load("character/dwarf/female/dwarffemale.m2", {"display": "NONE"})
    acts = [s.action for s in top.wow_m2.sequences if s.action]
    assert sum(len(a.pose_markers) for a in acts) > 0
    clear()
    top = m2_import.load("spells/fireball_missile_high.m2", {"display": "NONE"})
    assert any(o.get("wow_emission_rate") is not None for o in top.all_objects if o.wow_m2_object.kind == "particle")


@test
def m2_anim_authoring():
    clear()
    path = "creature/wolf/wolf.m2"
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    p = top.wow_m2
    n = len(p.sequences)
    p.active_sequence = 0
    ctx = bpy.context
    arm = next(o for o in top.all_objects if o.type == "ARMATURE")
    ctx.view_layer.objects.active = arm
    assert bpy.ops.wow.m2_sequence_duplicate() == {"FINISHED"} and len(p.sequences) == n + 1
    assert bpy.ops.wow.m2_sequence_add(anim_id=0, duration=500) == {"FINISHED"} and len(p.sequences) == n + 2
    assert p.sequences[-1].action is not None and p.sequences[-1].duration == 500
    fresh = m2_import.read(path)
    m2_anim_export.build(top, fresh.root)
    assert len(fresh.root.sequences) == n + 2
    ids = [int(s.id) for s in fresh.root.sequences]
    assert [int(s.variation_index) for s in fresh.root.sequences][-2:] == [ids[:-2].count(ids[-2]), ids[:-1].count(ids[-1])]
    assert bpy.ops.wow.m2_sequence_remove() == {"FINISHED"} and bpy.ops.wow.m2_sequence_remove() == {"FINISHED"}
    assert len(p.sequences) == n
    d = anim_diff(path)
    assert d is None, d


@test
def m2_anim_bake_bezier():
    clear()
    path = "creature/wolf/wolf.m2"
    top = m2_import.load(path, {"display": "NONE", "collision": False})
    p = top.wow_m2
    seq = next(s for s in p.sequences if s.action is not None and not s.flags & m2_anim.ALIAS_FLAG)
    n = 0
    for lay in seq.action.layers:
        for st in lay.strips:
            for cb in st.channelbags:
                for fc in cb.fcurves:
                    for k in fc.keyframe_points:
                        k.interpolation = "BEZIER"
                        n += 1
    assert n
    fresh = m2_import.read(path)
    rep = common.submodule("report").Report()
    m2_anim_export.build(top, fresh.root, report=rep)
    b = next(b for b in fresh.root.bones if len(b.rotation.timestamps) and len(b.rotation.timestamps[0]) > 1)
    assert int(b.rotation.interpolation_type) == 1
    assert not rep.errors, str(rep)


@test
def m2_validate_findings():
    clear()
    top = m2_import.load("creature/wolf/wolf.m2", {"display": "NONE", "collision": False})
    base = m2_export.export(top, options={"dry_run": True})
    assert not base.errors and not [i for i in base.items if i[0] == "WARNING" and i[1] != "animation"], str(base)
    ob = common.submodule("m2_mesh_export").submesh_objects(top)[0][0]
    import bmesh
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    bmesh.ops.delete(bm, geom=list(bm.faces), context="FACES_ONLY")
    bm.to_mesh(ob.data)
    bm.free()
    rep = m2_export.export(top, options={"dry_run": True})
    hit = [(i, o) for i, o in zip(rep.items, rep.objects) if "no faces" in i[2]]
    assert hit and hit[0][1] is ob, str(rep)


@test
def m2_new_model_exports_and_reimports():
    clear()
    top = m2_export.new_m2("world/custom/testcube.m2")
    assert top.wow_m2.is_new
    ob = common.submodule("m2_mesh_export").submesh_objects(top)[0][0]
    import bmesh
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    bm.to_mesh(ob.data)
    bm.free()
    r = m2_export.export(top)
    assert not r.errors and r.dirty and "world/custom/testcube.m2" in r.files, str(r)
    clear()
    again = m2_import.load("world/custom/testcube.m2", {"display": "NONE", "collision": False})
    sub = common.submodule("m2_mesh_export").submesh_objects(again)[0]
    assert [len(o.data.polygons) for o in sub][0] == 12, [len(o.data.polygons) for o in sub]


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


def matches(only, path):
    """SWEEP_ONLY is a comma-separated list of path fragments."""
    return any(f in path for f in only.split(","))


def progress(label, i, n):
    if i % 200 == 0:
        print(f"  {label}: {i}/{n}", flush=True)


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
        entries = [e for e in entries if matches(only, e[1])]
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
        entries = [e for e in entries if matches(only, e[1])]
    entries = entries[::step]
    for i, (fdid, path, _) in enumerate(entries):
        progress("m2 anim", i, len(entries))
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


def geo_diff(path):
    """First difference after rebuilding the geometry of an unedited import, or None."""
    clear()
    top = m2_import.load(path, {"display": "NONE", "collision": False, "animations": "NONE", "attachments": False,
                                "events": False, "cameras": False, "lights": False, "emitters": False})
    m, ref = m2_import.read(path), m2_import.read(path)
    rep = m2_export.Report(files_written=0, files_removed=0)
    m2_export.m2_mesh_export.build(top, m, rep, m2_export.DEFAULTS)
    return rep.errors and str(rep) or m2_export.differs(ref, m)


def sweep_m2_geo():
    index.load(client.session().content, client.prefs().cache_dir)
    fails, t0 = [], time.time()
    only, step = os.environ.get("SWEEP_ONLY"), int(os.environ.get("SWEEP_STEP", "1"))
    entries = [e for e in index.entries("m2") if not only or matches(only, e[1])][::step]
    for i, (fdid, path, _) in enumerate(entries):
        progress("m2 geometry", i, len(entries))
        if path in KNOWN_BAD:
            continue
        try:
            d = geo_diff(path)
            if d:
                fails.append((path, d))
        except Exception as e:  # noqa: BLE001
            fails.append((path, f"{type(e).__name__}: {e}"[:200]))
    print(f"m2 geometry sweep: {len(entries)} M2s, {len(fails)} differences, {time.time() - t0:.0f}s")
    for f in fails[:400]:
        print("  FAIL", *f)
    return fails


def sweep_wmo_export(force):
    index.load(client.session().content, client.prefs().cache_dir)
    fails, t0 = [], time.time()
    step = int(os.environ.get("SWEEP_STEP", "1"))
    wmos = index.entries("wmo")[::step]
    only = os.environ.get("SWEEP_ONLY")
    if only:
        wmos = [e for e in wmos if matches(only, e[1])]
    slowest = []
    for i, (fdid, path, _) in enumerate(wmos):
        progress(f"wmo export (force={force})", i, len(wmos))
        if path in KNOWN_BAD:
            continue
        t = time.time()
        try:
            coll, r = export_files(path, force, {"doodads": os.environ.get("SWEEP_DOODADS") == "1"})
            if r.errors:
                fails.append((path, str(r)[:200]))
            elif not force and differing(r.files):
                fails.append((path, f"differs: {differing(r.files)[:3]} rebuilt {r.stats['groups_rebuilt']} {str(r)[:150]}"))
            elif force:
                d = rebuild_diff(path, r.files)
                if d:
                    fails.append((path, str(d[:3])))
        except Exception as e:  # noqa: BLE001
            fails.append((path, f"{type(e).__name__}: {e}"[:200]))
        slowest.append((time.time() - t, path))
    slowest.sort(reverse=True)
    print(f"wmo export sweep (force={force}): {len(wmos)} WMOs, {len(fails)} failures, {time.time() - t0:.0f}s, slowest {slowest[:3]}")
    for f in fails[:400]:
        print("  FAIL", *f)
    return fails


def sweep_m2_clean():
    """Unedited import -> export must report no change and no errors (this is also the forced-rebuild comparison)."""
    index.load(client.session().content, client.prefs().cache_dir)
    fails, t0 = {}, time.time()
    only, step = os.environ.get("SWEEP_ONLY"), int(os.environ.get("SWEEP_STEP", "1"))
    entries = [e for e in index.entries("m2") if not only or matches(only, e[1])][::step]
    for i, (fdid, path, _) in enumerate(entries):
        progress("m2 clean", i, len(entries))
        if path in KNOWN_BAD:
            continue
        try:
            clear()
            top = m2_import.load(path, {"display": "NONE"})
            r = m2_export.export(top, options={"dry_run": True})
            if r.errors:
                fails[path] = "errors: " + str(r)[:160]
            elif r.dirty:
                fails[path] = f"dirty: {r.diff}"[:160]
        except Exception as e:  # noqa: BLE001
            fails[path] = f"{type(e).__name__}: {e}"[:160]
    print(f"m2 clean sweep: {len(entries)} M2s, {len(fails)} not clean, {time.time() - t0:.0f}s")
    return fails


def corpus():
    """Sweeps compared with tests/corpus_baseline.json ({kind: {path: message}} for m2_clean, wmo_export, wmo_rebuild, m2_anim); new failures fail, fixed ones are listed."""
    base_file = common.Path(__file__).with_name("corpus_baseline.json")
    base = json.loads(base_file.read_text()) if base_file.is_file() else {}
    found = {"m2_clean": sweep_m2_clean(), "wmo_export": dict(sweep_wmo_export(False)),
             "wmo_rebuild": dict(sweep_wmo_export(True)), "m2_anim": dict(sweep_m2_anim())}
    if "--update-baseline" in ARGS:
        if os.environ.get("SWEEP_STEP", "1") != "1" or os.environ.get("SWEEP_ONLY"):
            raise SystemExit("--update-baseline needs a full run (no SWEEP_STEP/SWEEP_ONLY)")
        added = [(k, p) for k, got in found.items() for p in sorted(set(got) - set(base.get(k, {})))]
        if added and "--allow-new" not in ARGS:
            raise SystemExit(f"--update-baseline would add {len(added)} entries (e.g. {added[0]}); pass --allow-new to accept them")
        base_file.write_text(json.dumps(found, indent=1, sort_keys=True) + "\n")
        return 0
    bad = 0
    for kind, got in found.items():
        known = base.get(kind, {})
        new = sorted(set(got) - set(known))
        fixed = sorted(set(known) - set(got))
        for p in new:
            print("  NEW", kind, p, got[p])
        for p in fixed:
            print("  FIXED", kind, p)
        bad += len(new)
    return bad


@test
def write_item_operator_dry_run():
    clear()
    r = bpy.ops.wow.write_item("EXEC_DEFAULT", template_display=26577, textures="2000000061", variants="2000000071:1:0", dry_run=True)
    assert r == {"FINISHED"}, r
    try:
        bpy.ops.wow.write_item("EXEC_DEFAULT", template_display=26577, sections="0:bad", dry_run=True)
        raise AssertionError("bad sections accepted")
    except RuntimeError:
        pass


@test
def import_item_display_unknown():
    try:
        bpy.ops.wow.import_item_display("EXEC_DEFAULT", display_id=999999999)
        raise AssertionError("unknown display accepted")
    except RuntimeError:
        pass


@test
def import_item_display_texture():
    gd = common.submodule("game_data")
    lay = gd.Layout.discover(client.dirs()[0])
    db = lay.open()
    try:
        did = db.execute(f"SELECT d.ID FROM {gd.IDI} d JOIN {gd.TFD} t ON t.MaterialResourcesID = d.ModelMaterialResourcesID1 "
                         f"WHERE d.ModelResourcesID1 > 0 AND d.ModelMaterialResourcesID1 > 0 AND d.ID < {gd.DISPLAY_FIRST_ID} ORDER BY d.ID LIMIT 1")[0][0]
        _, textures = gd.item_display_models(db, did)
    finally:
        db.close()
    bpy.ops.wow.import_item_display("EXEC_DEFAULT", display_id=did)
    fds = {p.texture_fdids[k] for m in bpy.data.materials for p in m.wow_m2_batches for k in range(2)}
    assert textures[0] in fds, (textures, fds)


@test
def creature_derivation_check():
    """GeoBox and collision size derived from the imported M2 against stock CreatureModelData rows."""
    gd = common.submodule("game_data")
    db = gd.Layout.discover(client.dirs()[0]).open()
    try:
        rows = db.dicts(f"SELECT * FROM {gd.CMD} WHERE FileDataID > 0 AND ID < 60000 AND GeoBox4 > GeoBox1 ORDER BY ID")
    finally:
        db.close()
    index.load(client.session().content, client.prefs().cache_dir)
    rows = [r for r in rows if index.path_of(r["FileDataID"]).endswith(".m2")][::max(1, len(rows) // 50)][:50]
    errs, n = {"geo": [], "width": [], "height": []}, 0
    for r in rows:
        clear()
        top = m2_import.load(index.path_of(r["FileDataID"]), {"display": "NONE"})
        p = top.wow_m2
        info = gd.ModelInfo(p.fdid, p.path, tuple(p.bounding_min), tuple(p.bounding_max), tuple(p.collision_min), tuple(p.collision_max))
        scale = max(abs(r[f"GeoBox{i}"]) for i in range(1, 7)) or 1.0
        errs["geo"].append(max(abs(a - r[f"GeoBox{i + 1}"]) for i, a in enumerate(gd.geo_box(info))) / scale)
        w, h = gd.collision_size(info)
        if r["CollisionWidth"] > 0:
            errs["width"].append(abs(w - r["CollisionWidth"]) / r["CollisionWidth"])
        if r["CollisionHeight"] > 0:
            errs["height"].append(abs(h - r["CollisionHeight"]) / r["CollisionHeight"])
        n += 1
    med = {k: sorted(v)[len(v) // 2] for k, v in errs.items()}
    print(f"derivation: {n} models, median relative error {med}")
    assert n >= 20 and med["width"] < 0.01 and med["height"] < 0.01, med


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
if "--all-m2-geo" in ARGS:
    failed += len(sweep_m2_geo())
if "--m2-clean" in ARGS:
    for k, v in sweep_m2_clean().items():
        print("  FAIL", k, v)
if "--corpus" in ARGS:
    failed += corpus()
print("RESULT", "FAIL" if failed else "OK")
sys.exit(1 if failed else 0)
