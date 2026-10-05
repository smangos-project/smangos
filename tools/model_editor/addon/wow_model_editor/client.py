"""The 1.15 client file system session over the extracted Content tree."""
from pathlib import Path

import bpy
import wowlib
from wowlib import fs as wfs

VERSION = wowlib.versions.classic_era

_session = None


def prefs():
    return bpy.context.preferences.addons[__package__].preferences


class Session:
    def __init__(self, content, project):
        self.content = Path(content)
        self.project = Path(project) if project else None
        self.fs = wfs.FileSystem.open(wfs.FileSystemSettings(
            self.content, VERSION,
            project_directory=self.project,
            listfile_csv=self.content / "fdid.index",
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
