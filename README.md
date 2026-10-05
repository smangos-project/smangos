# smangos

A vmangos-based server for the 1.15 Classic Era client. It reuses the vmangos game logic and speaks the modern client protocol; the world database is a DoltLite file (`.world/world.doltlite`), not MySQL.

## Setup

### 1. Client files

Put a stock 1.15 Classic Era client in `.client/_classic_era_/` (next to `.build.info` and `Data/`), plus the vanilla 1.12 `.dbc` files in `.client/vanilla/dbc/`. Then extract the client's data into `.client/Content/` (you supply your own `LISTFILE` of `<fileDataId>;<path>` lines):

    cmake -S . -B build -DBUILD_CASC_EXTRACTOR=ON
    cmake --build build --target casc_extractor
    build/tools/casc_extractor/casc_extractor --listfile LISTFILE [--cdn HOSTS] [--zero-encrypted]

`--cdn` downloads files missing from local storage, `--zero-encrypted` zero-fills db2 sections with unknown keys. Reruns skip finished files; see `client/README.md`.

### 2. Map data

Extract maps, vmaps and mmaps into `.server/data/` (mmaps for Northshire only; the whole world takes hours):

    cmake -S . -B build -DBUILD_MAP_EXTRACTOR=ON
    cmake --build build --target map_extractor
    build/tools/map_extractor/map_extractor --stages map,vmap --threads 16
    build/tools/map_extractor/map_extractor --stages mmap --map 0 --tiles 47,31,49,33 --threads 16

Set `mmap.enabled = 1` in `.server/config/mangosd.conf` to use mmaps. See `tools/map_extractor/README.md`.

### 3. Build

Build the server and the database tool (pass `-DSERVER_MAP_DATA_DIR=<folder>` if the data is not in `.server/data`):

    cmake -S . -B build
    cmake --build build -j --target server_setup database_tool

### 4. World database

Download the pinned vmangos dump, apply the migrations and import the client tables (several minutes; `tools/database_tool/README.md` has the rest):

    build/tools/database_tool/database_tool create

Make the client load the server's own db2 files (originals are backed up in `.world/dbfilesclient-stock`):

    build/tools/database_tool/database_tool export --format db2 --client

### 5. Run

Edit `.server/config/common.conf`, then:

    build/src/server/server

### 6. Client (optional, Windows or Linux with wine)

See `client/README.md` for the patched launcher, `client/launch-wine.sh` and `client/autologin.sh`.

## Tests

    cmake --build build --target bot_tests

Runs the RealBots unit tests and the conformance suite against `.server`. See `src/game/RealBots/README.md`.
