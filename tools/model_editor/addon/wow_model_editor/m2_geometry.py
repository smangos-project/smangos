"""Submesh mesh data -> M2 vertex pool, skin layout and bone combos. Pure numpy, no Blender or wowlib types."""
from dataclasses import dataclass, field

import numpy as np

from . import wmo_geometry as wg

MAX_VERTICES = 65535
MAX_COMBO_BONES = 256
POS_TOL, NORMAL_TOL, UV_TOL = 1e-5, 2e-2, 1e-5
MAX_BONES_PER_VERTEX = 4


@dataclass
class SubmeshIn:
    """One submesh object after gathering: positions, triangle corners and per-vertex skinning."""
    co: np.ndarray
    tri_vi: np.ndarray
    tri_normal: np.ndarray
    tri_uv: list  # two (ntri, 3, 2) arrays
    bones: np.ndarray
    weights: np.ndarray
    src_vertex: np.ndarray
    skin_section_id: int = 0
    level: int = 0
    center_bone: int = 0
    src: "SubmeshSrc | None" = None


@dataclass
class SubmeshSrc:
    """Source values of the submesh an object came from, used to keep unedited data bit-identical."""
    index: int
    combo: np.ndarray
    combo_index: int
    bone_influences: int
    center_position: np.ndarray
    sort_center_position: np.ndarray
    sort_radius: float
    positions: np.ndarray


@dataclass
class SubmeshOut:
    vertex_start: int
    vertex_count: int
    index_start: int
    index_count: int
    combo: np.ndarray
    combo_index: int
    bone_influences: int
    center_bone_index: int
    center_position: np.ndarray
    sort_center_position: np.ndarray
    sort_radius: float
    skin_section_id: int
    level: int
    src_index: int


@dataclass
class Pool:
    pos: np.ndarray
    normal: np.ndarray
    uv0: np.ndarray
    uv1: np.ndarray
    bones: np.ndarray
    weights: np.ndarray
    skin_bones: np.ndarray
    indices: np.ndarray
    submeshes: list
    bone_lookup: np.ndarray
    warnings: list = field(default_factory=list)


def quantize_weights(w):
    """Rows of float weights -> u8 rows that sum to 255 (largest remainder), zero rows stay zero."""
    w = np.asarray(w, np.float64)
    tot = w.sum(axis=1, keepdims=True)
    scaled = np.where(tot > 0, w / np.where(tot > 0, tot, 1) * 255.0, 0.0)
    base = np.floor(scaled)
    short = (np.where(tot[:, 0] > 0, 255, 0) - base.sum(axis=1)).astype(np.int64)
    frac = scaled - base
    order = np.argsort(-frac, axis=1, kind="stable")
    rank = np.empty_like(order)
    np.put_along_axis(rank, order, np.arange(w.shape[1])[None, :].repeat(len(w), 0), axis=1)
    base += (rank < short[:, None]) & (w > 0)
    return base.astype(np.uint8)


def pick_influences(groups, nv):
    """(bones, float weights), both (nv, 4), from per-vertex lists of (bone, weight); the 4 largest are kept."""
    bones = np.zeros((nv, MAX_BONES_PER_VERTEX), np.uint8)
    w = np.zeros((nv, MAX_BONES_PER_VERTEX), np.float64)
    for v, gs in enumerate(groups):
        if len(gs) > MAX_BONES_PER_VERTEX:
            gs = sorted(gs, key=lambda x: -x[1])[:MAX_BONES_PER_VERTEX]
        for k, (b, wt) in enumerate(sorted(gs, key=lambda x: -x[1])):
            bones[v, k], w[v, k] = b, wt
    return bones, w


def _same_influences(new_b, new_w, src_b, src_w):
    """Rows where both bone/weight sets agree (order ignored, weights within 1/255)."""
    ok = np.zeros(len(new_b), bool)
    for i in range(len(new_b)):
        a = {int(b): int(w) for b, w in zip(new_b[i], new_w[i]) if w}
        s = {int(b): int(w) for b, w in zip(src_b[i], src_w[i]) if w}
        ok[i] = a.keys() == s.keys() and all(abs(a[k] - s[k]) <= 1 for k in a)
    return ok


def split(sub, src_arrays):
    """Unique corner records of one submesh object: (pos, normal, uv0, uv1, bones, weights, tris, src_ids)."""
    nv = len(sub.co)
    cols = [np.arange(nv), sub.bones, sub.weights, sub.src_vertex]
    co, nrm, uv, (vid, bones, weights, src_id), tris = wg.split_vertices(
        sub.co, sub.tri_vi, sub.tri_normal, sub.tri_uv, cols)
    uv0, uv1 = uv
    if src_arrays is not None:
        has = src_id >= 0
        sid = np.where(has, src_id, 0)
        spos, snrm, suv0, suv1, sbones, sweights = (a[sid] for a in src_arrays)
        used = np.zeros(nv, bool)
        used[sub.tri_vi.reshape(-1)] = True
        orphan = ~used[vid]
        co = np.where(has[:, None], wg.merge_source(co, spos, POS_TOL), co)
        ln = np.linalg.norm(snrm, axis=1, keepdims=True)
        close = (np.abs(nrm - snrm / np.where(ln > 0, ln, 1)) <= NORMAL_TOL).all(axis=1, keepdims=True)
        nrm = np.where(has[:, None], np.where(orphan[:, None] | close | (np.linalg.norm(nrm, axis=1, keepdims=True) < 1e-3), snrm, nrm), nrm)
        uv0 = np.where(has[:, None], np.where(orphan[:, None], suv0, wg.merge_source(uv0, suv0, UV_TOL)), uv0)
        uv1 = np.where(has[:, None], np.where(orphan[:, None], suv1, wg.merge_source(uv1, suv1, UV_TOL)), uv1)
        same = has & _same_influences(bones, weights, sbones, sweights)
        bones = np.where(same[:, None], sbones, bones)
        weights = np.where(same[:, None], sweights, weights)
    return co, nrm, uv0, uv1, bones, weights, tris, src_id


def _combo_for(sub, bones, weights, warnings, name):
    used = np.unique(bones[weights > 0])
    src = sub.src
    if src is not None and len(src.combo) and np.isin(used, src.combo).all():
        return src.combo.astype(np.uint16), True
    if len(used) > MAX_COMBO_BONES:
        raise ValueError(f"{name}: uses {len(used)} bones, more than the {MAX_COMBO_BONES} a submesh can address")
    return used.astype(np.uint16), False


def _influence_count(weights, src):
    n = int((weights > 0).sum(axis=1).max()) if len(weights) else 0
    if n >= 2:
        return n
    return src.bone_influences if src is not None else 0


def _derived(pos, src):
    if src is not None and src.positions.shape == pos.shape and np.allclose(src.positions, pos, atol=POS_TOL, rtol=0):
        return src.center_position, src.sort_center_position, src.sort_radius
    if not len(pos):
        z = np.zeros(3, np.float32)
        return z, z, 0.0
    centre = pos.mean(axis=0).astype(np.float32)
    sort_centre = ((pos.min(axis=0) + pos.max(axis=0)) / 2).astype(np.float32)
    radius = float(np.linalg.norm(pos - sort_centre, axis=1).max())
    return centre, sort_centre, radius


def build(subs, src_arrays, src_lookup, names):
    """Assemble the vertex pool and skin layout from `subs` (SubmeshIn list, final submesh order).

    `src_arrays` is the source (pos, normal, uv0, uv1, bones, weights) of all root vertices; `src_lookup` the source
    bone_lookup_table. Raises ValueError on hard limits."""
    parts, warnings = [], []
    combos, kept = [], True
    for si, sub in enumerate(subs):
        rec = split(sub, src_arrays if sub.src is not None else None)
        combo, was_kept = _combo_for(sub, rec[4], rec[5], warnings, names[si])
        kept &= was_kept and sub.src is not None
        parts.append(rec)
        combos.append(combo)
    total = sum(len(p[0]) for p in parts)
    if total > MAX_VERTICES:
        raise ValueError(f"{total} vertices exceed the {MAX_VERTICES} a skin can hold; split the model")
    keep_table = kept and src_lookup is not None
    if keep_table:
        lookup, starts = np.asarray(src_lookup, np.uint16), [s.src.combo_index for s in subs]
    else:
        lookup = np.concatenate(combos).astype(np.uint16) if combos else np.zeros(0, np.uint16)
        starts = np.cumsum([0] + [len(c) for c in combos])[:-1].tolist()
    outs, vstart, istart = [], 0, 0
    all_idx, skin_bones = [], []
    for si, (sub, rec, combo) in enumerate(zip(subs, parts, combos)):
        pos, nrm, uv0, uv1, bones, weights, tris, _ = rec
        nvert, nidx = len(pos), tris.size
        pos_in_combo = np.zeros_like(bones)
        if len(combo):
            order = np.argsort(combo, kind="stable")
            hit = np.clip(np.searchsorted(combo[order], bones), 0, len(combo) - 1)
            pos_in_combo = np.where(combo[order][hit] == bones, order[hit], 0).astype(np.uint8)
        skin_bones.append(pos_in_combo.astype(np.uint8))
        all_idx.append((tris + vstart).reshape(-1).astype(np.uint16))
        centre, sort_centre, radius = _derived(pos, sub.src)
        cb = sub.center_bone
        if len(combo) and cb not in combo:
            cb = int(combo[0])
        outs.append(SubmeshOut(vstart, nvert, istart, nidx, combo, starts[si], _influence_count(weights, sub.src), cb,
                               centre, sort_centre, radius, sub.skin_section_id, sub.level,
                               sub.src.index if sub.src is not None else -1))
        vstart += nvert
        istart += nidx
    cat = lambda i, shape, dt: (np.concatenate([p[i] for p in parts]) if parts else np.zeros(shape, dt))  # noqa: E731
    pool = Pool(cat(0, (0, 3), np.float32), cat(1, (0, 3), np.float32), cat(2, (0, 2), np.float32),
                cat(3, (0, 2), np.float32), cat(4, (0, 4), np.uint8), cat(5, (0, 4), np.uint8),
                np.concatenate(skin_bones) if skin_bones else np.zeros((0, 4), np.uint8),
                np.concatenate(all_idx) if all_idx else np.zeros(0, np.uint16), outs, lookup, warnings)
    return pool
