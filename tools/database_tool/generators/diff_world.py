#!/usr/bin/env python3
"""Cell-level diff of two world databases, and a gate against expected diffs.

  diff_world.py A B [--tables t1,t2] [--key t=c1,c2 ...] [--json out.json]
  diff_world.py A B --expect e1.json [e2.json ...] [--key ...]

Run with LD_PRELOAD=build/tools/database_tool/libdoltlite.so.0 for .doltlite files.
Rows match on the primary key, or on --key columns (then the key columns and ID are not
compared, for tables whose ids get renumbered). Floats compare with relative tolerance 1e-6.
The JSON format, also used for expected files:
  {"<table>": {"added": [key, ...], "removed": [key, ...], "changed": [[key, column, old, new], ...]}}
With --expect, the actual diff must equal the union of the expected files on
(table, kind, key, column) and on the new value; exit 1 lists the surplus and missing entries.
"""
import argparse
import json
import math
import sqlite3
import sys

SKIP_TABLES = {"migrations", "sqlite_sequence", "dbc_files"}


def connect(path):
    return sqlite3.connect(f"file:{path}?mode=ro", uri=True)


def tables(db):
    return {r[0] for r in db.execute("SELECT name FROM sqlite_master WHERE type = 'table'")}


def columns(db, table):
    return [r[1] for r in db.execute(f'PRAGMA table_info("{table}")')]


def pk_columns(db, table):
    info = sorted((r[5], r[1]) for r in db.execute(f'PRAGMA table_info("{table}")') if r[5])
    return [name for _, name in info]


def same(a, b):
    if a == b:
        return True
    if isinstance(a, float) or isinstance(b, float):
        try:
            a, b = float(a), float(b)
        except (TypeError, ValueError):
            return False
        if math.isnan(a) and math.isnan(b):
            return True
        return abs(a - b) <= 1e-6 * max(abs(a), abs(b), 1e-30)
    return False


def load(db, table, key, cols):
    rows = {}
    for r in db.execute(f'SELECT {",".join(f"[{c}]" for c in cols)} FROM "{table}"'):
        rec = dict(zip(cols, r))
        k = tuple(rec[c] for c in key) if key else tuple(r)
        rows[k] = rec
    return rows


def diff_table(a, b, table, key_override):
    cols_a, cols_b = columns(a, table), columns(b, table)
    cols = [c for c in cols_a if c in cols_b]
    key = key_override or pk_columns(a, table)
    ignore = set(key)
    if key_override:
        ignore |= {"ID"}
    ra, rb = load(a, table, key, cols), load(b, table, key, cols)
    out = {"added": [], "removed": [], "changed": []}
    if set(cols_a) != set(cols_b):
        out["columns"] = {"only_a": [c for c in cols_a if c not in cols_b], "only_b": [c for c in cols_b if c not in cols_a]}
    for k in rb.keys() - ra.keys():
        out["added"].append(list(k))
    for k in ra.keys() - rb.keys():
        out["removed"].append(list(k))
    if key:
        for k in ra.keys() & rb.keys():
            x, y = ra[k], rb[k]
            for c in cols:
                if c not in ignore and not same(x[c], y[c]):
                    out["changed"].append([list(k), c, x[c], y[c]])
    for v in out.values():
        if isinstance(v, list):
            v.sort(key=lambda e: json.dumps(e, default=str))
    return out if any(out.get(n) for n in ("added", "removed", "changed", "columns")) else None


def diff(a_path, b_path, only, keys):
    a, b = connect(a_path), connect(b_path)
    ta, tb = tables(a), tables(b)
    result = {}
    for t in sorted(ta - tb):
        result[t] = {"table": "removed"}
    for t in sorted(tb - ta):
        result[t] = {"table": "added"}
    for t in sorted(ta & tb):
        if t in SKIP_TABLES or (only and t not in only):
            continue
        d = diff_table(a, b, t, keys.get(t))
        if d:
            result[t] = d
    return result


def entries(d):
    for t, v in d.items():
        if "table" in v:
            yield (t, "table", v["table"], None), None
            continue
        for k in v.get("added", []):
            yield (t, "added", json.dumps(k), None), None
        for k in v.get("removed", []):
            yield (t, "removed", json.dumps(k), None), None
        for k, c, _, new in v.get("changed", []):
            yield (t, "changed", json.dumps(k), c), new


def main():
    p = argparse.ArgumentParser()
    p.add_argument("a")
    p.add_argument("b")
    p.add_argument("--tables")
    p.add_argument("--key", action="append", default=[], help="table=col1,col2")
    p.add_argument("--json")
    p.add_argument("--expect", nargs="+")
    args = p.parse_args()
    keys = {}
    for k in args.key:
        t, cs = k.split("=", 1)
        keys[t] = cs.split(",")
    only = set(args.tables.split(",")) if args.tables else None
    result = diff(args.a, args.b, only, keys)
    if args.json:
        with open(args.json, "w") as f:
            json.dump(result, f, default=str, indent=0)
    if not args.expect:
        for t, v in result.items():
            if "table" in v:
                print(f"{t}: table {v['table']}")
            else:
                print(f"{t}: +{len(v['added'])} -{len(v['removed'])} ~{len(v['changed'])}" + (f" columns {v['columns']}" if "columns" in v else ""))
        return 1 if result else 0
    expected = {}
    for path in args.expect:
        with open(path) as f:
            for key, new in entries(json.load(f)):
                expected[key] = new
    actual = dict(entries(result))
    surplus = sorted(k for k in actual if k not in expected or (k[1] == "changed" and not same(actual[k], expected[k])))
    missing = sorted(k for k in expected if k not in actual)
    for k in surplus[:50]:
        print("unexpected:", k, actual[k])
    for k in missing[:50]:
        print("missing:", k, expected[k])
    print(f"{len(actual)} actual entries, {len(expected)} expected, {len(surplus)} unexpected, {len(missing)} missing")
    return 1 if surplus or missing else 0


if __name__ == "__main__":
    sys.exit(main())
