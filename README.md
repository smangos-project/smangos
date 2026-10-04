# smangos

A vmangos-based server for the 1.15 Classic Era client. It reuses the vmangos game logic and speaks the modern client protocol; the world database is a DoltLite file (`.world/world.doltlite`), not MySQL.

## Setup

### 1. Client files

Put a stock 1.15 Classic Era client in `.client/_classic_era_/` (the folder with `WowClassic.exe`) and provide these folders in `.client/`:

- `Content/dbfilesclient/`: the client's `.db2` files.
- `vanilla/dbc/`: the vanilla 1.12 `.dbc` files.

`database_tool` checks both against the hashes in `tools/database_tool/Inputs/` and refuses to run on anything else.

### 2. Map data

Extract `maps`, `vmaps` and `mmaps` for the client and note the folder that holds them.

### 3. Build

    cmake -S . -B build -DSERVER_MAP_DATA_DIR=<your maps folder>
    cmake --build build -j --target server_setup database_tool

`server_setup` builds `server` and creates the missing parts of `.server/` (`config/`, `data/`, `logs/`). It never overwrites existing files.

### 4. World database

    build/tools/database_tool/database_tool create

This downloads the pinned vmangos dump into `.world/cache/` (GitHub, with a mirror as fallback), imports it, applies `sql/migrations/` and imports the client's db2 tables into `.world/world.doltlite`. It takes several minutes and refuses to run if the file already exists. See `tools/database_tool/README.md` for the other commands.

To make the client load the server's own db2 files (needed for the riding trainer and other server-side spell changes), run `database_tool export --format db2 --client`. The original files are backed up in `.world/dbfilesclient-stock`.

### 5. Run

    build/src/server/server

Edit the database settings in `.server/config/common.conf` first. `--services mangosd` or `--services bnetserver` starts only one service.

### 6. Client (optional, Windows or Linux with wine)

The patched launcher lets the client connect to this server. See `client/README.md` for building it, `client/launch-wine.sh` for starting it on Linux and `client/autologin.sh` for logging in without the login screen.

## Tests

    cmake --build build --target bot_tests

Runs the RealBots unit tests and the conformance suite against `.server`. See `src/game/RealBots/README.md`.
