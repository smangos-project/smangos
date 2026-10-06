"""FileDataIDs for files created by the editors, kept in a custom listfile shared with the other tools."""
import os
from pathlib import Path

from . import client, index

FIRST_ID = 2_000_000_000


def listfile_path():
    p = client.prefs()
    if p.custom_listfile:
        return Path(p.custom_listfile)
    content = Path(p.content_dir)
    return content.parent.parent / ".local" / "listfile.custom.txt"


def read(path=None):
    """{fdid: path} of the custom listfile."""
    path = path or listfile_path()
    out = {}
    if path.is_file():
        for line in path.read_text(encoding="utf-8").splitlines():
            fdid, sep, name = line.partition(";")
            if sep and fdid.isdigit():
                out[int(fdid)] = name
    return out


def _write(entries, path):
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_name(path.name + ".tmp")
    tmp.write_text("".join(f"{k};{v}\n" for k, v in sorted(entries.items())), encoding="utf-8")
    os.replace(tmp, path)


def allocate(client_path):
    """The fdid of `client_path`: the stock one, an existing custom one, or a new id."""
    key = client_path.replace("\\", "/").lower()
    stock = index.fdid_of(key)
    if stock:
        return stock
    path = listfile_path()
    entries = read(path)
    for k, v in entries.items():
        if v.lower() == key:
            return k
    fdid = max([FIRST_ID - 1, *entries]) + 1
    entries[fdid] = key
    _write(entries, path)
    return fdid
