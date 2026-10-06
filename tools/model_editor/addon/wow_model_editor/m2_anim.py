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


def _fixed16(v):
    return np.asarray(v)["value"].astype(np.float64)[:, None] / 32767.0


def _enc_fixed16(f):
    out = []
    for x in np.rint(np.asarray(f, np.float64)[:, 0] * 32767.0).astype(np.int16).tolist():
        c = common.fixed16()
        c.value = x
        out.append(c)
    return wowlib.Vectorfixed16(out)


def _quat4(v):
    """C4Quaternion (x, y, z, w) to Blender (w, x, y, z)."""
    a = np.asarray(v)
    return np.stack([a["w"], a["x"], a["y"], a["z"]], 1).astype(np.float64)


def _enc_quat4(f):
    out = []
    for w, x, y, z in np.asarray(f, np.float32).tolist():
        q = common.C4Quaternion()
        q.w, q.x, q.y, q.z = w, x, y, z
        out.append(q)
    return wowlib.VectorC4Quaternion(out)


def _scalar(v):
    return np.asarray(v, np.float64)[:, None]


def _enc_scalar(vec_cls, dtype):
    def enc(f):
        return vec_cls(np.asarray(f, np.float64)[:, 0].astype(dtype).tolist())
    return enc


def _enc_round(vec_cls, dtype):
    def enc(f):
        return vec_cls(np.rint(np.asarray(f, np.float64)[:, 0]).astype(dtype).tolist())
    return enc


def _spline_float(v):
    """M2 spline keys of floats as (value, in tangent, out tangent) columns."""
    return np.array([(k.value, k.in_tan, k.out_tan) for k in v], np.float64).reshape(len(v), 3)


def _enc_spline_float(f):
    out = []
    for val, i, o in np.asarray(f, np.float32).tolist():
        k = record.M2SplineKeyFloat()
        k.value, k.in_tan, k.out_tan = val, i, o
        out.append(k)
    return wowlib.VectorWowlibFormatsM2RootRecordM2SplineKeyFloat(out)


def _spline_vec3(v):
    return np.array([(k.value.x, k.value.y, k.value.z, k.in_tan.x, k.in_tan.y, k.in_tan.z, k.out_tan.x, k.out_tan.y, k.out_tan.z) for k in v], np.float64).reshape(len(v), 9)


def _enc_spline_vec3(f):
    out = []
    for row in np.asarray(f, np.float32).tolist():
        k = record.M2SplineKeyC3Vector()
        for name, (x, y, z) in zip(("value", "in_tan", "out_tan"), (row[0:3], row[3:6], row[6:9])):
            c = common.C3Vector()
            c.x, c.y, c.z = x, y, z
            setattr(k, name, c)
        out.append(k)
    return wowlib.VectorWowlibFormatsM2RootRecordM2SplineKeyWowlibFormatsCommonC3Vector(out)


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
        k = model.bone_keys[i] if hasattr(model, "bone_keys") else i
        yield Binding(f"bone/{k}/translation", b.translation, arm, pb, path, "location", 3, _vec3, _enc_vec3, name)
        yield Binding(f"bone/{k}/rotation", b.rotation, arm, pb, path, "rotation_quaternion", 4, _cquat, _enc_cquat, name)
        yield Binding(f"bone/{k}/scale", b.scale, arm, pb, path, "scale", 3, _vec3, _enc_vec3, name)


def _prop_binding(arm, key, track, attr, n, default, decode, encode):
    if attr not in arm:
        arm[attr] = default
    return Binding(key, track, arm, arm, "", attr, n, decode, encode)


def model_bindings(model):
    """Colors, texture weights and texture transforms as custom properties of the armature."""
    arm, root = model.armature, model.root
    if arm is None:
        return
    for i, c in enumerate(root.colors):
        yield _prop_binding(arm, f"color/{i}/color", c.color, f"wow_color_{i}", 3, (1.0, 1.0, 1.0), _vec3, _enc_vec3)
        yield _prop_binding(arm, f"color/{i}/alpha", c.alpha, f"wow_alpha_{i}", 1, 1.0, _fixed16, _enc_fixed16)
    for i, w in enumerate(root.texture_weights):
        yield _prop_binding(arm, f"texweight/{i}", w.weight, f"wow_texweight_{i}", 1, 1.0, _fixed16, _enc_fixed16)
    for i, t in enumerate(root.texture_transforms):
        yield _prop_binding(arm, f"texxform/{i}/translation", t.translation, f"wow_tt_translation_{i}", 3, (0.0, 0.0, 0.0), _vec3, _enc_vec3)
        yield _prop_binding(arm, f"texxform/{i}/rotation", t.rotation, f"wow_tt_rotation_{i}", 4, (1.0, 0.0, 0.0, 0.0), _quat4, _enc_quat4)
        yield _prop_binding(arm, f"texxform/{i}/scaling", t.scaling, f"wow_tt_scaling_{i}", 3, (1.0, 1.0, 1.0), _vec3, _enc_vec3)


def _obj_binding(ob, key, track, field, n, default, decode, encode):
    return _prop_binding(ob, key, track, f"wow_{field}", n, default, decode, encode)


def _u8():
    return _scalar, _enc_round(wowlib.VectorUnsignedChar, np.uint8)


def _f32():
    return _scalar, _enc_scalar(wowlib.VectorFloat, np.float32)


def object_bindings(model):
    """Tracks of attachments, lights, cameras, particle and ribbon emitters as custom properties of their objects."""
    ob_of = model.objects
    root = model.root
    u8, f32 = _u8(), _f32()
    vec3 = (_vec3, _enc_vec3)
    for i, a in enumerate(root.attachments):
        ob = ob_of.get(("attachment", i))
        if ob is not None:
            yield _obj_binding(ob, f"attachment/{i}/animate_attached", a.animate_attached, "animate_attached", 1, 1.0, *u8)
    for i, l in enumerate(root.lights):
        ob = ob_of.get(("light", i))
        if ob is None:
            continue
        for field, n, default, codec in (("ambient_color", 3, (0.0, 0.0, 0.0), vec3), ("ambient_intensity", 1, 0.0, f32), ("diffuse_color", 3, (1.0, 1.0, 1.0), vec3),
                                         ("diffuse_intensity", 1, 1.0, f32), ("attenuation_start", 1, 0.0, f32), ("attenuation_end", 1, 0.0, f32), ("visibility", 1, 1.0, u8)):
            yield _obj_binding(ob, f"light/{i}/{field}", getattr(l, field), field, n, default, *codec)
    for i, c in enumerate(root.cameras):
        ob = ob_of.get(("camera", i))
        if ob is None:
            continue
        zero9 = (0.0,) * 9
        yield _obj_binding(ob, f"camera/{i}/positions", c.positions, "positions", 9, zero9, _spline_vec3, _enc_spline_vec3)
        yield _obj_binding(ob, f"camera/{i}/target_position", c.target_position, "target_position", 9, zero9, _spline_vec3, _enc_spline_vec3)
        yield _obj_binding(ob, f"camera/{i}/roll", c.roll, "roll", 3, (0.0,) * 3, _spline_float, _enc_spline_float)
        yield _obj_binding(ob, f"camera/{i}/fov", c.fov, "fov", 3, (0.0,) * 3, _spline_float, _enc_spline_float)
    for i, e in enumerate(root.particle_emitters):
        ob = ob_of.get(("particle", i))
        if ob is None:
            continue
        for field in ("emission_speed", "speed_variation", "vertical_range", "horizontal_range", "gravity", "lifespan", "emission_rate", "emission_area_length", "emission_area_width", "z_source"):
            yield _obj_binding(ob, f"particle/{i}/{field}", getattr(e, field), field, 1, 0.0, *f32)
        yield _obj_binding(ob, f"particle/{i}/enabled_in", e.enabled_in, "enabled_in", 1, 1.0, *u8)
    for i, e in enumerate(root.ribbon_emitters):
        ob = ob_of.get(("ribbon", i))
        if ob is None:
            continue
        yield _obj_binding(ob, f"ribbon/{i}/color", e.color, "color", 3, (1.0, 1.0, 1.0), *vec3)
        yield _obj_binding(ob, f"ribbon/{i}/alpha", e.alpha, "alpha", 1, 1.0, _fixed16, _enc_fixed16)
        yield _obj_binding(ob, f"ribbon/{i}/height_above", e.height_above, "height_above", 1, 0.0, *f32)
        yield _obj_binding(ob, f"ribbon/{i}/height_below", e.height_below, "height_below", 1, 0.0, *f32)
        yield _obj_binding(ob, f"ribbon/{i}/tex_slot", e.tex_slot, "tex_slot", 1, 0.0, _scalar, _enc_round(wowlib.VectorShortUnsignedInt, np.uint16))
        yield _obj_binding(ob, f"ribbon/{i}/visibility", e.visibility, "visibility", 1, 1.0, *u8)


def event_tracks(model):
    """(event index, object, track) of every event with an object."""
    for i, e in enumerate(model.root.events):
        ob = model.objects.get(("event", i))
        if ob is not None:
            yield i, ob, e.enabled


def object_map(top):
    return {(o.wow_m2_object.kind, o.wow_m2_object.index): o for o in top.all_objects if o.wow_m2_object.kind}


def all_bindings(model):
    return list(bone_bindings(model)) + list(model_bindings(model)) + list(object_bindings(model))


def classify(binding):
    t = binding.track
    n = t.timeline_count()
    if t.global_sequence != NONE:
        binding.kind = "global"
    elif n == 0:
        binding.kind = "empty"
    elif n == 1 and t.key_count(0) <= 1:
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


def marker_name(i):
    return f"event_{i}"


def add_event_markers(action, events, ti):
    """Event firing times of timeline `ti` as pose markers named after the event index."""
    for i, _ob, t in events:
        if t.timeline_count() <= ti:
            continue
        for ts in t.timeline_timestamps(ti):
            action.pose_markers.new(marker_name(i)).frame = int(ts)


def _drive(data, path, index, ob, prop, expr="v"):
    fc = data.driver_add(path, index) if index >= 0 else data.driver_add(path)
    d = fc.driver
    d.type = "SCRIPTED"
    d.expression = expr
    v = d.variables.new()
    v.name, v.type = "v", "SINGLE_PROP"
    v.targets[0].id = ob
    v.targets[0].data_path = prop


def drive_object_data(model):
    """Blender light colour/energy/range and camera field of view follow the wow_* properties."""
    for (kind, i), ob in model.objects.items():
        if kind == "light" and ob.type == "LIGHT":
            for c in range(3):
                _drive(ob.data, "color", c, ob, f'["wow_diffuse_color"][{c}]')
            _drive(ob.data, "energy", -1, ob, '["wow_diffuse_intensity"]', "v * 100")
            if ob.data.use_custom_distance:
                _drive(ob.data, "cutoff_distance", -1, ob, '["wow_attenuation_end"]', "max(v, 0.01)")
        elif kind == "camera" and ob.type == "CAMERA" and "wow_fov" in ob:
            if 0 < ob["wow_fov"][0] < math.pi:
                _drive(ob.data, "lens", -1, ob, '["wow_fov"][0]', f"{ob.data.sensor_height} / (2 * tan(v / 2))")


def import_animations(model, top):
    """Fills `wow_m2.sequences` / `loops` and builds their actions; returns the binding list."""
    root = model.root
    p = top.wow_m2
    model.objects = object_map(top)
    bindings = all_bindings(model)
    events = list(event_tracks(model))
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
    if static:
        top[STATIC_PROP] = static
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
        add_event_markers(e.action, [ev for ev in events if ev[2].global_sequence == NONE], si)
    for k, loop in enumerate(root.global_loops):
        e = p.loops.add()
        e.timestamp = int(loop.timestamp)
        sel = [(b, 0) for b in by_loop.get(k, ()) if b.track.timeline_count() and b.track.key_count(0)]
        e.action = build_action(f"{model.stem}_loop{k}", sel, int(loop.timestamp))
        add_event_markers(e.action, [ev for ev in events if ev[2].global_sequence == k], 0)
    p.active_sequence = -1
    drive_object_data(model)
    return bindings
