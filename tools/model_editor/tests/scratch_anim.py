import sys, time
import bpy
sys.path.insert(0, __file__.rsplit("/", 1)[0])
import common
common.enable()
m2_import = common.submodule("m2_import"); m2_seq=common.submodule("m2_seq")
for path in sys.argv[sys.argv.index("--")+1:]:
    t=time.time()
    top=m2_import.load(path)
    print(path, "import", round(time.time()-t,2), len(top.wow_m2.sequences), len(bpy.data.actions))
    a=next(s for s in top.wow_m2.sequences if s.action)
    print(a.action.name, [ (s.identifier) for s in a.action.slots], len(a.action.layers[0].strips[0].channelbags[0].fcurves))
    m2_seq.play(top,0)
    ob=[o for o in top.objects if o.type=='ARMATURE'][0]
    print(ob.animation_data.action, ob.animation_data.action_slot, [t.name for t in ob.animation_data.nla_tracks])
