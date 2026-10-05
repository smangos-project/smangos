"""fdid.index loading and the m2/wmo catalogue."""
import json
import os
import re
from pathlib import Path

_GROUP_WMO = re.compile(r"_(\d{3}|lod\d+)\.wmo$")

_state = {"key": None, "by_fdid": {}, "by_path": {}, "entries": []}


def _read_index(content):
    by_fdid, by_path = {}, {}
    with open(Path(content) / "fdid.index", encoding="utf-8", errors="replace") as f:
        for line in f:
            fdid, sep, path = line.rstrip("\r\n").partition(";")
            if sep and fdid.isdigit():
                fdid = int(fdid)
                by_fdid[fdid] = path
                by_path[path.lower()] = fdid
    return by_fdid, by_path


def _scan(content, by_fdid):
    entries = []
    for fdid, path in by_fdid.items():
        low = path.lower()
        if low.endswith(".m2"):
            kind = "m2"
        elif low.endswith(".wmo") and not _GROUP_WMO.search(low):
            kind = "wmo"
        else:
            continue
        if os.path.isfile(os.path.join(content, low)):
            entries.append((fdid, low, kind))
    entries.sort(key=lambda e: e[1])
    return entries


def load(content, cache_dir=None):
    """Load fdid.index and the catalogue; the catalogue is cached as JSON keyed by index mtime and size."""
    content = str(content)
    st = os.stat(os.path.join(content, "fdid.index"))
    key = [content, st.st_mtime_ns, st.st_size]
    if _state["key"] == key:
        return
    by_fdid, by_path = _read_index(content)
    entries = None
    cache = Path(cache_dir) / "catalogue.json" if cache_dir else None
    if cache and cache.is_file():
        try:
            data = json.loads(cache.read_text())
            if data["key"] == key:
                entries = [tuple(e) for e in data["entries"]]
        except (ValueError, KeyError, OSError):
            pass
    if entries is None:
        entries = _scan(content, by_fdid)
        if cache:
            cache.parent.mkdir(parents=True, exist_ok=True)
            cache.write_text(json.dumps({"key": key, "entries": entries}))
    _state.update(key=key, by_fdid=by_fdid, by_path=by_path, entries=entries)


def path_of(fdid):
    return _state["by_fdid"].get(int(fdid))


def fdid_of(path):
    return _state["by_path"].get(path.replace("\\", "/").lower())


def entries(kind=None):
    if kind in (None, "all"):
        return _state["entries"]
    return [e for e in _state["entries"] if e[2] == kind]


def search(query, kind="all", limit=200):
    """Entries whose path contains every whitespace-separated token, or whose fdid equals a numeric query."""
    query = query.strip().lower().replace("\\", "/")
    pool = entries(kind)
    if not query:
        return pool[:limit]
    if query.isdigit():
        hits = [e for e in pool if e[0] == int(query)]
        if hits:
            return hits
    tokens = query.split()
    out = []
    for e in pool:
        if all(t in e[1] for t in tokens):
            out.append(e)
            if len(out) >= limit:
                break
    return out
