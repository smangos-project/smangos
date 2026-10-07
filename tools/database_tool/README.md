# database_tool

Creates and updates every server database the way vmangos documents it, except the world
database, which is a DoltLite (versioned SQLite) file at `.world/world.doltlite` instead of a
MySQL schema. The server reads it through `WorldDatabase.Info` in `common.conf`; this tool builds
and validates it. `logon`, `characters` and `logs` stay ordinary MySQL databases.

This file is the command reference.

## Building

```
cmake -S . -B build -DBUILD_DATABASE_TOOL=ON -DBUILD_TESTING=ON
cmake --build build --target database_tool
```

Links `shared` (for `DatabaseMysql`), `doltlite` and `lzmasdk`, all fetched automatically. The
DoltLite and MySQL client runtime DLLs are copied next to the built executable.

## Paths

Every path is fixed, compiled in as `DATABASE_TOOL_SOURCE_DIR` (the repo root) -- there are no
directory options:

- World database: `<source root>/.world/world.doltlite`
- World base-dump/archive cache: `<source root>/.world/cache/`
- World migrations: `<source root>/sql/migrations/*_world.sql`
- MySQL migrations: `<source root>/sql/migrations/*_<db>.sql`
- Client DB2/DBC files: `<source root>/.client/Content/dbfilesclient/<name>.db2` (or `.dbc`),
  lowercase names

## Commands

```
database_tool create   [--databases world] [--no-dbc-import] [--base-only] [--listfile FILE]
database_tool update   [--databases world]
database_tool import   --format db2|dbc [--tables A,B] [--dir D]
database_tool export   --format db2|dbc [--tables A,B] [--dir D] [--client]
database_tool fdid     import --listfile FILE | import --custom FILE... | export | allocate PATH... | retire PATH...
database_tool port-migration <file>...
database_tool compare-world --mysql "<host;port;user;pass;db>"
database_tool reference
database_tool spec init
database_tool spec set --source <srv|vdbc|new> --table <t> [--column <c>] [--disposition D] [--target T] [--rule R] [--reason "..."] [--reviewed yes|no] [--overwrite]
database_tool verify   [--final] [--update] [--checks V0,V1,...] [--tables a,b]
database_tool verify init [--vdbc Name,Name|all]
```

Exit codes: `0` success, `1` failure, `2` for `port-migration` when any file was flagged.

`status` (plan.md's command set) isn't implemented yet, and neither is a `--config` flag: `create`
and `update` only drive `world`; `logon`/`characters`/`logs` aren't wired into the CLI yet (see
`Commands/CreateMysqlDatabase.h` to drive them directly against a `common.conf`-style file).

### fdid

`file_data_id` in `.world` holds every FileDataID (stock rows from the listfile, custom ids from
2000000000 up; ids are never deleted or reused, `retire` sets a flag). `fdid import --listfile`
loads stock rows (also `create --listfile`), `fdid import --custom` folds in old `custom.index` /
legacy allocation files, `fdid allocate` prints `id;path` for each path (adding missing ones), and
`fdid export` writes `.client/Content/fdid.index` (also run by `export --client`) and removes the
old `custom.index`. Allocation runs in one `BEGIN IMMEDIATE` transaction, so concurrent tools never
get the same id.

No command makes dolt commits; committing `.world` is left to the user.

### create

Refuses if `.world/world.doltlite` already exists (delete `.world/` to recreate it). Downloads and
checks the pinned base dump into `.world/cache/`, imports it, applies every pending `_world`
migration, creates the `dbc_*` tables from the generated metadata, then imports every resolved
`dbfilesclient/*.db2` unless `--no-dbc-import` is given.

### update

Applies pending `_world` migrations to an existing `.world`.

### import / export

World only. `import` reads `dbfilesclient/<fileStem>.db2` (WDC5) or `.dbc` (WDBC) for every table
with a resolved WoWDBDefs definition (or just the ones named in `--tables`, comma-separated),
recreating and repopulating that table's `dbc_<name>` and its `dbc_files` bookkeeping row;
`--dir` reads another directory. `export` does the reverse, writing each file from `dbc_<name>`
into `.world/dbfilesclient/` (or `--dir`). `--format db2` always writes
WDC5 with one section, sorted by id (dbc-formats.md), laid out the way the client's own files
are: numeric scalars Immediate, numeric arrays after a compressed field PalletArray, everything
else uncompressed (the client misreads uncompressed fields placed after compressed ones). Int
fields keep their declared width unless a value doesn't fit it signed or unsigned. Any relation
field gets a parent lookup, inline or not. Sparse layout is kept for the four tables that need
it. `--format dbc` writes plain WDBC (no
compression or sections at all). A table with no file (`import`) or no `dbc_files` row (`export`)
is reported, not an error.

`export --format db2 --client` writes straight into the client's `.client/Content/dbfilesclient/`
(where `import` reads from) and only touches files whose rows differ from what the file already
holds, so stock files keep their bytes. Before overwriting a file for the first time it copies the
original to `.world/dbfilesclient-stock/`; copy those back to restore the stock client.

### port-migration

Mechanically converts one or more legacy MySQL-dialect `_world` migration files (the
`CREATE PROCEDURE`/`DELIMITER` envelope, `@variable`s, escapes, etc.) to the plain-SQLite dialect
`database_tool update` runs, in place. `ALTER TABLE`, a multi-table `UPDATE`, or anything it can't
evaluate is left untouched and reported (exit `2`); those need a hand port (world-sql.md).

### compare-world

Diffs every table `.world` and a MySQL `mangos` database have in common: table sets, column
names/order, row counts, and the first differing row per table (by primary key, or by every
column if a table has none). Floats are compared as float32. Exit `0` only if identical.

### reference

Builds `.world/reference.doltlite` from three pinned,
never-edited inputs: `.world/base.doltlite` (the pristine pinned dump, made by `create --base-only`, which applies no migrations and writes no dbc; every non-`dbc_`/bookkeeping table
renamed `srv_<table>`, collapsed to the vanilla build/patch), `.client/vanilla/dbc/*.dbc`
(`vdbc_<table>`, read with the 1.12 WDBC layout) and `dbfilesclient/*.db2` (`cdbc_<table>`, the
existing WDC5 import, untouched). Refuses if `reference.doltlite` already exists, like `create`. A
table that fails to build is reported per-table rather than failing the whole run (exit `1` if any
did).

`tools/database_tool/rebuild-check.sh` runs the whole clean-room sequence (base world, reference, create, `verify --final`).

### spec

Inventories the reference database's columns into
`Spec/columns.tsv` and `Spec/dbc-columns.tsv` (checked into the repo and hand-reviewed over time,
unlike everything else this tool writes), then helps fill them in bulk.

- `spec init`: reads `.world/reference.doltlite` (run `reference` first) and appends any column not
  already listed -- every `srv_*`/`vdbc_*` column to `columns.tsv`, and every `cdbc_*` column of a
  table with a same-named `vdbc_*` table (i.e. one that can receive a vanilla value) to
  `dbc-columns.tsv`. Idempotent: never rewrites a row that already has a value, and a row whose
  column the reference database no longer has is reported, not deleted.
- `spec set`: bulk-fills matching rows of `columns.tsv` (`--source`/`--table` required, `--column`
  optional -- omitted means every column of that table). Skips a row whose `disposition` is already
  set unless `--overwrite` is given.

### verify

Checks `.world/world.doltlite` against `.world/reference.doltlite` and the spec (plan.md 6.2). Exit
`0` only when no check reports a problem.

- V0: the world's tables and server columns against `Spec/tables.tsv`/`columns.tsv`.
- V1: `Verify/srv/<table>.sql` rebuilds each `srv_` table (columns `drop` in `columns.tsv` may be
  left out), up to the exact differing-row counts in `Verify/srv/allow.tsv`.
- V2: `Verify/vdbc/<Name>.sql` rebuilds each `vdbc_` table, up to the exact differing-row counts in
  `Verify/vdbc/allow.tsv` (`(missing)` for missing rows). Rows only the world has are fine. A table
  whose columns are all `drop` needs no mapping; one without a mapping yet is a note.
- V3: every `dbc_` row that differs from the stock client (`cdbc_`) is counted in
  `Verify/client-delta.tsv`.
- V4: every client-metadata foreign key and every server reference declared in
  `Verify/references.tsv` resolves, up to the dangling counts listed there.
- V5: no server column equals a `dbc_` column in every row of a `pairs.tsv` id join.

V0 describes the finished database: without `--final` it only prints a count. `--update`
rewrites the measured lists (`client-delta.tsv`, `references.tsv`) instead of checking them.

A mapping file is one SELECT with a `-- key: a, b` line (absent: rows match on every column).
`verify init` writes an identity mapping (collapsed to 5875/patch 10) for every `srv_` table without
one, and with `--vdbc` the mapping `columns.tsv` implies for the named vanilla tables: each kept
column read back from its `dbc_` target through the rule's inverse.

## Departures from db2

`create` builds `dbc_*` from the stock 1.15 db2 files plus `sql/hotfixes/*.sql`; migrations may
only depart from that for one of four reasons:

- F: a vmangos fix (its value differs from vanilla 1.12), e.g. `spell_mod`/`spell_effect_mod`
  cells in 20261001010000, item cells in 20260930210000, raid `CorpseMapID` -1, cones.
- R: a row the client lacks that vmangos content or code references (1,384 spells and their
  sub-rows, 114 items, areatriggers, sound kits, display infos, mail template, skill line 762).
- S: vmangos C++ needs the vanilla meaning of an id or bit. Each entry names the code; changing
  that code retires the entry.
- G: spell gameplay. On every column `SpellMgr::LoadSpells` reads, a vmangos spell keeps its vmangos
  value even where that equals 1.12 (`LOADED` in `generators/spell_delta.py`), and effects db2
  adds to it are cleared. Classic changed balance (honor items, NPC spells), procs (Seal Fate),
  spellmod semantics (extra effects) and spell families there. The server therefore loads the same
  spell data as vmangos; db2 supplies text, visuals and the columns it doesn't read.

| Table / ids | Migration | Code that needs it |
|---|---|---|
| `chatchannels` 24 `Flags` 0 | 20260930010000 | `Channel.cpp` treats flag 0x40000 as an LFG channel |
| `transportanimation` 176080-176085 `Pos1/2` | 20260930020000 | `TransportMgr::LoadTransportAnimationAndRotation` expects the vanilla axis order |
| `lock` 1647 slot 2 | 20260930060000 | `GameObject`/`Spell` lock checks would accept the second (SoD) key |
| `skillraceclassinfo` `Flags` 0x400 | 20260930060000 | `Player::UpdateSkillsForLevel`, `Player::UpdateSpellTrainedSkills` test `SKILL_FLAG_MONO_VALUE` |
| `spellitemenchantment` 3-5, 25, 26, 283, 284, 523, 525, 1665, 1666, 1669, 1900 | 20260930070000 | SoD runes reuse the ids; vmangos items/spells apply them through `Player::ApplyEnchantment` |
| `factiontemplate` 21, 50, 54, 1014, 1254, 1475 | 20260930090000 | SoD hostility changes; `FactionTemplateEntry::IsFriendlyTo`/`IsHostileTo` |
| `creaturedisplayinfo` `Gender` (546) | 20260930140000 | `creature_display_info_addon` gender, read through `SQLStorages.cpp` |
| `spell`/`spellname` 379, 713, 974, 1234, 1329, 23598, 23920, 30167 | 20261001010000 | Classic reused the ids; vmangos items, creatures and scripts cast the old spell |

`generators/dbc_overrides.py REFERENCE WORLD` lists every departure cell with its vanilla and
vmangos values; V3 (`Verify/client-delta.tsv`) counts them per table.

## Testing

`ctest`/`tests-database-tool` covers everything with fixtures; four suites are opt-in, since they
need real external resources:

- `DATABASE_TOOL_TEST_MYSQL_CONNECTION="host;port;user;pass"` (no database name) -- the
  `[mysql]`-tagged tests, which create and drop their own `test_*` databases.
- `DATABASE_TOOL_WORLD_DUMP=1` -- the `[network]`-tagged tests, which download the real ~16 MB
  base dump and import it.
- `DATABASE_TOOL_VERIFY=1` -- runs `verify` on `.world/world.doltlite` and
  `.world/reference.doltlite`.
- `DATABASE_TOOL_DBFILESCLIENT=<path>` -- a copy of the real `dbfilesclient` directory; checks
  that every file with a resolved definition imports with no error, and that writing it back out
  and re-reading it reproduces the same rows.
