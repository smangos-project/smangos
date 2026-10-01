#pragma once

#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

struct CaptureOptions {
    int maxWidth = 1280;
    bool png = false;
    bool hasRegion = false;
    RECT region = {}; // client pixels: left, top, right, bottom
};

struct CaptureResult {
    bool ok = false;
    std::string error;
    std::vector<std::uint8_t> bytes;
    std::string mime;
    int width = 0; // encoded image
    int height = 0;
    double scale = 1.0; // encoded pixels per client pixel
    int originX = 0; // client position of the encoded image's top-left corner
    int originY = 0;
};

// Captures the window's client area (or a region of it), downscaled to maxWidth and encoded as PNG or JPEG.
CaptureResult CaptureWindow(HWND window, CaptureOptions const& options);
