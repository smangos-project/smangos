"""Blender materials -> M2 batches, materials, textures and lookups. Only edited batches are touched."""
import copy
import posixpath
import re

from wowlib.formats.m2.root import record

from . import fdids, index, textures


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


def _apply(m, b, p, mat, images, report):
    """Writes the differences of material item `p` against source batch `b` into the model."""
    root = m.root
    changed = False
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
    n = max(min(int(b.texture_count), 2), 0)
    if p.texture_count != int(b.texture_count):
        report.add("WARNING", "materials", f"batch {p.batch_index}: texture count changes are ignored")
    src = [_source_slot(root, b, k) for k in range(n)]
    src_fd = []
    for k in range(n):
        ti = src[k][0]
        src_fd.append(_source_fdid(m, ti))
    want_tex, want_uv = [], []
    tex_changed = uv_changed = False
    for k in range(n):
        ti, typ, _, uv = src[k]
        t_new = int(p.texture_types[k])
        fd_new = src_fd[k] if typ == 0 else 0
        if t_new == 0:
            img = images.fdid(_node_image(mat, p.batch_index, k))
            if img is not None and img != fd_new:
                fd_new = img
            elif int(p.texture_fdids[k]) != fd_new and typ == 0:
                fd_new = int(p.texture_fdids[k])
        if (t_new, fd_new) == (typ, src_fd[k] if typ == 0 else 0) and ti >= 0:
            want_tex.append(ti)
        else:
            _filenames_to_fdids(m, report)
            like = root.textures[ti] if ti >= 0 else None
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
    return changed


def build(top, m, report, opts, model_path):
    """Applies edited batch fields, materials and textures of the submesh objects to `m`."""
    from . import m2_mesh_export

    prof = m.skins[0].profile
    by_section = {}
    for b in prof.batches:
        by_section.setdefault(int(b.skin_section_index), []).append(b)
    for items in by_section.values():
        items.sort(key=lambda b: (int(b.material_layer), int(b.priority_plane)))
    images = _Images(model_path, opts, report)
    old, _ = m2_mesh_export.submesh_objects(top)
    for ob in old:
        si = ob.wow_m2_submesh.index
        mat = ob.material_slots[0].material if ob.material_slots else None
        if mat is None or not len(mat.wow_m2_batches) or si not in by_section:
            continue
        batches = by_section[si]
        if len(batches) != len(mat.wow_m2_batches):
            report.add("WARNING", "materials", f"{ob.name}: material has {len(mat.wow_m2_batches)} batches, model {len(batches)}; skipped")
            continue
        for b, p in zip(batches, mat.wow_m2_batches):
            _apply(m, b, p, mat, images, report)
    images.write()
    return images.files
