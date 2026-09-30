# Classic client loader

Starts the retail-engine Classic Era client (`WowClassic.exe`) against this server.

- `client_loader/` -> `custom_launcher.exe`: starts the client suspended and injects `custom.dll` before any client code runs.
- `client_library/` -> `custom.dll`: the in-process patches and hooks (cert bypass, CASC bypass, client patches, net diagnostics), built on `hook_lib` + MinHook.
- `common/` : header shared by both (launcher <-> dll protocol, dll-only command line options).

## Build (MSVC, x64)

    cmake -S client -B client/build -A x64
    cmake --build client/build --config Release

Copy `custom_launcher.exe` and `custom.dll` next to `WowClassic.exe` and run the launcher; `WowClassic.exe.custom.log` is written next to the client.
See `client_library/docs/CascBypass.md` for the CASC bypass details.
