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

from . import client, fdids, index, textures, wmo_bsp, wmo_geometry as geo, wmo_import, wmo_root_export
from .props import u32
from .report import Report as _Report

BBOX_PAD = 0.028
F_BSP, F_COLOR, F_EXTERIOR, F_INTERIOR = 0x1, 0x4, 0x8, 0x2000
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
    for ob in [*group_objects(root).values(), *new_group_objects(root)]:
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


def apply_group(w, gi, arr, src, report):
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
    nodes, faces = wmo_bsp.build(arr.vertices, tris)
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


def update_group(w, gi, ob, report, force):
    body = w.groups[gi].body
    src = source_arrays(body)
    ids = slot_material_ids(ob, report)
    cls, mats = source_batch_info(body, body.header)
    c = mesh_corners(ob, ids, default_group_class(ob.wow_wmo_group.flags), src)
    c["src_class"], c["src_mat"] = cls, mats
    arr = geo.build_group(c)
    arr, changed = compare(arr, src)
    if not changed and not force:
        return False
    report.add("INFO", ob.name, "rebuilt: " + ", ".join(changed or ["forced"]))
    apply_group(w, gi, arr, src, report)
    report.stats["groups_rebuilt"] += 1
    return True


def new_group_objects(root):
    """Mesh objects in the groups collection without a group index; they are appended as new groups."""
    objs = [o for c in root.children_recursive if c.name.split(".")[0] == "groups" for o in c.objects
            if o.type == "MESH" and o.wow_wmo_group.index < 0 and not o.name.startswith(COLLISION_NAME)]
    return sorted(set(objs), key=lambda o: o.name)


def add_group(w, ob, path, report):
    """Append an empty group for `ob` (to be filled by update_group); returns its index."""
    gi = len(w.groups)
    w.groups.new()
    info = w.root.group_infos.new()
    p = ob.wow_wmo_group
    flags = u32(p.flags) or F_EXTERIOR
    h = w.groups[gi].body.header
    h.flags = type(h.flags)(flags)
    h.group_name = w.root.group_names.add(p.group_name)
    h.descriptive_group_name = w.root.group_names.add(p.descriptive_name)
    info.name_offset = h.group_name
    info.flags = type(info.flags)(flags)
    h.unique_id = 0
    h.group_liquid = p.group_liquid
    h.batch_type_d = 0
    w.root.group_fdids.append(fdids.allocate(group_path(path, gi)))
    report.new_groups[ob.name] = gi
    return gi


def update_root_boxes(w):
    boxes = np.array([[g.body.header.bounding_box.min.x, g.body.header.bounding_box.min.y, g.body.header.bounding_box.min.z,
                       g.body.header.bounding_box.max.x, g.body.header.bounding_box.max.y, g.body.header.bounding_box.max.z]
                      for g in w.groups])
    if len(boxes):
        _set_box(w.root.header.bounding_box, boxes[:, :3].min(axis=0), boxes[:, 3:].max(axis=0))


def check_output(w, report):
    """Cross-checks the structural validator does not cover: material ranges and texture ids."""
    n = len(w.root.materials)
    for gi, g in enumerate(w.groups):
        for b in g.body.batches:
            m = b.material_id_large if b.flags & wmo_import.BATCH_LARGE_MATERIAL else b.material_id
            if m >= n and m != 0xFF:
                report.add("ERROR", f"group {gi}", f"batch material {m} out of range ({n} materials)")
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


def warn_stale_lods(path, dirty, report):
    """LOD group files are not exported; they stay as in Content and go stale when their group changes."""
    content = Path(client.session().content)
    for gi in sorted(dirty):
        base = Path(group_path(path, gi))
        for lod in (content / base.parent).glob(base.stem + "_lod*.wmo"):
            report.add("WARNING", f"group {gi}", f"{lod.name} is not updated and no longer matches the edited group")


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


def export(root, out_path=None, options=None):
    """Export WMO collection `root`; returns a Report. `options`: force, dry_run."""
    opts = {"force": False, "dry_run": False}
    opts.update(options or {})
    report = Report()
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
    for gi in range(len(w.groups)):
        ob = objs.get(gi)
        if ob is None:
            report.add("WARNING", f"group {gi}", "no object, group kept as in the source")
            continue
        try:
            if update_group(w, gi, ob, report, opts["force"]):
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
    for gi in liquid_groups:
        refresh_group_box(w, gi)
        dirty.add(gi)
    if dirty:
        update_root_boxes(w)
        if not p.is_new:
            warn_stale_lods(path, dirty, report)
    if report.errors:
        return report
    vr = w.validate()
    if not vr.ok:
        for issue in vr.issues:
            report.add("WARNING", "validate", str(issue))
    root_bytes, group_bytes = serialize(w)
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
