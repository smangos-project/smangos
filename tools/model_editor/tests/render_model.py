"""blender -b --python render_model.py -- OUT.png WMO_OR_M2_PATH [azimuth_deg elevation_deg]: imports a WMO and renders it from outside with textures."""
import math
import sys
import time

import addon_utils
import bpy
import mathutils

sys.path.insert(0, __file__.rsplit("/", 1)[0])
import common  # noqa: E402

out, path, *rest = sys.argv[sys.argv.index("--") + 1:]
seq = int(rest.pop()[4:]) if rest and rest[-1].startswith("seq=") else None
frames = [int(f) for f in rest.pop()[7:].split(",")] if rest and rest[-1].startswith("frames=") else [None]
az, el = (float(rest[0]), float(rest[1])) if rest else (35.0, 30.0)
common.enable()
for o in list(bpy.data.objects):
    bpy.data.objects.remove(o)
t = time.time()
if path.endswith(".m2"):
    coll = common.submodule("m2_import").load(path, {"lights": False})
else:
    coll = common.submodule("wmo_import").load(path, {"lights": False})
print("import %.1fs" % (time.time() - t))
for o in list(coll.all_objects):
    if o.hide_render:
        bpy.data.objects.remove(o)

objs = [o for o in coll.all_objects if o.type == "MESH" and not o.hide_viewport and o.display_type != "WIRE"]
pts = [o.matrix_world @ mathutils.Vector(c) for o in objs for c in o.bound_box]
lo = mathutils.Vector([min(p[i] for p in pts) for i in range(3)])
hi = mathutils.Vector([max(p[i] for p in pts) for i in range(3)])
ctr, size = (lo + hi) / 2, (hi - lo).length
cam = bpy.data.objects.new("cam", bpy.data.cameras.new("cam"))
bpy.context.scene.collection.objects.link(cam)
d = mathutils.Vector((math.cos(math.radians(el)) * math.cos(math.radians(az)),
                      math.cos(math.radians(el)) * math.sin(math.radians(az)), math.sin(math.radians(el))))
cam.location = ctr + d * size * 1.6
cam.rotation_euler = (ctr - cam.location).to_track_quat("-Z", "Y").to_euler()
cam.data.clip_end = size * 4
bpy.context.scene.camera = cam
sc = bpy.context.scene
sc.render.engine = "BLENDER_EEVEE"
sc.render.resolution_x, sc.render.resolution_y = 1280, 800
sc.render.filepath = out
sc.world = bpy.data.worlds.new("w")
sc.world.use_nodes = True
sc.world.node_tree.nodes["Background"].inputs[0].default_value = (0.55, 0.65, 0.8, 1)
sun = bpy.data.objects.new("sun", bpy.data.lights.new("sun", "SUN"))
sun.data.energy = 3
sun.rotation_euler = (math.radians(50), 0, math.radians(30))
sc.collection.objects.link(sun)
if seq is not None:
    common.submodule("m2_seq").play(coll, seq)
for f in frames:
    if f is not None:
        sc.frame_set(f)
    sc.render.filepath = out.replace(".png", f"_{f}.png") if f is not None else out
    bpy.ops.render.render(write_still=True)
    print("rendered", sc.render.filepath)
