#define NOMINMAX
#include "SwapChainCapture.hpp"

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include "hook_lib/Log.hpp"

#include <MinHook.h>

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>

using Microsoft::WRL::ComPtr;

namespace {

using PresentFn = HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain*, UINT, UINT);
using Present1Fn = HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain1*, UINT, UINT, DXGI_PRESENT_PARAMETERS const*);
using ExecuteFn = void(STDMETHODCALLTYPE*)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);

PresentFn OriginalPresent = nullptr;
Present1Fn OriginalPresent1 = nullptr;
ExecuteFn OriginalExecute = nullptr;

std::mutex HookMutex;
bool HooksInstalled = false;

std::mutex StateMutex;
std::condition_variable Done;
ID3D12CommandQueue* DirectQueue = nullptr;
bool Requested = false;
bool Finished = false;
bool Succeeded = false;
int FrameWidth = 0;
int FrameHeight = 0;
std::vector<std::uint8_t> FramePixels;

bool ToBgrx(DXGI_FORMAT format, std::uint8_t const* source, std::uint8_t* target, UINT width)
{
    for (UINT x = 0; x < width; ++x) {
        std::uint8_t const* in = source + x * 4;
        std::uint8_t* out = target + x * 4;
        switch (format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            out[0] = in[2];
            out[1] = in[1];
            out[2] = in[0];
            break;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            out[0] = in[0];
            out[1] = in[1];
            out[2] = in[2];
            break;
        case DXGI_FORMAT_R10G10B10A2_UNORM: {
            std::uint32_t value;
            std::memcpy(&value, in, 4);
            out[2] = static_cast<std::uint8_t>((value & 0x3ff) >> 2);
            out[1] = static_cast<std::uint8_t>(((value >> 10) & 0x3ff) >> 2);
            out[0] = static_cast<std::uint8_t>(((value >> 20) & 0x3ff) >> 2);
            break;
        }
        default:
            return false;
        }
        out[3] = 0xff;
    }
    return true;
}

D3D12_RESOURCE_BARRIER Transition(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = resource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter = after;
    return barrier;
}

bool CopyBackBuffer(IDXGISwapChain* swapChain, ID3D12CommandQueue* queue)
{
    ComPtr<IDXGISwapChain3> swapChain3;
    ComPtr<ID3D12Resource> backBuffer;
    ComPtr<ID3D12Device> device;
    if (!queue || FAILED(swapChain->QueryInterface(IID_PPV_ARGS(&swapChain3))) ||
        FAILED(swapChain3->GetBuffer(swapChain3->GetCurrentBackBufferIndex(), IID_PPV_ARGS(&backBuffer))) ||
        FAILED(backBuffer->GetDevice(IID_PPV_ARGS(&device))))
        return false;

    D3D12_RESOURCE_DESC const desc = backBuffer->GetDesc();
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
    UINT64 total = 0;
    device->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, nullptr, nullptr, &total);

    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type = D3D12_HEAP_TYPE_READBACK;
    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = total;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ComPtr<ID3D12Resource> readback;
    ComPtr<ID3D12CommandAllocator> allocator;
    ComPtr<ID3D12GraphicsCommandList> list;
    ComPtr<ID3D12Fence> fence;
    if (FAILED(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST,
                                               nullptr, IID_PPV_ARGS(&readback))) ||
        FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))) ||
        FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr,
                                         IID_PPV_ARGS(&list))) ||
        FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
        return false;

    D3D12_TEXTURE_COPY_LOCATION destination = {};
    destination.pResource = readback.Get();
    destination.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    destination.PlacedFootprint = footprint;
    D3D12_TEXTURE_COPY_LOCATION source = {};
    source.pResource = backBuffer.Get();
    source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

    D3D12_RESOURCE_BARRIER toCopy = Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT,
                                               D3D12_RESOURCE_STATE_COPY_SOURCE);
    D3D12_RESOURCE_BARRIER toPresent = Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE,
                                                  D3D12_RESOURCE_STATE_PRESENT);
    list->ResourceBarrier(1, &toCopy);
    list->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
    list->ResourceBarrier(1, &toPresent);
    if (FAILED(list->Close()))
        return false;

    ID3D12CommandList* lists[] = {list.Get()};
    OriginalExecute(queue, 1, lists);
    HANDLE event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!event)
        return false;
    bool ok = SUCCEEDED(queue->Signal(fence.Get(), 1)) && SUCCEEDED(fence->SetEventOnCompletion(1, event)) &&
              WaitForSingleObject(event, 2000) == WAIT_OBJECT_0;
    CloseHandle(event);
    if (!ok)
        return false;

    void* mapped = nullptr;
    D3D12_RANGE range = {0, static_cast<SIZE_T>(total)};
    if (FAILED(readback->Map(0, &range, &mapped)))
        return false;
    UINT const width = static_cast<UINT>(desc.Width);
    UINT const height = desc.Height;
    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(width) * height * 4);
    for (UINT y = 0; y < height && ok; ++y)
        ok = ToBgrx(desc.Format, static_cast<std::uint8_t const*>(mapped) + footprint.Offset +
                                     static_cast<std::size_t>(y) * footprint.Footprint.RowPitch,
                    &pixels[static_cast<std::size_t>(y) * width * 4], width);
    D3D12_RANGE written = {0, 0};
    readback->Unmap(0, &written);
    if (!ok) {
        HOOK_LOG_WARN("swapchain capture: unsupported back buffer format %d", static_cast<int>(desc.Format));
        return false;
    }

    FrameWidth = static_cast<int>(width);
    FrameHeight = static_cast<int>(height);
    FramePixels = std::move(pixels);
    return true;
}

PresentCallback OverlayCallback = nullptr;

void OnPresent(IDXGISwapChain* swapChain)
{
    ID3D12CommandQueue* queue;
    {
        std::lock_guard<std::mutex> lock(StateMutex);
        queue = DirectQueue;
    }
    if (OverlayCallback && queue)
        OverlayCallback(swapChain, queue, [](ID3D12CommandQueue* q, UINT n, ID3D12CommandList* const* l) { OriginalExecute(q, n, l); });
    {
        std::lock_guard<std::mutex> lock(StateMutex);
        if (!Requested)
            return;
    }
    bool const ok = CopyBackBuffer(swapChain, queue);
    {
        std::lock_guard<std::mutex> lock(StateMutex);
        Requested = false;
        Finished = true;
        Succeeded = ok;
    }
    Done.notify_all();
}

HRESULT STDMETHODCALLTYPE PresentDetour(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
{
    OnPresent(swapChain);
    return OriginalPresent(swapChain, syncInterval, flags);
}

HRESULT STDMETHODCALLTYPE Present1Detour(IDXGISwapChain1* swapChain, UINT syncInterval, UINT flags,
                                         DXGI_PRESENT_PARAMETERS const* parameters)
{
    OnPresent(swapChain);
    return OriginalPresent1(swapChain, syncInterval, flags, parameters);
}

void STDMETHODCALLTYPE ExecuteDetour(ID3D12CommandQueue* queue, UINT count, ID3D12CommandList* const* lists)
{
    if (queue->GetDesc().Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
        std::lock_guard<std::mutex> lock(StateMutex);
        DirectQueue = queue;
    }
    OriginalExecute(queue, count, lists);
}

void* VirtualMethod(void* object, int index)
{
    return (*static_cast<void***>(object))[index];
}

bool Hook(void* target, void* detour, void** original, char const* name)
{
    MH_STATUS status = MH_CreateHook(target, detour, original);
    if (status == MH_OK)
        status = MH_EnableHook(target);
    if (status != MH_OK)
        HOOK_LOG_ERROR("swapchain capture: hook %s failed: %s", name, MH_StatusToString(status));
    return status == MH_OK;
}

// Takes the vtables from a throwaway device, queue and swap chain on a hidden window.
bool InstallHooks()
{
    HMODULE d3d12 = GetModuleHandleW(L"d3d12.dll");
    HMODULE dxgi = GetModuleHandleW(L"dxgi.dll");
    if (!d3d12 || !dxgi)
        return false;
    auto createDevice = reinterpret_cast<PFN_D3D12_CREATE_DEVICE>(GetProcAddress(d3d12, "D3D12CreateDevice"));
    auto createFactory = reinterpret_cast<HRESULT(WINAPI*)(UINT, REFIID, void**)>(
        GetProcAddress(dxgi, "CreateDXGIFactory2"));
    if (!createDevice || !createFactory)
        return false;

    HWND window = CreateWindowExW(0, L"STATIC", L"", WS_POPUP, 0, 0, 64, 64, nullptr, nullptr, nullptr, nullptr);
    if (!window)
        return false;

    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> queue;
    ComPtr<IDXGIFactory2> factory;
    ComPtr<IDXGISwapChain1> swapChain;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    DXGI_SWAP_CHAIN_DESC1 swapDesc = {};
    swapDesc.Width = 64;
    swapDesc.Height = 64;
    swapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.BufferCount = 2;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    bool ok = SUCCEEDED(createDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))) &&
              SUCCEEDED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue))) &&
              SUCCEEDED(createFactory(0, IID_PPV_ARGS(&factory))) &&
              SUCCEEDED(factory->CreateSwapChainForHwnd(queue.Get(), window, &swapDesc, nullptr, nullptr, &swapChain));
    if (ok) {
        // IDXGISwapChain::Present, IDXGISwapChain1::Present1, ID3D12CommandQueue::ExecuteCommandLists
        ok = Hook(VirtualMethod(swapChain.Get(), 8), reinterpret_cast<void*>(&PresentDetour),
                  reinterpret_cast<void**>(&OriginalPresent), "Present") &&
             Hook(VirtualMethod(swapChain.Get(), 22), reinterpret_cast<void*>(&Present1Detour),
                  reinterpret_cast<void**>(&OriginalPresent1), "Present1") &&
             Hook(VirtualMethod(queue.Get(), 10), reinterpret_cast<void*>(&ExecuteDetour),
                  reinterpret_cast<void**>(&OriginalExecute), "ExecuteCommandLists");
    }
    swapChain.Reset();
    queue.Reset();
    device.Reset();
    DestroyWindow(window);
    HOOK_LOG_INFO("swapchain capture: hooks %s", ok ? "installed" : "not installed");
    return ok;
}

} // namespace

bool EnsureSwapChainHooks()
{
    std::lock_guard<std::mutex> lock(HookMutex);
    if (!HooksInstalled)
        HooksInstalled = InstallHooks();
    return HooksInstalled;
}

void SetPresentCallback(PresentCallback callback)
{
    OverlayCallback = callback;
}

bool CaptureSwapChain(int& width, int& height, std::vector<std::uint8_t>& pixels)
{
    if (!EnsureSwapChainHooks())
        return false;

    std::unique_lock<std::mutex> lock(StateMutex);
    // Present runs once a frame; the first frames after installing may still miss the queue.
    bool ok = false;
    for (int attempt = 0; attempt < 3 && !ok; ++attempt) {
        Requested = true;
        Finished = false;
        if (!Done.wait_for(lock, std::chrono::seconds(2), [] { return Finished; })) {
            Requested = false;
            return false;
        }
        ok = Succeeded;
    }
    if (!ok)
        return false;
    width = FrameWidth;
    height = FrameHeight;
    pixels = std::move(FramePixels);
    return true;
}
