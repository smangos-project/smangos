"""The 1.15 client file system session over the extracted Content tree."""
from pathlib import Path

import bpy
import wowlib
from wowlib import fs as wfs

VERSION = wowlib.versions.classic_era

_session = None


def prefs():
    return bpy.context.preferences.addons[__package__].preferences


CUSTOM_FDID_START = 2_000_000_000


def working_listfile(content):
    """Copy of fdid.index plus the custom listfile; wowlib appends the ids it allocates to its listfile, so it must not be the shared one."""
    from . import fdids
    content = Path(content)
    src, custom = content / "fdid.index", fdids.listfile_path()
    cache = Path(bpy.path.abspath(prefs().cache_dir)) if prefs().cache_dir else content.parent / ".model-editor-cache"
    cache.mkdir(parents=True, exist_ok=True)
    dst, stamp = cache / "listfile.csv", cache / "listfile.stamp"
    key = "|".join(f"{q.stat().st_mtime_ns}:{q.stat().st_size}" if q.is_file() else "-" for q in (src, custom))
    if not (dst.is_file() and stamp.is_file() and stamp.read_text() == key):
        with open(dst, "w", encoding="utf-8") as out:
            out.write(src.read_text(encoding="utf-8"))
            for fdid, name in sorted(fdids.read(custom).items()):
                out.write(f"{fdid};{name}\n")
        stamp.write_text(key)
    return dst


class Session:
    def __init__(self, content, project):
        self.content = Path(content)
        self.project = Path(project) if project else None
        self.listfile = working_listfile(self.content)
        self.fs = wfs.FileSystem.open(wfs.FileSystemSettings(
            self.content, VERSION,
            project_directory=self.project,
            listfile_csv=self.listfile,
            custom_fdid_start=wowlib.FileDataID(CUSTOM_FDID_START),
            loose_client=True,
        ))

    def close(self):
        if self.fs.is_open:
            self.fs.close()


def dirs():
    p = prefs()
    return p.content_dir, p.output_dir


def session():
    """Open (or reopen after a preference change) the shared file system."""
    global _session
    content, project = dirs()
    if not content:
        raise RuntimeError("Set the Content directory in the add-on preferences")
    content = str(Path(bpy.path.abspath(content)))
    project = str(Path(bpy.path.abspath(project))) if project else ""
    if _session and (str(_session.content), str(_session.project or "")) != (content, project):
        close()
    if _session is None:
        if not (Path(content) / "fdid.index").is_file():
            raise RuntimeError(f"{content} has no fdid.index")
        _session = Session(content, project)
    return _session


def close():
    global _session
    if _session:
        _session.close()
        _session = None


def key(path_or_fdid):
    if isinstance(path_or_fdid, int):
        return wowlib.FileKey(wowlib.FileDataID(path_or_fdid))
    return wowlib.FileKey(path_or_fdid)


def register():
    pass


def unregister():
    close()
