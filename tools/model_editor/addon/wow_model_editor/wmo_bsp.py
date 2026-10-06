"""Axis-aligned BSP over a group's triangles (MOBN/MOBR) plus checkers used by the tests."""
import numpy as np

NODE_DTYPE = np.dtype([("flags", "<u2"), ("negChild", "<i2"), ("posChild", "<i2"), ("nFaces", "<u2"),
                       ("faceStart", "<u4"), ("planeDist", "<f4")])
LEAF = 4
LEAF_FACES = 128
MAX_DEPTH = 24
MAX_FACES = 65535
MAX_DUPLICATION = 1.5


def build(verts, tris, leaf_faces=LEAF_FACES, max_depth=MAX_DEPTH):
    """(nodes, face_indices) for (n,3) float vertices and (m,3) int triangles.

    Nodes split their box on the longest axis at the median triangle centroid; triangles reaching into both halves
    are listed in both children.
    """
    m = len(tris)
    if m > MAX_FACES:
        raise ValueError(f"{m} faces exceed the {MAX_FACES} a group BSP can address")
    if m == 0:
        return np.zeros(0, NODE_DTYPE), np.zeros(0, np.uint16)
    tv = np.asarray(verts, np.float64)[np.asarray(tris)]
    lo, hi = tv.min(axis=1), tv.max(axis=1)
    cen = tv.mean(axis=1)
    nodes = []
    faces = []
    nface = 0
    stack = [(np.arange(m), 0, -1, 0)]
    while stack:
        ids, depth, parent, side = stack.pop()
        idx = len(nodes)
        if parent >= 0:
            nodes[parent][1 if side == 0 else 2] = idx
        flo, fhi = lo[ids].min(axis=0), hi[ids].max(axis=0)
        split = None
        if len(ids) > leaf_faces and depth < max_depth:
            ext = fhi - flo
            for axis in np.argsort(-ext):
                if ext[axis] <= 1e-6:
                    break
                c = cen[ids, axis]
                d = float(np.median(c))
                neg = ids[lo[ids, axis] <= d]
                pos = ids[hi[ids, axis] >= d]
                if min(len(neg), len(pos)) == 0 or max(len(neg), len(pos)) >= len(ids):
                    d = float((flo[axis] + fhi[axis]) / 2)
                    neg = ids[lo[ids, axis] <= d]
                    pos = ids[hi[ids, axis] >= d]
                if (0 < min(len(neg), len(pos)) and max(len(neg), len(pos)) < len(ids)
                        and len(neg) + len(pos) <= MAX_DUPLICATION * len(ids)):
                    split = (int(axis), d, neg, pos)
                    break
        if split is None:
            nodes.append([LEAF, -1, -1, len(ids), nface, 0.0])
            faces.append(ids)
            nface += len(ids)
        else:
            axis, d, neg, pos = split
            nodes.append([axis, -1, -1, 0, 0, d])
            stack.append((pos, depth + 1, idx, 1))
            stack.append((neg, depth + 1, idx, 0))
    if len(nodes) > 32767:
        raise ValueError("BSP has too many nodes")
    arr = np.zeros(len(nodes), NODE_DTYPE)
    for i, n in enumerate(nodes):
        arr[i] = (n[0], n[1], n[2], n[3], n[4], n[5])
    return arr, np.concatenate(faces).astype(np.uint16)


def candidates(nodes, face_idx, a, b):
    """Faces in every leaf the segment a-b reaches."""
    out = set()
    stack = [(0, np.asarray(a, np.float64), np.asarray(b, np.float64))]
    while stack:
        i, p, q = stack.pop()
        if i < 0 or i >= len(nodes):
            continue
        n = nodes[i]
        if n["flags"] == LEAF:
            out.update(face_idx[n["faceStart"]:n["faceStart"] + n["nFaces"]].tolist())
            continue
        ax, d = int(n["flags"]), float(n["planeDist"])
        pa, qa = p[ax], q[ax]
        if pa <= d and qa <= d:
            stack.append((int(n["negChild"]), p, q))
        elif pa >= d and qa >= d:
            stack.append((int(n["posChild"]), p, q))
        else:
            t = (d - pa) / (qa - pa)
            m = p + (q - p) * t
            first, second = (("negChild", "posChild") if pa < d else ("posChild", "negChild"))
            stack.append((int(n[first]), p, m))
            stack.append((int(n[second]), m, q))
    return out


def segment_hits(verts, tris, a, b, eps=1e-9):
    """Indices of triangles crossed by segment a-b (Moeller-Trumbore, brute force)."""
    tv = np.asarray(verts, np.float64)[np.asarray(tris)]
    a, b = np.asarray(a, np.float64), np.asarray(b, np.float64)
    d = b - a
    e1, e2 = tv[:, 1] - tv[:, 0], tv[:, 2] - tv[:, 0]
    h = np.cross(d, e2)
    det = np.einsum("ij,ij->i", e1, h)
    ok = np.abs(det) > eps
    inv = np.where(ok, 1.0 / np.where(ok, det, 1.0), 0.0)
    s = a - tv[:, 0]
    u = inv * np.einsum("ij,ij->i", s, h)
    qv = np.cross(s, e1)
    v = inv * (qv @ d)
    t = inv * np.einsum("ij,ij->i", e2, qv)
    return np.nonzero(ok & (u >= 0) & (v >= 0) & (u + v <= 1) & (t >= 0) & (t <= 1))[0]


def check(nodes, face_idx, verts, tris, segments):
    """Number of segments whose brute-force hits are not all among the BSP candidates."""
    bad = 0
    for a, b in segments:
        hits = set(segment_hits(verts, tris, a, b).tolist())
        if hits and not hits <= candidates(nodes, face_idx, a, b):
            bad += 1
    return bad


def covers_all(nodes, face_idx, n_faces):
    """Every face index below n_faces appears in some leaf."""
    return len(np.unique(face_idx)) == n_faces and (len(face_idx) == 0 or int(face_idx.max()) < n_faces)
