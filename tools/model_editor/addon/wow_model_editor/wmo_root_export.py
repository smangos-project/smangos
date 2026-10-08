"""Writes edits to root-level WMO objects (doodads, lights, fogs, portals, liquids) back into the parsed WMO."""
import numpy as np

from . import fdids
from .props import u32

TOL = 1e-5
LIGHT_TYPES = {"POINT": 0, "SPOT": 1, "SUN": 2}


def _close(a, b, tol=TOL):
    return all(abs(float(x) - float(y)) <= tol for x, y in zip(a, b))


def _vec(v):
    return (v.x, v.y, v.z)


def _set_vec(v, vals):
    v.x, v.y, v.z = (float(x) for x in vals)


def _set_rgba(c, vals):
    c.r, c.g, c.b, c.a = (int(x) for x in vals)


def _same_rotation(src, rot):
    """True if the source quaternion, normalised as Blender does, equals `rot` (either sign)."""
    sq = np.array(src, np.float64)
    if not np.linalg.norm(sq):
        return True  # zero quaternion: Blender substitutes some rotation, so it can't be edited meaningfully
    sq = sq / np.linalg.norm(sq)
    rq = np.array((rot.x, rot.y, rot.z, rot.w))
    return _close(sq, rq, 1e-5) or _close(-sq, rq, 1e-5)


def _world(ob):
    """Objects are unparented; matrix_world is stale for hidden collections."""
    return ob.matrix_basis.decompose()


def update_doodads(w, root, report):
    """Edits existing doodad definitions and inserts new objects; returns True if anything changed."""
    r = w.root
    objs = [o for o in root.all_objects if o.type == "EMPTY" and (o.wow_wmo_doodad.index >= 0 or o.wow_wmo_doodad.fdid)]
    if not objs:
        return False
    names = list(r.doodad_fdids)
    n_names = len(names)
    n_src = len(r.doodad_defs)
    seen = set()
    changed = False
    new = []
    for ob in sorted(objs, key=lambda o: o.name):
        p = ob.wow_wmo_doodad
        fdid = u32(p.fdid)
        if p.path and not fdid:
            fdid = fdids.allocate(p.path)
        if fdid and fdid not in names:
            names.append(fdid)
        if p.name_index < len(names) and (not fdid or names[p.name_index] == fdid):
            name_index = p.name_index
        else:
            name_index = names.index(fdid)
        loc, rot, scale = _world(ob)
        want = {"pos": tuple(loc), "q": (rot.x, rot.y, rot.z, rot.w), "scale": float(sum(scale) / 3),
                "color": tuple(p.color), "flags": p.flags, "name_index": name_index}
        if p.index < 0 or p.index >= n_src:
            new.append((ob, want))
            continue
        seen.add(p.index)
        d = r.doodad_defs[p.index]
        q = d.orientation
        if not (_close(_vec(d.position), want["pos"], 1e-4) and _same_rotation((q.x, q.y, q.z, q.w), rot)
                and abs(d.scale - want["scale"]) < 1e-5):
            _set_vec(d.position, want["pos"])
            q.x, q.y, q.z, q.w = want["q"]
            d.scale = want["scale"]
            changed = True
        c = d.color
        if (c.r, c.g, c.b, c.a) != want["color"]:
            _set_rgba(c, want["color"])
            changed = True
        nf = (want["flags"] & 0xFF) << 24 | want["name_index"]
        if int(d.name_and_flags) != nf:
            d.name_and_flags = nf
            changed = True
    missing = sorted(set(range(n_src)) - seen)
    drop = set(missing) if any(c.name.startswith("doodads") for c in root.children) else set()
    if missing and not drop:
        report.add("WARNING", "doodads", f"{len(missing)} doodad objects missing, definitions kept")
    if len(names) != n_names:
        r.doodad_fdids.resize(len(names))
        for i, v in enumerate(names):
            r.doodad_fdids[i] = v
        r.header.n_doodad_names = int(r.header.n_doodad_names) + len(names) - n_names
        changed = True
    if new or drop:
        rebuild_doodads(w, new, drop, report)
        changed = True
    return changed


def _def_tuple(d):
    q, c = d.orientation, d.color
    return (int(d.name_and_flags), int(d.name_index), _vec(d.position), (q.x, q.y, q.z, q.w), float(d.scale),
            (c.r, c.g, c.b, c.a))


def _write_def(d, t):
    d.name_and_flags = t[0]
    _set_vec(d.position, t[2])
    q = d.orientation
    q.x, q.y, q.z, q.w = t[3]
    d.scale = t[4]
    _set_rgba(d.color, t[5])


def rebuild_doodads(w, new, drop, report):
    """Rewrite MODD ordered by set: drop `drop` (old indices), add `new` at the end of their set; remaps set ranges,
    group doodad references and compacts unused names."""
    r = w.root
    n = len(r.doodad_defs)
    sets = [(s.name, s.start_index, s.count) for s in r.doodad_sets]
    items = [_def_tuple(r.doodad_defs[i]) for i in range(n)]
    set_of_old = {}
    for si, (_, start, count) in enumerate(sets):
        for i in range(start, start + count):
            set_of_old[i] = si
    by_set = {}
    for ob, want in new:
        si = min(max(ob.wow_wmo_doodad.doodad_set, 0), len(sets) - 1) if sets else -1
        by_set.setdefault(si, []).append((ob, want))
    order = []
    for old in range(n):
        if old not in drop:
            order.append(("old", old, None))
        si = set_of_old.get(old)
        if si in by_set and old == sets[si][1] + sets[si][2] - 1:
            order += [("new", ob, want) for ob, want in by_set.pop(si)]
    for lst in by_set.values():
        order += [("new", ob, want) for ob, want in lst]
    remap, out, out_set, new_ids = {}, [], [], {}
    for kind, a, b in order:
        if kind == "old":
            remap[a] = len(out)
            out.append(items[a])
            out_set.append(set_of_old.get(a))
        else:
            new_ids[a.name] = len(out)
            out.append(((b["flags"] & 0xFF) << 24 | b["name_index"], b["name_index"], b["pos"], b["q"], b["scale"], b["color"]))
            out_set.append(min(max(a.wow_wmo_doodad.doodad_set, 0), len(sets) - 1) if sets else None)
    used = sorted({t[0] & 0xFFFFFF for t in out})
    names = list(r.doodad_fdids)
    if drop and len(used) < len(names):
        name_map = {o: j for j, o in enumerate(used)}
        out = [((t[0] & ~0xFFFFFF) | name_map[t[0] & 0xFFFFFF], *t[1:]) for t in out]
        r.doodad_fdids.resize(len(used))
        for j, o in enumerate(used):
            r.doodad_fdids[j] = names[o]
        r.header.n_doodad_names = len(used)
    r.doodad_defs.resize(len(out))
    for i, t in enumerate(out):
        _write_def(r.doodad_defs[i], t)
    ends = 0
    for si, s in enumerate(r.doodad_sets):
        members = [i for i, x in enumerate(out_set) if x == si]
        s.start_index, s.count = (members[0], len(members)) if members else (ends, 0)
        ends = s.start_index + s.count
    r.header.n_doodad_defs = len(out)
    boxes = [g.body.header.bounding_box for g in w.groups]
    for g in w.groups:
        old_refs = [int(x) for x in g.body.doodad_refs]
        refs = [remap.get(x, x) for x in old_refs if x in remap or x >= n]
        if refs != old_refs:
            g.body.doodad_refs.resize(len(refs))
            for i, v in enumerate(refs):
                g.body.doodad_refs[i] = v
            if not refs:
                g.body.header.flags = type(g.body.header.flags)(u32(int(g.body.header.flags)) & ~0x800)
    for kind, a, b in order:
        if kind != "new":
            continue
        idx = new_ids[a.name]
        hit = False
        for g, box in zip(w.groups, boxes):
            p = b["pos"]
            if all(lo <= v <= hi for v, lo, hi in zip(p, _vec(box.min), _vec(box.max))):
                g.body.doodad_refs.append(idx)
                g.body.header.flags = type(g.body.header.flags)(u32(int(g.body.header.flags)) | 0x800)
                hit = True
        if not hit:
            report.add("WARNING", a.name, "doodad is outside every group box and is not referenced by any group")
        a.wow_wmo_doodad.index = -1


def update_lights(w, root, report):
    r = w.root
    changed = False
    seen = set()
    new = []
    for ob in sorted((o for o in root.all_objects if o.type == "LIGHT" and o.wow_wmo_light.index is not None), key=lambda o: o.name):
        p = ob.wow_wmo_light
        data = ob.data
        loc, rot, _ = _world(ob)
        color = tuple(int(round(c * 255)) for c in data.color) + (p.color[3],)
        intensity = data.energy / 100.0
        atten_end = data.cutoff_distance if data.use_custom_distance else p.atten_end
        if abs(atten_end - max(p.atten_end, 0.01)) <= 1e-4:
            atten_end = p.atten_end
        if p.index < 0 or p.index >= len(r.lights):
            new.append((ob, loc, rot, color, intensity, atten_end))
            continue
        seen.add(p.index)
        l = r.lights[p.index]
        if not _close(_vec(l.position), tuple(loc), 1e-4):
            _set_vec(l.position, loc)
            changed = True
        q = l.rotation
        if not _same_rotation((q.x, q.y, q.z, q.w), rot):
            q.x, q.y, q.z, q.w = rot.x, rot.y, rot.z, rot.w
            changed = True
        if (l.color.r, l.color.g, l.color.b) != color[:3]:
            _set_rgba(l.color, (*color[:3], l.color.a))
            changed = True
        if abs(l.intensity - intensity) > 1e-4:
            l.intensity = intensity
            changed = True
        if abs(l.atten_end - atten_end) > 1e-4:
            l.atten_end = atten_end
            changed = True
        if l.atten_start != p.atten_start or l.use_atten != p.use_atten or l.type != p.type:
            l.atten_start, l.use_atten, l.type = p.atten_start, p.use_atten, p.type
            changed = True
    if len(seen) < len(r.lights):
        report.add("WARNING", "lights", f"{len(r.lights) - len(seen)} light objects missing, lights kept")
    for ob, loc, rot, color, intensity, atten_end in new:
        l = r.lights.new()
        p = ob.wow_wmo_light
        l.type = LIGHT_TYPES.get(ob.data.type, 0)
        l.use_atten = 1
        _set_vec(l.position, loc)
        l.rotation.x, l.rotation.y, l.rotation.z, l.rotation.w = rot.x, rot.y, rot.z, rot.w
        _set_rgba(l.color, color[:3] + (255,))
        l.intensity, l.atten_start, l.atten_end = intensity, 0.0, atten_end
        r.header.n_lights = int(r.header.n_lights) + 1
        changed = True
    return changed


def update_fogs(w, root, report):
    r = w.root
    changed = False
    seen = set()
    for ob in sorted((o for o in root.all_objects if o.type == "EMPTY" and o.name.startswith("fog_")), key=lambda o: o.name):
        p = ob.wow_wmo_fog
        loc = tuple(ob.matrix_basis.translation)
        if p.index < 0 or p.index >= len(r.fogs):
            f = r.fogs.new()
            changed = True
        else:
            seen.add(p.index)
            f = r.fogs[p.index]
        want = (p.flags, p.larger_radius, p.smaller_radius, p.fog_end, p.fog_start_scalar, tuple(p.fog_color),
                p.uw_end, p.uw_start_scalar, tuple(p.uw_color))
        cur = (int(f.flags), f.larger_radius, f.smaller_radius, f.fog.end, f.fog.start_scalar,
               (f.fog.color.r, f.fog.color.g, f.fog.color.b, f.fog.color.a), f.under_water_fog.end,
               f.under_water_fog.start_scalar, (f.under_water_fog.color.r, f.under_water_fog.color.g,
                                               f.under_water_fog.color.b, f.under_water_fog.color.a))
        same = _close(_vec(f.position), loc, 1e-4) and all(
            (a == b) if isinstance(a, tuple) or isinstance(a, int) else abs(a - b) < 1e-5 for a, b in zip(cur, want))
        if not same:
            _set_vec(f.position, loc)
            f.flags = type(f.flags)(u32(p.flags)) if not isinstance(f.flags, int) else u32(p.flags)
            f.larger_radius, f.smaller_radius = p.larger_radius, p.smaller_radius
            f.fog.end, f.fog.start_scalar = p.fog_end, p.fog_start_scalar
            _set_rgba(f.fog.color, p.fog_color)
            f.under_water_fog.end, f.under_water_fog.start_scalar = p.uw_end, p.uw_start_scalar
            _set_rgba(f.under_water_fog.color, p.uw_color)
            changed = True
    if len(seen) < len(r.fogs):
        report.add("WARNING", "fogs", "fog objects missing, fogs kept")
    return changed


def _plane(pts):
    """Newell normal and plane distance of a polygon."""
    n = np.zeros(3)
    for i in range(len(pts)):
        a, b = pts[i], pts[(i + 1) % len(pts)]
        n += np.cross(a, b)
    ln = np.linalg.norm(n)
    n = n / ln if ln else np.array([0.0, 0.0, 1.0])
    return n, float(-np.dot(n, np.mean(pts, axis=0)))


def update_portals(w, root, report):
    r = w.root
    changed = False
    for ob in sorted((o for o in root.all_objects if o.type == "MESH" and o.name.startswith("portal_")), key=lambda o: o.name):
        i = ob.wow_wmo_portal.index
        if i >= len(r.portals):
            report.add("WARNING", ob.name, "new portals are not supported, ignored")
            continue
        pt = r.portals[i]
        me = ob.data
        co = np.array([tuple(ob.matrix_world @ v.co) for v in me.vertices], np.float32)
        if len(co) != pt.count:
            report.add("ERROR", ob.name, f"portal needs {pt.count} vertices, has {len(co)}")
            continue
        src = np.array([_vec(r.portal_vertices[pt.start_vertex + k]) for k in range(pt.count)], np.float32)
        if np.abs(co - src).max(initial=0) > 1e-4:
            for k in range(pt.count):
                _set_vec(r.portal_vertices[pt.start_vertex + k], co[k])
            n, dist = _plane(co.astype(np.float64))
            _set_vec(pt.plane.normal, n)
            pt.plane.distance = dist
            changed = True
    return changed


def update_liquid(g, ob, report):
    L = g.body.liquid
    if L.empty:
        report.add("WARNING", ob.name, "group has no liquid, ignored")
        return False
    me = ob.data
    nv = L.verts_dim.x * L.verts_dim.y
    if len(me.vertices) != nv:
        report.add("ERROR", ob.name, f"liquid needs {nv} vertices, has {len(me.vertices)}")
        return False
    changed = False
    z = [(ob.matrix_world @ v.co).z for v in me.vertices]
    for k, v in enumerate(L.vertices):
        if abs(v.height - z[k]) > 1e-4:
            v.height = z[k]
            changed = True
    if "wow_tile_flags" in me.attributes:
        flags = np.zeros(len(me.polygons), np.int32)
        me.attributes["wow_tile_flags"].data.foreach_get("value", flags)
        hide = np.zeros(len(me.polygons), bool)
        me.polygons.foreach_get("hide", hide)
        for k in range(min(len(flags), len(L.tiles))):
            f = int(flags[k])
            f = f | 0x08 if hide[k] else f & ~0x08
            if int(L.tiles[k].flags) != f:
                L.tiles[k].flags = f
                changed = True
    mid = ob.wow_wmo_liquid.material_id
    if L.material_id != mid:
        L.material_id = mid
        changed = True
    return changed


def update_liquids(w, root, report):
    changed = set()
    for ob in root.all_objects:
        if ob.type == "MESH" and ob.name.startswith("liquid_") and ob.wow_wmo_liquid.verts_dim[0]:
            gi = ob.wow_wmo_liquid.group_index
            if gi < len(w.groups) and update_liquid(w.groups[gi], ob, report):
                changed.add(gi)
    return changed


def update_root_objects(w, root, report):
    """Returns the set of group indices whose liquids changed; root edits are applied in place."""
    for fn in (update_doodads, update_lights, update_fogs, update_portals):
        if fn(w, root, report):
            report.add("INFO", fn.__name__.replace("update_", ""), "updated")
    return update_liquids(w, root, report)
