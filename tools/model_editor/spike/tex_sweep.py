"""Texture FDIDs used by all WMO materials: count the ones that don't decode (JPEG BLP, missing)."""
import tempfile
from collections import Counter

from env import VERSION, open_fs, wowlib
from wowlib.formats import blp, wmo as wmomod
import index_paths

out = tempfile.mkdtemp()
with open_fs(out) as fs:
    used = set()
    for p in index_paths.wmos():
        try:
            w = wmomod.WMO.for_version(VERSION)
            w.read(fs, wowlib.FileKey(p))
        except Exception:  # noqa: BLE001
            continue
        for m in w.root.materials:
            used.update(t for t in (m.texture1, m.texture2, m.texture3) if t)
    why = Counter()
    bad = []
    for t in sorted(used):
        try:
            b = blp.BLP()
            b.read(fs, wowlib.FileKey(wowlib.FileDataID(t)))
            b.decode()
        except Exception as e:  # noqa: BLE001
            why[str(e)[:60]] += 1
            bad.append(t)
    print(len(used), "textures,", len(bad), "undecodable", dict(why), bad[:10])
