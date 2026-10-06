"""Blender animation data back into a freshly read wowlib M2 (sequences, global loops and tracks); no file write."""
import bpy
import numpy as np
import wowlib

from . import m2_anim
from .m2_anim import ALIAS_FLAG, NONE


class _Shim:
    """The slice of an import `Model` that the binding builders use."""

    def __init__(self, top, root):
        self.root = root
        self.armature = next((o for o in top.all_objects if o.type == "ARMATURE"), None)
        names = {}
        if self.armature is not None:
            for b in self.armature.data.bones:
                names[b.wow_m2_bone.index] = b.name
        self.bone_names = [names.get(i, "") for i in range(len(root.bones))]
        self.objects = m2_anim.object_map(top)


def _channelbag(action, obj):
    if action is None:
        return None
    slot = next((s for s in action.slots if s.identifier == "OB" + obj.name), None)
    if slot is None or not action.layers or not action.layers[0].strips:
        return None
    return action.layers[0].strips[0].channelbag(slot)


def _read(b, action):
    """(timestamps, values (k, n), interpolation) of binding `b` in `action`, or None without keys."""
    cb = _channelbag(action, b.obj)
    if cb is None:
        return None
    curves = [cb.fcurves.find(b.data_path, index=c) for c in range(b.n)]
    curves = [fc for fc in curves if fc is not None and len(fc.keyframe_points)]
    if not curves:
        return None
    frames = np.unique(np.concatenate([_co(fc)[0] for fc in curves]))
    default = b.get_static()[0]
    vals = np.empty((len(frames), b.n), np.float64)
    for c in range(b.n):
        fc = cb.fcurves.find(b.data_path, index=c)
        if fc is None or not len(fc.keyframe_points):
            vals[:, c] = default[c]
            continue
        f, v = _co(fc)
        if len(f) == len(frames) and np.array_equal(f, frames):
            vals[:, c] = v
        else:
            vals[:, c] = [fc.evaluate(x) for x in frames]
    first = curves[0].keyframe_points[0].interpolation
    ipo = {"CONSTANT": 0, "LINEAR": 1}.get(first, int(b.track.interpolation_type) if int(b.track.interpolation_type) >= 2 else 2)
    return np.rint(frames).astype(np.int64), vals, ipo


def _co(fc):
    kp = fc.keyframe_points
    buf = np.empty(2 * len(kp), np.float32)
    kp.foreach_get("co", buf)
    return buf[0::2].astype(np.float64), buf[1::2].astype(np.float64)


def _vec_uint(a):
    return wowlib.VectorUnsignedInt([int(x) for x in a])


def _set_timelines(b, items):
    """items: list of (ts, vals) or None per timeline."""
    t = b.track
    ts_cls, val_cls = type(t.timestamps), type(t.values)
    ts = [_vec_uint(i[0]) if i is not None else _vec_uint([]) for i in items]
    vals = [b.encode(i[1]) if i is not None else b.encode(np.zeros((0, b.n))) for i in items]
    t.timestamps, t.values = ts_cls(ts), val_cls(vals)


def write_sequences(root, seqs):
    if not len(root.sequences):
        return
    cls = type(root.sequences[0])
    ids = [s.anim_id for s in seqs]
    unchanged = ids == [int(s.id) for s in root.sequences]
    out = []
    for i, e in enumerate(seqs):
        s = cls()
        s.id = e.anim_id
        s.variation_index = ids[:i].count(e.anim_id)
        nxt = next((j for j in range(i + 1, len(ids)) if ids[j] == e.anim_id), -1)
        s.variation_next = nxt
        s.alias_next = e.alias_next
        s.flags = type(s.flags)(e.flags)
        s.duration, s.frequency = e.duration, e.frequency
        s.blend_time_in, s.blend_time_out, s.movespeed = e.blend_time_in, e.blend_time_out, e.movespeed
        s.replay.minimum, s.replay.maximum = e.replay_min, e.replay_max
        s.bounds.extent.min.x, s.bounds.extent.min.y, s.bounds.extent.min.z = e.bounds_min
        s.bounds.extent.max.x, s.bounds.extent.max.y, s.bounds.extent.max.z = e.bounds_max
        s.bounds.radius = e.bounds_radius
        s.padding = e.padding
        out.append(s)
    root.sequences.clear()
    for s in out:
        root.sequences.append(s)
    if unchanged:
        return
    lookup = [-1] * max(len(root.sequence_lookups), max(ids, default=-1) + 1)
    for i, a in enumerate(ids):
        if lookup[a] < 0:
            lookup[a] = i
    root.sequence_lookups.clear()
    for v in lookup:
        root.sequence_lookups.append(v)


def write_loops(root, loops):
    out = []
    for e in loops:
        m = type(root.global_loops[0])() if len(root.global_loops) else None
        if m is None:
            break
        m.timestamp = e.timestamp
        out.append(m)
    if len(out) == len(loops):
        root.global_loops.clear()
        for m in out:
            root.global_loops.append(m)


def _marker_times(action, i):
    return sorted(int(m.frame) for m in action.pose_markers if m.name == m2_anim.marker_name(i)) if action is not None else []


def _write_events(model, seq_actions, loop_actions):
    for i, _ob, t in m2_anim.event_tracks(model):
        loops = [(k, _marker_times(a, i)) for k, a in enumerate(loop_actions)]
        loops = [(k, ts) for k, ts in loops if ts]
        per_seq = [_marker_times(a, i) for a in seq_actions]
        if loops:
            t.timestamps = type(t.timestamps)([_vec_uint(loops[0][1])])
            t.global_sequence = loops[0][0]
        elif any(per_seq) or t.timeline_count():
            t.timestamps = type(t.timestamps)([_vec_uint(ts) for ts in per_seq])
            t.global_sequence = NONE


def build(top, root):
    """Rewrites sequences, loops and every animated track of `root` from the Blender data under `top`."""
    p = top.wow_m2
    model = _Shim(top, root)
    write_sequences(root, list(p.sequences))
    write_loops(root, list(p.loops))
    static = set(top.get(m2_anim.STATIC_PROP, []))
    seq_actions = [None if s.flags & ALIAS_FLAG else s.action for s in p.sequences]
    loop_actions = [l.action for l in p.loops]
    for b in m2_anim.all_bindings(model):
        loops = [(k, _read(b, a)) for k, a in enumerate(loop_actions)]
        loops = [(k, r) for k, r in loops if r is not None]
        per_seq = [_read(b, a) for a in seq_actions]
        if loops:
            k, r = loops[0]
            _set_timelines(b, [(r[0], r[1])])
            b.track.global_sequence, b.track.interpolation_type = k, r[2]
        elif any(r is not None for r in per_seq):
            _set_timelines(b, [(r[0], r[1]) if r else None for r in per_seq])
            b.track.global_sequence = NONE
            b.track.interpolation_type = next(r[2] for r in per_seq if r)
        elif b.key in static:
            _set_timelines(b, [(np.zeros(1, np.int64), b.get_static())])
            b.track.global_sequence = NONE
        else:
            _set_timelines(b, [])
    _write_events(model, seq_actions, loop_actions)
