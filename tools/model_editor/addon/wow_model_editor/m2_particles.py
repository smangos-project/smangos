"""Static parameters of M2 particle emitters: record <-> `wow_m2_particle` and the default emitter."""
import numpy as np
from wowlib import VectorC2Vector, VectorC3Vector, VectorShortUnsignedInt, Vectorfixed16
from wowlib.formats import common
from wowlib.formats.m2.root import record as rec

TOL = 1e-5
SCALARS = ("emitter_type", "blending_type", "rows", "columns", "priority_plane", "particle_color_index", "tail_length",
           "drag", "base_spin", "base_spin_variation", "spin_speed", "spin_speed_variation", "wind_time",
           "follow_speed1", "follow_speed2", "follow_scale1", "follow_scale2", "twinkle_speed", "twinkle_percent",
           "inherit_velocity_scale", "emission_rate_variation", "lifespan_variation")
VEC3 = ("wind_vector",)


def _v3(v):
    return (float(v.x), float(v.y), float(v.z))


def _set_v3(dst, vals):
    dst.x, dst.y, dst.z = (float(x) for x in vals)


def _ramp(block, kind):
    ks = list(block.keys)
    ts = [int(t) for t in block.timestamps]
    if kind == "c3":
        vals = [(k.x, k.y, k.z) for k in ks]
    elif kind == "c2":
        vals = [(k.x, k.y, 0.0) for k in ks]
    else:
        vals = [(k.value / 32767.0, 0.0, 0.0) for k in ks]
    return ts, [tuple(float(x) for x in v) for v in vals]


def _write_ramp(block, kind, ts, vals):
    if kind == "c3":
        keys = []
        for v in vals:
            c = common.C3Vector()
            _set_v3(c, v)
            keys.append(c)
        block.keys = VectorC3Vector(keys)
    elif kind == "c2":
        keys = []
        for v in vals:
            c = common.C2Vector()
            c.x, c.y = float(v[0]), float(v[1])
            keys.append(c)
        block.keys = VectorC2Vector(keys)
    else:
        keys = []
        for v in vals:
            c = common.fixed16()
            c.value = int(np.clip(np.rint(v[0] * 32767.0), -32768, 32767))
            keys.append(c)
        block.keys = Vectorfixed16(keys)
    block.timestamps = VectorShortUnsignedInt([int(t) for t in ts])


RAMPS = (("color_track", "color_keys", "c3"), ("alpha_track", "alpha_keys", "fixed"), ("scale_track", "scale_keys", "c2"))


def read_into(p, e):
    """Fill property group `p` from record `e`."""
    p.flags = _s32(int(e.flags))
    for name in SCALARS:
        setattr(p, name, getattr(e, name))
    for name in VEC3:
        setattr(p, name, _v3(getattr(e, name)))
    tid = int(e.texture_id)
    p.texture_slots = (tid & 31, (tid >> 5) & 31, (tid >> 10) & 31)
    p.tumble_min, p.tumble_max = _v3(e.tumble.minimum), _v3(e.tumble.maximum)
    p.twinkle_scale = (e.twinkle_scale.min, e.twinkle_scale.max)
    p.scale_vary = (e.scale_vary.x, e.scale_vary.y)
    for field, coll, kind in RAMPS:
        ts, vals = _ramp(getattr(e, field), kind)
        c = getattr(p, coll)
        c.clear()
        for t, v in zip(ts, vals):
            k = c.add()
            k.time, k.value = t, v


def _s32(v):
    return v - (1 << 32) if v >= 1 << 31 else v


def _differs(a, b):
    return any(abs(x - y) > TOL for x, y in zip(a, b))


def write_from(p, e):
    """Apply property group `p` to record `e`; fields equal within tolerance are left alone."""
    flags = p.flags & 0xFFFFFFFF
    if flags != int(e.flags):
        e.flags = type(e.flags)(flags) if not isinstance(e.flags, int) else flags
    for name in SCALARS:
        cur, new = getattr(e, name), getattr(p, name)
        if isinstance(cur, float):
            if abs(cur - new) > TOL:
                setattr(e, name, float(new))
        elif int(cur) != int(new):
            setattr(e, name, int(new))
    for name in VEC3:
        if _differs(_v3(getattr(e, name)), getattr(p, name)):
            _set_v3(getattr(e, name), getattr(p, name))
    s = p.texture_slots
    tid = (s[0] & 31) | (s[1] & 31) << 5 | (s[2] & 31) << 10
    if tid != int(e.texture_id):
        e.texture_id = tid
    for box, mine in ((e.tumble.minimum, p.tumble_min), (e.tumble.maximum, p.tumble_max)):
        if _differs(_v3(box), mine):
            _set_v3(box, mine)
    if _differs((e.twinkle_scale.min, e.twinkle_scale.max), p.twinkle_scale):
        e.twinkle_scale.min, e.twinkle_scale.max = (float(x) for x in p.twinkle_scale)
    if _differs((e.scale_vary.x, e.scale_vary.y), p.scale_vary):
        e.scale_vary.x, e.scale_vary.y = (float(x) for x in p.scale_vary)
    for field, coll, kind in RAMPS:
        block = getattr(e, field)
        ts, vals = _ramp(block, kind)
        keys = list(getattr(p, coll))
        nts, nvals = [k.time for k in keys], [tuple(k.value) for k in keys]
        if ts == nts and len(vals) == len(nvals) and not any(_differs(a, b) for a, b in zip(vals, nvals)):
            continue
        _write_ramp(block, kind, nts, nvals)


def set_defaults(p):
    """A visible additive emitter: three-key fade in/out, unit scale."""
    for key, col, alpha, scale in ((0, (255.0, 255.0, 255.0), 0.0, (1.0, 1.0)), (16384, (255.0, 255.0, 255.0), 1.0, (1.0, 1.0)),
                                   (32767, (255.0, 255.0, 255.0), 0.0, (1.0, 1.0))):
        for coll, v in ((p.color_keys, col), (p.alpha_keys, (alpha, 0.0, 0.0)), (p.scale_keys, (*scale, 0.0))):
            k = coll.add()
            k.time, k.value = key, v


RIBBON_SCALARS = ("edges_per_second", "edge_lifetime", "gravity", "texture_rows", "texture_cols", "priority_plane",
                  "ribbon_color_index", "texture_transform_lookup_index")


def ribbon_read_into(p, e):
    for name in RIBBON_SCALARS:
        setattr(p, name, getattr(e, name))
    for coll, vec in ((p.texture_indices, e.texture_indices), (p.material_indices, e.material_indices)):
        coll.clear()
        for v in vec:
            coll.add().value = int(v)


def ribbon_write_from(p, e):
    for name in RIBBON_SCALARS:
        cur, new = getattr(e, name), getattr(p, name)
        if isinstance(cur, float):
            if abs(cur - new) > TOL:
                setattr(e, name, float(new))
        elif int(cur) != int(new):
            setattr(e, name, int(new))
    for coll, vec in ((p.texture_indices, e.texture_indices), (p.material_indices, e.material_indices)):
        want = [int(k.value) for k in coll]
        if want != [int(v) for v in vec]:
            vec.clear()
            for v in want:
                vec.append(v)
