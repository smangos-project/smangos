"""Group mesh data -> WMO group arrays (vertex split, batches, MOPY). Pure numpy, no Blender or wowlib types."""
from dataclasses import dataclass, field

import numpy as np

TRANS, INT, EXT = 0, 1, 2
COLLISION_MAT = 0xFF
MAX_VERTICES = 65535
DEFAULT_RENDER_FLAGS = 0x20
DEFAULT_COLLISION_FLAGS = 0x08
NORMAL_QUANT, UV_QUANT = 256.0, 10000.0
VERT_TOL, NORMAL_TOL, UV_TOL = 1e-5, 4e-3, 1e-5


@dataclass
class GroupArrays:
    vertices: np.ndarray
    normals: np.ndarray
    uvs: list
    colors: list
    indices: np.ndarray
    poly_flags: np.ndarray
    poly_mat: np.ndarray
    face_order: np.ndarray
    batches: np.ndarray
    batch_class: np.ndarray
    extra: dict = field(default_factory=dict)


BATCH_DTYPE = np.dtype([("start", "<i4"), ("count", "<i4"), ("min_index", "<i4"), ("max_index", "<i4"), ("material", "<i4")])


def _group_ids(keys):
    """(order, group id per sorted row, first row of each group) for the rows of an int64 key matrix."""
    order = np.lexsort(keys.T[::-1])
    sk = keys[order]
    new = np.ones(len(sk), bool)
    if len(sk) > 1:
        new[1:] = (sk[1:] != sk[:-1]).any(axis=1)
    return order, np.cumsum(new) - 1, np.nonzero(new)[0]


def split_vertices(co, tri_vi, tri_normal, tri_uv, vcol):
    """Unique (vertex, normal, uv...) corners, ordered by source vertex. Returns (arrays, tri indices)."""
    nv, nt = len(co), len(tri_vi)
    vi = tri_vi.reshape(-1)
    first_corner = np.full(nv, -1, np.int64)
    first_corner[vi[::-1]] = np.arange(len(vi))[::-1]
    ref = first_corner[vi]

    def snapped(values, tol, quant):
        """Rounded values, with corners that match their vertex's first corner collapsed to zero."""
        v = values.reshape(len(vi), -1)
        same = (np.abs(v - v[ref]) <= tol).all(axis=1)
        q = np.rint(v * quant)
        q[same] = 0
        return q

    cols = [vi[:, None], snapped(tri_normal, NORMAL_TOL, NORMAL_QUANT)]
    cols += [snapped(uv, UV_TOL * 4, UV_QUANT) for uv in tri_uv]
    key = np.concatenate(cols, axis=1).astype(np.int64)
    used = np.zeros(nv, bool)
    used[vi] = True
    orphans = np.nonzero(~used)[0]
    if len(orphans):
        pad = np.zeros((len(orphans), key.shape[1]), np.int64)
        pad[:, 0] = orphans
        key = np.concatenate([key, pad])
    order, gid, first = _group_ids(key)
    ncorner = nt * 3
    rep = order[first]
    inverse = np.empty(len(key), np.int64)
    inverse[order] = gid
    src_vi = key[rep, 0]
    out_co = co[src_vi]
    nrm = np.zeros((len(rep), 3), np.float32)
    uvs = [np.zeros((len(rep), 2), np.float32) for _ in tri_uv]
    real = rep < ncorner
    nrm[real] = tri_normal.reshape(-1, 3)[rep[real]]
    nrm[~real] = (0.0, 0.0, 1.0)
    for dst, uv in zip(uvs, tri_uv):
        dst[real] = uv.reshape(-1, 2)[rep[real]]
    cols_out = [c[src_vi] for c in vcol]
    return out_co, nrm, uvs, cols_out, inverse[:ncorner].reshape(nt, 3)


def default_flags(face_flags, face_mat, render):
    """Flags for faces without any: the commonest of their material in the group, else the plain default."""
    flags = face_flags.copy()
    blank = flags == 0
    if not blank.any():
        return flags
    common = {}
    for m in np.unique(face_mat[blank]):
        sel = (face_mat == m) & ~blank
        if m >= 0 and sel.any():
            vals, counts = np.unique(face_flags[sel], return_counts=True)
            common[int(m)] = int(vals[counts.argmax()])
    for m in np.unique(face_mat[blank]):
        sel = blank & (face_mat == m)
        flags[sel] = common.get(int(m), DEFAULT_RENDER_FLAGS if m >= 0 else DEFAULT_COLLISION_FLAGS)
    return flags


def assign_batches(face_mat, face_batch, src_class, src_mat, default_class):
    """Batch key and class per face; keys below len(src_class) are source batches, -1 means collision-only."""
    nt = len(face_mat)
    nsrc = len(src_class)
    src_class = np.asarray(src_class, np.int32)
    src_mat = np.asarray(src_mat, np.int32)
    safe = np.clip(face_batch, 0, max(nsrc - 1, 0))
    has = (face_batch >= 0) & (face_batch < nsrc)
    cls = np.where(has, src_class[safe] if nsrc else 0, default_class).astype(np.int32)
    key = np.where(has & (src_mat[safe] == face_mat) if nsrc else np.zeros(nt, bool), face_batch, -2).astype(np.int64)
    first = {}
    for bi in range(nsrc):
        first.setdefault((int(src_class[bi]), int(src_mat[bi])), bi)
    extra = {}
    for f in np.nonzero((key == -2) & (face_mat >= 0))[0]:
        k = (int(cls[f]), int(face_mat[f]))
        if k not in first:
            first[k] = nsrc + len(extra)
            extra[first[k]] = k
        key[f] = first[k]
    key[face_mat < 0] = -1
    return key, cls, extra


def build_group(c):
    """Rebuild one group's arrays from corner data `c` (a dict, see the fields read below)."""
    co = np.asarray(c["co"], np.float32)
    tri_vi = np.asarray(c["tri_vi"], np.int64)
    nt = len(tri_vi)
    face_mat = np.asarray(c["face_mat"], np.int32)
    flags = default_flags(np.asarray(c["face_flags"], np.int32), face_mat, True)
    key, cls, extra = assign_batches(face_mat, np.asarray(c["face_batch"], np.int32), c.get("src_class", ()),
                                     c.get("src_mat", ()), c.get("default_class", INT))
    nsrc = len(c.get("src_class", ()))
    bclass = {bi: int(c["src_class"][bi]) for bi in range(nsrc)}
    bclass.update({bi: k[0] for bi, k in extra.items()})
    bmat = {bi: int(c["src_mat"][bi]) for bi in range(nsrc)}
    bmat.update({bi: k[1] for bi, k in extra.items()})
    used = sorted({int(k) for k in key if k >= 0}, key=lambda bi: (bclass[bi], bi))
    rank = {bi: p for p, bi in enumerate(used)}
    pos = np.array([rank[int(k)] if k >= 0 else len(used) for k in key], np.int64)
    order = np.argsort(pos, kind="stable")
    tri_vi, nrm_t = tri_vi[order], np.asarray(c["tri_normal"])[order]
    uv_t = [np.asarray(u)[order] for u in c["tri_uv"]]
    vcol = [np.asarray(v, np.uint8) for v in c["vcol"]]
    out_co, nrm, uvs, cols, tris = split_vertices(co, tri_vi, nrm_t, uv_t, vcol)
    if len(out_co) > MAX_VERTICES:
        raise ValueError(f"{len(out_co)} vertices exceed the {MAX_VERTICES} a group can hold; split the group")
    spos = pos[order]
    batches = np.zeros(len(used), BATCH_DTYPE)
    bcls = np.zeros(len(used), np.int32)
    for p, bi in enumerate(used):
        sel = np.nonzero(spos == p)[0]
        s, n = int(sel[0]), len(sel)
        t = tris[s:s + n]
        batches[p] = (s * 3, n * 3, int(t.min()), int(t.max()), bmat[bi])
        bcls[p] = bclass[bi]
    fm = np.where(key[order] >= 0, face_mat[order], COLLISION_MAT)
    return GroupArrays(out_co, nrm, uvs, cols, tris.reshape(-1).astype(np.int64), flags[order].astype(np.int32),
                       fm.astype(np.int32), order, batches, bcls)


def transform(co, normals, matrix):
    """Apply a 4x4 matrix to positions and its inverse transpose to normals."""
    m = np.asarray(matrix, np.float64)
    if np.array_equal(m, np.eye(4)):
        return co, normals
    p = co.astype(np.float64) @ m[:3, :3].T + m[:3, 3]
    n = normals.astype(np.float64) @ np.linalg.inv(m[:3, :3])
    ln = np.linalg.norm(n, axis=1, keepdims=True)
    return p.astype(np.float32), (n / np.where(ln == 0, 1, ln)).astype(np.float32)


def merge_source(new, src, tol):
    """`new` with the exact source values wherever both agree within `tol` (same length required)."""
    if new.shape != src.shape:
        return new
    return np.where(np.abs(new - src) <= tol, src, new)
