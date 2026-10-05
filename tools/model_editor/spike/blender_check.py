"""Run inside `blender -b --python blender_check.py`: wowlib import, 1.15 round trips, numpy -> mesh transfer."""
import sys
import tempfile
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import bpy  # noqa: E402
import numpy as np  # noqa: E402

import rt  # noqa: E402
from env import CONTENT, VERSION, open_fs, wowlib  # noqa: E402
from wowlib.formats import m2 as m2mod, wmo as wmomod  # noqa: E402

print("BLENDER", bpy.app.version_string, "numpy", np.__version__, "py", sys.version.split()[0])
BIG_WMO = "world/wmo/kalimdor/ogrimmar/ogrimmar.wmo"
out = tempfile.mkdtemp(prefix="wl-bl-")
fails = 0
with open_fs(out) as fs:
    for p in ["world/wmo/cameron.wmo", "world/wmo/kalimdor/ogrimmar/kl_pvpbarracks.wmo",
              "creature/wolf/wolf.m2", "character/human/male/humanmale.m2"]:
        r = rt.roundtrip(fs, out, p)
        print(p, r["status"], r["stage"], r["error"], r.get("stats"))
        fails += r["status"] != "ok"

    m = m2mod.M2.for_version(VERSION)
    m.read(fs, wowlib.FileKey("character/human/male/humanmale.m2"))
    verts = m.root.vertices
    print("vertices type", type(verts).__name__, len(verts))
    v0 = verts[0]
    pos = np.array([(v.pos.x, v.pos.y, v.pos.z) for v in verts], dtype=np.float32)
    print("pos sample", pos[0], "bone idx", list(v0.bone_indices), "weights", list(v0.bone_weights))

    # zero-copy numpy surface: WMO group body arrays
    w = wmomod.WMO.for_version(VERSION)
    w.read(fs, wowlib.FileKey(BIG_WMO))
    body = w.groups[0].body
    for name in ("vertices", "normals", "texcoords", "indices", "batches"):
        a = getattr(body, name)
        arr = np.asarray(a)
        print("wmo", name, type(a).__name__, arr.dtype, arr.shape, "owns" if arr.flags.owndata else "view")

    # mesh transfer speed: 50k verts / 100k tris through foreach_set
    base = np.asarray(body.vertices).view(np.float32).reshape(-1, 3)
    n = 50_000
    co = np.resize(base, (n, 3)) if len(base) else np.zeros((n, 3), np.float32)
    tris = (np.arange(n // 3 * 3, dtype=np.int32)).reshape(-1, 3)
    me = bpy.data.meshes.new("t")
    t0 = time.time()
    me.vertices.add(n)
    me.vertices.foreach_set("co", co.ravel())
    me.loops.add(tris.size)
    me.loops.foreach_set("vertex_index", tris.ravel())
    me.polygons.add(len(tris))
    me.polygons.foreach_set("loop_start", np.arange(0, tris.size, 3, dtype=np.int32))
    me.update()
    print("mesh transfer", n, "verts", round((time.time() - t0) * 1000), "ms")
print("RESULT", "FAIL" if fails else "OK")
