"""Writes edits to root-level WMO objects (doodads, lights, fogs, portals, liquids) back into the parsed WMO."""
import copy

import numpy as np

from . import fdids, wmo_import
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
    objs = [o for o in root.all_objects if o.type == "EMPTY" and (o.wow_wmo_doodad.index >= 0 or o.wow_wmo_doodad.fdid or o.wow_wmo_doodad.path)]
    if not objs:
        return False
    names = list(r.doodad_fdids)
    n_names = len(names)
    n_src = len(r.doodad_defs)
    seen = set()
    changed = False
    new, moved = [], {}
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
        if not _close(_vec(d.position), want["pos"], 1e-4):
            moved[p.index] = want["pos"]
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
    layout = _set_layout(root, r)
    if layout and not layout[3] and not (new or drop):
        layout = None
    if new or drop or layout:
        rebuild_doodads(w, new, drop, report, layout, moved)
        changed = True
    else:
        rereference_doodads(w, moved)
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


def _set_layout(root, r):
    """Final doodad sets from the child collections of `doodads`: (names, {old doodad: set}, {object name: set}, changed)."""
    dc = next((c for c in root.children if c.name.startswith("doodads")), None)
    if dc is None:
        return None
    old_sets = [(s.name, s.start_index, s.count) for s in r.doodad_sets]
    colls = [c for c in dc.children if not c.name.startswith("unassigned")]
    final_of_old_set, names, set_of_obj = {}, [], {}
    for fi, c in enumerate(colls):
        old = c.get("wow_set")
        if old is not None and old < len(old_sets) and old not in final_of_old_set:
            final_of_old_set[old] = fi
            names.append(old_sets[old][0] if c.name == (old_sets[old][0] or f"set_{old}") else c.name)
        else:
            names.append(c.name)
        for ob in c.all_objects:
            set_of_obj[ob.name] = fi
    of_old = {}
    for si, (_, start, count) in enumerate(old_sets):
        for i in range(start, start + count):
            of_old.setdefault(i, final_of_old_set.get(si))
    changed = names != [x[0] for x in old_sets]
    for ob in root.all_objects:
        d = ob.wow_wmo_doodad
        if ob.type == "EMPTY" and 0 <= d.index < len(r.doodad_defs) and of_old.get(d.index) != set_of_obj.get(ob.name):
            of_old[d.index] = set_of_obj.get(ob.name)
            changed = True
    return names, of_old, set_of_obj, changed


def rebuild_doodads(w, new, drop, report, layout=None, moved=None):
    """Rewrite MODD ordered by set: drop `drop` (old indices), add `new` at the end of their set; remaps set ranges,
    group doodad references and compacts unused names. `layout` (from _set_layout) replaces the set structure."""
    r = w.root
    n = len(r.doodad_defs)
    sets = [(s.name, s.start_index, s.count) for s in r.doodad_sets]
    items = [_def_tuple(r.doodad_defs[i]) for i in range(n)]
    if layout:
        set_names, set_of_old, set_of_new, _ = layout
    else:
        set_names = [x[0] for x in sets]
        set_of_old = {}
        for si, (_, start, count) in enumerate(sets):
            for i in range(start, start + count):
                set_of_old[i] = si
        set_of_new = {ob.name: min(max(ob.wow_wmo_doodad.doodad_set, 0), len(sets) - 1) for ob, _ in new} if sets else {}
    members, loose = [[] for _ in set_names], []
    for old in range(n):
        if old not in drop:
            (members[set_of_old[old]] if set_of_old.get(old) is not None else loose).append(("old", old, None))
    for ob, want in new:
        si = set_of_new.get(ob.name)
        (members[si] if si is not None else loose).append(("new", ob, want))
    order = [x for m in members for x in m] + loose
    remap, out, out_set, new_ids = {}, [], [], {}
    for kind, a, b in order:
        if kind == "old":
            remap[a] = len(out)
            out.append(items[a])
            out_set.append(set_of_old.get(a))
        else:
            new_ids[a.name] = len(out)
            out.append(((b["flags"] & 0xFF) << 24 | b["name_index"], b["name_index"], b["pos"], b["q"], b["scale"], b["color"]))
            out_set.append(set_of_new.get(a.name))
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
    if len(r.doodad_sets) != len(set_names):
        r.doodad_sets.resize(len(set_names))
    if layout:
        report.set_names = list(set_names)
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
        hits = groups_containing(w, b["pos"])
        set_group_refs(w, "doodad_refs", 0x800, idx, hits)
        if not hits:
            report.add("WARNING", a.name, "doodad is outside every group box and is not referenced by any group")
        a.wow_wmo_doodad.index = -1
    rereference_doodads(w, {remap[o]: pos for o, pos in (moved or {}).items() if o in remap})
    return remap


def rereference_doodads(w, moved):
    """Recompute the group references of doodads (`{index: position}`) that moved."""
    for idx, pos in moved.items():
        set_group_refs(w, "doodad_refs", 0x800, idx, groups_containing(w, pos))


def groups_containing(w, pos, radius=0.0):
    """Indices of the groups whose bounding box, grown by `radius`, contains `pos`."""
    return [gi for gi, g in enumerate(w.groups)
            if all(lo - radius <= v <= hi + radius
                   for v, lo, hi in zip(pos, _vec(g.body.header.bounding_box.min), _vec(g.body.header.bounding_box.max)))]


def set_group_refs(w, attr, flag, idx, hits):
    """Make exactly the groups `hits` reference `idx` in their `attr` vector (sorted); keeps the group `flag` in sync."""
    for gi, g in enumerate(w.groups):
        vec = getattr(g.body, attr)
        old = [int(x) for x in vec]
        refs = sorted(set(old) - {idx} | ({idx} if gi in hits else set()))
        if refs != old:
            _set_vec_ints(vec, refs)
        h = g.body.header
        has = bool(refs)
        if has != bool(int(h.flags) & flag) and (refs != old or has):
            h.flags = type(h.flags)(u32(int(h.flags)) & ~flag | (flag if has else 0))


def reference_new_groups(w, new_groups):
    """Groups added by the export start without references: give them the doodads and lights inside their box."""
    r = w.root
    for gi in new_groups:
        g = w.groups[gi]
        for attr, flag, items in (("doodad_refs", 0x800, [(i, _vec(d.position), 0.0) for i, d in enumerate(r.doodad_defs)]),
                                  ("light_refs", 0x200, [(i, _vec(l.position), float(l.atten_end)) for i, l in enumerate(r.lights)])):
            refs = [i for i, pos, rad in items if gi in groups_containing(w, pos, rad)]
            if refs:
                _set_vec_ints(getattr(g.body, attr), refs)
                h = g.body.header
                h.flags = type(h.flags)(u32(int(h.flags)) | flag)


def compact(vec, keep):
    """Keep only the records at old indices `keep` (ascending); returns {old: new}."""
    recs = [copy.copy(vec[i]) for i in keep]
    vec.clear()
    for x in recs:
        vec.append(x)
    return {old: new for new, old in enumerate(keep)}


HAS_LOD = 0x10
GROUP_LOD = 0x400


def lod_tables(r):
    """GFID as `tables[lod][group]`; a root without the Lod flag has the base table only."""
    ids = [int(x) for x in r.group_fdids]
    n = len(r.group_infos)
    if not int(r.header.flags) & HAS_LOD or n == 0:
        return [ids]
    ids += [0] * (-len(ids) % n)
    return [ids[k:k + n] for k in range(0, len(ids), n)]


def set_lod_tables(r, tables):
    _set_vec_ints(r.group_fdids, [v for t in tables for v in t])


def append_group_info(r, base_fdid):
    """Registers a new group in GFID (no LOD files) and MGI2; call before adding its group info."""
    tables = lod_tables(r)
    tables[0].append(base_fdid)
    for t in tables[1:]:
        t.append(0)
    set_lod_tables(r, tables)
    if len(r.group_infos2):
        r.group_infos2.new()


def clear_group_lods(r, groups, report=None):
    """Drops the LOD files of `groups`: the base group then draws at every distance."""
    tables = lod_tables(r)
    cleared = []
    for gi in sorted(groups):
        if any(t[gi] for t in tables[1:]) or int(r.group_infos[gi].flags) & GROUP_LOD:
            cleared.append(gi)
            for t in tables[1:]:
                t[gi] = 0
            r.group_infos[gi].flags = type(r.group_infos[gi].flags)(int(r.group_infos[gi].flags) & ~GROUP_LOD)
            if len(r.group_infos2):
                r.group_infos2[gi].lod_index = int(r.group_infos2[gi].lod_index) & ~0xFF
    if cleared and report:
        report.add("INFO", "lods", f"LOD files of groups {cleared} dropped because the groups changed")
    set_lod_tables(r, tables)
    prune_lod_header(r)


def prune_lod_header(r):
    """Removes the Lod flag and empty LOD tables once no group has a LOD file."""
    tables = lod_tables(r)
    if int(r.header.flags) & HAS_LOD and not any(any(t) for t in tables[1:]):
        r.header.flags = type(r.header.flags)(int(r.header.flags) & ~HAS_LOD)
        r.header.num_lod = 0
        set_lod_tables(r, tables[:1])
        for info in r.group_infos:
            info.flags = type(info.flags)(int(info.flags) & ~GROUP_LOD)


def _set_vec_ints(vec, vals):
    vec.resize(len(vals))
    for i, v in enumerate(vals):
        vec[i] = v


def update_lights(w, root, report):
    r = w.root
    changed = False
    seen = set()
    new, moved = [], []
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
            moved.append(p.index)
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
            if p.index not in moved:
                moved.append(p.index)
            changed = True
        wtype = p.type if wmo_import.LIGHT_KINDS.get(p.type) == data.type else LIGHT_TYPES.get(data.type, 0)
        if l.atten_start != p.atten_start or l.use_atten != p.use_atten or l.type != wtype:
            l.atten_start, l.use_atten, l.type = p.atten_start, p.use_atten, wtype
            changed = True
    gone = set(range(len(r.lights))) - seen
    if gone and any(c.name.startswith("lights") for c in root.children):
        remap = compact(r.lights, [i for i in range(len(r.lights)) if i in seen])
        moved = [remap[i] for i in moved if i in remap]
        for g in w.groups:
            old = [int(x) for x in g.body.light_refs]
            kept = [remap[x] for x in old if x in remap]
            if kept != old:
                _set_vec_ints(g.body.light_refs, kept)
        r.header.n_lights = len(r.lights)
        changed = True
    elif gone:
        report.add("WARNING", "lights", f"{len(gone)} light objects missing, lights kept")
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
        moved.append(len(r.lights) - 1)
        changed = True
    for idx in moved:
        l = r.lights[idx]
        set_group_refs(w, "light_refs", 0x200, idx, groups_containing(w, _vec(l.position), float(l.atten_end)))
    return changed


def update_fogs(w, root, report):
    r = w.root
    changed = False
    seen = set()
    n_src = len(r.fogs)
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
    gone = set(range(n_src)) - seen
    if gone:
        remap = compact(r.fogs, [i for i in range(len(r.fogs)) if i in seen or i >= n_src])
        for g in w.groups:
            ids = [int(x) for x in g.body.header.fog_ids]
            new = [remap.get(x, 0) for x in ids]
            if new != ids:
                for k, v in enumerate(new):
                    g.body.header.fog_ids[k] = v
        changed = True
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


def _portal_polygon(ob):
    me = ob.data
    order = list(me.polygons[0].vertices) if len(me.polygons) else range(len(me.vertices))
    return np.array([tuple(ob.matrix_world @ me.vertices[i].co) for i in order], np.float32)


def _group_box(g):
    b = g.body.header.bounding_box
    return np.array(_vec(b.min)), np.array(_vec(b.max))


def _auto_refs(w, co):
    """The two smallest groups whose box the polygon touches, with the side of the plane each lies on."""
    n, d = _plane(co.astype(np.float64))
    lo, hi = co.min(axis=0) - 0.05, co.max(axis=0) + 0.05
    hits = []
    for gi, g in enumerate(w.groups):
        glo, ghi = _group_box(g)
        if np.all(glo <= hi) and np.all(ghi >= lo):
            hits.append((float(np.prod(ghi - glo)), gi, 1 if float(np.dot(n, (glo + ghi) / 2) + d) >= 0 else -1))
    hits.sort()
    return [(gi, side) for _, gi, side in hits[:2]]


def rebuild_portals(w, objs, report):
    """Rewrites MOPT/MOPV/MOPR and each group's portal slice for the given portal objects (old indices keep their refs)."""
    r = w.root
    old_refs = []
    for g in w.groups:
        h = g.body.header
        old_refs.append([(int(x.portal_index), int(x.group_index), int(x.side)) for x in r.portal_refs[h.portal_start:h.portal_start + h.portal_count]])
    index_of = {}
    polys, per_group = [], [[] for _ in w.groups]
    for ob in objs:
        co = _portal_polygon(ob)
        if len(co) < 3:
            report.add("ERROR", ob.name, f"portal needs at least 3 vertices, has {len(co)}")
            continue
        i = ob.wow_wmo_portal.index
        index_of[ob] = len(polys)
        polys.append(co)
    new_index = {}
    for ob, k in index_of.items():
        i = ob.wow_wmo_portal.index
        if 0 <= i < len(r.portals):
            new_index[i] = k
    for gi, refs in enumerate(old_refs):
        for pi, other, side in refs:
            if pi in new_index:
                per_group[gi].append((new_index[pi], other, side))
    for ob, k in index_of.items():
        i = ob.wow_wmo_portal.index
        if 0 <= i < len(r.portals):
            continue
        refs = [(e.group_index, e.side) for e in ob.wow_wmo_portal.refs if e.group_index < len(w.groups)]
        if len(refs) < 2:
            refs = _auto_refs(w, polys[k])
        if len(refs) < 2:
            report.add("WARNING", ob.name, "portal touches fewer than two groups")
        for gi, side in refs:
            for oi, _ in refs:
                if oi != gi:
                    per_group[gi].append((k, oi, side))
    r.portals.clear()
    r.portal_vertices.clear()
    r.portal_refs.clear()
    for co in polys:
        pt = r.portals.new()
        pt.start_vertex, pt.count = len(r.portal_vertices), len(co)
        for c in co:
            _set_vec(r.portal_vertices.new(), c)
        n, dist = _plane(co.astype(np.float64))
        _set_vec(pt.plane.normal, n)
        pt.plane.distance = dist
    for gi, refs in enumerate(per_group):
        h = w.groups[gi].body.header
        h.portal_start, h.portal_count = len(r.portal_refs), len(refs)
        for pi, other, side in refs:
            e = r.portal_refs.new()
            e.portal_index, e.group_index, e.side = pi, other, side


def update_portals(w, root, report):
    r = w.root
    changed = False
    objs = sorted((o for o in root.all_objects if o.type == "MESH" and o.name.startswith("portal_")), key=lambda o: o.name)
    seen = {o.wow_wmo_portal.index for o in objs}
    n_src = len(r.portals)
    has_coll = any(c.name.startswith("portals") for c in root.children)
    gone = set(range(n_src)) - seen
    new = [o for o in objs if not 0 <= o.wow_wmo_portal.index < n_src]
    if new or (gone and has_coll):
        order = sorted((o for o in objs if 0 <= o.wow_wmo_portal.index < n_src), key=lambda o: o.wow_wmo_portal.index) + new
        rebuild_portals(w, order, report)
        return True
    if gone:
        report.add("WARNING", "portals", f"{len(gone)} portal objects missing, portals kept")
    for ob in objs:
        pt = r.portals[ob.wow_wmo_portal.index]
        co = _portal_polygon(ob)
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


def update_convex_planes(w, root, report):
    r = w.root
    ob = next((o for o in root.all_objects if o.type == "MESH" and o.name.startswith("convex_planes")), None)
    if ob is None:
        return False
    src = r.convex_volume_planes
    me = ob.data
    if len(me.polygons) != len(src):
        report.add("ERROR", ob.name, f"convex planes are one face each: {len(src)} faces expected, has {len(me.polygons)}")
        return False
    changed = False
    m = ob.matrix_basis
    for i, poly in enumerate(me.polygons):
        co = np.array([tuple(m @ me.vertices[k].co) for k in poly.vertices], np.float64)
        n, d = _plane(co)
        pl = src[i]
        if not (_close(n, _vec(pl.normal), 1e-4) and abs(d - pl.distance) <= 1e-4):
            _set_vec(pl.normal, n)
            pl.distance = d
            changed = True
    return changed


def _create_liquid(g, ob, report):
    """Gives a liquid-less group the grid described by new liquid object `ob`."""
    p = ob.wow_wmo_liquid
    xv, yv = p.verts_dim
    if xv < 2 or yv < 2 or len(ob.data.vertices) != xv * yv:
        report.add("ERROR", ob.name, f"new liquid needs a {xv}x{yv} vertex grid")
        return False
    L = g.body.liquid
    L.verts_dim.x, L.verts_dim.y = xv, yv
    L.tiles_dim.x, L.tiles_dim.y = xv - 1, yv - 1
    _set_vec(L.base_coords, p.base_coords)
    L.material_id = p.material_id
    L.vertices.resize(xv * yv)
    L.tiles.resize((xv - 1) * (yv - 1))
    h = g.body.header
    h.flags = type(h.flags)(int(h.flags) | 0x1000)
    if not int(h.group_liquid):
        h.group_liquid = 1
    return True


def update_liquid(g, ob, report):
    L = g.body.liquid
    if L.empty:
        if not _create_liquid(g, ob, report):
            return False
        created = True
    else:
        created = False
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
    return changed or created


def update_liquids(w, root, report):
    changed = set()
    for ob in root.all_objects:
        if ob.type == "MESH" and ob.name.startswith("liquid_") and ob.wow_wmo_liquid.verts_dim[0]:
            gi = ob.wow_wmo_liquid.group_index
            if gi < len(w.groups) and update_liquid(w.groups[gi], ob, report):
                changed.add(gi)
    return changed


def update_header(w, root, report):
    h = w.root.header
    p = root.wow_wmo
    changed = False
    if u32(p.flags) != u32(int(h.flags)):
        h.flags = type(h.flags)(u32(p.flags))
        changed = True
    if p.wmo_id != h.wmo_id:
        h.wmo_id = p.wmo_id
        changed = True
    amb = tuple(p.ambient_color)
    if amb != (h.ambient_color.r, h.ambient_color.g, h.ambient_color.b, h.ambient_color.a):
        _set_rgba(h.ambient_color, amb)
        changed = True
    return changed


def rebuild_group_names(w):
    """Rewrites MOGN with the strings the groups still use and repoints MOGI and the group headers at them."""
    r = w.root
    old = r.group_names
    new = type(old)()
    new.add("")
    seen = {}

    def remap(offset):
        offset = int(offset)
        try:
            text = old.at(offset) if 0 <= offset < old.size else None
        except Exception:  # noqa: BLE001
            text = None
        if text is None:
            return offset
        if text not in seen:
            seen[text] = new.add(text)
        return seen[text]

    for i, g in enumerate(w.groups):
        h = g.body.header
        h.group_name = remap(h.group_name)
        h.descriptive_group_name = remap(h.descriptive_group_name)
        r.group_infos[i].name_offset = remap(r.group_infos[i].name_offset)
    r.group_names = new


def fix_header_counts(w):
    """MOHD counts that mirror table sizes (the doodad counts are not: stock headers keep stale values)."""
    r = w.root
    for name, n in (("n_lights", len(r.lights)), ("n_textures", len(r.materials))):
        if int(getattr(r.header, name)) != n:
            setattr(r.header, name, n)


def delete_groups(w, gone, report, order=None):
    """Drops groups `gone` with their group info, portals touching them and every reference to them; returns {old: new}.

    `order` lists the surviving old indices in their new order (default: unchanged)."""
    r = w.root
    n = len(w.groups)
    tables = lod_tables(r)
    order = [i for i in (order if order is not None else range(n)) if i not in gone]
    remap = {old: new for new, old in enumerate(order)}
    dead_portals = {int(x.portal_index) for x in r.portal_refs if int(x.group_index) in gone}
    for gi in gone:
        h = w.groups[gi].body.header
        dead_portals |= {int(x.portal_index) for x in r.portal_refs[h.portal_start:h.portal_start + h.portal_count]}
    pkeep = [i for i in range(len(r.portals)) if i not in dead_portals]
    premap = {old: new for new, old in enumerate(pkeep)}
    refs = []
    for gi in range(n):
        h = w.groups[gi].body.header
        refs.append([(premap[int(x.portal_index)], remap[int(x.group_index)], int(x.side))
                     for x in r.portal_refs[h.portal_start:h.portal_start + h.portal_count]
                     if int(x.portal_index) in premap and int(x.group_index) in remap])
    verts = [[copy.copy(r.portal_vertices[k]) for k in range(r.portals[i].start_vertex, r.portals[i].start_vertex + r.portals[i].count)] for i in pkeep]
    compact(r.portals, pkeep)
    r.portal_vertices.clear()
    for pt, vs in zip(r.portals, verts):
        pt.start_vertex = len(r.portal_vertices)
        for v in vs:
            r.portal_vertices.append(v)
    r.portal_refs.clear()
    for gi in order:
        h = w.groups[gi].body.header
        h.portal_start, h.portal_count = len(r.portal_refs), len(refs[gi])
        for pi, other, side in refs[gi]:
            e = r.portal_refs.new()
            e.portal_index, e.group_index, e.side = pi, other, side
    if order != list(range(n)):
        groups = [copy.copy(w.groups[i]) for i in order]
        w.groups.clear()
        for g in groups:
            w.groups.append(g)
        compact(r.group_infos, order)
        if len(r.group_infos2):
            compact(r.group_infos2, order)
    for g in w.groups:
        h = g.body.header
        for f in ("parent_or_first_child_split_group_index", "next_split_child_group_index"):
            v = int(getattr(h, f))
            if v in remap:
                setattr(h, f, remap[v])
            elif v in gone:
                setattr(h, f, 0xFFFF)
    set_lod_tables(r, [tables[0][:len(remap)]] + [[t[i] for i in order] for t in tables[1:]])
    if gone:
        report.add("INFO", "groups", f"{len(gone)} groups deleted, {len(dead_portals)} portals removed")
    return remap


def update_root_objects(w, root, report):
    """Returns the set of group indices whose liquids changed; root edits are applied in place."""
    for fn in (update_header, update_doodads, update_lights, update_fogs, update_portals, update_convex_planes):
        if fn(w, root, report):
            report.add("INFO", fn.__name__.replace("update_", ""), "updated")
    reference_new_groups(w, sorted(getattr(report, "new_groups", {}).values()))
    return update_liquids(w, root, report)
