"""Blender preview materials for WMO materials. The raw SMOMaterial fields in `wow_wmo_material` are the source of truth."""
import bpy

from . import textures
from .props import u32

F_UNLIT, F_TWO_SIDED, F_CLAMP_S, F_CLAMP_T = 0x1, 0x4, 0x40, 0x80
ALPHA_KEY = 224 / 255

TWO_LAYER = {6, 7, 8, 13, 14, 15, 16}
ENV = {3, 5, 7, 12}
GROUP_NAME = "WoW WMO Surface"
COLLISION = "wow_collision"


def surface_group():
    ng = bpy.data.node_groups.get(GROUP_NAME)
    if ng:
        return ng
    ng = bpy.data.node_groups.new(GROUP_NAME, "ShaderNodeTree")
    itf = ng.interface
    for name, typ, default in (("Color", "NodeSocketColor", (1, 1, 1, 1)),
                               ("Alpha", "NodeSocketFloat", 1.0),
                               ("Vertex Color", "NodeSocketColor", (1, 1, 1, 1)),
                               ("Emission", "NodeSocketFloat", 0.0)):
        s = itf.new_socket(name, in_out="INPUT", socket_type=typ)
        s.default_value = default
    itf.new_socket("Shader", in_out="OUTPUT", socket_type="NodeSocketShader")
    n = ng.nodes
    inp, out = n.new("NodeGroupInput"), n.new("NodeGroupOutput")
    mul = n.new("ShaderNodeMix")
    mul.data_type, mul.blend_type = "RGBA", "MULTIPLY"
    mul.inputs["Factor"].default_value = 1.0
    bsdf = n.new("ShaderNodeBsdfPrincipled")
    bsdf.inputs["Roughness"].default_value = 1.0
    bsdf.inputs["Specular IOR Level"].default_value = 0.0
    L = ng.links
    L.new(inp.outputs["Color"], mul.inputs["A"])
    L.new(inp.outputs["Vertex Color"], mul.inputs["B"])
    L.new(mul.outputs["Result"], bsdf.inputs["Base Color"])
    L.new(mul.outputs["Result"], bsdf.inputs["Emission Color"])
    L.new(inp.outputs["Emission"], bsdf.inputs["Emission Strength"])
    L.new(inp.outputs["Alpha"], bsdf.inputs["Alpha"])
    L.new(bsdf.outputs["BSDF"], out.inputs["Shader"])
    inp.location, mul.location, bsdf.location, out.location = (-300, 0), (-100, 100), (150, 0), (450, 0)
    return ng


def collision_material():
    mat = bpy.data.materials.get(COLLISION)
    if mat is None:
        mat = bpy.data.materials.new(COLLISION)
        mat.use_nodes = True
        nt = mat.node_tree
        nt.nodes.clear()
        out = nt.nodes.new("ShaderNodeOutputMaterial")
        tr = nt.nodes.new("ShaderNodeBsdfTransparent")
        nt.links.new(tr.outputs[0], out.inputs["Surface"])
        mat.surface_render_method = "BLENDED"
        mat.diffuse_color = (0.3, 0.3, 0.3, 0.0)
    return mat


def _tex(nt, fdid, flags, uv_layer=None, field=None):
    node = nt.nodes.new("ShaderNodeTexImage")
    node.label = field or ""
    node.image = textures.get_image(fdid)
    node.interpolation = "Linear"
    node.extension = "EXTEND" if flags & (F_CLAMP_S | F_CLAMP_T) == (F_CLAMP_S | F_CLAMP_T) else "REPEAT"
    if uv_layer:
        uv = nt.nodes.new("ShaderNodeUVMap")
        uv.uv_map = uv_layer
        nt.links.new(uv.outputs["UV"], node.inputs["Vector"])
    return node


def build_preview(mat, multi_uv=False):
    """(Re)build the node tree of `mat` from its wow_wmo_material fields."""
    p = mat.wow_wmo_material
    flags, shader, blend = u32(p.flags), p.shader, p.blend_mode
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    N, L = nt.nodes, nt.links
    out = N.new("ShaderNodeOutputMaterial")
    grp = N.new("ShaderNodeGroup")
    grp.node_tree = surface_group()
    vc = N.new("ShaderNodeVertexColor")
    vc.layer_name = "col0"
    L.new(vc.outputs["Color"], grp.inputs["Vertex Color"])

    t1 = _tex(nt, p.texture1, flags, field="texture1")
    color, alpha = t1.outputs["Color"], t1.outputs["Alpha"]
    if shader in TWO_LAYER and p.texture2:
        t2 = _tex(nt, p.texture2, flags, "uv_1" if multi_uv else None, "texture2")
        fac = N.new("ShaderNodeVertexColor")
        fac.layer_name = "col1"
        mix = N.new("ShaderNodeMix")
        mix.data_type = "RGBA"
        L.new(fac.outputs["Alpha"], mix.inputs["Factor"])
        L.new(color, mix.inputs["A"])
        L.new(t2.outputs["Color"], mix.inputs["B"])
        color = mix.outputs["Result"]
    elif shader in ENV and p.texture2:
        t2 = _tex(nt, p.texture2, flags, field="texture2")
        add = N.new("ShaderNodeMix")
        add.data_type, add.blend_type = "RGBA", "ADD"
        add.inputs["Factor"].default_value = 0.15
        L.new(color, add.inputs["A"])
        L.new(t2.outputs["Color"], add.inputs["B"])
        color = add.outputs["Result"]
    L.new(color, grp.inputs["Color"])
    grp.inputs["Emission"].default_value = 1.0 if flags & F_UNLIT else 0.0

    shader_out = grp.outputs["Shader"]
    mat.surface_render_method = "DITHERED" if blend in (0, 1) else "BLENDED"
    mat.use_transparency_overlap = blend not in (0, 1)
    if blend == 1:
        clip = N.new("ShaderNodeMath")
        clip.operation = "GREATER_THAN"
        clip.inputs[1].default_value = ALPHA_KEY
        L.new(alpha, clip.inputs[0])
        L.new(clip.outputs[0], grp.inputs["Alpha"])
    elif blend >= 2:
        L.new(alpha, grp.inputs["Alpha"])
    if blend == 3:
        em = N.new("ShaderNodeEmission")
        L.new(color, em.inputs["Color"])
        tr = N.new("ShaderNodeBsdfTransparent")
        mixs = N.new("ShaderNodeMixShader")
        L.new(alpha, mixs.inputs["Fac"])
        L.new(tr.outputs[0], mixs.inputs[1])
        L.new(em.outputs[0], mixs.inputs[2])
        shader_out = mixs.outputs[0]
    elif blend in (4, 5):
        bw = N.new("ShaderNodeRGBToBW")
        L.new(color, bw.inputs["Color"])
        inv = N.new("ShaderNodeMath")
        inv.operation, inv.use_clamp = "MULTIPLY_ADD", True
        inv.inputs[1].default_value = -1.0 if blend == 4 else -2.0
        inv.inputs[2].default_value = 1.0
        L.new(bw.outputs["Val"], inv.inputs[0])
        black = N.new("ShaderNodeEmission")
        black.inputs["Color"].default_value = (0, 0, 0, 1)
        tr = N.new("ShaderNodeBsdfTransparent")
        mixs = N.new("ShaderNodeMixShader")
        L.new(inv.outputs[0], mixs.inputs["Fac"])
        L.new(tr.outputs[0], mixs.inputs[1])
        L.new(black.outputs[0], mixs.inputs[2])
        shader_out = mixs.outputs[0]
    L.new(shader_out, out.inputs["Surface"])
    mat.use_backface_culling = not flags & F_TWO_SIDED
    for i, n in enumerate(N):
        n.location = (i * 40, -i * 30)


def create_wmo_material(name, fields, index, multi_uv=False):
    """Material from a wowlib SMOMaterial; `fields` is the SMOMaterial."""
    from .props import s32
    mat = bpy.data.materials.new(name)
    p = mat.wow_wmo_material
    p.material_index = index
    p.flags = s32(int(fields.flags))
    p.shader, p.blend_mode = fields.shader, fields.blend_mode
    p.texture1, p.texture2, p.texture3 = (s32(fields.texture1), s32(fields.texture2), s32(fields.texture3))
    for attr in ("sidn_color", "frame_sidn_color", "diff_color"):
        c = getattr(fields, attr)
        setattr(p, attr, (c.r, c.g, c.b, c.a))
    p.ground_type, p.color2, p.flags2 = fields.ground_type, s32(fields.color2), s32(fields.flags2)
    p.run_time_data = [s32(v) for v in fields.run_time_data]
    build_preview(mat, multi_uv)
    return mat


class WOW_OT_rebuild_material(bpy.types.Operator):
    bl_idname = "wow.rebuild_material"
    bl_label = "Rebuild WoW material preview"

    @classmethod
    def poll(cls, context):
        return context.material is not None

    def execute(self, context):
        build_preview(context.material)
        return {"FINISHED"}


class WOW_PT_material(bpy.types.Panel):
    bl_label = "WoW WMO material"
    bl_space_type, bl_region_type, bl_context = "PROPERTIES", "WINDOW", "material"

    @classmethod
    def poll(cls, context):
        return context.material is not None and context.material.wow_wmo_material.material_index >= 0

    def draw(self, context):
        p = context.material.wow_wmo_material
        col = self.layout.column()
        for name in ("flags", "shader", "blend_mode", "texture1", "texture2", "texture3", "sidn_color",
                     "frame_sidn_color", "diff_color", "ground_type", "color2", "flags2"):
            col.prop(p, name)
        col.operator("wow.rebuild_material")


classes = (WOW_OT_rebuild_material, WOW_PT_material)
