# smangos

A vmangos-based server for the 1.15 Classic Era client. It reuses the vmangos game logic and speaks the modern client protocol; the world database is a DoltLite file (`.world/world.doltlite`), not MySQL.

## Setup

### 1. Clone repository

```
git clone https://github.com/smangos-project/smangos.git --recurse-submodules
```

### 2. Install dependencies

You need a C++20 compiler, CMake, Ninja, OpenSSL (>= 1.1.1), zlib, libcurl, the MySQL/MariaDB client library, and a MySQL/MariaDB server for the `realmd`, `characters` and `logs` databases (`mangos`/`mangos` on `127.0.0.1:3306` by default, see `.server/config/common.conf`).

<details>
<summary>Arch Linux</summary>

```
sudo pacman -S base-devel git cmake ninja openssl zlib curl mariadb
```
</details>

<details>
<summary>Ubuntu</summary>

```
sudo apt-get install build-essential git cmake ninja-build libssl-dev zlib1g-dev libcurl4-openssl-dev libmysqlclient-dev mariadb-server
```
</details>

<details>
<summary>Windows</summary>

Install Visual Studio 2022 (Desktop development with C++), CMake, Ninja and git, plus the OpenSSL 3 installer to its default location (`C:/Program Files/OpenSSL-Win64`, override with `-DOPENSSL_ROOT_DIR=...`). Run the commands below from a "x64 Native Tools" prompt. Executables then end up in `bin/<Config>` (e.g. `bin/Release`) instead of `build/.../Release`. For `--cdn` in `casc_extractor`, see `dep/windows/optional_dependencies`.
</details>

### 3. Client files

- Place a 1.15 Classic Era client in `.client/_classic_era_/` (so that `.client/_classic_era_/Data` and `.client/_classic_era_/.build.info` exist)
- Place the vanilla 1.12 `.dbc` files in `.client/vanilla/dbc/`

### 4. Build

    cmake -S . -B build -G "Ninja Multi-Config" -DBUILD_CASC_EXTRACTOR=ON -DBUILD_MAP_EXTRACTOR=ON -DBUILD_DATABASE_TOOL=ON
    cmake --build build --config Release

With Ninja Multi-Config every executable is placed in a per-configuration folder, e.g. `build/src/server/Release/server` and `build/tools/map_extractor/Release/map_extractor`. The commands below assume `Release`; substitute `Debug` or `RelWithDebInfo` if you build those.

### 5. Extract CASC

Extract the client's data into `.client/Content/` (you supply your own `LISTFILE` of `<fileDataId>;<path>` lines):

    build/tools/casc_extractor/Release/casc_extractor --listfile LISTFILE [--cdn HOSTS] [--zero-encrypted]

`--cdn` downloads files missing from local storage, `--zero-encrypted` zero-fills db2 sections with unknown keys. Reruns skip finished files; see `client/README.md`.

### 6. Set up the databases

Create the MySQL `realmd`, `characters` and `logs` databases and a user matching `.server/config/common.conf`, then build the world database (`.world/world.doltlite`):

    build/tools/database_tool/Release/database_tool create

Rebuild the client db2 files (originals are backed up in `.world/dbfilesclient-stock`):

    build/tools/database_tool/Release/database_tool export --format db2 --client

See `tools/database_tool/README.md` for the other commands.

### 7. Extract map data

Extract maps, vmaps and mmaps into `.server/data/` (mmaps for Northshire only; the whole world takes hours):

    build/tools/map_extractor/Release/map_extractor --stages map,vmap --threads 16
    build/tools/map_extractor/Release/map_extractor --stages mmap --map 0 --tiles 47,31,49,33 --threads 16

Set `mmap.enabled = 1` in `.server/config/mangosd.conf` to use mmaps. See `tools/map_extractor/README.md`.

### 8. Run

Adjust `.server/config/common.conf` if needed, then start the server from the repo root:

    build/src/server/Release/server

Create an account on the server console, e.g. `account create NAME PASS`.

### 9. Client (optional, Windows or Linux with wine)

See `client/README.md` for the patched launcher, `client/launch-wine.sh` and `client/autologin.sh`. On Windows the launcher is part of the main build (`BUILD_CLIENT`); on Linux it is cross-compiled separately with mingw-w64.
