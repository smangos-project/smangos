"""Creature display lookups (CreatureModelData, CreatureDisplayInfo, CreatureDisplayInfoGeosetData)."""
import os
from collections import namedtuple
from pathlib import Path

from . import client, db2

Display = namedtuple("Display", "id model_id textures scale geosets")
_cache = {}


def _table(name):
    s = client.session()
    for base in (s.project, s.content):
        if base is None:
            continue
        path = Path(base) / "dbfilesclient" / f"{name}.db2"
        if path.is_file():
            break
    else:
        raise FileNotFoundError(f"{name}.db2")
    stamp = (str(path), os.stat(path).st_mtime_ns)
    hit = _cache.get(name)
    if hit is None or hit[0] != stamp:
        _cache[name] = hit = (stamp, db2.read(path, name))
    return hit[1]


def clear():
    _cache.clear()


def for_model(fdid):
    """Displays whose CreatureModelData uses the M2 `fdid`, ordered by display id."""
    models = {k for k, v in _table("creaturemodeldata").items() if v[2] == fdid}
    if not models:
        return []
    geosets = {}
    for v in _table("creaturedisplayinfogeosetdata").values():
        geosets.setdefault(v[2], []).append((v[0], v[1]))
    out = []
    for did, v in sorted(_table("creaturedisplayinfo").items()):
        if v[1] in models:
            out.append(Display(did, v[1], tuple(v[25]), v[4], tuple(geosets.get(did, ()))))
    return out


def display(fdid, display_id):
    return next((d for d in for_model(fdid) if d.id == display_id), None)
