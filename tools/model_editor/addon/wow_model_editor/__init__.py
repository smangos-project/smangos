import importlib

_names = ("prefs", "client", "index", "props", "textures", "materials", "m2_materials", "wmo_import", "wmo_export", "m2_anim", "m2_anim_export", "m2_seq", "m2_import", "ops_import", "ui")
_mods = []


def register():
    import bpy
    for n in _names:
        m = importlib.import_module(f".{n}", __package__)
        _mods.append(m)
        for c in getattr(m, "classes", ()):
            bpy.utils.register_class(c)
        if hasattr(m, "register"):
            m.register()


def unregister():
    import bpy
    for m in reversed(_mods):
        if hasattr(m, "unregister"):
            m.unregister()
        for c in reversed(getattr(m, "classes", ())):
            bpy.utils.unregister_class(c)
    _mods.clear()
