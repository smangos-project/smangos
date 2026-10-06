"""M2 animation tracks as Blender actions: one action per sequence and per global loop, 1 frame = 1 ms."""
import math

import bpy
import numpy as np
import wowlib
from wowlib.formats import common
from wowlib.formats.m2.root import record

from . import m2_names

NONE = 0xFFFF
ALIAS_FLAG = 0x40
CONSTANT, LINEAR, BEZIER = 0, 1, 2
INTERP = {0: CONSTANT, 1: LINEAR, 2: BEZIER, 3: BEZIER}
STATIC_PROP = "wow_static_tracks"


def _vec3(v):
    a = np.asarray(v)
    return np.stack([a["x"], a["y"], a["z"]], 1).astype(np.float64)


def _cquat(v):
    """Compressed quaternion (x, y, z, w int16) to Blender (w, x, y, z); raw 32767 decodes to -0.0 so both zeros re-encode."""
    a = np.asarray(v)
    out = np.empty((len(a), 4), np.float64)
    for j, f in enumerate("wxyz"):
        r = a[f].astype(np.int32)
        col = np.where(r < 0, r + 32768.0, r - 32767.0) / 32767.0
        out[:, j] = np.where(r == 32767, -0.0, col)
    return out


def _enc_cquat(f):
    f = np.asarray(f, np.float64)
    x = np.rint(f * 32767.0)
    v = np.where(np.signbit(f), x + 32767.0, x - 32768.0).astype(np.int16)
    out = []
    for w, qx, qy, qz in v.tolist():
        q = record.M2CompQuat()
        q.w, q.x, q.y, q.z = w, qx, qy, qz
        out.append(q)
    return wowlib.VectorM2CompQuat(out)


def _enc_vec3(f):
    out = []
    for x, y, z in np.asarray(f, np.float32).tolist():
        c = common.C3Vector()
        c.x, c.y, c.z = x, y, z
        out.append(c)
    return wowlib.VectorC3Vector(out)


class Binding:
    """One M2 track and the Blender property (object, path, attr) that carries it."""

    def __init__(self, key, track, obj, owner, path, attr, n, decode, encode, group=""):
        self.key, self.track, self.obj, self.owner = key, track, obj, owner
        self.path, self.attr, self.n = path, attr, n
        self.decode, self.encode, self.group = decode, encode, group
        self.kind = ""

    @property
    def data_path(self):
        return f"{self.path}.{self.attr}" if self.path else f'["{self.attr}"]'

    def set_static(self, vals):
        v = tuple(vals[0]) if self.n > 1 else float(vals[0][0])
        if self.path:
            setattr(self.owner, self.attr, v)
        else:
            self.owner[self.attr] = v

    def get_static(self):
        v = getattr(self.owner, self.attr) if self.path else self.owner[self.attr]
        return np.array([tuple(v) if self.n > 1 else (v,)], np.float64)


def bone_bindings(model):
    arm = model.armature
    if arm is None:
        return
    for i, b in enumerate(model.root.bones):
        name = model.bone_names[i]
        pb = arm.pose.bones[name]
        pb.rotation_mode = "QUATERNION"
        path = f'pose.bones["{name}"]'
        yield Binding(f"bone/{i}/translation", b.translation, arm, pb, path, "location", 3, _vec3, _enc_vec3, name)
        yield Binding(f"bone/{i}/rotation", b.rotation, arm, pb, path, "rotation_quaternion", 4, _cquat, _enc_cquat, name)
        yield Binding(f"bone/{i}/scale", b.scale, arm, pb, path, "scale", 3, _vec3, _enc_vec3, name)


def all_bindings(model):
    return list(bone_bindings(model))


def classify(binding):
    t = binding.track
    n = t.timeline_count()
    if t.global_sequence != NONE:
        binding.kind = "global"
    elif n == 0:
        binding.kind = "empty"
    elif n == 1:
        binding.kind = "static"
    else:
        binding.kind = "seq"
    return binding.kind


def timeline(binding, i):
    """(timestamps int64, values float64 (k, n)) of timeline i."""
    t = binding.track
    ts = np.asarray(t.timeline_timestamps(i), np.int64)
    return ts, binding.decode(t.timeline_values(i))


def _write_fcurves(cb, b, ts, vals, interp):
    k = len(ts)
    co = np.empty(2 * k, np.float32)
    co[0::2] = ts
    for c in range(b.n):
        fc = cb.fcurves.new(b.data_path, index=c, group_name=b.group) if b.group else cb.fcurves.new(b.data_path, index=c)
        kp = fc.keyframe_points
        kp.add(k)
        co[1::2] = vals[:, c]
        kp.foreach_set("co", co)
        kp.foreach_set("handle_left", co)
        kp.foreach_set("handle_right", co)
        kp.foreach_set("interpolation", np.full(k, interp, np.int32))
        fc.update()


def build_action(name, selected, frame_end):
    """Action holding `(binding, timeline index)` pairs, one slot per animated object."""
    act = bpy.data.actions.new(name)
    act.use_fake_user = True
    act.use_frame_range = True
    act.frame_start, act.frame_end = 0, max(frame_end, 1)
    layer = act.layers.new("Layer")
    strip = layer.strips.new(type="KEYFRAME")
    bags = {}
    for b, ti in selected:
        ts, vals = timeline(b, ti)
        if not len(ts):
            continue
        cb = bags.get(b.obj.name)
        if cb is None:
            slot = act.slots.new("OBJECT", b.obj.name)
            cb = bags[b.obj.name] = strip.channelbag(slot, ensure=True)
        _write_fcurves(cb, b, ts, vals, INTERP.get(int(b.track.interpolation_type), LINEAR))
    return act


def import_animations(model, top):
    """Fills `wow_m2.sequences` / `loops` and builds their actions; returns the binding list."""
    root = model.root
    p = top.wow_m2
    bindings = all_bindings(model)
    static = []
    by_seq = []
    by_loop = {}
    for b in bindings:
        kind = classify(b)
        if kind == "static":
            ts, vals = timeline(b, 0)
            if len(ts):
                b.set_static(vals)
                static.append(b.key)
        elif kind == "global":
            by_loop.setdefault(int(b.track.global_sequence), []).append(b)
        elif kind == "seq":
            by_seq.append(b)
    if static and model.armature is not None:
        model.armature[STATIC_PROP] = static
    scene = bpy.context.scene
    scene.render.fps, scene.render.fps_base = 1000, 1.0
    scene.sync_mode = "FRAME_DROP"
    for si, s in enumerate(root.sequences):
        e = p.sequences.add()
        e.anim_id, e.variation_index, e.variation_next, e.alias_next = int(s.id), int(s.variation_index), int(s.variation_next), int(s.alias_next)
        e.flags, e.duration, e.frequency = int(s.flags), int(s.duration), int(s.frequency)
        e.blend_time_in, e.blend_time_out, e.movespeed, e.padding = int(s.blend_time_in), int(s.blend_time_out), float(s.movespeed), int(s.padding)
        e.replay_min, e.replay_max = int(s.replay.minimum), int(s.replay.maximum)
        ext = s.bounds.extent
        e.bounds_min, e.bounds_max = (ext.min.x, ext.min.y, ext.min.z), (ext.max.x, ext.max.y, ext.max.z)
        e.bounds_radius = float(s.bounds.radius)
        if int(s.flags) & ALIAS_FLAG:
            continue
        sel = [(b, si) for b in by_seq if si < b.track.timeline_count() and b.track.key_count(si)]
        name = f"{model.stem}_{m2_names.anim_name(s.id)}_{int(s.variation_index)}"
        e.action = build_action(name, sel, int(s.duration))
    for k, loop in enumerate(root.global_loops):
        e = p.loops.add()
        e.timestamp = int(loop.timestamp)
        sel = [(b, 0) for b in by_loop.get(k, ()) if b.track.timeline_count() and b.track.key_count(0)]
        e.action = build_action(f"{model.stem}_loop{k}", sel, int(loop.timestamp))
    p.active_sequence = -1
    return bindings
