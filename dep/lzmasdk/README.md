# LZMA SDK

Igor Pavlov's public-domain LZMA SDK, used to unpack the pinned world base dump
(`.local/plan/wms/database_tool/world-sql.md`). Only the files reachable from the SDK's own
`C/Util/7z/7zMain.c` ("7zDec", a reference command-line 7z decoder) are built: enough to open a
`.7z` archive and extract a file, nothing that encodes one.

## Pin: the `ip7z/7zip` GitHub mirror, not 7-zip.org

The LZMA SDK's own distribution on 7-zip.org is only offered as a `.7z` archive (`lzma2301.7z` at
the time of writing), which can't bootstrap itself on a machine with no 7-Zip already installed.
`ip7z/7zip` is the project's own GitHub mirror (its post-2024 home for tagged source releases) and
ships the same `C/` tree as a plain, git-cloneable/tarball-able directory. `CMakeLists.txt` pins tag
`24.09` via `FetchContent` with a `URL_HASH`. Bumping the pin means updating both.

## `Extract7z.h`/`.c`: the only files not vendored from upstream

`Extract7z.c` adapts the extraction loop from `C/Util/7z/7zMain.c` down to "extract the first
non-directory file to a given path," dropping everything else that example handles (multiple
files, full paths, listing, testing, Unicode filename conversion, timestamps). It's a plain C file
so the SDK's own headers -- `7zFile.h` unconditionally pulls in `<Windows.h>` (via `7zWindows.h`)
on Windows -- never reach `WorldArchive.cpp` or any other C++ translation unit in
`tools/database_tool`. `Extract7z.h` itself only includes `<stddef.h>`.

## What's built

Decode-only: `7zArcIn`/`7zDec` (archive parsing and codec dispatch), `LzmaDec`/`Lzma2Dec` (the
common case), `Bcj2`/`Bra`/`Bra86`/`BraIA64`/`Delta` (the filters 7z solid blocks can use), and
`Ppmd7`/`Ppmd7Dec` (the other codec `7zDec.c` can dispatch to). No encoder, no CLI, no multi-volume
or encrypted-archive support -- none of it is needed for the one plain LZMA-compressed, single-file
archive this tool unpacks.
