# DoltLite

[DoltLite](https://github.com/dolthub/doltlite) is a SQLite fork that adds Git-style version
control (`dolt_commit`, `dolt_log`, branches, merges, ...) as plain SQL on an ordinary `sqlite3`
connection. `tools/database_tool` uses it for the world database
(`.local/plan/wms/database_tool/plan.md`).

## Decision: FetchContent the NuGet package, not a source build

Spiked both options from the plan. Chose **FetchContent the pinned `DoltHub.Doltlite` NuGet
package** and use its `runtimes/<rid>/native` binaries directly:

- It's the exact binary WorldMapStudio already loads (no second native build to keep in sync).
- Building `libdoltlite` from source means cross-compiling Go with cgo against a vendored SQLite
  fork; the NuGet package already does that for all four platforms we care about.
- The only extra work on Windows is generating an MSVC import library, which is mechanical and
  now automated (see below).

The `ExternalProject` source-build fallback from the plan was not needed.

## What the spike verified

- `doltlite.dll` (win-x64) exports the full `sqlite3_*` C API (306 symbols total) plus
  `doltlite_init_lazy`/`doltlite_set_chunk_source` (remote/lazy chunk loading, not used here) and
  `doltliteServe*` (DoltLite's own SQL server, not used here). No `Doltlite.Init()`-equivalent
  C entry point exists or is needed: that call in the .NET wrapper only points SQLitePCLRaw at
  this native library instead of stock SQLite. A raw `sqlite3_open()` on a fresh file is enough;
  `dolt_commit`/`dolt_log` work immediately as SQL.
- `dumpbin /DEPENDENTS` confirms the plan's claim: the DLL needs `libgcc_s_seh-1.dll` and
  `libwinpthread-1.dll` beyond what Windows already ships (`bcrypt`, `crypt32`, `kernel32`,
  `msvcrt`, `ws2_32`). Both are vendored under `win-x64/`.
- `sqlite3_libversion()` reports `3.54.0`, matching the pinned commit's `VERSION` file. This is
  DoltLite's own fork version, ahead of upstream SQLite's numbering; there is no matching upstream
  `sqlite-amalgamation` release to borrow a header from.
- End-to-end smoke test (`smoketest/`, `doltlite_smoketest`): open a fresh file, `CREATE TABLE`,
  `INSERT`, `SELECT dolt_commit('-A', '-m', ...)`, then read the message back from `dolt_log`.
  Passes when linked against the generated import library.

## sqlite3.h

The NuGet package ships only the .NET wrapper and the native binaries, not a C header. DoltLite's
own header is generated at their build time by `tool/mksqlite3h.tcl` from `src/sqlite.h.in`,
substituting `--VERS--`, `--VERSION-NUMBER--`, `--SOURCE-ID--` (and closing the header guard that
the template leaves for that script to add). `CMakeLists.txt` downloads `src/sqlite.h.in` from the
pinned commit and does the same substitution at configure time, using the version/source-id values
read from that commit's `VERSION`/`manifest`/`manifest.uuid` files. `--SCM-BRANCH--`/`--SCM-TAGS--`/
`--SCM-DATETIME--` are diagnostic-only strings (never compiled against) and get placeholders.

`sqlite3rtree.h`/`sqlite3session.h`/`fts5.h`, which `mksqlite3h.tcl` also appends, were skipped:
the DLL's export table has no `sqlite3session_*`/FTS5 symbols, and the two `sqlite3_rtree_*`
symbols it does export are declared directly in `sqlite.h.in`.

## MSVC import library

DoltLite's win-x64 binary is built with MinGW and ships no `.lib`. MSVC can't link a DLL directly,
so `CMakeLists.txt` generates one at configure time: `dumpbin /EXPORTS` lists the 306 exported
names, `GenerateImportLib.cmake` writes them to a `.def`, and `lib.exe /DEF:... /MACHINE:X64`
produces `doltlite.lib`. This needs `dumpbin.exe`/`lib.exe`, which ship with every MSVC
installation next to `cl.exe`/`link.exe` and are found from `CMAKE_LINKER`'s directory.

## Row-insert timing

A prepared-statement `INSERT` loop in a single transaction, no secondary indexes, on this machine:
1,000,000 rows in 1.27s (~787k rows/s), 40MB file. Comfortably fast enough for
`MysqlDumpImport`'s 912,894-row base dump; no PRAGMA tuning spike needed yet. Revisit if
`compare-world` or `create` turns out slow in practice.
