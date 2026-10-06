"""Armature -> M2 bones; renumbers every bone reference of a freshly read model when bones were added or deleted."""
import copy

import numpy as np
from wowlib.formats.m2.root import record as rec

from .m2_mesh_export import find_armature

PIVOT_TOL = 1e-6


class Rig:
    """Old -> new bone numbering and the bone names in final order."""

    def __init__(self, names, new_of_old, old_of_new):
        self.names, self.new_of_old, self.old_of_new = names, new_of_old, old_of_new
        self.by_name = {n: i for i, n in enumerate(names)}
        self.identity = list(new_of_old) == list(range(len(new_of_old))) and len(names) == len(new_of_old)

    def map(self, old, default=0):
        """New index of source bone `old`; `default` when the bone is gone or out of range."""
        if 0 <= old < len(self.new_of_old) and self.new_of_old[old] >= 0:
            return self.new_of_old[old]
        return default

    def map_array(self, a, default=0):
        lut = np.append(np.asarray(self.new_of_old, np.int64), -1)
        out = lut[np.clip(np.asarray(a, np.int64), -1, len(self.new_of_old))]
        return np.where(out < 0, default, out)


def plan(top, root):
    """Rig for the armature under `top`, or None when the model has no armature or no source bones."""
    arm = find_armature(top)
    n = len(root.bones)
    if arm is None or not n:
        return None
    old, new = [], []
    for b in arm.data.bones:
        i = b.wow_m2_bone.index
        (old if 0 <= i < n else new).append(b)
    old.sort(key=lambda b: b.wow_m2_bone.index)
    new.sort(key=lambda b: (len(b.parent_recursive), b.name))
    order = old + new
    new_of_old = [-1] * n
    for j, b in enumerate(order[:len(old)]):
        new_of_old[b.wow_m2_bone.index] = j
    return Rig([b.name for b in order], new_of_old,
               [b.wow_m2_bone.index for b in old] + [-1] * len(new))


def _close(a, b):
    return all(abs(x - y) <= PIVOT_TOL * max(1.0, abs(y)) for x, y in zip(a, b))


def _remap_refs(root, m, rig):
    """Renumber vertex weights, lookups, submesh centres and object bone fields of `m`."""
    for v in root.vertices:
        for k in range(4):
            v.bone_indices[k] = int(rig.map(int(v.bone_indices[k])))
    lookup = [rig.map(int(x)) for x in root.bone_lookup_table]
    root.bone_lookup_table.clear()
    root.bone_lookup_table.extend(lookup)
    for skin in m.skins:
        for sm in skin.profile.submeshes:
            sm.center_bone_index = rig.map(int(sm.center_bone_index))
    for i in range(len(root.key_bone_lookup)):
        root.key_bone_lookup[i] = rig.map(int(root.key_bone_lookup[i]), -1) if root.key_bone_lookup[i] >= 0 else -1
    for vec, field in ((root.attachments, "bone"), (root.events, "bone"), (root.lights, "bone"),
                       (root.particle_emitters, "bone_id"), (root.ribbon_emitters, "bone_index")):
        for rec_ in vec:
            setattr(rec_, field, rig.map(int(getattr(rec_, field))))


def build(top, m, report, rig):
    """Rewrite `m.root.bones` from the armature and renumber all references. `rig` comes from `plan`."""
    root = m.root
    arm = find_armature(top)
    src = list(root.bones)
    bones = []
    for j, name in enumerate(rig.names):
        blb = arm.data.bones[name]
        p = blb.wow_m2_bone
        o = rig.old_of_new[j]
        b = copy.copy(src[o]) if o >= 0 else copy.copy(src[0])
        if o < 0:
            b.flags, b.key_bone_id, b.submesh_id, b.bone_name_crc = rec.BoneFlags(0), -1, 0, 0
        else:
            if int(b.flags) != p.flags & 0xFFFFFFFF:
                b.flags = rec.BoneFlags(p.flags & 0xFFFFFFFF)
            if int(b.key_bone_id) != p.key_bone_id:
                b.key_bone_id = p.key_bone_id
            if int(b.submesh_id) != p.submesh_id:
                b.submesh_id = p.submesh_id
            if int(b.bone_name_crc) != p.bone_name_crc & 0xFFFFFFFF:
                b.bone_name_crc = p.bone_name_crc & 0xFFFFFFFF
        head = tuple(float(x) for x in blb.head_local)
        pv = b.pivot
        if o < 0 or not _close(head, (pv.x, pv.y, pv.z)):
            pv.x, pv.y, pv.z = head
        if blb.parent is not None:
            b.parent_bone = rig.by_name[blb.parent.name]
        elif o < 0 or 0 <= int(b.parent_bone) < len(src) and int(b.parent_bone) != o:
            b.parent_bone = -1
        bones.append(b)
    if not rig.identity:
        _remap_refs(root, m, rig)
    same = len(bones) == len(src)
    if not same:
        report.add("INFO", "rig", f"{len(src)} -> {len(bones)} bones")
    root.bones.clear()
    for b in bones:
        root.bones.append(b)
    _fill_key_lookup(root)


def _fill_key_lookup(root):
    """Keys that no lookup entry points at yet get the first bone claiming them."""
    kl = root.key_bone_lookup
    for i, b in enumerate(root.bones):
        k = int(b.key_bone_id)
        if 0 <= k < len(kl) and kl[k] < 0:
            kl[k] = i
