"""Blender preview materials for M2 batches. The `wow_m2_batches` fields on the material are the source of truth."""
import bpy

from . import m2_names, materials, textures

F_UNLIT, F_TWO_SIDED = 0x1, 0x4
ADDITIVE = {3, 4, 7}
MULTIPLY = {5, 6}


def _texture(nt, fdid, tex_type, uv_set, offset, scale):
    node = nt.nodes.new("ShaderNodeTexImage")
    node.image = textures.get_image(fdid)
    node.extension = "REPEAT"
    if tex_type:
        node.label = f"type {tex_type}"
        node["wow_texture_type"] = tex_type
    if uv_set < 0:
        coord = nt.nodes.new("ShaderNodeTexCoord")
        src = coord.outputs["Reflection"]
    else:
        uv = nt.nodes.new("ShaderNodeUVMap")
        uv.uv_map = f"uv_{min(uv_set, 1)}"
        src = uv.outputs["UV"]
    if tuple(offset) != (0.0, 0.0) or tuple(scale) != (1.0, 1.0):
        mp = nt.nodes.new("ShaderNodeMapping")
        mp.inputs["Location"].default_value = (offset[0], offset[1], 0.0)
        mp.inputs["Scale"].default_value = (scale[0], scale[1], 1.0)
        nt.links.new(src, mp.inputs["Vector"])
        src = mp.outputs["Vector"]
    nt.links.new(src, node.inputs["Vector"])
    return node


def _math(nt, op, a, b):
    n = nt.nodes.new("ShaderNodeMath")
    n.operation = op
    for i, v in enumerate((a, b)):
        if isinstance(v, (int, float)):
            n.inputs[i].default_value = v
        else:
            nt.links.new(v, n.inputs[i])
    return n.outputs[0]


def _mix(nt, blend, fac, a, b):
    n = nt.nodes.new("ShaderNodeMix")
    n.data_type, n.blend_type = "RGBA", blend
    for sock, v in ((n.inputs["Factor"], fac), (n.inputs["A"], a), (n.inputs["B"], b)):
        if isinstance(v, (int, float)):
            sock.default_value = v
        else:
            nt.links.new(v, sock)
    return n.outputs["Result"]


def _layer(nt, b):
    """(color, alpha) sockets of one batch: its textures combined, times the static color and weight."""
    nodes = []
    for k in range(min(max(b.texture_count, 1), 2)):
        nodes.append(_texture(nt, b.texture_fdids[k], b.texture_types[k], b.uv_sets[k],
                              b.preview_uv_offset, b.preview_uv_scale))
    color, alpha = nodes[0].outputs["Color"], nodes[0].outputs["Alpha"]
    if len(nodes) > 1:
        color = _mix(nt, "ADD" if b.uv_sets[1] < 0 else "MULTIPLY", 1.0, color, nodes[1].outputs["Color"])
    rgba = b.preview_color
    if tuple(rgba[:3]) != (1.0, 1.0, 1.0):
        color = _tint(nt, color, rgba)
    factor = rgba[3] * b.preview_weight
    if factor != 1.0:
        alpha = _math(nt, "MULTIPLY", alpha, factor)
    return color, alpha


def _tint(nt, color, rgba):
    n = nt.nodes.new("ShaderNodeMix")
    n.data_type, n.blend_type = "RGBA", "MULTIPLY"
    n.inputs["Factor"].default_value = 1.0
    n.inputs["B"].default_value = (rgba[0], rgba[1], rgba[2], 1.0)
    nt.links.new(color, n.inputs["A"])
    return n.outputs["Result"]


def build_preview(mat):
    """(Re)build the node tree of `mat` from its wow_m2_batches."""
    batches = sorted(mat.wow_m2_batches, key=lambda b: (b.material_layer, b.priority_plane))
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    N, L = nt.nodes, nt.links
    out = N.new("ShaderNodeOutputMaterial")
    grp = N.new("ShaderNodeGroup")
    grp.node_tree = materials.surface_group()
    if not batches:
        L.new(grp.outputs["Shader"], out.inputs["Surface"])
        return
    color, alpha = _layer(nt, batches[0])
    first = batches[0]
    for b in batches[1:]:
        c, a = _layer(nt, b)
        mode = b.blending_mode
        if mode in ADDITIVE:
            color = _mix(nt, "ADD", a if mode != 3 else 1.0, color, c)
        elif mode in MULTIPLY:
            color = _mix(nt, "MULTIPLY", 1.0, color, c)
        elif mode == 0:
            color = c
        else:
            color = _mix(nt, "MIX", a, color, c)
    L.new(color, grp.inputs["Color"])
    blend, flags = first.blending_mode, first.render_flags
    grp.inputs["Emission"].default_value = 1.0 if flags & F_UNLIT else 0.0
    shader = grp.outputs["Shader"]
    mat.surface_render_method = "DITHERED" if blend in (0, 1) else "BLENDED"
    mat.use_transparency_overlap = blend not in (0, 1)
    if blend == 1:
        L.new(_math(nt, "GREATER_THAN", alpha, materials.ALPHA_KEY), grp.inputs["Alpha"])
    elif blend >= 2:
        L.new(alpha, grp.inputs["Alpha"])
    if blend in ADDITIVE:
        em = N.new("ShaderNodeEmission")
        L.new(color, em.inputs["Color"])
        mixs = N.new("ShaderNodeMixShader")
        if blend == 3:
            mixs.inputs["Fac"].default_value = 1.0
        else:
            L.new(alpha, mixs.inputs["Fac"])
        L.new(N.new("ShaderNodeBsdfTransparent").outputs[0], mixs.inputs[1])
        L.new(em.outputs[0], mixs.inputs[2])
        shader = mixs.outputs[0]
    L.new(shader, out.inputs["Surface"])
    mat.use_backface_culling = not flags & F_TWO_SIDED
    for i, n in enumerate(N):
        n.location = (i * 40, -i * 30)


def set_batch(p, b, index, root_material, texture_indices, types, fdids, uv_sets, preview):
    """Fill a wow_m2_batches item from a wowlib M2Batch and its resolved lookups."""
    for f in ("flags", "priority_plane", "shader_id", "skin_section_index", "geoset_index", "color_index",
              "material_index", "material_layer", "texture_count", "texture_combo_index", "texture_coord_combo_index",
              "texture_weight_combo_index", "texture_transform_combo_index"):
        setattr(p, f, int(getattr(b, f)))
    p.batch_index = index
    p.render_flags, p.blending_mode = int(root_material.flags), int(root_material.blending_mode)
    p.texture_indices, p.texture_types, p.uv_sets = texture_indices, types, uv_sets
    p.texture_fdids = fdids
    p.preview_color, p.preview_weight = preview[0], preview[1]
    p.preview_uv_offset, p.preview_uv_scale = preview[2], preview[3]


def set_replaceable(mat, types_to_fdid):
    """Point replaceable-texture slots at new images; returns True when anything changed."""
    changed = False
    for p in mat.wow_m2_batches:
        for k in range(2):
            t = p.texture_types[k]
            if t and t in types_to_fdid and p.texture_fdids[k] != types_to_fdid[t]:
                fd = list(p.texture_fdids)
                fd[k] = types_to_fdid[t]
                p.texture_fdids = fd
                changed = True
    if changed and mat.node_tree:
        for node in mat.node_tree.nodes:
            t = node.get("wow_texture_type")
            if node.bl_idname == "ShaderNodeTexImage" and t in types_to_fdid:
                node.image = textures.get_image(types_to_fdid[t])
    return changed


class WOW_OT_rebuild_m2_material(bpy.types.Operator):
    bl_idname = "wow.rebuild_m2_material"
    bl_label = "Rebuild WoW M2 material preview"

    @classmethod
    def poll(cls, context):
        return context.material is not None and len(context.material.wow_m2_batches) > 0

    def execute(self, context):
        build_preview(context.material)
        return {"FINISHED"}


class WOW_PT_m2_material(bpy.types.Panel):
    bl_label = "WoW M2 batches"
    bl_space_type, bl_region_type, bl_context = "PROPERTIES", "WINDOW", "material"

    @classmethod
    def poll(cls, context):
        return context.material is not None and len(context.material.wow_m2_batches) > 0

    def draw(self, context):
        for p in context.material.wow_m2_batches:
            box = self.layout.box()
            box.label(text=f"Batch {p.batch_index}: layer {p.material_layer}, {m2_names.BLENDING[p.blending_mode % 8]}")
            col = box.column(align=True)
            for name in ("flags", "shader_id", "priority_plane", "render_flags", "texture_count", "texture_fdids",
                         "texture_types", "uv_sets"):
                col.prop(p, name)
        self.layout.operator("wow.rebuild_m2_material")


classes = (WOW_OT_rebuild_m2_material, WOW_PT_m2_material)
