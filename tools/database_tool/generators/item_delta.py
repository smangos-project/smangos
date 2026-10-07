#!/usr/bin/env python3
"""Prune the item override migration down to real vmangos departures from db2.

Usage (LD_PRELOAD=build/tools/database_tool/libdoltlite.so.0, from the repo root):
  item_delta.py REFERENCE [MIGRATION] [--vmangos DIR] [--dry-run]

Rewrites the UPDATE statements of MIGRATION (default sql/migrations/20260930210000_world.sql)
in place and drops these cells:
  - AllowableClass / AllowableRace whose playable bits equal db2's (the server only tests the
    playable class / race bits, e.g. db2 -1 and vmangos 511 both mean "everyone");
  - armor and weapon damage that no vmangos item_template migration touches (client db2 wins).
Every other cell stays. Running it again on the pruned file changes nothing.
"""
import argparse, collections, glob, re, sqlite3, sys

RACEMASK_ALL_PLAYABLE = 0xFF
CLASSMASK_ALL_PLAYABLE = 0x5DF
DB2_WINS = {'Resistances1', 'MinDamage1', 'MaxDamage1'}
VCOL = {'Resistances1': 'armor', 'MinDamage1': 'dmg_min1', 'MaxDamage1': 'dmg_max1'}
UPDATE = re.compile(r"UPDATE `dbc_(itemsparse|itemeffect|item)` SET (.*) WHERE `ID` = (\d+);$")
ASSIGN = re.compile(r"`(\w+)` = ('(?:[^']|'')*'|[-\d.e]+)")


def vmangos_touches(root):
    """(entry, column) pairs that vmangos UPDATEs, and entries it re-inserts as full rows."""
    touched, full = set(), set()
    files = glob.glob(f'{root}/sql/old_migrations/*.sql') + glob.glob(f'{root}/sql/migrations/*_world.sql')
    if not files:
        sys.exit(f'no vmangos migrations under {root}')
    for f in files:
        for st in re.split(r';\s*\n', open(f, errors='replace').read()):
            if not re.search(r'item_template', st, re.I):
                continue
            m = re.match(r'\s*update\s+`?item_template`?\s+set\s+(.*?)\s+where\s+(.*)', st, re.I | re.S)
            if m:
                cols = [c.lower() for c in re.findall(r'`?(\w+)`?\s*=', m.group(1))]
                ids = [int(x) for x in re.findall(r'\d+', re.sub(r'patch\D*\d+', '', m.group(2)))]
                touched.update((e, c) for e in ids for c in cols)
                continue
            m = re.match(r'\s*(insert|replace)\s+into\s+`?item_template`?.*?values\s*(.*)', st, re.I | re.S)
            if m:
                full.update(int(e) for e in re.findall(r'\(\s*(\d+)\s*,', m.group(2)))
    return touched, full


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('reference')
    ap.add_argument('migration', nargs='?', default='sql/migrations/20260930210000_world.sql')
    ap.add_argument('--vmangos', default='.local/reference/vmangos')
    ap.add_argument('--dry-run', action='store_true')
    a = ap.parse_args()

    ref = sqlite3.connect(f'file:{a.reference}?mode=ro', uri=True)
    touched, full = vmangos_touches(a.vmangos)

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
        if col in DB2_WINS and (rid, VCOL[col]) not in touched and rid not in full:
            return 'db2'
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
