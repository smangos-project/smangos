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
database_tool create   [--databases world] [--no-dbc-import] [--base-only]
database_tool update   [--databases world]
database_tool import   --format db2|dbc [--tables A,B] [--dir D]
database_tool export   --format db2|dbc [--tables A,B] [--dir D] [--client]
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
- V1: `Verify/srv/<table>.sql` rebuilds each `srv_` table exactly (columns `drop` in
  `columns.tsv` may be left out).
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
