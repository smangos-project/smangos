"""Pre-write checks of an M2 export; findings go into the Report with an optional Blender object to select."""
from .m2_mesh_export import bone_map, find_armature, submesh_objects

MAX_VERTICES = 65535
MAX_SUBMESH_BONES = 256


def _pot(n):
    return n > 0 and n & (n - 1) == 0


def check_blender(top, report):
    """Problems visible in the scene: empty submeshes, stray or excess weights, NPOT textures."""
    bmap = bone_map(find_armature(top))
    old, new = submesh_objects(top)
    for ob in old + new:
        me = ob.data
        if not len(me.polygons):
            report.add("WARNING", ob.name, "submesh has no faces and will be dropped", ob)
            continue
        if bmap:
            unknown = sorted({vg.name for vg in ob.vertex_groups if vg.name not in bmap})
            used = {ob.vertex_groups[g.group].name for v in me.vertices for g in v.groups if g.weight > 0}
            for name in sorted(used & set(unknown)):
                report.add("WARNING", ob.name, f"weights on vertex group '{name}' which is not a bone; ignored", ob)
            if max((sum(1 for g in v.groups if g.weight > 0) for v in me.vertices), default=0) > 4:
                report.add("WARNING", ob.name, "vertices with more than 4 influences keep their strongest 4", ob)
    seen = set()
    for ob in old + new:
        for mat in ob.data.materials:
            for node in (mat.node_tree.nodes if mat and mat.node_tree else ()):
                img = getattr(node, "image", None)
                if img is None or img.name in seen:
                    continue
                seen.add(img.name)
                w, h = img.size
                if w and h and not (_pot(w) and _pot(h)):
                    report.add("WARNING", img.name, f"texture is {w}x{h}, not a power of two", ob)


def check_source_refs(root, rig, report):
    """Objects of the source model that sit on bones deleted from the armature."""
    if rig is None:
        return
    for field, vec, label in (("bone", root.attachments, "attachment"), ("bone", root.events, "event"),
                              ("bone", root.lights, "light"), ("bone_id", root.particle_emitters, "emitter"),
                              ("bone_index", root.ribbon_emitters, "ribbon")):
        for i, r in enumerate(vec):
            b = int(getattr(r, field))
            if 0 <= b < len(rig.new_of_old) and rig.new_of_old[b] < 0:
                report.add("WARNING", f"{label} {i}", f"is attached to deleted bone {b}; moved to bone 0")


def check_model(m, report):
    """Limits and consistency of the built model."""
    root = m.root
    if len(root.vertices) > MAX_VERTICES:
        report.add("ERROR", "geometry", f"{len(root.vertices)} vertices (max {MAX_VERTICES})")
    nb = len(root.bones)
    for i, b in enumerate(root.bones):
        p, seen = int(b.parent_bone), set()
        while 0 <= p < nb:
            if p in seen or p == i:
                report.add("ERROR", f"bone {i}", "parent chain loops")
                break
            seen.add(p)
            p = int(root.bones[p].parent_bone)
        if p >= nb:
            report.add("ERROR", f"bone {i}", f"parent {p} out of range")
    for si, skin in enumerate(m.skins):
        for j, sm in enumerate(skin.profile.submeshes):
            if sm.bone_count > MAX_SUBMESH_BONES:
                report.add("WARNING", f"skin {si} submesh {j}", f"uses {sm.bone_count} bones (client limit {MAX_SUBMESH_BONES})")
            if not sm.index_count:
                report.add("WARNING", f"skin {si} submesh {j}", "is empty")
    fd = list(m.chunks.texture_fdids)
    for i, t in enumerate(root.textures):
        if int(t.type) == 0 and not (i < len(fd) and fd[i]):
            report.add("ERROR", f"texture {i}", "file texture without a file id")
