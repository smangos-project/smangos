#pragma once

#include <cstdint>
#include <functional>
#include <string>

struct OverlayColor {
    std::uint8_t r, g, b, a;
};

using OverlayLineSink = std::function<void(float const* from, float const* to, OverlayColor color)>;
// Called every frame with the camera eye in the server world frame; emits world-space lines.
using OverlayProvider = std::function<void(float const* eye, OverlayLineSink const& line)>;

// Draws world-space lines over the client's frames. Starts once the client has loaded D3D12.
void StartOverlay();
void SetOverlayEnabled(bool enabled);
void SetOverlayParameter(std::string const& name, float value);
void AddOverlayProvider(OverlayProvider provider);
