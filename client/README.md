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
