"""Bakes a Blender camera animation into the spline tracks of an M2 camera."""
import math

import bpy
import mathutils

from .m2_objects_export import _camera_target


def _roll(ob, pos, target):
    fwd = mathutils.Vector(target) - mathutils.Vector(pos)
    if fwd.length < 1e-6:
        return 0.0
    fwd.normalize()
    right = fwd.cross(mathutils.Vector((0.0, 0.0, 1.0)))
    if right.length < 1e-6:
        return 0.0
    right.normalize()
    ref_up = right.cross(fwd)
    up = ob.matrix_world.to_3x3().col[1].normalized()
    return math.atan2(up.dot(right), up.dot(ref_up))


def sample(scene, ob, frames):
    """(positions, targets, fovs, rolls) of `ob` at each frame."""
    cur = scene.frame_current
    out = ([], [], [], [])
    try:
        for f in frames:
            scene.frame_set(int(f))
            dg = bpy.context.evaluated_depsgraph_get()
            ev = ob.evaluated_get(dg)
            pos = tuple(ev.matrix_world.translation)
            tgt = _camera_target(ev)
            for lst, v in zip(out, (pos, tgt, ev.data.angle_y, _roll(ev, pos, tgt))):
                lst.append(v)
    finally:
        scene.frame_set(cur)
    return out


def tangents(values, frames):
    """Catmull-Rom tangents (in == out) of vector keys, one-sided at the ends."""
    n = len(values)
    out = []
    for i in range(n):
        a, b = values[max(i - 1, 0)], values[min(i + 1, n - 1)]
        span = frames[min(i + 1, n - 1)] - frames[max(i - 1, 0)]
        k = 0.5 * (frames[min(i + 1, n - 1)] - frames[max(i - 1, 0)]) / span if span else 0.0
        out.append(tuple(k * (y - x) for x, y in zip(a, b)))
    return out


def _channelbag(action, ob):
    slot = next((s for s in action.slots if s.identifier == "OB" + ob.name), None)
    if slot is None:
        slot = action.slots.new("OBJECT", ob.name)
    if not action.layers:
        action.layers.new("Layer")
    layer = action.layers[0]
    if not layer.strips:
        layer.strips.new(type="KEYFRAME")
    return layer.strips[0].channelbag(slot, ensure=True)


def _write(cb, prop, n, frames, rows):
    for c in range(n):
        old = cb.fcurves.find(f'["{prop}"]', index=c)
        if old is not None:
            cb.fcurves.remove(old)
        fc = cb.fcurves.new(f'["{prop}"]', index=c)
        for f, row in zip(frames, rows):
            fc.keyframe_points.insert(f, row[c], options={"FAST"}).interpolation = "LINEAR"
        fc.update()


def bake(scene, ob, action, start, end, step):
    """Writes the camera's motion over [start, end] ms (every `step`) as keys of `action`."""
    frames = list(range(int(start), int(end) + 1, max(int(step), 1)))
    if frames[-1] != int(end):
        frames.append(int(end))
    pos, tgt, fov, roll = sample(scene, ob, frames)
    base_p, base_t = pos[0], tgt[0]
    rel_p = [tuple(a - b for a, b in zip(p, base_p)) for p in pos]
    rel_t = [tuple(a - b for a, b in zip(p, base_t)) for p in tgt]
    ob["wow_base"], ob["wow_target_base"] = base_p, base_t
    cb = _channelbag(action, ob)
    for prop, rel in (("wow_positions", rel_p), ("wow_target_position", rel_t)):
        tan = tangents(rel, frames)
        _write(cb, prop, 9, frames, [v + t + t for v, t in zip(rel, tan)])
    _write(cb, "wow_fov", 3, frames, [(v, 0.0, 0.0) for v in fov])
    _write(cb, "wow_roll", 3, frames, [(v, 0.0, 0.0) for v in roll])
    return frames
