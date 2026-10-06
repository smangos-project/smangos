import sys, time
import bpy
sys.path.insert(0, __file__.rsplit("/", 1)[0]); sys.path.insert(0, __file__.rsplit("/", 2)[0] + "/spike")
import common
from deep import deep_diff
common.enable()
m2_import = common.submodule("m2_import"); ex = common.submodule("m2_anim_export")
for path in sys.argv[sys.argv.index("--")+1:]:
    top=m2_import.load(path)
    fresh=m2_import.read(path)
    t=time.time(); ex.build(top, fresh.root); print("build", round(time.time()-t,2))
    ref=m2_import.read(path)
    for part in ("sequences","global_loops","sequence_lookups","bones"):
        print(path, part, deep_diff(getattr(ref.root,part), getattr(fresh.root,part), part))
