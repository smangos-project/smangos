"""Imports a 1.15 WMO as a collection tree: group meshes, materials, portals, lights, doodads, liquids and fogs."""
import math
import posixpath

import bpy
import numpy as np
from wowlib.formats import wmo as wwmo

from . import client, index, materials
from .props import s32

GROUP_EXTERIOR, GROUP_INTERIOR = 0x8, 0x2000
BATCH_LARGE_MATERIAL = 0x2
UNIT_SIZE = 533.33333 / 128
LIGHT_KINDS = {0: "POINT", 1: "SPOT", 2: "SUN", 3: "POINT"}


def _link(parent, name):
    c = bpy.data.collections.new(name)
    parent.children.link(c)
    return c


def _rgba(c):
    return (c.r, c.g, c.b, c.a)


def _string(block, offset):
    if offset is None or offset < 0:
        return ""
    try:
        return block.at(offset)
    except Exception:  # noqa: BLE001
        return ""


def _face_attr(me, name, values):
    a = me.attributes.new(name, "INT", "FACE")
    a.data.foreach_set("value", values.astype(np.int32))


def _int_view(vec, attr, dtype=np.int32):
    return np.fromiter((getattr(v, attr) for v in vec), dtype, len(vec))


def build_group_mesh(name, body, mats, collision_mat, wmo_name):
    """Mesh with the file's 1:1 vertex layout; returns (mesh, face count)."""
    co = np.asarray(body.vertices).view(np.float32).reshape(-1, 3)
    nv = len(co)
    idx = np.asarray(body.large_indices if len(body.large_indices) else body.indices, dtype=np.int32)
    tris = idx[: len(idx) // 3 * 3].reshape(-1, 3)
    nt = len(tris)
    me = bpy.data.meshes.new(name)
    me.vertices.add(nv)
    me.vertices.foreach_set("co", co.ravel())
    me.loops.add(nt * 3)
    me.loops.foreach_set("vertex_index", tris.ravel())
    me.polygons.add(nt)
    me.polygons.foreach_set("loop_start", np.arange(0, nt * 3, 3, dtype=np.int32))
    me.update()

    polys = np.asarray(body.polys if len(body.polys) else body.polys2)
    pf = np.zeros(nt, np.int32)
    pm = np.full(nt, 255, np.int32)
    n = min(nt, len(polys))
    pf[:n], pm[:n] = polys["flags"][:n], polys["materialId"][:n]
    face_mat = np.full(nt, -1, np.int32)
    face_batch = np.full(nt, -1, np.int32)
    for bi, b in enumerate(body.batches):
        mid = b.material_id_large if b.flags & BATCH_LARGE_MATERIAL else b.material_id
        s, c = b.start_index // 3, b.count // 3
        face_mat[s:s + c] = mid
        face_batch[s:s + c] = bi

    slots = {}
    slot_idx = np.zeros(nt, np.int32)
    for mid in np.unique(face_mat):
        slots[int(mid)] = len(slots)
        me.materials.append(collision_mat if mid < 0 or mid >= len(mats) else mats[mid])
        slot_idx[face_mat == mid] = slots[int(mid)]
    me.polygons.foreach_set("material_index", slot_idx)
    _face_attr(me, "wow_poly_flags", pf)
    _face_attr(me, "wow_poly_material", pm)
    _face_attr(me, "wow_batch", face_batch)

    for k, tc in enumerate(body.texcoords):
        uv = np.asarray(tc)
        uv = np.stack([uv["x"], 1.0 - uv["y"]], axis=1).astype(np.float32)
        layer = me.uv_layers.new(name=f"uv_{k}")
        layer.data.foreach_set("uv", uv[tris.ravel()].ravel())

    ncol = max(len(body.vertex_colors), 1)
    for k in range(ncol):
        a = me.color_attributes.new(f"col{k}", "BYTE_COLOR", "POINT")
        if k < len(body.vertex_colors) and len(body.vertex_colors[k]) == nv:
            vc = body.vertex_colors[k]
            col = np.array([(c.r, c.g, c.b, c.a) for c in vc], np.float32) / 255.0
        else:
            col = np.ones((nv, 4), np.float32)
        a.data.foreach_set("color_srgb", col.ravel())

    me.shade_smooth()
    nrm = np.asarray(body.normals).view(np.float32).reshape(-1, 3)
    if len(nrm) == nv:
        a = me.attributes.new("wow_normal", "FLOAT_VECTOR", "POINT")
        a.data.foreach_set("vector", nrm.ravel())
        me.normals_split_custom_set_from_vertices(nrm)
    return me, nt


def _group_name(root, header, info):
    names = root.group_names
    gname = _string(names, header.group_name)
    desc = _string(names, header.descriptive_group_name)
    return gname, desc


def import_groups(w, coll, mats, opts, wmo_name):
    root = w.root
    collision = materials.collision_material()
    objs = []
    for gi, g in enumerate(w.groups):
        info = root.group_infos[gi]
        flags = int(g.body.header.flags)
        if opts["group_filter"] == "EXTERIOR" and not flags & GROUP_EXTERIOR:
            continue
        if opts["group_filter"] == "INTERIOR" and not flags & GROUP_INTERIOR:
            continue
        h = g.body.header
        gname, desc = _group_name(root, h, info)
        name = f"{wmo_name}_{gi:03d}" + (f"_{gname}" if gname else "")
        me, _ = build_group_mesh(name, g.body, mats, collision, wmo_name)
        ob = bpy.data.objects.new(name, me)
        coll.objects.link(ob)
        p = ob.wow_wmo_group
        p.index = gi
        p.flags, p.flags2 = s32(flags), s32(int(h.flags2))
        p.group_liquid, p.unique_id = h.group_liquid, s32(h.unique_id)
        p.fog_ids = list(h.fog_ids)
        p.group_name, p.descriptive_name = gname, desc
        p.name_offset = info.name_offset
        p.group_name_offset, p.descriptive_name_offset = h.group_name, h.descriptive_group_name
        p.int_batch_count, p.trans_batch_count, p.ext_batch_count = h.int_batch_count, h.trans_batch_count, h.ext_batch_count
        p.batch_type_d = h.batch_type_d
        p.parent_split, p.next_split = h.parent_or_first_child_split_group_index, h.next_split_child_group_index
        p.fdid = root.group_fdids[gi] if gi < len(root.group_fdids) else 0
        objs.append(ob)
    return objs


def import_portals(root, coll):
    refs = {}
    for r in root.portal_refs:
        refs.setdefault(r.portal_index, []).append(r)
    verts = root.portal_vertices
    for i, pt in enumerate(root.portals):
        pts = [(v.x, v.y, v.z) for v in verts[pt.start_vertex:pt.start_vertex + pt.count]]
        me = bpy.data.meshes.new(f"portal_{i}")
        if len(pts) >= 3:
            me.from_pydata(pts, [], [list(range(len(pts)))])
        ob = bpy.data.objects.new(f"portal_{i}", me)
        ob.display_type = "WIRE"
        coll.objects.link(ob)
        p = ob.wow_wmo_portal
        p.index = i
        p.plane_normal = (pt.plane.normal.x, pt.plane.normal.y, pt.plane.normal.z)
        p.plane_distance = pt.plane.distance
        for r in refs.get(i, []):
            e = p.refs.add()
            e.group_index, e.portal_index, e.side = r.group_index, r.portal_index, r.side


def import_lights(root, coll):
    for i, l in enumerate(root.lights):
        kind = LIGHT_KINDS.get(l.type, "POINT")
        data = bpy.data.lights.new(f"light_{i}", kind)
        data.color = (l.color.r / 255, l.color.g / 255, l.color.b / 255)
        data.energy = 100.0 * l.intensity
        if kind != "SUN":
            data.use_custom_distance = True
            data.cutoff_distance = max(l.atten_end, 0.01)
        ob = bpy.data.objects.new(f"light_{i}", data)
        ob.location = (l.position.x, l.position.y, l.position.z)
        ob.rotation_mode = "QUATERNION"
        ob.rotation_quaternion = (l.rotation.w, l.rotation.x, l.rotation.y, l.rotation.z)
        coll.objects.link(ob)
        p = ob.wow_wmo_light
        p.index, p.type, p.use_atten = i, l.type, l.use_atten
        p.color, p.intensity = _rgba(l.color), l.intensity
        p.atten_start, p.atten_end = l.atten_start, l.atten_end
        p.rotation = (l.rotation.x, l.rotation.y, l.rotation.z, l.rotation.w)


def doodad_library(fdid, path):
    """Hidden collection with the mesh-only M2 for `fdid`, shared by every instance in the file."""
    lib = bpy.data.collections.get("wmo_doodad_lib")
    if lib is None:
        lib = bpy.data.collections.new("wmo_doodad_lib")
        bpy.context.scene.collection.children.link(lib)
        bpy.context.view_layer.layer_collection.children[lib.name].exclude = True
    for c in lib.children:
        if c.get("wow_fdid") == fdid:
            return c
    from . import m2_import
    c = m2_import.load(path, {"mesh_only": True, "display": "NONE", "armature": False}, parent=lib)
    c["wow_fdid"] = fdid
    return c


def import_doodads(root, parent, meshes=None):
    if meshes is None:
        meshes = len(root.doodad_defs) < 500
    sets = list(root.doodad_sets)
    set_colls = []
    for si, s in enumerate(sets):
        c = _link(parent, s.name or f"set_{si}")
        c["wow_set"] = si
        c.hide_viewport = c.hide_render = si != 0
        set_colls.append(c)
    unassigned = None
    owner = {}
    for si, s in enumerate(sets):
        for di in range(s.start_index, s.start_index + s.count):
            owner.setdefault(di, si)
    fdids = root.doodad_fdids
    for di, d in enumerate(root.doodad_defs):
        si = owner.get(di)
        if si is None:
            unassigned = unassigned or _link(parent, "unassigned")
            coll = unassigned
        else:
            coll = set_colls[si]
        fdid = fdids[d.name_index] if d.name_index < len(fdids) else 0
        path = index.path_of(fdid) or ""
        ob = bpy.data.objects.new(posixpath.splitext(posixpath.basename(path))[0] or f"doodad_{di}", None)
        ob.empty_display_type, ob.empty_display_size = "ARROWS", 0.5
        if meshes and path.endswith(".m2"):
            try:
                ob.instance_type, ob.instance_collection = "COLLECTION", doodad_library(fdid, path)
            except Exception:  # noqa: BLE001
                pass
        ob.location = (d.position.x, d.position.y, d.position.z)
        ob.rotation_mode = "QUATERNION"
        q = d.orientation
        ob.rotation_quaternion = (q.w, q.x, q.y, q.z)
        ob.scale = (d.scale,) * 3
        coll.objects.link(ob)
        p = ob.wow_wmo_doodad
        p.index, p.doodad_set = di, -1 if si is None else si
        p.fdid, p.path, p.name_index = s32(fdid), path, d.name_index
        p.flags = d.name_and_flags >> 24
        p.color, p.scale = _rgba(d.color), d.scale


def import_liquid(group_obj, g, gi, coll):
    L = g.body.liquid
    if L.empty:
        return None
    xv, yv = L.verts_dim.x, L.verts_dim.y
    xt, yt = L.tiles_dim.x, L.tiles_dim.y
    if xv * yv != len(L.vertices):
        return None
    bx, by = L.base_coords.x, L.base_coords.y
    heights = np.fromiter((v.height for v in L.vertices), np.float32, xv * yv)
    gx, gy = np.meshgrid(np.arange(xv), np.arange(yv))
    co = np.stack([bx + gx.ravel() * UNIT_SIZE, by + gy.ravel() * UNIT_SIZE, heights], axis=1).astype(np.float32)
    tx, ty = np.meshgrid(np.arange(xt), np.arange(yt))
    tx, ty = tx.ravel(), ty.ravel()
    v0 = ty * xv + tx
    quads = np.stack([v0, v0 + 1, v0 + xv + 1, v0 + xv], axis=1).astype(np.int32)
    nq = len(quads)
    tflags = _int_view(L.tiles, "flags")
    me = bpy.data.meshes.new(f"liquid_{gi:03d}")
    me.vertices.add(len(co))
    me.vertices.foreach_set("co", co.ravel())
    me.loops.add(nq * 4)
    me.loops.foreach_set("vertex_index", quads.ravel())
    me.polygons.add(nq)
    me.polygons.foreach_set("loop_start", np.arange(0, nq * 4, 4, dtype=np.int32))
    me.update()
    _face_attr(me, "wow_tile_flags", tflags[:nq])
    me.polygons.foreach_set("hide", (tflags[:nq] & 0x08) != 0)
    for attr in ("flow1", "flow1_pct", "flow2", "filler"):
        a = me.attributes.new(f"wow_{attr}", "INT", "POINT")
        a.data.foreach_set("value", _int_view(L.vertices, attr))
    ob = bpy.data.objects.new(me.name, me)
    ob.display_type = "WIRE"
    coll.objects.link(ob)
    p = ob.wow_wmo_liquid
    p.group_index, p.material_id = gi, L.material_id
    p.verts_dim, p.tiles_dim = (xv, yv), (xt, yt)
    p.base_coords = (L.base_coords.x, L.base_coords.y, L.base_coords.z)
    return ob


def import_fogs(root, coll):
    for i, f in enumerate(root.fogs):
        ob = bpy.data.objects.new(f"fog_{i}", None)
        ob.empty_display_type, ob.empty_display_size = "SPHERE", max(f.larger_radius, 0.01)
        ob.location = (f.position.x, f.position.y, f.position.z)
        coll.objects.link(ob)
        p = ob.wow_wmo_fog
        p.index, p.flags = i, f.flags
        p.larger_radius, p.smaller_radius = f.larger_radius, f.smaller_radius
        p.fog_end, p.fog_start_scalar, p.fog_color = f.fog.end, f.fog.start_scalar, _rgba(f.fog.color)
        p.uw_end, p.uw_start_scalar, p.uw_color = f.under_water_fog.end, f.under_water_fog.start_scalar, _rgba(f.under_water_fog.color)


def read(path):
    w = wwmo.WMO.for_version(client.VERSION)
    w.read(client.session().fs, client.key(path))
    return w


def load(client_path, options=None, parent=None):
    """Import `client_path` (e.g. world/wmo/cameron.wmo); returns the root collection."""
    opts = {"doodads": True, "lights": True, "portals": True, "liquids": True, "group_filter": "ALL"}
    opts.update(options or {})
    s = client.session()
    index.load(s.content, client.prefs().cache_dir)
    path = client_path.replace("\\", "/").lower()
    w = read(path)
    root = w.root
    base = posixpath.basename(path)
    stem = posixpath.splitext(base)[0]
    top = _link(parent or bpy.context.scene.collection, base)
    p = top.wow_wmo
    h = root.header
    p.path, p.fdid, p.mver = path, s32(index.fdid_of(path) or 0), root.mver
    p.flags, p.wmo_id, p.num_lod, p.ambient_color = s32(int(h.flags)), h.wmo_id, h.num_lod, _rgba(h.ambient_color)

    multi_uv = any(len(g.body.texcoords) > 1 for g in w.groups)
    mats = [materials.create_wmo_material(f"{stem}_{i}", m, i, multi_uv) for i, m in enumerate(root.materials)]
    group_objs = import_groups(w, _link(top, "groups"), mats, opts, stem)
    if opts["portals"]:
        import_portals(root, _link(top, "portals"))
    if opts["lights"]:
        import_lights(root, _link(top, "lights"))
    if opts["doodads"]:
        import_doodads(root, _link(top, "doodads"), opts.get("doodad_meshes"))
    if opts["liquids"]:
        lc = _link(top, "liquids")
        for ob in group_objs:
            gi = ob.wow_wmo_group.index
            import_liquid(ob, w.groups[gi], gi, lc)
    import_fogs(root, _link(top, "fogs"))
    return top
