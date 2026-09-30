[![CI Build](https://github.com/vmangos/core/actions/workflows/ci-build.yaml/badge.svg)](https://github.com/vmangos/core/actions/workflows/ci-build.yaml)


# Progressive Vanilla
This project is an independent continuation of the Elysium / LightsHope codebases, focused on delivering the most complete and accurate content progression system possible, including support for the patch appropriate game clients.

### Currently supported builds
- 1.12.1.5875+
- 1.11.2.5464
- 1.10.2.5302
- 1.9.4.5086
- 1.8.4.4878
- 1.7.1.4695
- 1.6.1.4544
- 1.5.1.4449
<!--- 1.4.2.4375
- 1.3.1.4297
- 1.2.4.4222-->

### Project guidelines
- Accuracy: The point of an emulator is to recreate the functionality of that which it is emulating as closely as possible. Therefore any custom behaviour should be behind a config option and off by default.
- DB Scripting: Content should be separate from the core itself, so hardcoding scripts should be avoided where possible. When database scripting functionality is insufficient, we simply expand it.
- Full progression: The ultimate goal of this project is to have complete progression starting from patch 1.2 through 1.12. This means every piece of data must be marked with the patch in which it was added or changed to it's current state.
- Tools are great: Content creation should not require programming knowledge. We hope to eventually provide tools that allow for user-friendly editing of database scripts and content, with all data presented in human-readable form.

### Running the server

The server runs from `.server` (git-ignored): `config/` holds `common.conf` (databases, directories, logging),
`mangosd.conf` and `bnetserver.conf`, `data/` the map data, `logs/` every log.

1. Configure once with `-DSERVER_MAP_DATA_DIR=<your maps folder>` (`vmaps` and `mmaps` next to it are linked too).
2. Build the `server_setup` target. It builds `server` and creates whatever of `.server` is missing; it never overwrites.
3. Edit the database strings in `.server/config/common.conf`.
4. Run `bin/<config>/server` from anywhere. `--services mangosd` or `--services bnetserver` runs only one
   (default: `Server.Services` in `common.conf`); `--server-dir <dir>` uses another server directory.

### Running the bot tests

`cmake --build build --config <config> --target bot_tests` builds `server`, `tests-realbots` and `tests-conformance`, then runs the
RealBots unit tests and the conformance suite against `.server`. It starts the server when nothing listens on the RPC port, creates
the bot accounts listed in `.server/config/tests-conformance.conf` (made by `server_setup`) through its console, and stops it
afterwards; a server that is already running is used as it is. Extra switches (`-Strict`, `-NoServer`) go in `-DBOT_TESTS_ARGS="-Strict"`.

### Downloads
- [![Development Release](https://github.com/vmangos/core/actions/workflows/development-release.yaml/badge.svg)](https://github.com/vmangos/core/releases/tag/latest) Latest development build (Linux, macOS, and Windows)
- [![Development Database Dump](https://github.com/vmangos/core/actions/workflows/development-db-dump.yaml/badge.svg)](https://github.com/vmangos/core/releases/tag/db_latest) Latest MySQL 5.6 development database snapshot; no updates required

### Useful Links
- [Wiki](https://github.com/vmangos/wiki)
- [Discord](https://discord.gg/x9a2jt7)
- [Script Editor](https://github.com/brotalnia/scripteditor)
- [Script Converter](https://github.com/vmangos/ScriptConverter)
