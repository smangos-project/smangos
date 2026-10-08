"""Writes an imported M2 back as a 1.15 chunked M2 plus skins. Unchanged models are not written."""
import copy
import posixpath
from pathlib import Path

import bpy
from . import client, fdids, m2_anim_export, m2_import, m2_materials_export, m2_mesh_export, m2_objects_export, m2_rig_export, m2_validate
from .deep_compare import deep_diff
from .report import Report

last_report = None
DEFAULTS = {"force": False, "dry_run": False, "write_anyway": False}


def find_root(ob):
    """The M2 root collection of a collection or an object inside one."""
    if isinstance(ob, bpy.types.Collection):
        if ob.wow_m2.path:
            return ob
        for c in bpy.data.collections:
            if c.wow_m2.path and ob.name in c.children_recursive:
                return c
        return None
    for c in bpy.data.collections:
        if c.wow_m2.path and ob.name in c.all_objects:
            return c
    return None


DEFAULT_TEMPLATE = "duskwoodlamppost.m2"


def new_m2(path, template=None, parent=None):
    """A copy of `template` (client path, default a small doodad) that exports as new model `path`."""
    from . import index
    index.load(client.session().content, client.prefs().cache_dir)
    template = template or next(e[1] for e in index.entries("m2") if e[1].endswith("/" + DEFAULT_TEMPLATE))
    path = path.replace("\\", "/").lower()
    top = m2_import.load(template, {"display": "NONE", "collision": False}, parent)
    p = top.wow_m2
    p.template, p.path, p.fdid, p.is_new = template, path, 0, True
    top.name = posixpath.basename(path)
    return top


def skin_path(path, i):
    return f"{posixpath.splitext(path)[0]}{i:02d}.skin"


def differs(ref, m):
    """First differing field between two M2 entities (root and skins), or None."""
    return deep_diff(ref.root, m.root, "root") or deep_diff(ref.skins, m.skins, "skins")


def sync_shadow_batches(ref, m, report):
    """After skin 0 edits, opaque batches without a shadow batch (new meshes) cast shadows."""
    prof = m.skins[0].profile
    if not len(prof.shadow_batches) or not deep_diff(ref.skins[0], m.skins[0], "skin"):
        return
    have = {int(sb.submesh_id) for sb in prof.shadow_batches}
    proto = prof.shadow_batches[0]
    added = 0
    for b in prof.batches:
        si = int(b.skin_section_index)
        if si in have or int(m.root.materials[int(b.material_index)].blending_mode) != 0:
            continue
        sb = copy.copy(proto)
        sb.submesh_id, sb.texture_id, sb.color_id = si, int(b.texture_combo_index), int(b.color_index)
        prof.shadow_batches.append(sb)
        have.add(si)
        added += 1
    if added:
        report.add("INFO", "geometry", f"{added} shadow batch(es) added")


def sync_lods(ref, m, report):
    """LOD skins are replaced by copies of skin 0 once skin 0 was edited."""
    if len(m.skins) < 2 or not deep_diff(ref.skins[0], m.skins[0], "skin"):
        return
    for i in range(1, len(m.skins)):
        m.skins[i] = copy.deepcopy(m.skins[0])
    report.add("INFO", "geometry", f"{len(m.skins) - 1} LOD skin(s) replaced by copies of skin 0")


def rebuild(top, m, report, opts):
    """Applies every section of the Blender data to the freshly read model `m`."""
    m2_validate.check_blender(top, report)
    rig = m2_rig_export.plan(top, m.root)
    m2_validate.check_source_refs(m.root, rig, report)
    if rig is not None:
        m2_rig_export.build(top, m, report, rig)
    report.extra_files += m2_materials_export.build(top, m, report, opts, top.wow_m2.path)
    m2_mesh_export.build(top, m, report, opts, rig)
    objects = m2_objects_export.build(top, m, report, rig)
    m2_anim_export.build(top, m.root, rig, objects, report)
    m2_validate.check_model(m, report)


def export(top, out_path=None, options=None):
    """Export M2 collection `top`; returns a Report with `.model` and `.files` (client paths written)."""
    global last_report
    opts = dict(DEFAULTS)
    opts.update(options or {})
    report = Report(files_written=0, files_removed=0)
    last_report = report
    report.files, report.extra_files, report.model, report.dirty = [], [], None, False
    bpy.context.view_layer.update()
    p = top.wow_m2
    src = p.template or p.path
    path = (out_path or p.path).replace("\\", "/").lower()
    m = m2_import.read(src)
    ref = m2_import.read(src)
    rebuild(top, m, report, opts)
    report.model = m
    sync_shadow_batches(ref, m, report)
    sync_lods(ref, m, report)
    if report.errors:
        return report
    report.diff = differs(ref, m)
    report.dirty = report.diff is not None
    s = client.session()
    out_dir = s.project
    skins = [skin_path(path, i) for i in range(len(m.skins))]
    if not report.dirty and not opts["force"] and path == src:
        report.add("INFO", "export", "unchanged, nothing written")
        if out_dir is not None and not opts["dry_run"]:
            for f in (path, *skins):
                dst = Path(out_dir) / f
                if dst.is_file():
                    dst.unlink()
                    report.stats["files_removed"] += 1
        if report.stats["files_removed"]:
            client.close()
        return report
    if opts["dry_run"]:
        return report
    if out_dir is None:
        report.add("ERROR", "export", "Set the Output directory in the add-on preferences")
        return report
    if path != src:
        for f in (path, *skins):
            fdids.allocate(f)
        s = client.session()
    vr = m.validate()
    if not vr.ok:
        src_ok = ref.validate().ok
        for issue in vr.issues:
            report.add("WARNING" if (not src_ok or opts["write_anyway"]) else "ERROR", "validate", str(issue))
        if report.errors:
            return report
    m.write(s.fs, client.key(path))
    fdids.adopt_session_ids()
    if path == p.path:
        p.fdid = fdids.allocate(path)
    report.files = [path, *skins, *report.extra_files]
    report.stats["files_written"] = len(report.files)
    return report
