"""Blender materials -> M2 batches, materials, textures and lookups. Only edited batches are touched."""
import copy
import posixpath
import re

from wowlib.formats.m2.root import record

from . import fdids, index, m2_import, textures


def _slug(name):
    return re.sub(r"[^a-z0-9_]", "_", posixpath.splitext(name)[0].lower()) or "texture"


def _source_slot(root, b, k):
    """(texture index, type, fdid, uv set) of slot k of source batch `b`; fdid is 0 for replaceable textures."""
    li = b.texture_combo_index + k
    ti = int(root.texture_lookup_table[li]) if li < len(root.texture_lookup_table) else -1
    if not 0 <= ti < len(root.textures):
        return -1, 0, 0, 0
    mi = b.texture_coord_combo_index + k
    uv = int(root.texture_mapping_lookup_table[mi]) if mi < len(root.texture_mapping_lookup_table) else 0
    return ti, int(root.textures[ti].type), 0, uv


def _node_image(mat, batch_index, slot):
    if not mat.node_tree:
        return None
    for n in mat.node_tree.nodes:
        if n.bl_idname == "ShaderNodeTexImage" and n.get("wow_batch") == batch_index and n.get("wow_slot") == slot:
            return n.image
    return None


class _Images:
    """Resolves Blender images to texture fdids, encoding new or edited ones as BLP."""

    def __init__(self, model_path, opts, report):
        self.dir, self.opts, self.report = posixpath.dirname(model_path), opts, report
        self.done, self.files, self.pending = {}, [], []

    def fdid(self, img):
        """The fdid of `img`, or None for the placeholder."""
        if img is None or img.name == textures.PLACEHOLDER:
            return None
        key = img.name
        if key in self.done:
            return self.done[key]
        src = img.get("wow_fdid")
        if src and not img.is_dirty:
            self.done[key] = int(src)
            return int(src)
        path = f"{self.dir}/{_slug(img.name)}.blp"
        fdid = fdids.allocate(path)
        self.done[key] = fdid
        img["wow_fdid"], img["wow_path"] = fdid, path
        self.pending.append((img, path))
        return fdid

    def write(self):
        for img, path in self.pending:
            if not self.opts["dry_run"]:
                textures.export_blp(img, path)
            self.files.append(path)


def _source_fdid(m, ti):
    """File id of texture `ti`: the TXID entry, else the legacy filename resolved through the index."""
    fd = m.chunks.texture_fdids
    if 0 <= ti < len(fd) and fd[ti]:
        return int(fd[ti])
    t = m.root.textures[ti] if 0 <= ti < len(m.root.textures) else None
    if t is not None and not int(t.type) and t.filename:
        return index.fdid_of(t.filename) or 0
    return 0


def _filenames_to_fdids(m, report):
    """Converts legacy filename textures to TXID entries so the file never mixes both."""
    root, fd = m.root, m.chunks.texture_fdids
    if not any(not int(t.type) and t.filename for t in root.textures):
        return
    out = [_source_fdid(m, i) if not int(t.type) else 0 for i, t in enumerate(root.textures)]
    for i, t in enumerate(root.textures):
        if not int(t.type) and t.filename and not out[i]:
            report.add("ERROR", f"texture {i}", f"'{t.filename}' has no file id; cannot convert to TXID")
    for i, t in enumerate(root.textures):
        if not int(t.type):
            t.filename = ""
    fd.clear()
    fd.extend(out)


def _find_or_add_texture(m, type_, fdid, like):
    root = m.root
    fd = m.chunks.texture_fdids
    for i, t in enumerate(root.textures):
        if int(t.type) == type_ and (type_ or (i < len(fd) and int(fd[i]) == fdid)):
            return i
    t = copy.copy(like) if like is not None else record.M2Texture()
    t.type = type_
    root.textures.append(t)
    while len(fd) < len(root.textures) - 1:
        fd.append(0)
    fd.append(0 if type_ else fdid)
    return len(root.textures) - 1


def _find_or_add_material(root, flags, blend):
    for i, mt in enumerate(root.materials):
        if int(mt.flags) == flags and int(mt.blending_mode) == blend:
            return i
    mt = record.M2Material()
    mt.flags, mt.blending_mode = record.MaterialFlags(flags), blend
    root.materials.append(mt)
    return len(root.materials) - 1


def _differs(a, b, tol=1e-4):
    return any(abs(float(x) - float(y)) > tol for x, y in zip(a, b))


def _static_key(report, arm, key, attr, value):
    """Sets the armature property a new record's static track is written from."""
    if arm is not None:
        arm[attr] = value
    report.static_keys.append(key)


def _new_combo(lut, old, n_src, n, first):
    """Appends a lookup row per texture unit (copy of the batch's rows, unit 0 = `first`); returns its start."""
    start = len(lut)
    rows = [int(lut[old + k]) if k < n_src and old + k < len(lut) else -1 for k in range(n)]
    rows[0] = first
    for v in rows:
        lut.append(v & 0xFFFF if v < 0 and 'Unsigned' in type(lut).__name__ else v)
    return start


def _static_tracks(m, b, p, src, n, n_src, arm, report):
    """Static color, weight and texture transform edits become new records with static tracks; returns True on change."""
    root = m.root
    rgba, weight, off, scale = src
    changed = False
    if _differs(p.preview_color, rgba):
        i = len(root.colors)
        root.colors.append(record.M2ColorWotlkPlus())
        _static_key(report, arm, f"color/{i}/color", f"wow_color_{i}", tuple(p.preview_color[:3]))
        _static_key(report, arm, f"color/{i}/alpha", f"wow_alpha_{i}", float(p.preview_color[3]))
        b.color_index = i
        changed = True
    if _differs((p.preview_weight,), (weight,)):
        i = len(root.texture_weights)
        root.texture_weights.append(record.M2TextureWeightWotlkPlus())
        _static_key(report, arm, f"texweight/{i}", f"wow_texweight_{i}", float(p.preview_weight))
        b.texture_weight_combo_index = _new_combo(root.transparency_lookup_table, int(b.texture_weight_combo_index), n_src, n, i)
        changed = True
    if _differs(p.preview_uv_offset, off) or _differs(p.preview_uv_scale, scale):
        i = len(root.texture_transforms)
        root.texture_transforms.append(record.M2TextureTransformWotlkPlus())
        _static_key(report, arm, f"texxform/{i}/translation", f"wow_tt_translation_{i}", (p.preview_uv_offset[0], p.preview_uv_offset[1], 0.0))
        _static_key(report, arm, f"texxform/{i}/rotation", f"wow_tt_rotation_{i}", (1.0, 0.0, 0.0, 0.0))
        _static_key(report, arm, f"texxform/{i}/scaling", f"wow_tt_scaling_{i}", (p.preview_uv_scale[0], p.preview_uv_scale[1], 1.0))
        b.texture_transform_combo_index = _new_combo(root.texture_transforms_lookup_table, int(b.texture_transform_combo_index), n_src, n, i)
        changed = True
    return changed


def _apply(m, b, p, mat, images, report, arm=None):
    """Writes the differences of material item `p` against source batch `b` into the model."""
    root = m.root
    changed = False
    src_vals = m2_import._preview_values(root, b)
    for f in ("flags", "priority_plane", "shader_id", "material_layer"):
        if int(getattr(b, f)) != getattr(p, f):
            setattr(b, f, getattr(p, f))
            changed = True
    mi = int(b.material_index)
    src_mat = root.materials[mi] if mi < len(root.materials) else None
    sflags, sblend = (int(src_mat.flags), int(src_mat.blending_mode)) if src_mat is not None else (0, 0)
    if (p.render_flags, p.blending_mode) != (sflags, sblend):
        b.material_index = _find_or_add_material(root, p.render_flags, p.blending_mode)
        changed = True
    n_src = max(min(int(b.texture_count), 4), 0)
    n = max(min(int(p.texture_count), 4), 1)
    src = [_source_slot(root, b, k) if k < n_src else (-1, 0, 0, 0) for k in range(n)]
    src_fd = [_source_fdid(m, src[k][0]) for k in range(n)]
    want_tex, want_uv = [], []
    tex_changed = uv_changed = n != n_src
    for k in range(n):
        ti, typ, _, uv = src[k]
        t_new = int(p.texture_types[k])
        fd_new = src_fd[k] if typ == 0 else 0
        if t_new == 0:
            img = images.fdid(_node_image(mat, p.batch_index, k))
            if img is not None and img != fd_new:
                fd_new = img
            elif int(p.texture_fdids[k]) != fd_new:
                fd_new = int(p.texture_fdids[k])
            if not fd_new:
                report.add("ERROR", "materials", f"batch {p.batch_index}: texture {k} has no image")
        if (t_new, fd_new) == (typ, src_fd[k] if typ == 0 else 0) and ti >= 0:
            want_tex.append(ti)
        else:
            _filenames_to_fdids(m, report)
            like = root.textures[ti] if ti >= 0 else (root.textures[0] if len(root.textures) else None)
            want_tex.append(_find_or_add_texture(m, t_new, fd_new, like))
            tex_changed = True
        uv_new = int(p.uv_sets[k])
        want_uv.append(uv_new)
        if uv_new != (-1 if uv in (-1, 0xFFFF) or uv >= 2 else uv):
            uv_changed = True
    if tex_changed:
        b.texture_combo_index = len(root.texture_lookup_table)
        root.texture_lookup_table.extend(want_tex)
        changed = True
    if uv_changed:
        b.texture_coord_combo_index = len(root.texture_mapping_lookup_table)
        root.texture_mapping_lookup_table.extend(want_uv)
        changed = True
    changed |= _static_tracks(m, b, p, src_vals, n, n_src, arm, report)
    if n != n_src:
        for lut, attr in ((root.transparency_lookup_table, "texture_weight_combo_index"),
                          (root.texture_transforms_lookup_table, "texture_transform_combo_index")):
            old = int(getattr(b, attr))
            rows = [int(lut[old + k]) if k < n_src and old + k < len(lut) else (0xFFFF if 'Unsigned' in type(lut).__name__ else -1) for k in range(n)]
            setattr(b, attr, len(lut))
            for v in rows:
                lut.append(v)
        b.texture_count = n
    return changed


def _texture_entries(top, m, n_src, report):
    """Applies the model's texture list: edits to source entries, new entries appended (or matched to batch-added ones)."""
    root, fd = m.root, m.chunks.texture_fdids
    entries = list(top.wow_m2.textures)
    for i, e in enumerate(entries):
        typ, fdid, flags = int(e.type), int(e.fdid) & 0xFFFFFFFF, int(e.flags) & 0xFFFFFFFF
        if i >= n_src:
            j = _find_or_add_texture(m, typ, fdid, root.textures[0] if len(root.textures) else None)
            t = root.textures[j]
        elif i < len(root.textures):
            t = root.textures[i]
            if int(t.type) != typ or (not typ and fdid and fdid != _source_fdid(m, i)):
                _filenames_to_fdids(m, report)
                t.type = typ
                while len(fd) < len(root.textures):
                    fd.append(0)
                fd[i] = 0 if typ else fdid
            j = i
        else:
            continue
        if int(t.flags) != flags:
            t.flags = type(t.flags)(flags)


def _rebuild_batches(m, prof, structural, images, report, arm):
    """Rewrites the batch list of sections whose layers were added or removed."""
    snapshot = [copy.copy(b) for b in prof.batches]
    drop, fresh_after = set(), {}
    for idxs, items, mat in structural.values():
        kept = {p.slot for p in items if 0 <= p.slot < len(idxs)}
        drop.update(idxs[j] for j in range(len(idxs)) if j not in kept)
        tmpl = snapshot[idxs[min(kept)]] if kept else snapshot[idxs[0]]
        fresh = []
        for p in items:
            if p.slot < 0 or p.slot >= len(idxs):
                nb = copy.copy(tmpl)
                _apply(m, nb, p, mat, images, report, arm)
                fresh.append(nb)
        fresh_after[max(idxs)] = fresh
    out = []
    for i, b in enumerate(snapshot):
        if i not in drop:
            out.append(b)
        out.extend(fresh_after.get(i, []))
    prof.batches.clear()
    for b in out:
        prof.batches.append(b)
    report.add("INFO", "materials", f"{len(snapshot)} -> {len(out)} batches")


def build(top, m, report, opts, model_path):
    """Applies edited batch fields, materials and textures of the submesh objects to `m`."""
    from . import m2_mesh_export

    prof = m.skins[0].profile
    by_section = {}
    for i, b in enumerate(prof.batches):
        by_section.setdefault(int(b.skin_section_index), []).append(i)
    for items in by_section.values():
        items.sort(key=lambda i: (int(prof.batches[i].material_layer), int(prof.batches[i].priority_plane)))
    images = _Images(model_path, opts, report)
    n_src_tex = len(m.root.textures)
    structural = {}
    report.static_keys = []
    arm = next((o for o in top.all_objects if o.type == "ARMATURE"), None)
    old, _ = m2_mesh_export.submesh_objects(top)
    for ob in old:
        si = ob.wow_m2_submesh.index
        mat = ob.material_slots[0].material if ob.material_slots else None
        if mat is None or not len(mat.wow_m2_batches) or si not in by_section:
            continue
        idxs = by_section[si]
        items = list(mat.wow_m2_batches)
        keep = {p.slot for p in items if 0 <= p.slot < len(idxs)}
        if len(keep) != len(items) or len(keep) != len(idxs):
            structural[si] = (idxs, items, mat)
        for p in items:
            if 0 <= p.slot < len(idxs):
                _apply(m, prof.batches[idxs[p.slot]], p, mat, images, report, arm)
    if structural:
        _rebuild_batches(m, prof, structural, images, report, arm)
    _texture_entries(top, m, n_src_tex, report)
    images.write()
    return images.files
