#pragma once

// Optional HTTP server that lets a script or an AI see and drive the running client: screenshots, keys, text,
// mouse. Opt-in with `--control` or `--control=PORT` (default 8790), bound to 127.0.0.1 only.
// See docs/ControlServer.md for the endpoints.
void StartControlServerIfRequested();
