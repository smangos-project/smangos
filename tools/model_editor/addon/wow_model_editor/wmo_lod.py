"""WMO LOD group files (`<root>_NNN_lodK.wmo`): imported as hidden child objects of their group, exported from those objects."""
import re

import bpy
from wowlib.formats import wmo as wwmo

from . import client, fdids, index, materials, wmo_import
from . import wmo_root_export as rx

F_BSP = 0x1


def group_file(root_path, gi, lod=0):
    return re.sub(r"\.wmo$", f"_{gi:03d}" + (f"_lod{lod}" if lod else "") + ".wmo", root_path)


def file_bytes(path):
    """The file's bytes from the output directory, else Content; None when missing."""
    s = client.session()
    for base in (s.project, s.content):
        if base is not None and (base / path).is_file():
            return (base / path).read_bytes()
    return None


def lod_objects(root):
    return [o for o in root.all_objects if o.type == "MESH" and o.wow_wmo_group.lod > 0]


def level_wmo(root_bytes, base_groups, overrides):
    """A WMO whose group `gi` is `overrides[gi]` where given, else the base group."""
    w = wwmo.WMO.for_version(client.VERSION)
    w.read(root_bytes, [overrides.get(gi) or b for gi, b in enumerate(base_groups)])
    return w


def import_lods(w, coll, mats, group_objs, path, stem):
    """Creates `lods/lodK` collections with one hidden mesh per LOD file, parented to its group object."""
    tables = rx.lod_tables(w.root)
    by_index = {ob.wow_wmo_group.index: ob for ob in group_objs}
    collision = materials.collision_material()
    root_bytes = file_bytes(path)
    base_groups = [file_bytes(group_file(path, gi)) for gi in range(len(w.groups))]
    for k in range(1, len(tables)):
        files = {}
        for gi, fdid in enumerate(tables[k]):
            fp = index.path_of(fdid) if fdid else None
            data = file_bytes(fp) if fp else None
            if data is not None and gi in by_index:
                files[gi] = data
        if not files:
            continue
        wl = level_wmo(root_bytes, base_groups, files)
        c = bpy.data.collections.new(f"lod{k}")
        coll.children.link(c)
        for gi in files:
            base = by_index[gi]
            name = f"{base.name}_lod{k}"
            me, _ = wmo_import.build_group_mesh(name, wl.groups[gi].body, mats, collision, stem)
            ob = bpy.data.objects.new(name, me)
            c.objects.link(ob)
            ob.parent = base
            ob.hide_viewport = True
            p, bp = ob.wow_wmo_group, base.wow_wmo_group
            p.lod, p.flags, p.fdid = k, bp.flags, tables[k][gi]
            p.group_name, p.descriptive_name = bp.group_name, bp.descriptive_name
    return True


def base_index(ob, report, remap):
    """Final group index of the group object `ob`, or None when it no longer exists."""
    gi = ob.wow_wmo_group.index
    if gi < 0:
        return report.new_groups.get(ob.name)
    return gi if remap is None else remap.get(gi)


def apply_lods(w, root, path, report, files, remap, dirty):
    """Writes LOD group files for the LOD objects, keeps unchanged ones, and drops the LODs of changed groups without objects."""
    from . import wmo_export as ex
    r = w.root
    n = len(w.groups)
    p = root.wow_wmo
    wanted = {}
    for ob in lod_objects(root):
        base = ob.parent
        gi = base_index(base, report, remap) if base is not None and base.type == "MESH" else None
        if gi is None:
            report.add("WARNING", ob.name, "LOD object has no group and is ignored")
            continue
        wanted[(gi, ob.wow_wmo_group.lod)] = ob
    owned = set(range(n)) if p.lods_imported else {gi for gi, _ in wanted}
    rx.clear_group_lods(r, dirty - owned, report)
    top = max((k for _, k in wanted), default=0)
    tables = rx.lod_tables(r)
    while len(tables) <= top:
        tables.append([0] * n)
    for gi in owned:
        levels = sorted(k for g, k in wanted if g == gi)
        if levels != list(range(1, len(levels) + 1)):
            report.add("ERROR", f"group {gi}", f"LOD levels {levels} are not contiguous from 1")
            return
    kept = {(gi, k): tables[k][gi] for k in range(1, len(tables)) for gi in owned if (gi, k) in wanted}
    for k in range(1, len(tables)):
        for gi in owned:
            tables[k][gi] = kept.get((gi, k), 0)
    if wanted:
        root_bytes, base_groups = ex.serialize(w)
        for k in range(1, top + 1):
            todo = {gi: ob for (gi, lk), ob in wanted.items() if lk == k}
            if not todo:
                continue
            existing = {}
            for gi in todo:
                fp = index.path_of(tables[k][gi]) if tables[k][gi] else None
                data = file_bytes(fp) if fp else None
                if data is not None:
                    existing[gi] = data
            wl = level_wmo(root_bytes, base_groups, existing)
            built = []
            for gi, ob in sorted(todo.items()):
                if ex.update_group(wl, gi, ob, report, gi not in existing, bsp=False):
                    built.append(gi)
                    h = wl.groups[gi].body.header
                    h.flags = type(h.flags)((int(h.flags) | rx.GROUP_LOD) & ~F_BSP)
                    h.unique_id = 0
                    bb = w.groups[gi].body.header.bounding_box
                    ex._set_box(h.bounding_box, (bb.min.x, bb.min.y, bb.min.z), (bb.max.x, bb.max.y, bb.max.z))
            if built:
                _, out = ex.serialize(wl)
                for gi in built:
                    lp = group_file(path, gi, k)
                    files[lp] = out[gi]
                    tables[k][gi] = fdids.allocate(lp)
                report.add("INFO", f"lod{k}", f"{len(built)} LOD group files built")
    depth = max((k for k in range(len(tables)) if any(tables[k])), default=0)
    if depth == 0:
        rx.set_lod_tables(r, tables[:1])
        rx.prune_lod_header(r)
        return
    tables = tables[:depth + 1]
    if not len(r.group_infos2):
        for _ in range(n):
            r.group_infos2.new()
    for gi in range(n):
        have = max((k for k in range(1, depth + 1) if tables[k][gi]), default=0)
        info = r.group_infos[gi]
        if have or gi in owned:
            info.flags = type(info.flags)(int(info.flags) & ~rx.GROUP_LOD | (rx.GROUP_LOD if have else 0))
            r.group_infos2[gi].lod_index = (int(r.group_infos2[gi].lod_index) & ~0xFF) | have
    r.header.flags = type(r.header.flags)(int(r.header.flags) | rx.HAS_LOD)
    r.header.num_lod = depth + 1
    rx.set_lod_tables(r, tables)
