# Control server

**Code:** `client/client_library/src/ControlServer.cpp`, docs in `client/client_library/docs/ControlServer.md`

An optional HTTP server inside `custom.dll`, started with `custom_launcher.exe --control` (port 8790, `127.0.0.1` only). It lets an agent or script see and drive the running client: screenshots, keys, typed text, mouse input and batches of those. On Linux, `client/launch-wine.sh` runs the client under wine and `client/autologin.sh` logs a character into the world without any clicks.

## What you can do

- Launch the client, wait on `GET /status`, take screenshots and send input.
- Log in and enter the world unattended with `client/autologin.sh`.
- Reach debug endpoints: `/overlay`, `/postprocess`, `/frametrace`, `/mem`, `/dump`.
- Quit with `POST /quit`.

See the [client README](https://github.com/smangos-project/smangos/blob/development/client/README.md) for building and launching.
