# vmap/mmap debug overlay

**Code:** `client/client_library/src/Overlay.cpp`, `client/client_library/src/ServerData.cpp`

Draws the server's collision (vmaps) and navigation mesh (mmaps) as a world-space D3D12 overlay on top of the running client, depth-tested against the client's own scene. The overlay loads the extracted `vmaps/` and `mmaps/` around the camera, three by three tiles at a time. `SMANGOS_SERVER_DATA` sets the data directory.

## What you can do

- Toggle collision with `Ctrl+Shift+C` and the navmesh with `Ctrl+Shift+N`.
- Check where the server thinks the walkable and blocking surfaces are, to debug pathing, line of sight and extractor output.
- Tune it through the control server's `POST /overlay` endpoint (depth test, biases, map, enable).
