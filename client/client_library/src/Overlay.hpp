#pragma once

#include <cstdint>
#include <functional>
#include <string>

struct OverlayColor {
    std::uint8_t r, g, b, a;
};

using OverlayLineSink = std::function<void(float const* from, float const* to, OverlayColor color)>;
using OverlayTriangleSink = std::function<void(float const* a, float const* b, float const* c, OverlayColor color)>;
// Called every frame with the camera eye in the server world frame; emits world-space lines and faces.
using OverlayProvider =
    std::function<void(float const* eye, OverlayLineSink const& line, OverlayTriangleSink const& triangle)>;

// Draws world-space lines and faces, depth-tested against the client's scene, over the client's frames. Starts once the client has loaded D3D12.
void StartOverlay();
void SetOverlayEnabled(bool enabled);
void SetOverlayParameter(std::string const& name, float value);
void AddOverlayProvider(OverlayProvider provider);
