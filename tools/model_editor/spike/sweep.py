"""Corpus sweep: round-trip every .wmo/.m2 in fdid.index (that exists on disk), tally failures.
Usage: sweep.py [--limit N] [--workers N] [--json OUT]"""
import argparse
import collections
import json
import multiprocessing as mp
import re
import tempfile
import time

import rt
from env import CONTENT, FDID_INDEX, open_fs

_fs = _out = None


def _init():
    global _fs, _out
    _out = tempfile.mkdtemp(prefix="wl-sweep-")
    _fs = open_fs(_out)


def _work(path):
    return rt.roundtrip(_fs, _out, path)


def collect():
    paths = []
    for line in FDID_INDEX.read_text(errors="replace").splitlines():
        p = line.split(";", 1)[1].lower() if ";" in line else ""
        if p.endswith((".wmo", ".m2")) and (CONTENT / p).is_file():
            if p.endswith(".wmo") and re.search(r"_\d{3}(_lod\d)?\.wmo$", p):
                continue  # group file, read with its root
            paths.append(p)
    return sorted(set(paths))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--limit", type=int)
    ap.add_argument("--workers", type=int, default=8)
    ap.add_argument("--json")
    a = ap.parse_args()
    paths = collect()
    if a.limit:
        step = max(1, len(paths) // a.limit)
        paths = paths[::step][: a.limit]
    t0 = time.time()
    with mp.Pool(a.workers, _init) as pool:
        results = pool.map(_work, paths, chunksize=16)
    fails = [r for r in results if r["status"] != "ok"]
    cats = collections.Counter()
    for r in fails:
        msg = re.sub(r"'[^']*'", "'X'", r["error"])
        msg = re.sub(r"\d+", "N", msg)
        cats[(r["stage"], msg[:140])] += 1
    nid = [r for r in results if r["status"] == "ok" and not r.get("byte_identical")]
    kinds = collections.Counter(("wmo" if r["path"].endswith(".wmo") else "m2", r["status"]) for r in results)
    print(f"{len(results)} files in {time.time()-t0:.0f}s; {dict(kinds)}")
    print(f"ok but not byte-identical to source: m2={sum(not r['path'].endswith('.wmo') for r in nid)} wmo={sum(r['path'].endswith('.wmo') for r in nid)}")
    print(f"invalid per validate(): {sum(1 for r in results if r.get('valid') is False)}")
    for (stage, msg), n in cats.most_common():
        print(f"{n:6d}  [{stage}] {msg}")
    if a.json:
        with open(a.json, "w") as f:
            json.dump(results, f)


if __name__ == "__main__":
    main()
