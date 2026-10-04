#pragma once

#include <cstdint>
#include <vector>

// Copies the client's next presented D3D12 frame as top-down BGRX pixels. Hooks Present and
// ExecuteCommandLists on first use. Fails when no frame is presented within the timeout (e.g. minimized).
bool CaptureSwapChain(int& width, int& height, std::vector<std::uint8_t>& pixels);

struct IDXGISwapChain;
struct ID3D12CommandQueue;
struct ID3D12CommandList;

// Runs on the render thread inside Present. `execute` submits lists to the queue without re-entering the hook.
using ExecuteLists = void (*)(ID3D12CommandQueue*, unsigned, ID3D12CommandList* const*);
using PresentCallback = void (*)(IDXGISwapChain*, ID3D12CommandQueue*, ExecuteLists);

bool EnsureSwapChainHooks();
void SetPresentCallback(PresentCallback callback);
