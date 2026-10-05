"""blender -b --python render_wmo.py -- OUT.png WMO_PATH [azimuth_deg elevation_deg]: imports a WMO and renders it from outside with textures."""
import math
import sys
import time

import addon_utils
import bpy
import mathutils

sys.path.insert(0, __file__.rsplit("/", 1)[0])
import common  # noqa: E402

out, path, *rest = sys.argv[sys.argv.index("--") + 1:]
az, el = (float(rest[0]), float(rest[1])) if rest else (35.0, 30.0)
common.enable()
mod = type("M", (), {"wmo_import": common.submodule("wmo_import")})
t = time.time()
coll = mod.wmo_import.load(path, {"lights": False})
print("import %.1fs" % (time.time() - t))

objs = [o for o in coll.all_objects if o.type == "MESH" and "groups" in [c.name for c in o.users_collection]]
pts = [o.matrix_world @ mathutils.Vector(c) for o in objs for c in o.bound_box]
lo = mathutils.Vector([min(p[i] for p in pts) for i in range(3)])
hi = mathutils.Vector([max(p[i] for p in pts) for i in range(3)])
ctr, size = (lo + hi) / 2, (hi - lo).length
cam = bpy.data.objects.new("cam", bpy.data.cameras.new("cam"))
bpy.context.scene.collection.objects.link(cam)
d = mathutils.Vector((math.cos(math.radians(el)) * math.cos(math.radians(az)),
                      math.cos(math.radians(el)) * math.sin(math.radians(az)), math.sin(math.radians(el))))
cam.location = ctr + d * size * 0.9
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
bpy.ops.render.render(write_still=True)
print("rendered", out)
