"""Submesh objects -> M2 vertices, skin 0 and bone combos. Blender side of m2_geometry."""
import numpy as np

from . import m2_geometry as geo
from . import wmo_geometry as wg


def is_collision(ob):
    return ob.display_type == "WIRE" and ob.name.endswith("_collision")


def submesh_objects(top):
    """Submesh objects in final order: existing ones by source index, then new mesh objects by name."""
    old, new = [], []
    for ob in top.all_objects:
        if ob.type != "MESH" or is_collision(ob):
            continue
        (old if ob.wow_m2_submesh.index >= 0 else new).append(ob)
    old.sort(key=lambda o: o.wow_m2_submesh.index)
    new.sort(key=lambda o: o.name)
    return old, new


def bone_map(arm):
    """{bone name: m2 bone index} of the armature (empty without one)."""
    if arm is None:
        return {}
    return {b.name: b.wow_m2_bone.index for b in arm.data.bones if b.wow_m2_bone.index >= 0}


def find_armature(top):
    return next((o for o in top.all_objects if o.type == "ARMATURE"), None)


def source_arrays(root):
    """(pos, normal, uv0, uv1, bones, weights) of every root vertex."""
    vs = root.vertices
    n = len(vs)
    pos = np.empty((n, 3), np.float32)
    nrm = np.empty((n, 3), np.float32)
    uv0 = np.empty((n, 2), np.float32)
    uv1 = np.empty((n, 2), np.float32)
    bones = np.empty((n, 4), np.uint8)
    weights = np.empty((n, 4), np.uint8)
    for i, v in enumerate(vs):
        p, q, a, b = v.pos, v.normal, v.tex_coords[0], v.tex_coords[1]
        pos[i], nrm[i], uv0[i], uv1[i] = (p.x, p.y, p.z), (q.x, q.y, q.z), (a.x, a.y), (b.x, b.y)
        bones[i], weights[i] = v.bone_indices, v.bone_weights
    return pos, nrm, uv0, uv1, bones, weights


def index_start(sm):
    """First index of a skin section; `level` holds the bits above 16."""
    return int(sm.index_start) | int(sm.level) << 16


def source_submeshes(m, arrays):
    """{index: SubmeshSrc} for skin 0."""
    prof = m.skins[0].profile
    lookup = np.asarray(m.root.bone_lookup_table, np.uint16)
    skin_verts = np.asarray(prof.vertices, np.int64)
    out = {}
    for si, sm in enumerate(prof.submeshes):
        c0 = sm.bone_combo_index
        sv = skin_verts[sm.vertex_start:sm.vertex_start + sm.vertex_count]
        cp, sc = sm.center_position, sm.sort_center_position
        out[si] = geo.SubmeshSrc(si, lookup[c0:c0 + sm.bone_count], c0, int(sm.bone_influences),
                                 np.array((cp.x, cp.y, cp.z), np.float32), np.array((sc.x, sc.y, sc.z), np.float32),
                                 float(sm.sort_radius), arrays[0][sv])
    return out


def gather(ob, top, arm, bmap, report, rig=None):
    """SubmeshIn for a mesh object, or None when it has no vertices."""
    me = ob.data
    nv = len(me.vertices)
    if not nv:
        return None
    me.calc_loop_triangles()
    nt = len(me.loop_triangles)
    co = np.empty(nv * 3, np.float32)
    me.vertices.foreach_get("co", co)
    co = co.reshape(-1, 3)
    tri_vi = np.empty(nt * 3, np.int32)
    tri_loops = np.empty(nt * 3, np.int32)
    me.loop_triangles.foreach_get("vertices", tri_vi)
    me.loop_triangles.foreach_get("loops", tri_loops)
    nl = len(me.loops)
    cn = np.empty(nl * 3, np.float32)
    me.corner_normals.foreach_get("vector", cn)
    tri_normal = cn.reshape(-1, 3)[tri_loops].reshape(nt, 3, 3)
    tri_ref = None
    drift = me.attributes.get("wow_import_normal")
    if drift is not None and len(drift.data) == nl:
        ref = np.empty(nl * 3, np.float32)
        drift.data.foreach_get("vector", ref)
        tri_ref = ref.reshape(-1, 3)[tri_loops].reshape(nt, 3, 3)
    uvs = []
    for k in range(2):
        layer = me.uv_layers.get(f"uv_{k}") or (me.uv_layers[k] if k < len(me.uv_layers) else None)
        if layer is None:
            uvs.append(np.zeros((nt, 3, 2), np.float32))
            continue
        u = np.empty(nl * 2, np.float32)
        layer.data.foreach_get("uv", u)
        u = u.reshape(-1, 2)[tri_loops].reshape(nt, 3, 2)
        u[..., 1] = 1.0 - u[..., 1]
        uvs.append(u)
    mat = np.asarray(ob.matrix_world if arm is None else arm.matrix_world.inverted() @ ob.matrix_world)
    if not np.array_equal(mat, np.eye(4)):
        flat_n = tri_normal.reshape(-1, 3)
        co, flat_n = wg.transform(co, flat_n, mat)
        tri_normal = flat_n.reshape(nt, 3, 3)
        if tri_ref is not None:
            flat_r = tri_ref.reshape(-1, 3)
            tri_ref = wg.transform(flat_r, flat_r, mat)[1].reshape(nt, 3, 3)
    attr = me.attributes.get("wow_vertex")
    src_vertex = np.full(nv, -1, np.int32)
    if attr is not None:
        attr.data.foreach_get("value", src_vertex)
    p = ob.wow_m2_submesh
    center = rig.map(p.center_bone_index) if rig is not None else p.center_bone_index
    bones, weights, missing = _weights(ob, me, bmap, center if p.index >= 0 else 0)
    if missing and bmap:
        report.add("WARNING", "geometry", f"{ob.name}: {missing} vertices without weights use bone {center}")
    return geo.SubmeshIn(co, tri_vi.reshape(-1, 3), tri_normal, uvs, bones, weights, src_vertex,
                         p.skin_section_id, p.level, center, keep_source_skin=not bmap, tri_import_normal=tri_ref)


def _weights(ob, me, bmap, fallback):
    nv = len(me.vertices)
    gmap = [bmap.get(vg.name, -1) for vg in ob.vertex_groups]
    groups = [[(gmap[g.group], g.weight) for g in v.groups if g.weight > 0 and gmap[g.group] >= 0]
              for v in me.vertices]
    missing = sum(1 for g in groups if not g)
    for g in groups:
        if not g:
            g.append((fallback, 1.0))
    bones, w = geo.pick_influences(groups, nv)
    return bones, geo.quantize_weights(w), missing


def bounds_of(pos):
    lo, hi = pos.min(axis=0), pos.max(axis=0)
    centre = (lo + hi) / 2
    return lo, hi, float(np.linalg.norm(pos - centre, axis=1).max())


def importable_triangles(tris):
    """Triangles that survive the import's mesh validation: no repeated vertex, no repeated vertex set."""
    tris = tris[(tris[:, 0] != tris[:, 1]) & (tris[:, 1] != tris[:, 2]) & (tris[:, 0] != tris[:, 2])]
    _, first = np.unique(np.sort(tris, axis=1), axis=0, return_index=True)
    return tris[np.sort(first)]


def same_as_source(pool, m, arrays, srcs):
    """True when `pool` reproduces skin 0 and the root vertices of `m` exactly."""
    prof = m.skins[0].profile
    if len(pool.pos) != len(arrays[0]) or len(pool.submeshes) != len(prof.submeshes):
        return False
    for got, want in zip((pool.pos, pool.normal, pool.uv0, pool.uv1, pool.bones, pool.weights), arrays):
        if not np.array_equal(got, want):
            return False
    src_idx = np.asarray(prof.indices, np.int64)
    live = pool.weights > 0
    if not np.array_equal(pool.skin_bones[live], np.asarray(prof.bones).reshape(-1, 4)[live]):
        return False
    if not np.array_equal(pool.bone_lookup, np.asarray(m.root.bone_lookup_table)):
        return False
    for o, sm in zip(pool.submeshes, prof.submeshes):
        cp, sc = sm.center_position, sm.sort_center_position
        i0 = index_start(sm)
        tris = src_idx[i0:i0 + sm.index_count].reshape(-1, 3)
        tris = importable_triangles(tris)
        if not np.array_equal(pool.indices[o.index_start:o.index_start + o.index_count], tris.reshape(-1)):
            return False
        if (o.vertex_start, o.vertex_count, o.combo_index, len(o.combo), o.bone_influences,
                o.center_bone_index, o.skin_section_id) != (
                sm.vertex_start, sm.vertex_count, sm.bone_combo_index, sm.bone_count,
                sm.bone_influences, sm.center_bone_index, sm.skin_section_id):
            return False
        if (tuple(o.center_position), tuple(o.sort_center_position), o.sort_radius) != (
                (cp.x, cp.y, cp.z), (sc.x, sc.y, sc.z), sm.sort_radius):
            return False
    return True


def _remap(vec, field, kept):
    """Drops records whose submesh was deleted and renumbers the rest."""
    for i in range(len(vec) - 1, -1, -1):
        if getattr(vec[i], field) not in kept:
            vec.pop(i)
    for rec in vec:
        setattr(rec, field, kept[getattr(rec, field)])


def _set_vec(dst, vals):
    dst.x, dst.y, dst.z = (float(v) for v in vals)


def apply(m, pool, opts, report):
    """Write `pool` into model `m` (root vertices, skin 0, lookup, batches, bounds)."""
    root, prof = m.root, m.skins[0].profile
    n = len(pool.pos)
    root.vertices.resize(n)
    for i, v in enumerate(root.vertices):
        _set_vec(v.pos, pool.pos[i])
        _set_vec(v.normal, pool.normal[i])
        v.tex_coords[0].x, v.tex_coords[0].y = (float(x) for x in pool.uv0[i])
        v.tex_coords[1].x, v.tex_coords[1].y = (float(x) for x in pool.uv1[i])
        for k in range(4):
            v.bone_indices[k] = int(pool.bones[i, k])
            v.bone_weights[k] = int(pool.weights[i, k])
    prof.vertices.resize(n)
    prof.bones.resize(n)
    for i in range(n):
        prof.vertices[i] = i
        for k in range(4):
            prof.bones[i][k] = int(pool.skin_bones[i, k])
    prof.indices.resize(len(pool.indices))
    for i, x in enumerate(pool.indices):
        prof.indices[i] = int(x)
    root.bone_lookup_table.clear()
    root.bone_lookup_table.extend([int(x) for x in pool.bone_lookup])
    kept = {o.src_index: j for j, o in enumerate(pool.submeshes) if o.src_index >= 0}
    prof.submeshes.resize(len(pool.submeshes))
    for j, o in enumerate(pool.submeshes):
        sm = prof.submeshes[j]
        sm.skin_section_id, sm.level = o.skin_section_id, o.index_start >> 16
        sm.vertex_start, sm.vertex_count, sm.index_start, sm.index_count = o.vertex_start, o.vertex_count, o.index_start & 0xFFFF, o.index_count
        sm.bone_combo_index, sm.bone_count, sm.bone_influences = o.combo_index, len(o.combo), o.bone_influences
        sm.center_bone_index = o.center_bone_index
        _set_vec(sm.center_position, o.center_position)
        _set_vec(sm.sort_center_position, o.sort_center_position)
        sm.sort_radius = o.sort_radius
    _remap(prof.batches, "skin_section_index", kept)
    _remap(prof.shadow_batches, "submesh_id", kept)
    if n:
        lo, hi, radius = bounds_of(pool.pos)
        _set_vec(root.bounding_box.min, lo)
        _set_vec(root.bounding_box.max, hi)
        root.bounding_sphere_radius = radius


def build(top, m, report, opts, rig=None):
    """Rebuild the vertex pool and skin 0 from the submesh objects; leaves `m` alone when they match the source."""
    old, new = submesh_objects(top)
    objs = old + new
    arm = find_armature(top)
    bmap = dict(rig.by_name) if rig is not None else bone_map(arm)
    arrays = source_arrays(m.root)
    srcs = source_submeshes(m, arrays)
    subs, names = [], []
    for ob in objs:
        sub = gather(ob, top, arm, bmap, report, rig)
        if sub is None:
            continue
        sub.src = srcs.get(ob.wow_m2_submesh.index)
        subs.append(sub)
        names.append(ob.name)
    if not subs and srcs:
        report.add("ERROR", "geometry", "The model has no submesh objects")
        return
    try:
        pool = geo.build(subs, arrays, m.root.bone_lookup_table, names)
    except ValueError as e:
        report.add("ERROR", "geometry", str(e))
        return
    for w in pool.warnings:
        report.add("WARNING", "geometry", w)
    if same_as_source(pool, m, arrays, srcs):
        return
    report.add("INFO", "geometry", f"{len(pool.pos)} vertices, {len(pool.submeshes)} submeshes rebuilt")
    apply(m, pool, opts, report)
    if len(pool.pos):
        lo, hi, radius = bounds_of(pool.pos)
        top.wow_m2.bounding_min, top.wow_m2.bounding_max, top.wow_m2.bounding_radius = lo, hi, radius
