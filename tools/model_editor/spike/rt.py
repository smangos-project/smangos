"""Read -> write -> re-read round trip of one WMO/M2 path through wowlib over the loose 1.15 tree."""
import time
from pathlib import Path

from deep import deep_diff
from env import CONTENT, VERSION, wowlib
from wowlib.formats import m2 as m2mod
from wowlib.formats import wmo as wmomod


def _written(out):
    out = Path(out)
    return {str(p.relative_to(out)): p.read_bytes() for p in out.rglob("*") if p.is_file()}


def _clear(out):
    for p in sorted(Path(out).rglob("*"), reverse=True):
        p.unlink() if p.is_file() else p.rmdir()


def _m2_stats(m):
    r = m.root
    return dict(verts=len(r.vertices), bones=len(r.bones), seqs=len(r.sequences),
                tex=len(r.textures), mats=len(r.materials), skins=len(m.skins),
                att=len(r.attachments), ev=len(r.events), cams=len(r.cameras),
                lights=len(r.lights), parts=len(r.particle_emitters), ribbons=len(r.ribbon_emitters))


def _wmo_stats(w):
    r = w.root
    return dict(groups=len(w.groups), mats=len(r.materials), portals=len(r.portals),
                doodads=len(r.doodad_defs), sets=len(r.doodad_sets))


def roundtrip(fs, out, path):
    """Returns dict(status=ok|fail, stage, error, stats, byte_identical, secs)."""
    t0 = time.time()
    res = dict(path=path, status="ok", stage="", error="")
    key = wowlib.FileKey(path)
    try:
        is_wmo = path.endswith(".wmo")
        ent = (wmomod.WMO if is_wmo else m2mod.M2).for_version(VERSION)
        res["stage"] = "read"
        ent.read(fs, key)
        res["stats"] = (_wmo_stats if is_wmo else _m2_stats)(ent)
        res["stage"] = "validate"
        rep = ent.validate()
        res["valid"] = rep.ok
        res["stage"] = "write"
        _clear(out)
        ent.write(fs, key)
        first = _written(out)
        res["written"] = len(first)
        src_identical = True
        for rel, data in first.items():
            src = CONTENT / rel.replace("\\", "/")
            if not src.is_file() or src.read_bytes() != data:
                src_identical = False
        res["byte_identical"] = src_identical
        res["stage"] = "reread"
        ent2 = (wmomod.WMO if is_wmo else m2mod.M2).for_version(VERSION)
        ent2.read(fs, key)
        stats2 = (_wmo_stats if is_wmo else _m2_stats)(ent2)
        if stats2 != res["stats"]:
            raise RuntimeError(f"stats differ after reread: {res['stats']} vs {stats2}")
        res["stage"] = "semantic"
        d = deep_diff(ent.root, ent2.root, "root")
        if d is None:
            d = deep_diff(ent.groups, ent2.groups, "groups") if is_wmo else deep_diff(ent.skins, ent2.skins, "skins")
        if d:
            raise RuntimeError(f"semantic difference at {d}")
        res["stage"] = "rewrite"
        _clear(out)
        ent2.write(fs, key)
        if _written(out) != first:
            raise RuntimeError("second write differs from first (not idempotent)")
        res["stage"] = ""
    except Exception as e:  # noqa: BLE001
        res["status"] = "fail"
        res["error"] = f"{type(e).__name__}: {e}"[:300]
    res["secs"] = round(time.time() - t0, 3)
    return res
