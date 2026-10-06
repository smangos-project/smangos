"""FileDataIDs for files created by the editors: the shared registry `Content/custom.index` (see registry.py)."""
from pathlib import Path

from . import client, index, registry

FIRST_ID = registry.FIRST_ID
read_file = registry.read


def registry_path():
    """The registry file; legacy allocation files next to the default location are merged into it once."""
    p = client.prefs()
    if p.custom_listfile:
        return Path(p.custom_listfile)
    content = Path(p.content_dir)
    path = content / "custom.index"
    legacy = [content.parent.parent / ".local" / "listfile.custom.txt", content / "wms.index", content / "wms-custom-fdids.txt"]
    if any(f.is_file() for f in legacy):
        for c in registry.merge_legacy(path, legacy):
            print("fdid registry conflict:", c)
        client.close()
    return path


listfile_path = registry_path


def read():
    """{fdid: path} of the registry."""
    return registry.read(registry_path())


def path_of(fdid):
    return index.path_of(fdid) or read().get(fdid)


def allocate(client_path):
    """The fdid of `client_path`: the stock one, an existing custom one, or a new id."""
    key = registry.normalize(client_path)
    stock = index.fdid_of(key)
    if stock:
        return stock
    path = registry_path()
    known = registry.read(path)
    fdid = registry.allocate(path, key)
    if fdid not in known:
        client.close()
    return fdid


def release(client_paths):
    """Drops registry entries (for new files that were uninstalled)."""
    gone = registry.release(registry_path(), client_paths)
    if gone:
        client.close()
    return gone


def adopt_session_ids():
    """Copies ids wowlib allocated while writing (custom range) into the registry."""
    s = client.session()
    new = {}
    for line in s.listfile.read_text(encoding="utf-8").splitlines():
        fdid, sep, name = line.partition(";")
        if sep and fdid.isdigit() and int(fdid) >= FIRST_ID:
            new[int(fdid)] = name
    added = registry.add(registry_path(), new)
    if added:
        client.close()
    return added
