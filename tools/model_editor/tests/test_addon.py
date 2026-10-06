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
    lf = "/tmp/me-custom-listfile.txt"
    if os.path.exists(lf):
        os.remove(lf)
    client.prefs().custom_listfile = lf
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
    assert open(lf).read().strip() == f"{fdid};world/wmo/custom/my_tex.blp"
    r2 = wmo_export.export(coll, options={"dry_run": True})
    assert open(lf).read().count("\n") == 1 and not r2.errors


@test
def wmo_export_validation():
    path = REFS[0]
    clear()
    client.prefs().custom_listfile = "/tmp/me-custom-listfile2.txt"
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
    client.prefs().custom_listfile = "/tmp/me-custom-listfile3.txt"
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
    client.prefs().custom_listfile = "/tmp/me-custom-listfile4.txt"
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
    lf = fdids.listfile_path()
    saved = lf.read_text() if lf.is_file() else None
    try:
        if saved is not None:
            lf.unlink()
        client.close()
        top = m2_import.load("creature/wolf/wolf.m2", {"display": "NONE", "collision": False})
        r = m2_export.export(top, "creature/zzme/wolfcopy.m2", {"force": True})
        assert not r.errors, str(r)
        got = fdids.read()
        assert sorted(got.values()) == ["creature/zzme/wolfcopy.m2", "creature/zzme/wolfcopy00.skin", "creature/zzme/wolfcopy01.skin"], got
        m = m2_import.read("creature/zzme/wolfcopy.m2")
        skin_ids = [int(x) for x in m.chunks.skin_fdids]
        assert skin_ids == [k for k, v in sorted(got.items()) if v.endswith(".skin")], (skin_ids, got)
        assert not [ln for ln in common.Path(client.session().content, "fdid.index").read_text().splitlines()
                    if ln.partition(";")[0].isdigit() and int(ln.partition(";")[0]) >= 1_000_000_000]
    finally:
        if saved is None:
            lf.unlink(missing_ok=True)
        else:
            lf.write_text(saved)
        client.close()


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


def sweep_wmo_export(force):
    index.load(client.session().content, client.prefs().cache_dir)
    fails, t0 = [], time.time()
    step = int(os.environ.get("SWEEP_STEP", "1"))
    wmos = index.entries("wmo")[::step]
    only = os.environ.get("SWEEP_ONLY")
    if only:
        wmos = [e for e in wmos if only in e[1]]
    slowest = []
    for fdid, path, _ in wmos:
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
if "--all-wmo-export" in ARGS:
    failed += len(sweep_wmo_export(False))
if "--all-wmo-rebuild" in ARGS:
    failed += len(sweep_wmo_export(True))
if "--all-m2" in ARGS:
    failed += len(sweep_m2())
if "--all-m2-anim" in ARGS:
    failed += len(sweep_m2_anim())
print("RESULT", "FAIL" if failed else "OK")
sys.exit(1 if failed else 0)
