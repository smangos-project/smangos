#!/usr/bin/env python3
"""SQL for one client build's db2 hotfixes, applied to the dbc_* tables right after the db2 import.

  hotfix_sql.py HOTFIXES.jsonl WORLD REFERENCE OUT.sql [--build 58797]

HOTFIXES.jsonl is a wago.tools hotfix scrape (one record per line: push_id, record_id, status,
build, table_name, data, locale). Only records of --build count; per (table, record) the latest
push wins, and within a push the non-enUS row, which carries the text (BroadcastText).
Status 1 writes the full row, status 2 deletes it, status 3 (invalid) is skipped.
WORLD supplies the dbc_* column names and types. REFERENCE (cdbc_*, stock db2) is only used for
the self-check: a hotfix row that the stock file already has must convert to exactly that row.
Run with LD_PRELOAD=build/tools/database_tool/libdoltlite.so.0.
"""
import argparse
import collections
import json
import sqlite3
import struct
import sys


def latest_records(path, build):
    recs = {}
    for line in open(path):
        r = json.loads(line)
        if str(r["build"]) != build:
            continue
        k = (r["table_name"], r["record_id"])
        old = recs.get(k)
        if old is None or r["push_id"] > old["push_id"] or (
                r["push_id"] == old["push_id"] and old["locale"] == "enUS"):
            recs[k] = r
    return recs


def f32(v):
    return struct.unpack("<f", struct.pack("<f", float(v)))[0]


def convert(value, kind):
    if kind == "REAL":
        return f32(value)
    if kind == "TEXT":
        return "" if value is None else str(value)
    v = int(value)
    if v >= 2 ** 63:
        v -= 2 ** 64
    return v


def literal(v):
    if isinstance(v, str):
        # Control characters are spelled out: git normalises line endings in .sql files.
        quoted = "'" + v.replace("'", "''") + "'"
        for ch in "\r\n":
            quoted = quoted.replace(ch, f"' || char({ord(ch)}) || '")
        return quoted
    if isinstance(v, float):
        return repr(v)
    return str(v)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("hotfixes")
    ap.add_argument("world")
    ap.add_argument("reference")
    ap.add_argument("out")
    ap.add_argument("--build", default="58797")
    a = ap.parse_args()

    world = sqlite3.connect(f"file:{a.world}?mode=ro", uri=True)
    ref = sqlite3.connect(f"file:{a.reference}?mode=ro", uri=True)
    world_tables = {r[0] for r in world.execute("SELECT name FROM sqlite_master WHERE type='table'")}
    ref_tables = {r[0] for r in ref.execute("SELECT name FROM sqlite_master WHERE type='table'")}

    counts = collections.Counter()
    skipped_tables = collections.Counter()
    mismatches = []
    lines = []
    for (tn, rid), r in sorted(latest_records(a.hotfixes, a.build).items()):
        t = "dbc_" + tn.lower()
        if t not in world_tables:
            skipped_tables[tn] += 1
            continue
        if r["status"] == 3:
            counts["invalid"] += 1
            continue
        if r["status"] == 2:
            lines.append(f'DELETE FROM "{t}" WHERE "ID" = {rid};')
            counts["delete"] += 1
            continue
        cols = world.execute(f'PRAGMA table_info("{t}")').fetchall()
        data = list(r["data"])
        if len(data) != len(cols):
            sys.exit(f"{tn} {rid}: {len(data)} values for {len(cols)} columns")
        if data[0] in ("", None):
            data[0] = rid
        row = [convert(v, c[2]) for v, c in zip(data, cols)]
        if "c" + t in ref_tables:
            stock = ref.execute(f'SELECT * FROM "c{t}" WHERE "ID" = ?', (rid,)).fetchone()
            if stock is not None and tuple(stock) != tuple(row):
                diff = [(c[1], s, h) for c, s, h in zip(cols, stock, row) if s != h]
                if all(type(s) is type(h) for _, s, h in diff):
                    counts["change"] += 1
                else:
                    mismatches.append((tn, rid, diff))
            else:
                counts["equal" if stock is not None else "insert"] += 1
        names = ", ".join(f'"{c[1]}"' for c in cols)
        values = ", ".join(literal(v) for v in row)
        lines.append(f'INSERT OR REPLACE INTO "{t}" ({names}) VALUES ({values});')

    for tn, rid, diff in mismatches[:20]:
        print("type mismatch", tn, rid, diff[:5])
    if mismatches:
        sys.exit(f"{len(mismatches)} rows convert to other types than the stock rows")

    with open(a.out, "w") as out:
        out.write(f"-- db2 hotfixes of client build {a.build}, generated from the wago.tools hotfix scrape\n")
        out.write("-- by tools/database_tool/generators/hotfix_sql.py. Do not edit by hand.\n")
        for line in lines:
            out.write(line + "\n")
    print(dict(counts), "skipped tables:", dict(skipped_tables))


if __name__ == "__main__":
    main()
