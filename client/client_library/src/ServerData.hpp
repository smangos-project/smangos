#pragma once

// Streams the server's vmaps and mmaps around the camera and draws them through the overlay.
// SMANGOS_SERVER_DATA names the server data directory. Ctrl+Shift+C toggles collision, Ctrl+Shift+N the navmesh.
void StartServerData();
void SetServerDataMap(int map);
