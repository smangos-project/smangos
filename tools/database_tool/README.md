# database_tool

Creates and updates every server database the way vmangos documents it, except the world
database, which is a DoltLite (versioned SQLite) file at `.world/world.doltlite` instead of a
MySQL schema. The server itself doesn't use `.world` yet; this tool exists to build and validate
it on its own. `logon`, `characters` and `logs` stay ordinary MySQL databases.

See `.local/plan/wms/database_tool/plan.md`, `world-sql.md` and `dbc-formats.md` for the full
design. This file is just the command reference.

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
database_tool create   [--databases world] [--no-dbc-import]
database_tool update   [--databases world]
database_tool import   --format db2|dbc [--tables A,B]
database_tool export   --format db2|dbc [--tables A,B]
database_tool port-migration <file>...
database_tool compare-world --mysql "<host;port;user;pass;db>"
```

Exit codes: `0` success, `1` failure, `2` for `port-migration` when any file was flagged.

`status` (plan.md's command set) isn't implemented yet, and neither is a `--config` flag: `create`
and `update` only drive `world`; `logon`/`characters`/`logs` aren't wired into the CLI yet (see
`Commands/CreateMysqlDatabase.h` to drive them directly against a `mangosd.conf`-style file).

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
recreating and repopulating that table's `dbc_<name>` and its `dbc_files` bookkeeping row.
`export` does the reverse, rebuilding the file from `dbc_<name>`. `--format db2` always writes
WDC5, one section, every field uncompressed at its natural width, sorted by id (dbc-formats.md);
sparse layout is kept for the four tables that need it. `--format dbc` writes plain WDBC (no
compression or sections at all). A table with no file (`import`) or no `dbc_files` row (`export`)
is reported, not an error.

### port-migration

Mechanically converts one or more legacy MySQL-dialect `_world` migration files (the
`CREATE PROCEDURE`/`DELIMITER` envelope, `@variable`s, escapes, etc.) to the plain-SQLite dialect
`database_tool update` runs, in place. `ALTER TABLE`, a multi-table `UPDATE`, or anything it can't
evaluate is left untouched and reported (exit `2`); those need a hand port (world-sql.md).

### compare-world

Diffs every table `.world` and a MySQL `mangos` database have in common: table sets, column
names/order, row counts, and the first differing row per table (by primary key, or by every
column if a table has none). Floats are compared as float32. Exit `0` only if identical.

## Testing

`ctest`/`tests-database-tool` covers everything with fixtures; three suites are opt-in, since they
need real external resources:

- `DATABASE_TOOL_TEST_MYSQL_CONNECTION="host;port;user;pass"` (no database name) -- the
  `[mysql]`-tagged tests, which create and drop their own `test_*` databases.
- `DATABASE_TOOL_WORLD_DUMP=1` -- the `[network]`-tagged tests, which download the real ~16 MB
  base dump and import it.
- `DATABASE_TOOL_DBFILESCLIENT=<path>` -- a copy of the real `dbfilesclient` directory; checks
  that every file with a resolved definition imports with no error, and that writing it back out
  and re-reading it reproduces the same rows.
