import bpy


def register():
    import wowlib
    print("EXT wowlib", wowlib.versions.classic_era, wowlib.__file__)


def unregister():
    pass
