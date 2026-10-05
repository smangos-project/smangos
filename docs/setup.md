# Setup

## 1. Clone repository

```
git clone https://github.com/smangos-project/smangos.git --recurse-submodules
```

## 2. Install dependencies

<details>
<summary>Arch Linux</summary>

```
sudo pacman -S base-devel git clang cmake ninja openssl zlib curl mariadb
```
</details>

<details>
<summary>Ubuntu</summary>

```
sudo apt-get install build-essential git clang cmake ninja-build libssl-dev zlib1g-dev libcurl4-openssl-dev libmysqlclient-dev mariadb-server
```
</details>

<details>
<summary>Windows</summary>

- [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) (or the Build Tools), with the "Desktop development with C++" workload and the "C++ Clang tools for Windows" component (provides `clang-cl`)
- [Git for Windows](https://git-scm.com/download/win)
- [CMake](https://cmake.org/download/) (add it to PATH)
- [Ninja](https://github.com/ninja-build/ninja/releases) (add it to PATH)
- [OpenSSL 3 (64-bit)](https://slproweb.com/products/Win32OpenSSL.html), installed to the default `C:/Program Files/OpenSSL-Win64` (or pass `-DOPENSSL_ROOT_DIR=...`)
- Optional, for `casc_extractor --cdn`: libcurl, see `dep/windows/optional_dependencies`

Run the build commands from an "x64 Native Tools Command Prompt for VS 2022".
</details>

## 3. Client files

- Place a 1.15 Classic Era client in `.client/_classic_era_/` (so that `.client/_classic_era_/Data` and `.client/_classic_era_/.build.info` exist)
- The client executable at `.client/_classic_era_/WowClassic.exe` needs to be dejunked. You are on your own for this step.
- Place the vanilla 1.12 `.dbc` files in `.client/vanilla/dbc/`

## 4. Configure

Configure once with Clang and Ninja Multi-Config.

<details>
<summary>Linux</summary>

    cmake -S . -B build -G "Ninja Multi-Config" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DBUILD_CASC_EXTRACTOR=ON -DBUILD_MAP_EXTRACTOR=ON -DBUILD_DATABASE_TOOL=ON
</details>

<details>
<summary>Windows</summary>

    cmake -S . -B build -G "Ninja Multi-Config" -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DBUILD_CASC_EXTRACTOR=ON -DBUILD_MAP_EXTRACTOR=ON -DBUILD_DATABASE_TOOL=ON

</details>

## 5. Extract CASC

    cmake --build build --config RelWithDebInfo --target casc_extractor

Extract the client's data into `.client/Content/` (you supply your own `LISTFILE` of `<fileDataId>;<path>` lines):

    build/tools/casc_extractor/RelWithDebInfo/casc_extractor --listfile LISTFILE [--cdn HOSTS] [--zero-encrypted]

`--cdn` downloads files missing from local storage, `--zero-encrypted` zero-fills db2 sections with unknown keys. Reruns skip finished files; see `client/README.md`.

## 6. Set up the databases

Create the MySQL `realmd`, `characters` and `logs` databases and a user matching `.server/config/common.conf`, then build the world database (`.world/world.doltlite`):

    cmake --build build --config RelWithDebInfo --target database_tool
    build/tools/database_tool/RelWithDebInfo/database_tool create

Rebuild the client db2 files (originals are backed up in `.world/dbfilesclient-stock`):

    build/tools/database_tool/RelWithDebInfo/database_tool export --format db2 --client

See `tools/database_tool/README.md` for the other commands.

## 7. Extract map data

    cmake --build build --config RelWithDebInfo --target map_extractor

Extract maps, vmaps and mmaps into `.server/data/` (mmaps for Northshire only; the whole world takes hours):

    build/tools/map_extractor/RelWithDebInfo/map_extractor --stages map,vmap --threads 16
    build/tools/map_extractor/RelWithDebInfo/map_extractor --stages mmap --map 0 --tiles 47,31,49,33 --threads 16

Set `mmap.enabled = 1` in `.server/config/mangosd.conf` to use mmaps. See `tools/map_extractor/README.md`.

## 8. Run

    cmake --build build --config Debug --target server

Adjust `.server/config/common.conf` if needed, then start the server from the repo root:

    build/src/server/Debug/server

Create an account on the server console, e.g. `account create NAME PASS`.

## 9. Client (optional, Windows or Linux with wine)

See `client/README.md` for the patched launcher, `client/launch-wine.sh` and `client/autologin.sh`.