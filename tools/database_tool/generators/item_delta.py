#!/usr/bin/env python3
"""Prune the item override migration down to real vmangos departures from db2.

Usage (LD_PRELOAD=build/tools/database_tool/libdoltlite.so.0, from the repo root):
  item_delta.py REFERENCE [MIGRATION] [--dry-run]

Rewrites the UPDATE statements of MIGRATION (default sql/migrations/20260930210000_world.sql)
in place and drops AllowableClass / AllowableRace cells whose playable bits equal db2's (the
server only tests the playable class / race bits, e.g. db2 -1 and vmangos 511 both mean
"everyone"). Every other cell keeps the vmangos item_template value. Running it again on the
pruned file changes nothing.
"""
import argparse, collections, re, sqlite3

RACEMASK_ALL_PLAYABLE = 0xFF
CLASSMASK_ALL_PLAYABLE = 0x5DF
UPDATE = re.compile(r"UPDATE `dbc_(itemsparse|itemeffect|item)` SET (.*) WHERE `ID` = (\d+);$")
ASSIGN = re.compile(r"`(\w+)` = ('(?:[^']|'')*'|[-\d.e]+)")



def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('reference')
    ap.add_argument('migration', nargs='?', default='sql/migrations/20260930210000_world.sql')
    ap.add_argument('--dry-run', action='store_true')
    a = ap.parse_args()

    ref = sqlite3.connect(f'file:{a.reference}?mode=ro', uri=True)

    def client(table, rid, col):
        r = ref.execute(f'SELECT `{col}` FROM cdbc_{table} WHERE ID = ?', (rid,)).fetchone()
        return r and r[0]

    def drop(table, rid, col, val):
        if table != 'itemsparse':
            return None
        if col in ('AllowableClass', 'AllowableRace'):
            mask = CLASSMASK_ALL_PLAYABLE if col == 'AllowableClass' else RACEMASK_ALL_PLAYABLE
            if client(table, rid, col) & mask == int(val) & mask:
                return 'mask'
        return None

    out, dropped = [], collections.Counter()
    for line in open(a.migration).read().split('\n'):
        m = UPDATE.match(line)
        if not m:
            out.append(line)
            continue
        table, sets, rid = m.group(1), m.group(2), int(m.group(3))
        keep = []
        for col, val in ASSIGN.findall(sets):
            why = drop(table, rid, col, val)
            if why:
                dropped[(why, col)] += 1
            else:
                keep.append(f'`{col}` = {val}')
        if keep:
            out.append(f"UPDATE `dbc_{table}` SET {', '.join(keep)} WHERE `ID` = {rid};")

    for (why, col), n in sorted(dropped.items()):
        print(f'{why:5} {col:16} {n}')
    print('dropped', sum(dropped.values()))
    if not a.dry_run:
        open(a.migration, 'w').write('\n'.join(out))


if __name__ == '__main__':
    main()
