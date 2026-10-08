"""Writes an imported WMO collection back as a 1.15 root + group files. Untouched sections keep the source data."""
import io
import os
import posixpath
import re
import struct
from pathlib import Path

import bpy
import numpy as np
import wowlib
from wowlib.formats import wmo as wwmo

from . import client, fdids, index, textures, wmo_bsp, wmo_geometry as geo, wmo_import, wmo_lod, wmo_root_export
from .props import u32
from .report import Report as _Report

BBOX_PAD = 0.028
F_BSP, F_COLOR, F_EXTERIOR, F_INTERIOR = 0x1, 0x4, 0x8, 0x2000
F_COLOR2, F_UV2, F_UV3 = 0x1000000, 0x2000000, 0x40000000
F_ANTIPORTAL, F_UNREACHABLE = 0x4000000, 0x80
TWO_LAYER_SHADERS = {6, 7, 8, 13, 14, 15, 16}
COLLISION_NAME = "wow_collision"


class Report(_Report):
    def __init__(self):
        super().__init__(groups_rebuilt=0, files_written=0, files_removed=0)


def _root_collection(obj):
    """The WMO root collection for a collection, an object inside one, or the active collection chain."""
    if isinstance(obj, bpy.types.Collection):
        if obj.wow_wmo.path:
            return obj
        for c in bpy.data.collections:
            if c.wow_wmo.path and obj.name in c.children_recursive:
                return c
        return None
    for c in bpy.data.collections:
        if c.wow_wmo.path and obj.name in c.all_objects:
            return c
    return None


def find_root(context_or_obj):
    return _root_collection(context_or_obj)


def group_objects(root):
    return {o.wow_wmo_group.index: o for o in root.all_objects if o.type == "MESH" and o.wow_wmo_group.index >= 0}


def _flat(vec, dtype=np.float32, cols=None):
    a = np.asarray(vec)
    return a.view(dtype).reshape(-1, cols) if cols else a


def source_arrays(body):
    """The group's current arrays as plain numpy, for dirty checks."""
    verts = _flat(body.vertices, np.float32, 3).copy()
    normals = _flat(body.normals, np.float32, 3).copy()
    uvs = []
    for tc in body.texcoords:
        a = np.asarray(tc)
        uvs.append(np.stack([a["x"], a["y"]], axis=1).astype(np.float32))
    cols = []
    for vc in body.vertex_colors:
        a = np.asarray(vc)
        cols.append(np.stack([a["r"], a["g"], a["b"], a["a"]], axis=1).astype(np.uint8))
    idx = np.asarray(body.large_indices if len(body.large_indices) else body.indices).astype(np.int64)
    polys = np.asarray(body.polys if len(body.polys) else body.polys2)
    batches = np.array([(b.start_index, b.count, b.min_index, b.max_index,
                         b.material_id_large if b.flags & wmo_import.BATCH_LARGE_MATERIAL else b.material_id)
                        for b in body.batches], geo.BATCH_DTYPE)
    return {"verts": verts, "normals": normals, "uvs": uvs, "cols": cols, "indices": idx,
            "flags": polys["flags"].astype(np.int32),
            "pmat": np.where(polys["materialId"] == 0xFFFF, 0xFF, polys["materialId"]).astype(np.int32),
            "batches": batches, "mpy2": not len(body.polys) and len(body.polys2) > 0}


def mesh_corners(ob, slot_ids, default_class, src):
    """Corner data of `ob` in the dict form `wmo_geometry.build_group` reads."""
    me = ob.data
    me.calc_loop_triangles()
    nt = len(me.loop_triangles)
    loops = np.empty(nt * 3, np.int32)
    me.loop_triangles.foreach_get("loops", loops)
    poly = np.empty(nt, np.int32)
    me.loop_triangles.foreach_get("polygon_index", poly)
    nloop = len(me.loops)
    lv = np.empty(nloop, np.int32)
    me.loops.foreach_get("vertex_index", lv)
    nv = len(me.vertices)
    co = np.empty(nv * 3, np.float32)
    me.vertices.foreach_get("co", co)
    co = co.reshape(-1, 3)
    cn = np.empty(nloop * 3, np.float32)
    me.corner_normals.foreach_get("vector", cn)
    cn = cn.reshape(-1, 3)
    if "wow_normal" in me.attributes:
        wn = np.empty(nv * 3, np.float32)
        me.attributes["wow_normal"].data.foreach_get("vector", wn)
        wn = wn.reshape(-1, 3)
        has = np.linalg.norm(wn, axis=1) > 0.5
        cn = np.where(has[lv][:, None], wn[lv], cn)
    co, _ = geo.transform(co, np.zeros((nv, 3), np.float32), ob.matrix_world)
    m = np.array(ob.matrix_world)
    if not np.array_equal(m, np.eye(4)):
        _, cn = geo.transform(np.zeros((nloop, 3), np.float32), cn, ob.matrix_world)
    tri_uv = []
    for layer in me.uv_layers:
        a = np.empty(nloop * 2, np.float32)
        layer.data.foreach_get("uv", a)
        a = a.reshape(-1, 2).copy()
        a[:, 1] = 1.0 - a[:, 1]
        tri_uv.append(a[loops].reshape(nt, 3, 2))
    if not tri_uv:
        tri_uv.append(np.zeros((nt, 3, 2), np.float32))
    vcol = []
    for attr in me.color_attributes:
        if not attr.name.startswith("col"):
            continue
        a = np.empty(len(attr.data) * 4, np.float32)
        attr.data.foreach_get("color_srgb", a)
        a = np.clip(np.rint(a.reshape(-1, 4) * 255), 0, 255).astype(np.uint8)
        if attr.domain == "CORNER":
            per = np.zeros((nv, 4), np.uint8)
            per[lv[::-1]] = a[::-1]
            a = per
        vcol.append(a)
    if not src["cols"] and len(vcol) == 1 and (vcol[0] == 255).all():
        vcol = []
    flags = np.zeros(len(me.polygons), np.int32)
    me.attributes["wow_poly_flags"].data.foreach_get("value", flags) if "wow_poly_flags" in me.attributes else None
    batch = np.full(len(me.polygons), -1, np.int32)
    if "wow_batch" in me.attributes:
        me.attributes["wow_batch"].data.foreach_get("value", batch)
    slot = np.empty(len(me.polygons), np.int32)
    me.polygons.foreach_get("material_index", slot)
    ids = np.array(slot_ids, np.int32)
    fm = ids[np.clip(slot, 0, max(len(ids) - 1, 0))] if len(ids) else np.full(len(slot), -1, np.int32)
    return {"co": co, "tri_vi": lv[loops].reshape(nt, 3), "tri_normal": cn[loops].reshape(nt, 3, 3),
            "tri_uv": tri_uv, "vcol": vcol, "face_mat": fm[poly], "face_flags": flags[poly],
            "face_batch": batch[poly], "default_class": default_class}


def slot_material_ids(ob, report):
    out = []
    for s in ob.material_slots:
        mat = s.material
        if mat is None or mat.name.startswith(COLLISION_NAME):
            out.append(-1)
        elif mat.name in report.material_index:
            out.append(report.material_index[mat.name])
        else:
            report.add("ERROR", ob.name, f"material {mat.name} has no WMO material index")
            out.append(-1)
    return out


TEXTURE_FIELDS = ("texture1", "texture2", "texture3")
TEXTURE_DIR = "world/wmo/custom/"


def _node_images(mat):
    """{field: image} from the labelled texture nodes of a material preview."""
    if not mat.node_tree:
        return {}
    return {n.label: n.image for n in mat.node_tree.nodes
            if n.bl_idname == "ShaderNodeTexImage" and n.label in TEXTURE_FIELDS and n.image}


def resolve_textures(mat, files, report):
    """Texture fdids of `mat` (None keeps the stored one); new and edited images are encoded into `files`."""
    out = {}
    for field, img in _node_images(mat).items():
        if img.name == textures.PLACEHOLDER:
            continue
        fdid = img.get("wow_fdid")
        if fdid:
            out[field] = int(fdid)
            if not img.is_dirty:
                continue
            path = index.path_of(int(fdid)) or fdids.read().get(int(fdid))
        else:
            name = re.sub(r"[^a-z0-9_]", "_", os.path.splitext(img.name)[0].lower())
            path = f"{TEXTURE_DIR}{name}.blp"
            fdid = fdids.allocate(path)
            out[field] = fdid
            img["wow_fdid"], img["wow_path"] = fdid, path
        if path is None:
            report.add("WARNING", img.name, "image has no known path, not exported")
            continue
        files[path] = textures.encode_blp(img)
    return out


def material_fields(mat, tex):
    p = mat.wow_wmo_material
    d = {"flags": u32(p.flags), "shader": p.shader, "blend_mode": p.blend_mode, "ground_type": p.ground_type,
         "color2": u32(p.color2), "flags2": u32(p.flags2)}
    for f in TEXTURE_FIELDS:
        d[f] = u32(tex.get(f, getattr(p, f)))
    return d


def _color(c):
    return tuple(int(v) for v in c)


def apply_material(m, mat, tex):
    """Write `mat` into SMOMaterial `m`; returns True if anything changed."""
    p = mat.wow_wmo_material
    changed = False
    for k, v in material_fields(mat, tex).items():
        if int(getattr(m, k)) != v:
            setattr(m, k, type(getattr(m, k))(v) if not isinstance(getattr(m, k), int) else v)
            changed = True
    for attr in ("sidn_color", "frame_sidn_color", "diff_color"):
        c, want = getattr(m, attr), _color(getattr(p, attr))
        if (c.r, c.g, c.b, c.a) != want:
            c.r, c.g, c.b, c.a = want
            changed = True
    rtd = [u32(v) for v in p.run_time_data]
    if [int(v) for v in m.run_time_data] != rtd:
        for i, v in enumerate(rtd):
            m.run_time_data[i] = v
        changed = True
    return changed


def update_materials(w, root, files, report):
    """Sync wow materials used by the group objects into MOMT; new materials are appended."""
    mats = {}
    for ob in [*group_objects(root).values(), *new_group_objects(root), *wmo_lod.lod_objects(root)]:
        for s in ob.material_slots:
            m = s.material
            if m is not None and not m.name.startswith(COLLISION_NAME):
                mats[m.name] = m
    n = len(w.root.materials)
    changed = False
    for m in sorted(mats.values(), key=lambda m: (m.wow_wmo_material.material_index < 0, m.wow_wmo_material.material_index, m.name)):
        i = m.wow_wmo_material.material_index
        tex = resolve_textures(m, files, report)
        if i < 0:
            w.root.materials.new()
            i, n = n, n + 1
            changed = True
        elif i >= n:
            report.add("ERROR", m.name, f"material index {i} out of range")
            continue
        report.material_index[m.name] = i
        changed |= apply_material(w.root.materials[i], m, tex)
    return changed


def source_batch_info(body, header):
    """(class, material) per source batch; the file order is transparent, interior, exterior."""
    t, i = header.trans_batch_count, header.int_batch_count
    classes, mats = [], []
    for bi, b in enumerate(body.batches):
        classes.append(geo.TRANS if bi < t else geo.INT if bi < t + i else geo.EXT)
        mats.append(b.material_id_large if b.flags & wmo_import.BATCH_LARGE_MATERIAL else b.material_id)
    return classes, mats


def _equal(new, src):
    return new.shape == src.shape and bool(np.array_equal(new, src, equal_nan=new.dtype.kind == "f"))


def _batches_match(new, src):
    """Batch lists equal up to source quirks: counts not a multiple of 3 and a stale max_index."""
    if new.shape != src.shape:
        return False
    return (all(np.array_equal(new[f], src[f]) for f in ("start", "min_index", "material"))
            and np.array_equal(new["count"], src["count"] // 3 * 3))


def _poly_materials_match(new, src):
    """Materials equal on every batched triangle; triangles outside all batches carry no information."""
    return new.shape == src.shape and bool(np.all((new == src) | (new == 0xFF)))


def _merged(new, src, tol):
    return geo.merge_source(new, src, tol) if new.shape == src.shape else new


def compare(arr, src):
    """Merge exact source values into the rebuilt arrays; returns (arrays, names of the arrays that differ)."""
    arr.vertices = _merged(arr.vertices, src["verts"], geo.VERT_TOL)
    arr.normals = _merged(arr.normals, src["normals"], geo.NORMAL_TOL)
    arr.uvs = [_merged(u, s, geo.UV_TOL) for u, s in zip(arr.uvs, src["uvs"])] + arr.uvs[len(src["uvs"]):]
    checks = {"vertices": _equal(arr.vertices, src["verts"]), "normals": _equal(arr.normals, src["normals"]),
              "uvs": len(arr.uvs) == len(src["uvs"]) and all(_equal(a, b) for a, b in zip(arr.uvs, src["uvs"])),
              "colors": len(arr.colors) == len(src["cols"]) and all(_equal(a, b) for a, b in zip(arr.colors, src["cols"])),
              "indices": _equal(arr.indices, src["indices"]), "poly flags": _equal(arr.poly_flags, src["flags"]),
              "poly materials": _equal(arr.poly_mat, src["pmat"]), "batches": _equal(arr.batches, src["batches"])}
    if not checks["batches"] and _batches_match(arr.batches, src["batches"]) and _poly_materials_match(arr.poly_mat, src["pmat"]):
        arr.batches, arr.poly_mat = src["batches"], src["pmat"]
        checks["batches"] = checks["poly materials"] = True
    clean = [k for k, ok in checks.items() if not ok]
    return arr, clean


def _fill(vec, data):
    vec.resize(len(data))
    if len(data):
        np.asarray(vec)[...] = data


def _vec_set(vec, arr, names):
    vec.resize(len(arr))
    a = np.asarray(vec)
    for k, n in enumerate(names):
        a[n] = arr[:, k]


def _box_of(verts):
    if not len(verts):
        return (0, 0, 0), (0, 0, 0)
    return verts.min(axis=0) - BBOX_PAD, verts.max(axis=0) + BBOX_PAD


def _set_box(box, lo, hi):
    box.min.x, box.min.y, box.min.z = (float(v) for v in lo)
    box.max.x, box.max.y, box.max.z = (float(v) for v in hi)


def apply_group(w, gi, arr, src, report, bsp=True):
    """Write rebuilt arrays into group `gi` and regenerate the derived data."""
    body = w.groups[gi].body
    h = body.header
    n = len(arr.vertices)
    body.vertices.resize(n)
    np.asarray(body.vertices).view(np.float32).reshape(-1, 3)[...] = arr.vertices
    body.normals.resize(n)
    np.asarray(body.normals).view(np.float32).reshape(-1, 3)[...] = arr.normals
    body.clear_texcoord_sets()
    for u in arr.uvs:
        v = wowlib.VectorC2Vector()
        v.resize(n)
        a = np.asarray(v)
        a["x"], a["y"] = u[:, 0], u[:, 1]
        body.append_texcoord_set(v)
    body.clear_vertex_color_layers()
    for c in arr.colors:
        v = wowlib.VectorCImVector()
        v.resize(n)
        a = np.asarray(v)
        a["r"], a["g"], a["b"], a["a"] = c[:, 0], c[:, 1], c[:, 2], c[:, 3]
        body.append_vertex_color_layer(v)
    body.indices.resize(len(arr.indices))
    if len(arr.indices):
        np.asarray(body.indices)[...] = arr.indices.astype(np.uint16)
    if len(body.large_indices):
        body.large_indices.resize(0)
    nf = len(arr.poly_flags)
    if src["mpy2"]:
        body.polys.resize(0)
        body.polys2.resize(nf)
        p = np.asarray(body.polys2)
        p["flags"] = arr.poly_flags
        p["materialId"] = np.where(arr.poly_mat == geo.COLLISION_MAT, 0xFFFF, arr.poly_mat)
    else:
        body.polys.resize(nf)
        p = np.asarray(body.polys)
        p["flags"] = arr.poly_flags
        p["materialId"] = arr.poly_mat
    body.batches.clear()
    for b in arr.batches:
        x = body.batches.new()
        x.start_index, x.count, x.min_index, x.max_index = int(b["start"]), int(b["count"]), int(b["min_index"]), int(b["max_index"])
        mat = int(b["material"])
        x.material_id = mat if mat < 256 else 0
        x.material_id_large = mat
        x.flags = wmo_import.BATCH_LARGE_MATERIAL if mat > 255 else 0
    cls = arr.batch_class
    h.trans_batch_count = int((cls == geo.TRANS).sum())
    h.int_batch_count = int((cls == geo.INT).sum())
    h.ext_batch_count = int((cls == geo.EXT).sum())
    tris = arr.indices.reshape(-1, 3)
    nodes, faces = wmo_bsp.build(arr.vertices, tris) if bsp else ([], [])
    body.bsp_nodes.resize(len(nodes))
    if len(nodes):
        np.asarray(body.bsp_nodes)[...] = nodes
    body.bsp_face_indices.resize(len(faces))
    if len(faces):
        np.asarray(body.bsp_face_indices)[...] = faces
    if len(body.shadow_batches):
        report.add("WARNING", f"group {gi}", "shadow batches dropped")
        body.shadow_batches.clear()
    if len(body.query_faces):
        body.query_faces.clear()
    flags = u32(int(h.flags)) & ~(F_BSP | F_COLOR)
    flags |= F_BSP if len(nodes) else 0
    flags |= F_COLOR if arr.colors else 0
    for bit, on in ((F_COLOR2, len(arr.colors) > 1), (F_UV2, len(arr.uvs) > 1), (F_UV3, len(arr.uvs) > 2)):
        flags = flags | bit if on else flags & ~bit
    h.flags = type(h.flags)(flags)
    set_group_box(w, gi, arr.vertices)


def set_group_box(w, gi, verts):
    """Group and group-info boxes from the vertices plus any liquid."""
    body = w.groups[gi].body
    lo, hi = _box_of(verts)
    L = body.liquid
    if not L.empty and len(L.vertices):
        lv = np.array([(L.base_coords.x + (i % L.verts_dim.x) * wmo_import.UNIT_SIZE,
                        L.base_coords.y + (i // L.verts_dim.x) * wmo_import.UNIT_SIZE, v.height)
                       for i, v in enumerate(L.vertices)], np.float32)
        lo, hi = np.minimum(lo, lv.min(axis=0) - BBOX_PAD), np.maximum(hi, lv.max(axis=0) + BBOX_PAD)
    _set_box(body.header.bounding_box, lo, hi)
    _set_box(w.root.group_infos[gi].bounding_box, lo, hi)


def refresh_group_box(w, gi):
    set_group_box(w, gi, np.asarray(w.groups[gi].body.vertices).view(np.float32).reshape(-1, 3))


def default_group_class(flags):
    return geo.INT if u32(flags) & F_INTERIOR else geo.EXT


def is_antiportal(ob):
    p = ob.wow_wmo_group
    return bool(u32(p.flags) & F_ANTIPORTAL) or p.group_name == "antiportal"


def group_flags(ob):
    """The group's flags; antiportals always carry the antiportal flag, which the client reads from MOGI."""
    f = u32(ob.wow_wmo_group.flags)
    return f | F_ANTIPORTAL | F_UNREACHABLE if is_antiportal(ob) else f


def check_antiportal(ob, report):
    """Antiportals are occluder geometry: the client draws no batches for them, so materials are ignored."""
    if len(ob.data.polygons) == 0:
        report.add("WARNING", ob.name, "antiportal has no faces and occludes nothing")
    if any(s.material is not None for s in ob.material_slots):
        report.add("WARNING", ob.name, "antiportal materials are ignored")
    if not u32(ob.wow_wmo_group.flags) & F_ANTIPORTAL:
        report.add("INFO", ob.name, "named antiportal without the antiportal flag; flag set on export")


def update_group(w, gi, ob, report, force, bsp=True):
    body = w.groups[gi].body
    src = source_arrays(body)
    if is_antiportal(ob):
        check_antiportal(ob, report)
        ids = [-1] * len(ob.material_slots)
    else:
        ids = slot_material_ids(ob, report)
    cls, mats = source_batch_info(body, body.header)
    file_cls = list(cls)
    default = default_group_class(group_flags(ob))
    if default != default_group_class(int(body.header.flags)):
        cls = [x if x == geo.TRANS else default for x in cls]
    c = mesh_corners(ob, ids, default, src)
    c["src_class"], c["src_mat"] = cls, mats
    arr = geo.build_group(c)
    if not bsp:
        arr.poly_flags, arr.poly_mat = arr.poly_flags[:0], arr.poly_mat[:0]
    arr, changed = compare(arr, src)
    if not changed and [int(x) for x in arr.batch_class] != file_cls:
        changed = ["batch classes"]
    if not changed and not force:
        return False
    report.add("INFO", ob.name, "rebuilt: " + ", ".join(changed or ["forced"]))
    apply_group(w, gi, arr, src, report, bsp)
    report.stats["groups_rebuilt"] += 1
    return True


def update_group_meta(w, gi, ob, report):
    """Applies name, flag, fog and liquid-type edits from the group's properties; returns True if anything changed."""
    p = ob.wow_wmo_group
    h = w.groups[gi].body.header
    info = w.root.group_infos[gi]
    changed = False
    for field, attr, cur in (("group_name", "group_name", h.group_name), ("descriptive_name", "descriptive_group_name", h.descriptive_group_name)):
        want = getattr(p, field)
        if want != wmo_import._string(w.root.group_names, cur):
            setattr(h, attr, w.root.group_names.add(want))
            report.names_changed = True
            if field == "group_name":
                info.name_offset = h.group_name
            changed = True
    derived = F_BSP | F_COLOR | F_COLOR2 | F_UV2 | F_UV3
    want = (group_flags(ob) & ~derived) | (u32(int(h.flags)) & derived)
    if want != u32(int(h.flags)):
        h.flags = type(h.flags)(want)
        info.flags = type(info.flags)(want | int(info.flags) & wmo_root_export.GROUP_LOD)
        report.add("INFO", ob.name, f"flags -> {want:#x}")
        changed = True
    ids = [int(x) for x in p.fog_ids]
    if ids != [int(x) for x in h.fog_ids]:
        for k, v in enumerate(ids):
            h.fog_ids[k] = v
        changed = True
    if int(h.group_liquid) != p.group_liquid:
        h.group_liquid = p.group_liquid
        changed = True
    return changed


def group_order(root, w, deleted, report):
    """Surviving old group indices sorted by their objects' `position` (unset: current index), or None if unchanged."""
    by_index = group_objects(root)
    by_index.update({gi: o for o in new_group_objects(root) if (gi := report.new_groups.get(o.name)) is not None})
    keys = {}
    for gi in range(len(w.groups)):
        pos = by_index[gi].wow_wmo_group.position if gi in by_index else -1
        keys[gi] = pos if pos >= 0 else gi
    order = sorted((gi for gi in keys if gi not in deleted), key=lambda g: (keys[g], g))
    return None if order == sorted(order) else order


def new_group_objects(root):
    """Mesh objects in the groups collection without a group index; they are appended as new groups."""
    objs = [o for c in root.children_recursive if c.name.split(".")[0] == "groups" for o in c.objects
            if o.type == "MESH" and o.wow_wmo_group.index < 0 and not o.name.startswith(COLLISION_NAME)]
    return sorted(set(objs), key=lambda o: o.name)


def add_group(w, ob, path, report):
    """Append an empty group for `ob` (to be filled by update_group); returns its index."""
    gi = len(w.groups)
    w.groups.new()
    wmo_root_export.append_group_info(w.root, fdids.allocate(group_path(path, gi)))
    info = w.root.group_infos.new()
    p = ob.wow_wmo_group
    flags = group_flags(ob) or F_EXTERIOR
    h = w.groups[gi].body.header
    h.flags = type(h.flags)(flags)
    report.names_changed = True
    h.group_name = w.root.group_names.add(p.group_name)
    h.descriptive_group_name = w.root.group_names.add(p.descriptive_name)
    info.name_offset = h.group_name
    info.flags = type(info.flags)(flags)
    h.unique_id = 0
    h.group_liquid = p.group_liquid
    h.batch_type_d = 0
    report.new_groups[ob.name] = gi
    return gi


def update_root_boxes(w):
    boxes = np.array([[g.body.header.bounding_box.min.x, g.body.header.bounding_box.min.y, g.body.header.bounding_box.min.z,
                       g.body.header.bounding_box.max.x, g.body.header.bounding_box.max.y, g.body.header.bounding_box.max.z]
                      for g in w.groups])
    if len(boxes):
        _set_box(w.root.header.bounding_box, boxes[:, :3].min(axis=0), boxes[:, 3:].max(axis=0))


def check_lod_tables(w, report):
    r = w.root
    n = len(r.group_infos)
    if len(r.group_infos2) and len(r.group_infos2) != n:
        report.add("ERROR", "lods", f"{len(r.group_infos2)} MGI2 entries for {n} groups")
    if int(r.header.flags) & wmo_root_export.HAS_LOD:
        levels = int(r.header.num_lod) or 3
        if len(r.group_fdids) != levels * n:
            report.add("ERROR", "lods", f"GFID has {len(r.group_fdids)} entries, expected {levels} tables of {n}")
    elif len(r.group_fdids) != n:
        report.add("WARNING", "lods", f"GFID has {len(r.group_fdids)} entries for {n} groups")


def check_group_flags(gi, body, report):
    h = body.header
    f = int(h.flags)
    where = f"group {gi}"
    if f & F_INTERIOR and f & F_EXTERIOR:
        report.add("WARNING", where, "flags mark the group as both interior and exterior")
    if f & 0x1000 and (body.liquid.empty or not len(body.liquid.vertices)):
        report.add("WARNING", where, "liquid flag set but the group has no liquid")
    for flag, attr, label in ((0x800, "doodad_refs", "doodads"), (0x200, "light_refs", "lights")):
        if bool(f & flag) != bool(len(getattr(body, attr))):
            report.add("WARNING", where, f"{label} flag {flag:#x} does not match its references")
    if f & F_EXTERIOR and not f & F_INTERIOR and int(h.int_batch_count):
        report.add("WARNING", where, "exterior group has interior batches")


def check_output(w, report):
    """Cross-checks the structural validator does not cover: material ranges and texture ids."""
    n = len(w.root.materials)
    for gi, g in enumerate(w.groups):
        check_group_flags(gi, g.body, report)
        for b in g.body.batches:
            m = b.material_id_large if b.flags & wmo_import.BATCH_LARGE_MATERIAL else b.material_id
            if m >= n and m != 0xFF:
                report.add("ERROR", f"group {gi}", f"batch material {m} out of range ({n} materials)")
            elif m < n and int(w.root.materials[m].shader) in TWO_LAYER_SHADERS and len(g.body.texcoords) < 2:
                report.add("WARNING", f"group {gi}", f"material {m} shader {int(w.root.materials[m].shader)} needs a second UV map")
    check_lod_tables(w, report)
    custom = fdids.read()
    for mi, m in enumerate(w.root.materials):
        for f in TEXTURE_FIELDS:
            v = int(getattr(m, f))
            if v and not index.path_of(v) and v not in custom:
                report.add("WARNING", f"material {mi}", f"{f} {v} is not in any listfile")
    for path, data in report.files.items():
        if data[:4] == b"BLP2":
            wd, ht = struct.unpack_from("<II", data, 12)
            if wd & (wd - 1) or ht & (ht - 1):
                report.add("WARNING", path, f"texture size {wd}x{ht} is not a power of two")


def patch_set_names(root_bytes, names):
    """Writes the doodad set names into the MODS chunk (the bindings expose them read-only)."""
    out = bytearray(root_bytes)
    pos = 0
    while pos + 8 <= len(out):
        tag, size = bytes(out[pos:pos + 4]), int.from_bytes(out[pos + 4:pos + 8], "little")
        if tag == b"SDOM":
            for i, name in enumerate(names[:size // 0x20]):
                at = pos + 8 + i * 0x20
                out[at:at + 20] = name.encode()[:19].ljust(20, b"\0")
            break
        pos += 8 + size
    return bytes(out)


def serialize(w):
    root = io.BytesIO()
    groups = [io.BytesIO() for _ in w.groups]
    w.write(root, groups)
    return root.getvalue(), [g.getvalue() for g in groups]


def group_path(root_path, i):
    return re.sub(r"\.wmo$", f"_{i:03d}.wmo", root_path)


def write_files(files, out_dir, content_dir, report):
    """Write files whose bytes differ from the Content copy; drop output copies that match it."""
    for path, data in files.items():
        dst = Path(out_dir) / path
        src = Path(content_dir) / path
        same = src.is_file() and src.read_bytes() == data
        if same:
            if dst.is_file():
                dst.unlink()
                report.stats["files_removed"] += 1
            continue
        dst.parent.mkdir(parents=True, exist_ok=True)
        dst.write_bytes(data)
        report.stats["files_written"] += 1


def new_wmo(path, parent=None):
    """An empty WMO collection that exports a new file at client path `path`."""
    path = path.replace("\\", "/").lower()
    top = bpy.data.collections.new(posixpath.basename(path))
    (parent or bpy.context.scene.collection).children.link(top)
    top.wow_wmo.path = path
    top.wow_wmo.is_new = True
    for name in ("groups", "portals", "lights", "doodads", "liquids", "fogs"):
        c = bpy.data.collections.new(name)
        top.children.link(c)
    return top


last_report = None


def export(root, out_path=None, options=None):
    """Export WMO collection `root`; returns a Report. `options`: force, dry_run."""
    global last_report
    opts = {"force": False, "dry_run": False}
    opts.update(options or {})
    report = Report()
    last_report = report
    bpy.context.view_layer.update()
    p = root.wow_wmo
    src_path = p.path
    path = (out_path or src_path).replace("\\", "/").lower()
    if p.is_new:
        w = wwmo.WMO.for_version(client.VERSION)
        fdids.allocate(path)
    else:
        w = wmo_import.read(src_path)
    objs = group_objects(root)
    dirty = set()
    files = {}
    if update_materials(w, root, files, report):
        report.add("INFO", "materials", "MOMT updated")
    deleted = set()
    for gi in range(len(w.groups)):
        ob = objs.get(gi)
        if ob is None:
            if p.all_groups and not p.is_new and len(objs) < len(w.groups):
                deleted.add(gi)
            else:
                report.add("WARNING", f"group {gi}", "no object, group kept as in the source")
            continue
        try:
            if update_group(w, gi, ob, report, opts["force"]):
                dirty.add(gi)
            if update_group_meta(w, gi, ob, report):
                dirty.add(gi)
        except ValueError as e:
            report.add("ERROR", ob.name, str(e))
    for ob in new_group_objects(root):
        try:
            gi = add_group(w, ob, path, report)
            update_group(w, gi, ob, report, True)
            dirty.add(gi)
        except ValueError as e:
            report.add("ERROR", ob.name, str(e))
    liquid_groups = wmo_root_export.update_root_objects(w, root, report)
    remap = None
    order = group_order(root, w, deleted, report)
    if deleted or order is not None:
        remap = wmo_root_export.delete_groups(w, deleted, report, order)
        dirty = {remap[g] for g in dirty if g in remap}
        liquid_groups = [remap[g] for g in liquid_groups if g in remap]
        report.new_groups = {k: remap[v] for k, v in report.new_groups.items()}
    for gi in liquid_groups:
        refresh_group_box(w, gi)
        dirty.add(gi)
    if dirty or deleted:
        update_root_boxes(w)
    wmo_lod.apply_lods(w, root, path, report, files, remap, dirty)
    if deleted or report.names_changed:
        wmo_root_export.rebuild_group_names(w)
    wmo_root_export.fix_header_counts(w)
    if report.errors:
        return report
    vr = w.validate()
    if not vr.ok:
        for issue in vr.issues:
            report.add("WARNING", "validate", str(issue))
    root_bytes, group_bytes = serialize(w)
    if getattr(report, "set_names", None) is not None:
        root_bytes = patch_set_names(root_bytes, report.set_names)
    files[path] = root_bytes
    for i, data in enumerate(group_bytes):
        files[group_path(path, i)] = data
    report.files = files
    check_output(w, report)
    if report.errors:
        return report
    if not opts["dry_run"]:
        s = client.session()
        out_dir = s.project
        if out_dir is None:
            report.add("ERROR", "export", "Set the Output directory in the add-on preferences")
            return report
        write_files(files, out_dir, s.content, report)
    return report
