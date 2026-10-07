"""Attachments, events, lights, cameras, emitters and collision geometry back into a freshly read wowlib M2."""
import copy

import numpy as np
from wowlib.formats.common import C3Vector
from wowlib.formats.m2.root import record as rec

from . import m2_mesh_export
from .m2_import import _vec

POS_TOL = 1e-4


def _kind_objects(top, kind):
    """(existing by source index, new objects by name) of `kind` under `top`."""
    old, new = {}, []
    for ob in top.all_objects:
        p = ob.wow_m2_object
        if p.kind != kind:
            continue
        if p.index >= 0:
            old[p.index] = ob
        else:
            new.append(ob)
    new.sort(key=lambda o: o.name)
    return old, new


def _position(ob, arm):
    mw = ob.matrix_world if arm is None else arm.matrix_world.inverted() @ ob.matrix_world
    return tuple(float(x) for x in mw.translation)


def _set_pos(dst, pos, src=None):
    """Write `pos` into vector `dst` unless it already matches `src` within tolerance."""
    if src is not None and all(abs(a - b) <= POS_TOL for a, b in zip(pos, _vec(src))):
        return
    dst.x, dst.y, dst.z = pos


def _bone(ob, arm, rig, keep):
    if rig is not None and ob.parent is arm and ob.parent_type == "BONE" and ob.parent_bone in rig.by_name:
        return rig.by_name[ob.parent_bone]
    return keep


def _identifier(code):
    return int.from_bytes((code + "\0\0\0\0")[:4].encode("latin-1", "replace"), "little")


def _rebuild(vec, records):
    vec.clear()
    for r in records:
        vec.append(r)


def _simple(top, m, report, arm, rig, kind, vec, bone_field, kind_label):
    """Modify-only objects: bone and position of each source record from its Blender object."""
    objs, extra = _kind_objects(top, kind)
    for i, r in enumerate(vec):
        ob = objs.get(i)
        if ob is None:
            report.add("WARNING", "objects", f"{kind_label} {i} was deleted; deleting is not supported, kept")
            continue
        setattr(r, bone_field, _bone(ob, arm, rig, int(getattr(r, bone_field))))
        _set_pos(r.position, _position(ob, arm), r.position)
    if extra:
        report.add("WARNING", "objects", f"new {kind_label}s are not supported, {len(extra)} ignored")


def _attachments(top, m, report, arm, rig):
    root = m.root
    objs, new = _kind_objects(top, "attachment")
    src = list(root.attachments)
    order = [objs[i] for i in sorted(objs) if i < len(src)] + new
    out, index_of = [], {}
    cls = rec.M2AttachmentWotlkPlus
    for ob in order:
        p = ob.wow_m2_object
        a = copy.copy(src[p.index]) if 0 <= p.index < len(src) else cls()
        a.id, a.unknown = p.id & 0xFFFFFFFF, p.data & 0xFFFFFFFF
        a.bone = _bone(ob, arm, rig, int(a.bone))
        _set_pos(a.position, _position(ob, arm), src[p.index].position if 0 <= p.index < len(src) else None)
        index_of[ob.name] = len(out)
        out.append(a)
    if len(out) != len(src):
        report.add("INFO", "objects", f"{len(src)} -> {len(out)} attachments")
    _rebuild(root.attachments, out)
    ids = [int(a.id) for a in out]
    if ids != [int(a.id) for a in src]:
        lut = [0xFFFF] * (max(ids) + 1 if ids else 0)
        for j, i in enumerate(ids):
            lut[i] = j
        _rebuild(root.attachment_lookup_table, lut)
    return {("attachment", j): ob for j, ob in enumerate(order)}


def _events(top, m, report, arm, rig):
    root = m.root
    objs, new = _kind_objects(top, "event")
    src = list(root.events)
    order = [objs[i] for i in sorted(objs) if i < len(src)] + new
    out = []
    cls = rec.M2EventWotlkPlus
    for ob in order:
        p = ob.wow_m2_object
        e = copy.copy(src[p.index]) if 0 <= p.index < len(src) else cls()
        ident = _identifier(p.identifier or ob.name)
        e.identifier, e.data = ident, p.data & 0xFFFFFFFF
        e.bone = _bone(ob, arm, rig, int(e.bone))
        _set_pos(e.position, _position(ob, arm), src[p.index].position if 0 <= p.index < len(src) else None)
        out.append(e)
    if len(out) != len(src):
        report.add("INFO", "objects", f"{len(src)} -> {len(out)} events")
    _rebuild(root.events, out)
    return {("event", j): ob for j, ob in enumerate(order)}


def _cameras(top, m, report):
    root = m.root
    objs, extra = _kind_objects(top, "camera")
    for i, c in enumerate(root.cameras):
        ob = objs.get(i)
        if ob is None:
            report.add("WARNING", "objects", f"camera {i} was deleted; deleting is not supported, kept")
            continue
        if abs(c.near_clip - ob.data.clip_start) > POS_TOL:
            c.near_clip = ob.data.clip_start
        if abs(c.far_clip - ob.data.clip_end) > POS_TOL:
            c.far_clip = ob.data.clip_end
        _set_pos(c.position_base, tuple(float(x) for x in ob.matrix_world.translation), c.position_base)
        con = next((k for k in ob.constraints if k.type == "TRACK_TO" and k.target is not None), None)
        if con is not None:
            _set_pos(c.target_position_base, tuple(float(x) for x in con.target.matrix_world.translation),
                     c.target_position_base)
    if extra:
        report.add("WARNING", "objects", f"new cameras are not supported, {len(extra)} ignored")


def _lights(top, m, report, arm, rig):
    _simple(top, m, report, arm, rig, "light", m.root.lights, "bone", "light")


def _emitters(top, m, report, arm, rig):
    _simple(top, m, report, arm, rig, "particle", m.root.particle_emitters, "bone_id", "particle emitter")
    _simple(top, m, report, arm, rig, "ribbon", m.root.ribbon_emitters, "bone_index", "ribbon emitter")


def _tuple(v):
    return (float(v.x), float(v.y), float(v.z))


def _collision(top, m, report):
    root = m.root
    p = top.wow_m2
    if not any(p.collision_min) and not any(p.collision_max) and not p.collision_radius:
        return
    for end in ("min", "max"):
        box = getattr(root.collision_box, end)
        _set_pos(box, tuple(float(x) for x in getattr(p, f"collision_{end}")), box)
    if abs(root.collision_sphere_radius - p.collision_radius) > POS_TOL:
        root.collision_sphere_radius = p.collision_radius
    ob = next((o for o in top.all_objects if o.type == "MESH" and m2_mesh_export.is_collision(o)), None)
    if ob is None:
        return
    me = ob.data
    me.calc_loop_triangles()
    n = len(me.vertices)
    pos = np.empty(n * 3, np.float32)
    me.vertices.foreach_get("co", pos)
    pos = pos.reshape(-1, 3)
    tris = np.empty(len(me.loop_triangles) * 3, np.int64)
    me.loop_triangles.foreach_get("vertices", tris)
    cur = np.array([_tuple(v) for v in root.collision_vertices], np.float32).reshape(-1, 3)
    cur_t = np.asarray(root.collision_triangles, np.int64)
    if cur.shape == pos.shape and np.allclose(cur, pos, atol=POS_TOL) and np.array_equal(cur_t, tris):
        return
    if n > 0xFFFF:
        report.add("ERROR", "objects", f"collision mesh has {n} vertices (max 65535)")
        return
    tri = tris.reshape(-1, 3)
    pa, pb, pc = pos[tri[:, 0]], pos[tri[:, 1]], pos[tri[:, 2]]
    nrm = np.cross(pb - pa, pc - pa)
    ln = np.linalg.norm(nrm, axis=1, keepdims=True)
    nrm = np.where(ln > 0, nrm / np.where(ln > 0, ln, 1), (0, 0, 1))
    vecs = []
    for arr in (pos, nrm):
        out = []
        for x, y, z in arr.tolist():
            v = C3Vector()
            v.x, v.y, v.z = x, y, z
            out.append(v)
        vecs.append(out)
    _rebuild(root.collision_vertices, vecs[0])
    _rebuild(root.collision_normals, vecs[1])
    root.collision_triangles.clear()
    for i in tris.tolist():
        root.collision_triangles.append(i)
    report.add("INFO", "objects", f"collision mesh: {n} vertices, {len(tri)} triangles")


def build(top, m, report, rig):
    """Rewrite object sections of `m`; returns the {(kind, new index): object} map for animation export."""
    arm = m2_mesh_export.find_armature(top)
    objects = {}
    objects.update(_attachments(top, m, report, arm, rig))
    objects.update(_events(top, m, report, arm, rig))
    _cameras(top, m, report)
    _lights(top, m, report, arm, rig)
    _emitters(top, m, report, arm, rig)
    _collision(top, m, report)
    for kind, vec in (("camera", m.root.cameras), ("light", m.root.lights),
                      ("particle", m.root.particle_emitters), ("ribbon", m.root.ribbon_emitters)):
        o, _ = _kind_objects(top, kind)
        objects.update({(kind, i): ob for i, ob in o.items() if i < len(vec)})
    return objects
