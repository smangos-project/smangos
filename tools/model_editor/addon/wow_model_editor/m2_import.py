"""Imports a 1.15 M2 at rest: skin submeshes as meshes, armature with weights, preview materials, attachments and friends."""
import math
import posixpath

import bpy
import mathutils
import numpy as np
from wowlib.formats import m2 as wm2

from . import client, displays, index, m2_materials, m2_names
from .props import s32

MIN_BONE_LENGTH = 0.02
SKIN_TYPES = {11: 0, 12: 1, 13: 2}
DEFAULTS = {"skin": 0, "display": "AUTO", "display_id": 0, "attachments": True, "events": True, "cameras": True,
            "lights": True, "emitters": True, "collision": True, "armature": True, "mesh_only": False}
_import_warnings = []


def _link(parent, name):
    c = bpy.data.collections.new(name)
    parent.children.link(c)
    return c


def read(path):
    m = wm2.M2.for_version(client.VERSION)
    m.read(client.session().fs, client.key(path))
    return m


def _first(track, default=None):
    """First key of the first timeline, or `default` when the track has none."""
    try:
        if track.timeline_count() and track.key_count(0):
            return track.timeline_values(0)[0]
    except Exception:  # noqa: BLE001
        pass
    return default


def _f(x):
    v = x.as_float
    return v() if callable(v) else v


def _vec(v):
    return (v.x, v.y, v.z)


def _gather(root):
    """Positions, normals, uvs and skinning of all root vertices as numpy arrays."""
    vs = root.vertices
    n = len(vs)
    flat = np.empty((n, 14), np.float32)
    bones = np.empty((n, 4), np.uint8)
    weights = np.empty((n, 4), np.uint8)
    for i, v in enumerate(vs):
        p, nr, t0, t1 = v.pos, v.normal, v.tex_coords[0], v.tex_coords[1]
        flat[i] = (p.x, p.y, p.z, nr.x, nr.y, nr.z, t0.x, t0.y, t1.x, t1.y, 0, 0, 0, 0)
        bones[i] = v.bone_indices
        weights[i] = v.bone_weights
    return flat[:, 0:3], flat[:, 3:6], flat[:, 6:8], flat[:, 8:10], bones, weights


class Model:
    """Everything read from one M2 that the builders share."""

    def __init__(self, path, m, opts):
        self.path, self.m, self.root, self.opts = path, m, m.root, opts
        self.fdid = index.fdid_of(path) or 0
        self.base = posixpath.basename(path)
        self.stem = posixpath.splitext(self.base)[0]
        self.skin_index = min(max(opts["skin"], 0), max(len(m.skins) - 1, 0))
        self.profile = m.skins[self.skin_index].profile
        self.tex_fdids = list(getattr(m.chunks, "texture_fdids", []))
        self.display = None
        self.replaceable = {}
        self.armature = None
        self.bone_names = []


def resolve_replaceable(model):
    """{texture type: fdid} for the replaceable textures of the chosen display or by path heuristic."""
    out = {}
    opts = model.opts
    disp = None
    try:
        if opts["display"] == "ID":
            disp = displays.display(model.fdid, opts["display_id"])
        elif opts["display"] == "AUTO":
            disp = next(iter(displays.for_model(model.fdid)), None)
    except Exception as e:  # noqa: BLE001
        _import_warnings.append(f"displays unavailable: {e}")
    model.display = disp
    if disp:
        for typ, slot in SKIN_TYPES.items():
            if disp.textures[slot]:
                out[typ] = disp.textures[slot]
    folder = posixpath.dirname(model.path)
    for typ in {t.type for t in model.root.textures if t.type and t.type not in out}:
        if typ == 1:
            guess = index.fdid_of(f"{folder}/{model.stem}skin00_00.blp")
            if guess:
                out[typ] = guess
    return out


def _texture_info(model, batch, k):
    """(texture index, type, fdid, uv set) of texture slot k of a batch."""
    root = model.root
    li = batch.texture_combo_index + k
    ti = root.texture_lookup_table[li] if li < len(root.texture_lookup_table) else -1
    if ti < 0 or ti >= len(root.textures):
        return -1, 0, 0, 0
    t = root.textures[ti]
    fdid = model.tex_fdids[ti] if ti < len(model.tex_fdids) else 0
    if t.type:
        fdid = model.replaceable.get(int(t.type), 0)
    mi = batch.texture_coord_combo_index + k
    uv = root.texture_mapping_lookup_table[mi] if mi < len(root.texture_mapping_lookup_table) else 0
    uv = -1 if uv in (-1, 0xFFFF) or uv >= 2 else int(uv)
    return int(ti), int(t.type), int(fdid), uv


def _preview_values(root, batch):
    rgba, weight, off, scale = [1.0, 1.0, 1.0, 1.0], 1.0, [0.0, 0.0], [1.0, 1.0]
    ci = batch.color_index
    if 0 <= ci < len(root.colors):
        c = _first(root.colors[ci].color)
        a = _first(root.colors[ci].alpha)
        if c is not None:
            rgba[:3] = _vec(c)
        if a is not None:
            rgba[3] = _f(a)
    wi = batch.texture_weight_combo_index
    lut = root.transparency_lookup_table
    if wi < len(lut) and 0 <= lut[wi] < len(root.texture_weights):
        w = _first(root.texture_weights[lut[wi]].weight)
        if w is not None:
            weight = _f(w)
    ti = batch.texture_transform_combo_index
    lut = root.texture_transforms_lookup_table
    if ti < len(lut) and 0 <= lut[ti] < len(root.texture_transforms):
        tt = root.texture_transforms[lut[ti]]
        tr, sc = _first(tt.translation), _first(getattr(tt, "scaling", None) or getattr(tt, "scale", None))
        if tr is not None:
            off = [tr.x, tr.y]
        if sc is not None:
            scale = [sc.x, sc.y]
    return tuple(rgba), weight, tuple(off), tuple(scale)


def build_materials(model):
    """{submesh index: material} with one material per distinct batch stack."""
    root, profile = model.root, model.profile
    stacks = {}
    for bi, b in enumerate(profile.batches):
        stacks.setdefault(b.skin_section_index, []).append((bi, b))
    shared, out = {}, {}
    for si, items in stacks.items():
        items.sort(key=lambda x: (x[1].material_layer, x[1].priority_plane))
        entries = []
        for bi, b in items:
            slots = [_texture_info(model, b, k) for k in range(2)]
            entries.append((bi, b, slots, _preview_values(root, b)))
        key = tuple((int(b.flags), b.shader_id, b.material_index, b.material_layer, b.priority_plane,
                     b.texture_count, tuple(s[2] for s in slots), tuple(s[3] for s in slots), vals)
                    for _, b, slots, vals in entries)
        mat = shared.get(key)
        if mat is None:
            mat = bpy.data.materials.new(f"{model.stem}_mat{items[0][0]}")
            for bi, b, slots, vals in entries:
                rm = root.materials[b.material_index] if b.material_index < len(root.materials) else None
                if rm is None:
                    class _M:
                        flags = blending_mode = 0
                    rm = _M
                m2_materials.set_batch(
                    mat.wow_m2_batches.add(), b, bi, rm, [s[0] for s in slots], [s[1] for s in slots],
                    [s[2] for s in slots], [s[3] for s in slots], vals)
            m2_materials.build_preview(mat)
            shared[key] = mat
        out[si] = mat
    return out


def _tail_for(heads, children, i, parents):
    head = mathutils.Vector(heads[i])
    kids = children.get(i, [])
    if len(kids) == 1:
        d = mathutils.Vector(heads[kids[0]]) - head
        if d.length >= MIN_BONE_LENGTH:
            return head + d
    p = parents[i]
    d = head - mathutils.Vector(heads[p]) if p >= 0 else mathutils.Vector((0, 0, 1))
    if d.length < 1e-4:
        d = mathutils.Vector((0, 0, 1))
    return head + d.normalized() * MIN_BONE_LENGTH


def build_armature(model, coll):
    root = model.root
    bones = root.bones
    arm_data = bpy.data.armatures.new(f"{model.stem}_rig")
    arm = bpy.data.objects.new(arm_data.name, arm_data)
    coll.objects.link(arm)
    heads = [_vec(b.pivot) for b in bones]
    parents = [int(b.parent_bone) if 0 <= b.parent_bone < len(bones) else -1 for b in bones]
    children = {}
    for i, p in enumerate(parents):
        if p >= 0:
            children.setdefault(p, []).append(i)
    names = [m2_names.bone_name(i, int(b.key_bone_id)) for i, b in enumerate(bones)]
    prev = bpy.context.view_layer.objects.active
    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.mode_set(mode="EDIT")
    eb = arm_data.edit_bones
    for i, name in enumerate(names):
        e = eb.new(name)
        e.head = heads[i]
        e.tail = _tail_for(heads, children, i, parents)
    for i, p in enumerate(parents):
        if p >= 0 and p != i:
            eb[names[i]].parent = eb[names[p]]
    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.context.view_layer.objects.active = prev
    for i, b in enumerate(bones):
        p = arm_data.bones[names[i]].wow_m2_bone
        p.index, p.key_bone_id, p.flags = i, int(b.key_bone_id), s32(int(b.flags))
        p.parent_bone, p.submesh_id, p.bone_name_crc = int(b.parent_bone), int(b.submesh_id), s32(int(b.bone_name_crc))
        p.pivot = heads[i]
    arm.wow_source = model.path
    model.armature, model.bone_names = arm, names
    if len(bones) <= 1 and not len(model.root.sequences):
        arm.hide_viewport = True
    return arm


def _submesh_mesh(model, name, sm, arrays):
    co, nrm, uv0, uv1, bone_idx, bone_w = arrays
    prof = model.profile
    vstart, vcount = sm.vertex_start, sm.vertex_count
    skin_verts = np.asarray(prof.vertices, dtype=np.int64)[vstart:vstart + vcount]
    idx = np.asarray(prof.indices, dtype=np.int64)[sm.index_start:sm.index_start + sm.index_count]
    tris = (idx - vstart)[: len(idx) // 3 * 3].reshape(-1, 3)
    if len(tris) and (tris.min() < 0 or tris.max() >= min(vcount, len(skin_verts))):
        _import_warnings.append(f"{name}: triangle indices leave the submesh vertex range")
        used, inverse = np.unique(idx[: len(idx) // 3 * 3], return_inverse=True)
        skin_verts = np.asarray(prof.vertices, dtype=np.int64)[used]
        tris = inverse.reshape(-1, 3)
        vstart, vcount = 0, len(used)
        local_skin = used
    else:
        local_skin = np.arange(vstart, vstart + vcount)
    nv, nt = len(skin_verts), len(tris)
    me = bpy.data.meshes.new(name)
    me.vertices.add(nv)
    me.vertices.foreach_set("co", co[skin_verts].ravel())
    me.loops.add(nt * 3)
    me.loops.foreach_set("vertex_index", tris.ravel().astype(np.int32))
    me.polygons.add(nt)
    me.polygons.foreach_set("loop_start", np.arange(0, nt * 3, 3, dtype=np.int32))
    me.update()
    for k, uv in enumerate((uv0, uv1)):
        if k == 1 and not np.any(uv1):
            break
        u = np.stack([uv[skin_verts, 0], 1.0 - uv[skin_verts, 1]], axis=1).astype(np.float32)
        me.uv_layers.new(name=f"uv_{k}").data.foreach_set("uv", u[tris.ravel()].ravel())
    for attr, vals in (("wow_vertex", skin_verts), ("wow_skin_vertex", local_skin)):
        a = me.attributes.new(attr, "INT", "POINT")
        a.data.foreach_set("value", vals.astype(np.int32))
    me.shade_smooth()
    if nt:
        me.validate()
        n = np.nan_to_num(nrm[skin_verts].astype(np.float32))
        ln = np.linalg.norm(n, axis=1)
        n[ln < 1e-6] = (0.0, 0.0, 1.0)
        me.normals_split_custom_set_from_vertices(n)
    return me, skin_verts


def _weights(ob, model, skin_verts, bone_idx, bone_w):
    names = model.bone_names
    bi, bw = bone_idx[skin_verts], bone_w[skin_verts]
    groups = {}
    for slot in range(4):
        mask = bw[:, slot] > 0
        if not mask.any():
            continue
        key = bi[mask, slot].astype(np.int64) * 256 + bw[mask, slot]
        verts = np.nonzero(mask)[0]
        order = np.argsort(key, kind="stable")
        key, verts = key[order], verts[order]
        cuts = np.nonzero(np.diff(key))[0] + 1
        for ks, vs in zip(np.split(key, cuts), np.split(verts, cuts)):
            b, w = int(ks[0]) >> 8, int(ks[0]) & 255
            if b >= len(names):
                continue
            vg = groups.get(b)
            if vg is None:
                vg = groups[b] = ob.vertex_groups.new(name=names[b])
            vg.add(vs.tolist(), w / 255.0, "ADD")


def build_submeshes(model, coll, mats, arrays):
    prof = model.profile
    objs = []
    for si, sm in enumerate(prof.submeshes):
        name = f"{model.stem}_{sm.skin_section_id}_{si}"
        me, skin_verts = _submesh_mesh(model, name, sm, arrays)
        if si in mats:
            me.materials.append(mats[si])
        ob = bpy.data.objects.new(name, me)
        coll.objects.link(ob)
        p = ob.wow_m2_submesh
        p.index, p.skin_section_id, p.level = si, int(sm.skin_section_id), int(sm.level)
        p.bone_combo_index, p.bone_count, p.bone_influences = sm.bone_combo_index, sm.bone_count, sm.bone_influences
        p.center_bone_index = sm.center_bone_index
        p.center_position, p.sort_center_position = _vec(sm.center_position), _vec(sm.sort_center_position)
        p.sort_radius = sm.sort_radius
        ob.wow_source = model.path
        if model.armature is not None:
            _weights(ob, model, skin_verts, arrays[4], arrays[5])
            ob.parent = model.armature
            mod = ob.modifiers.new("Armature", "ARMATURE")
            mod.object = model.armature
        objs.append(ob)
    return objs


def geoset_visible(skin_section_id, display_geosets):
    """Default geoset visibility: id 0 and every group's first entry; display rows pick the entry of their group."""
    group, entry = divmod(skin_section_id, 100)
    if group == 0:
        return True
    want = display_geosets.get(group, 1)
    return entry == want


def apply_geosets(model, objs):
    over = {}
    if model.display:
        for gi, val in model.display.geosets:
            over[gi + 1] = val
    for ob in objs:
        vis = geoset_visible(ob.wow_m2_submesh.skin_section_id, over)
        ob.hide_viewport = ob.hide_render = not vis


def group_geosets(top, objs):
    root = _link(top, "geosets")
    groups = {}
    for ob in objs:
        g = ob.wow_m2_submesh.skin_section_id // 100
        c = groups.get(g)
        if c is None:
            c = groups[g] = _link(root, f"group_{g}")
        for old in list(ob.users_collection):
            old.objects.unlink(ob)
        c.objects.link(ob)


def _bone_parent(model, ob, bone_index, world_pos):
    arm = model.armature
    if arm is None or not 0 <= bone_index < len(model.bone_names):
        ob.location = world_pos
        return
    bone = arm.data.bones[model.bone_names[bone_index]]
    ob.parent, ob.parent_type, ob.parent_bone = arm, "BONE", bone.name
    ob.matrix_parent_inverse = (arm.matrix_world @ bone.matrix_local
                                @ mathutils.Matrix.Translation((0, bone.length, 0))).inverted()
    ob.location = world_pos


def _empty(model, coll, name, display, size, kind, index, bone, pos):
    ob = bpy.data.objects.new(name, None)
    ob.empty_display_type, ob.empty_display_size = display, size
    coll.objects.link(ob)
    _bone_parent(model, ob, bone, pos)
    ob.wow_source = model.path
    p = ob.wow_m2_object
    p.kind, p.index, p.bone, p.position = kind, index, bone, pos
    return ob


def import_attachments(model, top):
    coll = _link(top, "attachments")
    for i, a in enumerate(model.root.attachments):
        ob = _empty(model, coll, m2_names.attachment(a.id), "PLAIN_AXES", 0.1, "attachment", i, int(a.bone), _vec(a.position))
        p = ob.wow_m2_object
        p.id, p.data = s32(int(a.id)), s32(int(a.unknown))
        p.animated = bool(_first(a.animate_attached) is not None)


def import_events(model, top):
    coll = _link(top, "events")
    for i, e in enumerate(model.root.events):
        code = m2_names.event(e.identifier)
        ob = _empty(model, coll, code, "SPHERE", 0.05, "event", i, int(e.bone), _vec(e.position))
        p = ob.wow_m2_object
        p.identifier, p.data = code, s32(int(e.data))
        p.animated = _first(e.enabled) is not None


def import_cameras(model, top):
    coll = _link(top, "cameras")
    for i, c in enumerate(model.root.cameras):
        data = bpy.data.cameras.new(f"camera_{i}")
        data.clip_start, data.clip_end = c.near_clip, c.far_clip
        fov = _first(c.fov)
        if fov is not None and 0 < fov.value < math.pi:
            data.sensor_fit = "VERTICAL"
            data.angle_y = fov.value
        ob = bpy.data.objects.new(f"camera_{i}", data)
        coll.objects.link(ob)
        ob.location = _vec(c.position_base)
        tgt = bpy.data.objects.new(f"camera_{i}_target", None)
        tgt.empty_display_type, tgt.empty_display_size = "PLAIN_AXES", 0.1
        tgt.location = _vec(c.target_position_base)
        coll.objects.link(tgt)
        con = ob.constraints.new("TRACK_TO")
        con.target, con.track_axis, con.up_axis = tgt, "TRACK_NEGATIVE_Z", "UP_Y"
        for o in (ob, tgt):
            o.wow_source = model.path
        p = ob.wow_m2_object
        p.kind, p.index, p.id = "camera", i, s32(int(c.type))
        p.position, p.target = _vec(c.position_base), _vec(c.target_position_base)


def import_lights(model, top):
    coll = _link(top, "lights")
    for i, l in enumerate(model.root.lights):
        kind = "SUN" if l.type == 0 else "POINT"
        data = bpy.data.lights.new(f"light_{i}", kind)
        col, inten = _first(l.diffuse_color), _first(l.diffuse_intensity)
        if col is not None:
            data.color = _vec(col)
        data.energy = 100.0 * (inten if inten is not None else 1.0)
        end = _first(l.attenuation_end)
        if kind != "SUN" and end:
            data.use_custom_distance, data.cutoff_distance = True, max(end, 0.01)
        ob = bpy.data.objects.new(f"light_{i}", data)
        coll.objects.link(ob)
        _bone_parent(model, ob, int(l.bone), _vec(l.position))
        ob.wow_source = model.path
        p = ob.wow_m2_object
        p.kind, p.index, p.id, p.bone, p.position = "light", i, s32(int(l.type)), int(l.bone), _vec(l.position)


def import_emitters(model, top):
    coll = _link(top, "emitters")
    for i, e in enumerate(model.root.particle_emitters):
        ob = _empty(model, coll, f"particle_{i}", "CUBE", 0.1, "particle", i, int(e.bone_id), _vec(e.position))
        ob.wow_m2_object.id = s32(e.particle_id)
        ob.wow_m2_object.flags = s32(int(e.flags))
    for i, e in enumerate(model.root.ribbon_emitters):
        ob = _empty(model, coll, f"ribbon_{i}", "CIRCLE", 0.1, "ribbon", i, int(e.bone_index), _vec(e.position))
        ob.wow_m2_object.id = s32(e.ribbon_id)


def import_collision(model, top):
    root = model.root
    p = top.wow_m2
    for attr, box, rad in (("bounding", root.bounding_box, root.bounding_sphere_radius),
                           ("collision", root.collision_box, root.collision_sphere_radius)):
        setattr(p, f"{attr}_min", _vec(box.min))
        setattr(p, f"{attr}_max", _vec(box.max))
        setattr(p, f"{attr}_radius", rad)
    cv = np.array([_vec(v) for v in root.collision_vertices], np.float32).reshape(-1, 3)
    tris = np.asarray(root.collision_triangles, dtype=np.int32)
    tris = tris[: len(tris) // 3 * 3].reshape(-1, 3)
    if not len(cv) or not len(tris) or tris.max() >= len(cv):
        return
    me = bpy.data.meshes.new(f"{model.stem}_collision")
    me.from_pydata(cv.tolist(), [], tris.tolist())
    ob = bpy.data.objects.new(me.name, me)
    ob.display_type = "WIRE"
    ob.hide_viewport = ob.hide_render = True
    ob.wow_source = model.path
    top.objects.link(ob)


def fill_root_props(model, top):
    r, p = model.root, top.wow_m2
    p.path, p.fdid, p.name = model.path, s32(model.fdid), r.name
    p.format_version, p.global_flags = r.format_version, s32(int(r.global_flags))
    p.skin_index, p.skin_count = model.skin_index, len(model.m.skins)
    p.skin_fdids = (list(getattr(model.m.chunks, "skin_fdids", [])) + [0] * 4)[:4]
    p.display_id = model.display.id if model.display else -1
    p.sequence_count = len(r.sequences)
    for i, t in enumerate(r.textures):
        e = p.textures.add()
        e.type, e.flags = int(t.type), int(t.flags)
        e.fdid = s32(model.tex_fdids[i]) if i < len(model.tex_fdids) else 0
    for m in r.materials:
        e = p.materials.add()
        e.flags, e.blending_mode = int(m.flags), int(m.blending_mode)


def load(client_path, options=None, parent=None):
    """Import an M2 (client path); returns the root collection."""
    opts = dict(DEFAULTS)
    opts.update(options or {})
    s = client.session()
    index.load(s.content, client.prefs().cache_dir)
    path = client_path.replace("\\", "/").lower()
    m = read(path)
    model = Model(path, m, opts)
    top = _link(parent or bpy.context.scene.collection, model.base)
    model.replaceable = resolve_replaceable(model)
    fill_root_props(model, top)
    arrays = _gather(model.root)
    mats = build_materials(model)
    if opts["armature"] and not opts["mesh_only"] and len(model.root.bones):
        build_armature(model, top)
    objs = build_submeshes(model, top, mats, arrays)
    apply_geosets(model, objs)
    group_geosets(top, objs)
    if not opts["mesh_only"]:
        for flag, fn in (("attachments", import_attachments), ("events", import_events), ("cameras", import_cameras),
                         ("lights", import_lights), ("emitters", import_emitters)):
            if opts[flag]:
                fn(model, top)
        if opts["collision"]:
            import_collision(model, top)
    return top


def root_collection(ob):
    """The M2 root collection that owns an object (found through wow_source)."""
    path = ob.wow_source if ob is not None else ""
    for c in bpy.data.collections:
        if c.wow_m2.path and c.wow_m2.path == path:
            return c
    return None


def set_display(top, display_id):
    """Swap replaceable textures and geoset visibility to creature display `display_id` (-1 = none)."""
    p = top.wow_m2
    disp = displays.display(p.fdid, display_id) if display_id >= 0 else None
    p.display_id = disp.id if disp else -1
    types = {t: disp.textures[slot] for t, slot in SKIN_TYPES.items() if disp and disp.textures[slot]}
    objs = [o for o in top.all_objects if o.type == "MESH" and o.wow_m2_submesh.index >= 0]
    seen = set()
    for ob in objs:
        for mat in ob.data.materials:
            if mat is not None and mat not in seen:
                seen.add(mat)
                m2_materials.set_replaceable(mat, types)
    over = {gi + 1: val for gi, val in disp.geosets} if disp else {}
    for ob in objs:
        ob.hide_viewport = ob.hide_render = not geoset_visible(ob.wow_m2_submesh.skin_section_id, over)
    return disp


def take_warnings():
    out = list(_import_warnings)
    _import_warnings.clear()
    return out


class WOW_OT_m2_set_display(bpy.types.Operator):
    bl_idname = "wow.m2_set_display"
    bl_label = "Set creature display"

    display_id: bpy.props.IntProperty(default=-1)

    @classmethod
    def poll(cls, context):
        return root_collection(context.active_object) is not None

    def execute(self, context):
        try:
            set_display(root_collection(context.active_object), self.display_id)
        except Exception as e:  # noqa: BLE001
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        return {"FINISHED"}


classes = (WOW_OT_m2_set_display,)
