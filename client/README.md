# Classic client loader

Starts the retail-engine Classic Era client (`WowClassic.exe`) against this server.

- `client_loader/` -> `custom_launcher.exe`: starts the client suspended and injects `custom.dll` before any client code runs.
- `client_library/` -> `custom.dll`: the in-process patches and hooks (cert bypass, CASC bypass, client patches, net diagnostics), built on `hook_lib` + MinHook.
- `client_common/` : header shared by both (launcher <-> dll protocol, dll-only command line options).

## Build

Part of the root CMake project (`BUILD_CLIENT`, on by default; on Linux see below):

    cmake -S . -B build
    cmake --build build --config Release --target client_library client_loader

Outputs land in `bin/<Config>`. Copy `custom_launcher.exe` and `custom.dll` next to `WowClassic.exe` and run the launcher; `WowClassic.exe.custom.log` is written next to the client.
See `client_library/docs/CascBypass.md` for the CASC bypass details, and `client_library/docs/ControlServer.md` for the optional `--control` HTTP server (screenshots and input).

## Build and run on Linux (mingw-w64 + wine)

The client is still a Windows exe, so on Linux the library and launcher are cross-compiled with mingw-w64 and the client runs under wine; the launcher's APC injection works unchanged under wine, so there is no separate native loader.

    cmake -S client -B build-client -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=Release
    cmake --build build-client -j
    client/launch-wine.sh /path/to/_classic_era_ [client args...]

`client/` builds standalone (no server dependencies). `launch-wine.sh` copies `custom_launcher.exe` and `custom.dll` from `build-client/bin` next to `WowClassic.exe` and starts the launcher in a wine prefix (`~/.local/share/classic-client-wine` by default, `WINEPREFIX` to override; it must not be under a directory you don't own, e.g. `/tmp`). Needs `mingw-w64-gcc` and `wine` (with vkd3d for the client's D3D12 renderer). Verified: custom.dll loads, applies its patches and the client renders under wine 11.

## Extracting a CASC client into Content/

    client/extract-casc.py --listfile LISTFILE [--client .client] [--locale enUS]

Reads the local CASC storage (`.client/.build.info`, `Data/`) and writes every file named in the listfile (`<fileDataId>;<path>` per line, or bare paths) under `.client/Content/`, lowercased. It also writes `fdid.index` and `root.hashes`, which the CASC bypass reads, and lists what it could not extract (encrypted or missing entries) in `Content/_dump_failed.log`. Files already present with the right size are skipped, so an interrupted run can be resumed (`--force` to redo). Python 3 standard library only; `python3 client/test_extract_casc.py` runs its tests.

## Automatic login (Linux)

    client/autologin.sh [--restart-server] [ACCOUNT [PASSWORD [CHARACTER]]]

Gets the client into the world without the login screen (defaults: CLAUDE / claude, the last played character). It asks the server's REST portal for a login ticket, starts the client with `custom_launcher.exe --webtoken=TICKET` and presses Enter until the server reports the character online. A screenshot is saved to `/tmp/autologin.jpg`.

`--webtoken=TICKET` (launcher only) stores the ticket the way the Battle.net launcher does (DPAPI-encrypted `WEB_TOKEN` under `HKCU\Software\Custom Game Server Dev\Battle.net\Launch Options\WoW`) and adds `-launcherlogin`. Launcher login connects to the `CONNECTION_STRING` value in that key, not the `portal` cvar. The server preselects the account's last played realm and character, so the realm list is skipped.
