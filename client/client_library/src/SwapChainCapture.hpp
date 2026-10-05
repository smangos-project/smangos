#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>
#include <string>
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

// The depth buffer the client rendered its scene with, as seen through the hooked command lists.
struct SceneDepth {
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    D3D12_DEPTH_STENCIL_VIEW_DESC view = {};
    bool hasView = false;
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    UINT width = 0;
    UINT height = 0;
};

// The depth buffers bound during the previous frame, most used first; false before one has been seen.
bool GetSceneDepths(std::vector<SceneDepth>& depths);

// The client's finished world image and its depth, before the UI is drawn over it.
struct SceneTargets {
    Microsoft::WRL::ComPtr<ID3D12Resource> color;
    D3D12_RESOURCE_STATES colorState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    SceneDepth depth;
    bool hasDepth = false;
};

// Runs on the submitting thread once a frame, right before the command list that draws to the back buffer (the world
// composite and the UI) executes. Leave both resources in the states given.
using SceneCallback = void (*)(ID3D12CommandQueue*, SceneTargets const&, ExecuteLists);
void SetSceneCallback(SceneCallback callback);

// Records the command lists, render targets, draws and submissions of the next whole frame as text.
bool TraceFrame(std::string& text);
