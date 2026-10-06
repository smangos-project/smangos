"""Writes an imported M2 back as a 1.15 chunked M2 plus skins. Unchanged models are not written."""
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


def skin_path(path, i):
    return f"{posixpath.splitext(path)[0]}{i:02d}.skin"


def differs(ref, m):
    """First differing field between two M2 entities (root and skins), or None."""
    return deep_diff(ref.root, m.root, "root") or deep_diff(ref.skins, m.skins, "skins")


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
    src = p.path
    path = (out_path or src).replace("\\", "/").lower()
    m = m2_import.read(src)
    ref = m2_import.read(src)
    rebuild(top, m, report, opts)
    report.model = m
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
    report.files = [path, *skins, *report.extra_files]
    report.stats["files_written"] = len(report.files)
    return report
