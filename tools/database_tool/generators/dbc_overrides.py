#!/usr/bin/env python3
"""Classify how a world's dbc_* tables depart from stock db2.

  dbc_overrides.py REFERENCE WORLD [--tables t1,t2] [--json out.json] [--tsv out.tsv]

REFERENCE is a reference.doltlite (cdbc_* = stock db2, vdbc_* = 1.12 dbc, srv_* = vmangos dump).
Each changed cell gets the vanilla and vmangos values (vmangos through Spec/columns.tsv) and a label:
  vanilla   world == vanilla, vmangos agrees or has no copy: db2 should win
  vmangos   world == vmangos != vanilla: candidate F
  other     neither: needs review
Added rows are labelled 'vanilla-row' when vdbc_ has the row, 'vmangos-row' when only srv_ has it.
Run with LD_PRELOAD=build/tools/database_tool/libdoltlite.so.0.
"""
import argparse
import collections
import json
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from diff_world import columns, connect, pk_columns, same, tables  # noqa: E402

SPEC = os.path.join(os.path.dirname(__file__), "..", "Spec", "columns.tsv")
RULES = {
    "identity": lambda v: v,
    "area_flag_added_bit": lambda v: v | 0x40000000 if isinstance(v, int) else v,
    "none_parent_zero": lambda v: -1 if v == 0 else v,
}


def srv_mapping():
    """dbc table -> {client column: (srv table, srv column, rule)}, plus the key column per srv table."""
    cols, keys = collections.defaultdict(dict), {}
    for line in open(SPEC):
        f = line.rstrip("\n").split("\t")
        if len(f) < 6 or f[0] != "srv" or f[3] != "dbc" or f[5] not in RULES:
            continue
        table, column = f[4].split(".")
        if column == "ID":
            keys[f[1]] = f[2]
        cols[table[4:]][column] = (f[1], f[2], f[5])
    return cols, keys


def rows_by(db, table, key):
    c = columns(db, table)
    return c, {r[c.index(key)]: r for r in db.execute(f'SELECT * FROM "{table}"')}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("reference")
    ap.add_argument("world")
    ap.add_argument("--tables")
    ap.add_argument("--json", default="/tmp/dbc_overrides.json")
    ap.add_argument("--tsv", default="/tmp/dbc_overrides.tsv")
    a = ap.parse_args()
    R, W = connect(a.reference), connect(a.world)
    rt = tables(R)
    smap, skeys = srv_mapping()
    want = set(a.tables.split(",")) if a.tables else None
    out, lines = {}, []
    for t in sorted(x for x in tables(W) if x.startswith("dbc_")):
        n = t[4:]
        if "cdbc_" + n not in rt or (want and n not in want):
            continue
        key = pk_columns(W, t)
        cw, cr = columns(W, t), columns(R, "cdbc_" + n)
        cols = [c for c in cw if c in cr and c not in key]
        sel = ",".join(f'"{c}"' for c in key + cols)
        q = lambda db, tb: {tuple(r[: len(key)]): r[len(key):] for r in db.execute(f'SELECT {sel} FROM "{tb}"')}
        client, world = q(R, "cdbc_" + n), q(W, t)
        vanilla, vcols = {}, []
        if "vdbc_" + n in rt and len(key) == 1 and key[0] in columns(R, "vdbc_" + n):
            vcols = columns(R, "vdbc_" + n)
            vanilla = {(r[vcols.index(key[0])],): r for r in R.execute(f'SELECT * FROM "vdbc_{n}"')}
        srv = {}
        for col, (st, sc, rule) in smap.get(n, {}).items():
            if st not in srv and st in skeys:
                srv[st] = rows_by(R, "srv_" + st, skeys[st])
        cells = []

        def vm(k, col):
            if col not in smap.get(n, {}) or len(k) != 1:
                return "<none>"
            st, sc, rule = smap[n][col]
            if st not in srv or k[0] not in srv[st][1]:
                return "<none>"
            c, rows = srv[st]
            return RULES[rule](rows[k[0]][c.index(sc)])

        def va(k, col):
            if k not in vanilla or col not in vcols:
                return "<none>"
            return vanilla[k][vcols.index(col)]

        for k in sorted(world.keys() & client.keys()):
            for i, col in enumerate(cols):
                c, w = client[k][i], world[k][i]
                if same(c, w):
                    continue
                v, s = va(k, col), vm(k, col)
                if same(w, v) and (s == "<none>" or same(s, v)):
                    label = "vanilla"
                elif s != "<none>" and same(w, s):
                    label = "vmangos"
                else:
                    label = "other"
                cells.append([list(k), col, c, w, v, s, label])
        added = []
        for k in sorted(world.keys() - client.keys()):
            label = "vanilla-row" if k in vanilla else "vmangos-row" if any(
                len(k) == 1 and k[0] in rows for _, rows in srv.values()) else "other-row"
            added.append([list(k), label])
        removed = [list(k) for k in sorted(client.keys() - world.keys())]
        if cells or added or removed:
            out[n] = {"changed": cells, "added": added, "removed": removed}
            cnt = collections.Counter((c[1], c[6]) for c in cells)
            lines.append(f"{n}\trows +{len(added)} -{len(removed)} "
                         f"{dict(collections.Counter(x[1] for x in added))}\t"
                         + " ".join(f"{c}:{l}={m}" for (c, l), m in cnt.most_common()))
    json.dump(out, open(a.json, "w"), default=str)
    open(a.tsv, "w").write("\n".join(lines) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
