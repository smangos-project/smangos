#pragma once

#include <cstdint>
#include <vector>

// Copies the client's next presented D3D12 frame as top-down BGRX pixels. Hooks Present and
// ExecuteCommandLists on first use. Fails when no frame is presented within the timeout (e.g. minimized).
bool CaptureSwapChain(int& width, int& height, std::vector<std::uint8_t>& pixels);
