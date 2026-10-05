#define NOMINMAX
#include "SwapChainCapture.hpp"

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include "hook_lib/Log.hpp"

#include <MinHook.h>

#include <chrono>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>

using Microsoft::WRL::ComPtr;

namespace {

using PresentFn = HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain*, UINT, UINT);
using Present1Fn = HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain1*, UINT, UINT, DXGI_PRESENT_PARAMETERS const*);
using ExecuteFn = void(STDMETHODCALLTYPE*)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);

PresentFn OriginalPresent = nullptr;
Present1Fn OriginalPresent1 = nullptr;
ExecuteFn OriginalExecute = nullptr;

using CreateDsvFn = void(STDMETHODCALLTYPE*)(ID3D12Device*, ID3D12Resource*, D3D12_DEPTH_STENCIL_VIEW_DESC const*,
                                             D3D12_CPU_DESCRIPTOR_HANDLE);
using SetTargetsFn = void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*, UINT, D3D12_CPU_DESCRIPTOR_HANDLE const*, BOOL,
                                              D3D12_CPU_DESCRIPTOR_HANDLE const*);
using BarrierFn = void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*, UINT, D3D12_RESOURCE_BARRIER const*);

CreateDsvFn OriginalCreateDsv = nullptr;
SetTargetsFn OriginalSetTargets = nullptr;
BarrierFn OriginalBarrier = nullptr;

struct DsvInfo {
    ID3D12Resource* resource;
    D3D12_DEPTH_STENCIL_VIEW_DESC view;
    bool hasView;
    UINT width, height;
    int uses;
};

std::mutex DepthMutex;
std::unordered_map<SIZE_T, DsvInfo> DsvDescriptors;
std::unordered_map<ID3D12Resource*, DsvInfo> FrameSeen;
std::vector<SceneDepth> Candidates;
std::atomic<int> SelectedState{D3D12_RESOURCE_STATE_DEPTH_WRITE};
std::unordered_map<ID3D12Resource*, int> States;

using CreateRtvFn = void(STDMETHODCALLTYPE*)(ID3D12Device*, ID3D12Resource*, D3D12_RENDER_TARGET_VIEW_DESC const*,
                                             D3D12_CPU_DESCRIPTOR_HANDLE);
using CloseFn = HRESULT(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*);
using ResetFn = HRESULT(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*, ID3D12CommandAllocator*, ID3D12PipelineState*);
using DrawFn = void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*, UINT, UINT, UINT, UINT);
using DrawIndexedFn = void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*, UINT, UINT, UINT, INT, UINT);
using DispatchFn = void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*, UINT, UINT, UINT);
using CopyRegionFn = void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*, D3D12_TEXTURE_COPY_LOCATION const*, UINT, UINT,
                                              UINT, D3D12_TEXTURE_COPY_LOCATION const*, D3D12_BOX const*);
using CopyResourceFn = void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*, ID3D12Resource*, ID3D12Resource*);
using ClearRtvFn = void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*, D3D12_CPU_DESCRIPTOR_HANDLE, FLOAT const*, UINT,
                                            D3D12_RECT const*);
using EventFn = void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*, UINT, void const*, UINT);
using EndEventFn = void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*);
using IndirectFn = void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*, ID3D12CommandSignature*, UINT, ID3D12Resource*,
                                            UINT64, ID3D12Resource*, UINT64);
using BeginPassFn = void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList4*, UINT,
                                             D3D12_RENDER_PASS_RENDER_TARGET_DESC const*,
                                             D3D12_RENDER_PASS_DEPTH_STENCIL_DESC const*, D3D12_RENDER_PASS_FLAGS);

CreateRtvFn OriginalCreateRtv = nullptr;
CloseFn OriginalClose = nullptr;
ResetFn OriginalReset = nullptr;
DrawFn OriginalDraw = nullptr;
DrawIndexedFn OriginalDrawIndexed = nullptr;
DispatchFn OriginalDispatch = nullptr;
CopyRegionFn OriginalCopyRegion = nullptr;
CopyResourceFn OriginalCopyResource = nullptr;
ClearRtvFn OriginalClearRtv = nullptr;
EventFn OriginalBeginEvent = nullptr;
EventFn OriginalSetMarker = nullptr;
EndEventFn OriginalEndEvent = nullptr;
IndirectFn OriginalIndirect = nullptr;
BeginPassFn OriginalBeginPass = nullptr;

std::mutex RtvMutex;
std::unordered_map<SIZE_T, ID3D12Resource*> RtvDescriptors;

// 0 idle, 1 waiting for the frame to start, 2 recording, 3 finished.
std::atomic<int> TraceStage{0};
std::mutex TraceMutex;
std::condition_variable TraceDone;

struct TracedList {
    void* list;
    DWORD thread;
    std::vector<std::string> events;
    int draws = 0;
};

std::vector<TracedList> TraceLists;
std::unordered_map<void*, std::size_t> TraceCurrent;
std::vector<std::string> TraceSubmits;
std::unordered_map<ID3D12Resource*, int> TraceIds;
std::vector<std::string> TraceResources;

bool Tracing()
{
    return TraceStage.load(std::memory_order_relaxed) == 2;
}

// Caller holds TraceMutex.
std::string TraceResource(ID3D12Resource* resource)
{
    if (!resource)
        return "null";
    auto [it, added] = TraceIds.try_emplace(resource, static_cast<int>(TraceIds.size()) + 1);
    if (added) {
        D3D12_RESOURCE_DESC const desc = resource->GetDesc();
        char line[160];
        std::snprintf(line, sizeof(line), "R%d %p dim=%d %llux%u array=%u mips=%u fmt=%d samples=%u flags=0x%x", it->second,
                      static_cast<void*>(resource), static_cast<int>(desc.Dimension),
                      static_cast<unsigned long long>(desc.Width), desc.Height, desc.DepthOrArraySize, desc.MipLevels,
                      static_cast<int>(desc.Format), desc.SampleDesc.Count, static_cast<unsigned>(desc.Flags));
        TraceResources.push_back(line);
    }
    return "R" + std::to_string(it->second);
}

std::string RtvResource(SIZE_T handle)
{
    ID3D12Resource* resource = nullptr;
    {
        std::lock_guard<std::mutex> lock(RtvMutex);
        auto it = RtvDescriptors.find(handle);
        if (it != RtvDescriptors.end())
            resource = it->second;
    }
    if (!resource) {
        char text[32];
        std::snprintf(text, sizeof(text), "rtv?%llx", static_cast<unsigned long long>(handle));
        return text;
    }
    std::lock_guard<std::mutex> lock(TraceMutex);
    return TraceResource(resource);
}

// Caller holds TraceMutex.
TracedList& TraceListFor(void* list)
{
    auto it = TraceCurrent.find(list);
    if (it == TraceCurrent.end()) {
        TraceLists.push_back({list, GetCurrentThreadId(), {"(recording began before the frame)"}});
        it = TraceCurrent.emplace(list, TraceLists.size() - 1).first;
    }
    return TraceLists[it->second];
}

void FlushDraws(TracedList& traced)
{
    if (traced.draws) {
        traced.events.push_back("draws " + std::to_string(traced.draws));
        traced.draws = 0;
    }
}

void TraceEvent(void* list, std::string event)
{
    std::lock_guard<std::mutex> lock(TraceMutex);
    TracedList& traced = TraceListFor(list);
    FlushDraws(traced);
    traced.events.push_back(std::move(event));
}

void TraceDraw(void* list)
{
    std::lock_guard<std::mutex> lock(TraceMutex);
    ++TraceListFor(list).draws;
}

std::string EventName(UINT metadata, void const* data, UINT size)
{
    std::string name;
    if (!data)
        return name;
    if (metadata == 0) {
        for (wchar_t const* text = static_cast<wchar_t const*>(data); *text && name.size() < 80; ++text)
            name += static_cast<char>(*text < 128 ? *text : '?');
        return name;
    }
    for (UINT i = 0; i < size && name.size() < 80; ++i) {
        char const c = static_cast<char const*>(data)[i];
        if (c >= 32 && c < 127)
            name += c;
    }
    return name;
}

void STDMETHODCALLTYPE CreateRtvDetour(ID3D12Device* device, ID3D12Resource* resource,
                                       D3D12_RENDER_TARGET_VIEW_DESC const* desc, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
    OriginalCreateRtv(device, resource, desc, handle);
    std::lock_guard<std::mutex> lock(RtvMutex);
    RtvDescriptors[handle.ptr] = resource;
}

HRESULT STDMETHODCALLTYPE ResetDetour(ID3D12GraphicsCommandList* list, ID3D12CommandAllocator* allocator,
                                      ID3D12PipelineState* state)
{
    if (Tracing()) {
        std::lock_guard<std::mutex> lock(TraceMutex);
        TraceLists.push_back({list, GetCurrentThreadId(), {}});
        TraceCurrent[list] = TraceLists.size() - 1;
    }
    return OriginalReset(list, allocator, state);
}

HRESULT STDMETHODCALLTYPE CloseDetour(ID3D12GraphicsCommandList* list)
{
    if (Tracing())
        TraceEvent(list, "close");
    return OriginalClose(list);
}

void STDMETHODCALLTYPE DrawDetour(ID3D12GraphicsCommandList* list, UINT a, UINT b, UINT c, UINT d)
{
    if (Tracing())
        TraceDraw(list);
    OriginalDraw(list, a, b, c, d);
}

void STDMETHODCALLTYPE DrawIndexedDetour(ID3D12GraphicsCommandList* list, UINT a, UINT b, UINT c, INT d, UINT e)
{
    if (Tracing())
        TraceDraw(list);
    OriginalDrawIndexed(list, a, b, c, d, e);
}

void STDMETHODCALLTYPE DispatchDetour(ID3D12GraphicsCommandList* list, UINT x, UINT y, UINT z)
{
    if (Tracing())
        TraceEvent(list, "dispatch " + std::to_string(x) + "x" + std::to_string(y) + "x" + std::to_string(z));
    OriginalDispatch(list, x, y, z);
}

void STDMETHODCALLTYPE IndirectDetour(ID3D12GraphicsCommandList* list, ID3D12CommandSignature* signature, UINT count,
                                      ID3D12Resource* arguments, UINT64 offset, ID3D12Resource* counter,
                                      UINT64 counterOffset)
{
    if (Tracing())
        TraceEvent(list, "indirect max " + std::to_string(count));
    OriginalIndirect(list, signature, count, arguments, offset, counter, counterOffset);
}

void STDMETHODCALLTYPE CopyRegionDetour(ID3D12GraphicsCommandList* list, D3D12_TEXTURE_COPY_LOCATION const* dst, UINT x,
                                        UINT y, UINT z, D3D12_TEXTURE_COPY_LOCATION const* src, D3D12_BOX const* box)
{
    if (Tracing()) {
        std::lock_guard<std::mutex> lock(TraceMutex);
        TracedList& traced = TraceListFor(list);
        FlushDraws(traced);
        traced.events.push_back("copyregion " + TraceResource(src->pResource) + " -> " + TraceResource(dst->pResource));
    }
    OriginalCopyRegion(list, dst, x, y, z, src, box);
}

void STDMETHODCALLTYPE CopyResourceDetour(ID3D12GraphicsCommandList* list, ID3D12Resource* dst, ID3D12Resource* src)
{
    if (Tracing()) {
        std::lock_guard<std::mutex> lock(TraceMutex);
        TracedList& traced = TraceListFor(list);
        FlushDraws(traced);
        traced.events.push_back("copy " + TraceResource(src) + " -> " + TraceResource(dst));
    }
    OriginalCopyResource(list, dst, src);
}

void STDMETHODCALLTYPE ClearRtvDetour(ID3D12GraphicsCommandList* list, D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT const* color,
                                      UINT count, D3D12_RECT const* rects)
{
    if (Tracing()) {
        std::string const target = RtvResource(rtv.ptr);
        TraceEvent(list, "clear " + target);
    }
    OriginalClearRtv(list, rtv, color, count, rects);
}

void STDMETHODCALLTYPE BeginEventDetour(ID3D12GraphicsCommandList* list, UINT metadata, void const* data, UINT size)
{
    if (Tracing())
        TraceEvent(list, "begin '" + EventName(metadata, data, size) + "'");
    OriginalBeginEvent(list, metadata, data, size);
}

void STDMETHODCALLTYPE SetMarkerDetour(ID3D12GraphicsCommandList* list, UINT metadata, void const* data, UINT size)
{
    if (Tracing())
        TraceEvent(list, "marker '" + EventName(metadata, data, size) + "'");
    OriginalSetMarker(list, metadata, data, size);
}

void STDMETHODCALLTYPE EndEventDetour(ID3D12GraphicsCommandList* list)
{
    if (Tracing())
        TraceEvent(list, "end");
    OriginalEndEvent(list);
}

void STDMETHODCALLTYPE BeginPassDetour(ID3D12GraphicsCommandList4* list, UINT count,
                                       D3D12_RENDER_PASS_RENDER_TARGET_DESC const* targets,
                                       D3D12_RENDER_PASS_DEPTH_STENCIL_DESC const* depth, D3D12_RENDER_PASS_FLAGS flags)
{
    if (Tracing()) {
        std::string event = "renderpass";
        for (UINT i = 0; i < count; ++i)
            event += " " + RtvResource(targets[i].cpuDescriptor.ptr);
        TraceEvent(list, event + (depth ? " +depth" : ""));
    }
    OriginalBeginPass(list, count, targets, depth, flags);
}

void TraceTargets(void* list, UINT count, D3D12_CPU_DESCRIPTOR_HANDLE const* targets, BOOL single,
                  D3D12_CPU_DESCRIPTOR_HANDLE const* depth)
{
    std::string event = "targets";
    for (UINT i = 0; i < (single ? std::min(count, 1u) : count); ++i)
        event += " " + RtvResource(targets[i].ptr);
    if (single && count > 1)
        event += " (+" + std::to_string(count - 1) + " in range)";
    if (depth) {
        ID3D12Resource* resource = nullptr;
        {
            std::lock_guard<std::mutex> lock(DepthMutex);
            auto it = DsvDescriptors.find(depth->ptr);
            if (it != DsvDescriptors.end())
                resource = it->second.resource;
        }
        std::lock_guard<std::mutex> lock(TraceMutex);
        event += " depth " + TraceResource(resource);
    }
    TraceEvent(list, std::move(event));
}

void TraceBarriers(void* list, UINT count, D3D12_RESOURCE_BARRIER const* barriers)
{
    std::lock_guard<std::mutex> lock(TraceMutex);
    TracedList& traced = TraceListFor(list);
    FlushDraws(traced);
    std::string event = "barrier";
    for (UINT i = 0; i < count; ++i) {
        char text[64];
        if (barriers[i].Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION) {
            std::snprintf(text, sizeof(text), " %s:%x->%x", TraceResource(barriers[i].Transition.pResource).c_str(),
                          static_cast<unsigned>(barriers[i].Transition.StateBefore),
                          static_cast<unsigned>(barriers[i].Transition.StateAfter));
            event += text;
        } else {
            event += barriers[i].Type == D3D12_RESOURCE_BARRIER_TYPE_UAV ? " uav" : " alias";
        }
    }
    traced.events.push_back(std::move(event));
}

void TraceSubmit(ID3D12CommandQueue* queue, UINT count, ID3D12CommandList* const* lists)
{
    std::lock_guard<std::mutex> lock(TraceMutex);
    char text[64];
    std::snprintf(text, sizeof(text), "execute queue %p type %d:", static_cast<void*>(queue),
                  static_cast<int>(queue->GetDesc().Type));
    std::string line = text;
    for (UINT i = 0; i < count; ++i) {
        auto it = TraceCurrent.find(lists[i]);
        line += it == TraceCurrent.end() ? " (untraced)" : " L" + std::to_string(it->second);
    }
    TraceSubmits.push_back(std::move(line));
}

// Called from Present: begins recording on one frame and ends it on the next.
void TracePresent(IDXGISwapChain* swapChain)
{
    int stage = TraceStage.load();
    if (stage == 1) {
        std::lock_guard<std::mutex> lock(TraceMutex);
        TraceLists.clear();
        TraceCurrent.clear();
        TraceSubmits.clear();
        TraceIds.clear();
        TraceResources.clear();
        TraceStage = 2;
    } else if (stage == 2) {
        {
            std::lock_guard<std::mutex> lock(TraceMutex);
            ComPtr<IDXGISwapChain3> swapChain3;
            ComPtr<ID3D12Resource> backBuffer;
            if (SUCCEEDED(swapChain->QueryInterface(IID_PPV_ARGS(&swapChain3))) &&
                SUCCEEDED(swapChain3->GetBuffer(swapChain3->GetCurrentBackBufferIndex(), IID_PPV_ARGS(&backBuffer))))
                TraceSubmits.push_back("present back buffer " + TraceResource(backBuffer.Get()));
            TraceStage = 3;
        }
        TraceDone.notify_all();
    }
}

void STDMETHODCALLTYPE CreateDsvDetour(ID3D12Device* device, ID3D12Resource* resource,
                                       D3D12_DEPTH_STENCIL_VIEW_DESC const* desc, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
    OriginalCreateDsv(device, resource, desc, handle);
    if (!resource)
        return;
    D3D12_RESOURCE_DESC const resourceDesc = resource->GetDesc();
    DsvInfo info = {resource, desc ? *desc : D3D12_DEPTH_STENCIL_VIEW_DESC{}, desc != nullptr,
                    static_cast<UINT>(resourceDesc.Width), resourceDesc.Height, 0};
    std::lock_guard<std::mutex> lock(DepthMutex);
    DsvDescriptors[handle.ptr] = info;
}

void STDMETHODCALLTYPE SetTargetsDetour(ID3D12GraphicsCommandList* list, UINT count,
                                        D3D12_CPU_DESCRIPTOR_HANDLE const* targets, BOOL single,
                                        D3D12_CPU_DESCRIPTOR_HANDLE const* depth)
{
    OriginalSetTargets(list, count, targets, single, depth);
    if (Tracing())
        TraceTargets(list, count, targets, single, depth);
    if (!depth)
        return;
    std::lock_guard<std::mutex> lock(DepthMutex);
    auto it = DsvDescriptors.find(depth->ptr);
    if (it == DsvDescriptors.end())
        return;
    DsvInfo& seen = FrameSeen.try_emplace(it->second.resource, it->second).first->second;
    seen.view = it->second.view;
    seen.hasView = it->second.hasView;
    ++seen.uses;
}

void STDMETHODCALLTYPE BarrierDetour(ID3D12GraphicsCommandList* list, UINT count, D3D12_RESOURCE_BARRIER const* barriers)
{
    OriginalBarrier(list, count, barriers);
    if (Tracing())
        TraceBarriers(list, count, barriers);
    std::lock_guard<std::mutex> lock(DepthMutex);
    for (UINT i = 0; i < count; ++i)
        if (barriers[i].Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION &&
            barriers[i].Transition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
            auto it = States.find(barriers[i].Transition.pResource);
            if (it != States.end())
                it->second = barriers[i].Transition.StateAfter;
        }
}


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

void FinishDepthFrame()
{
    std::lock_guard<std::mutex> lock(DepthMutex);
    if (FrameSeen.empty())
        return;
    std::vector<DsvInfo> best;
    for (auto const& [resource, info] : FrameSeen)
        best.push_back(info);
    std::sort(best.begin(), best.end(), [](DsvInfo const& a, DsvInfo const& b) {
        return a.uses != b.uses ? a.uses > b.uses : a.resource < b.resource;
    });
    std::unordered_map<ID3D12Resource*, int> states;
    Candidates.clear();
    for (DsvInfo const& info : best) {
        auto it = States.find(info.resource);
        int const state = it == States.end() ? D3D12_RESOURCE_STATE_DEPTH_WRITE : it->second;
        states[info.resource] = state;
        SceneDepth depth;
        depth.resource = info.resource;
        depth.view = info.view;
        depth.hasView = info.hasView;
        depth.width = info.width;
        depth.height = info.height;
        depth.state = static_cast<D3D12_RESOURCE_STATES>(state);
        Candidates.push_back(std::move(depth));
    }
    States = std::move(states);
    FrameSeen.clear();
}

void OnPresent(IDXGISwapChain* swapChain)
{
    FinishDepthFrame();
    TracePresent(swapChain);
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
    if (Tracing())
        TraceSubmit(queue, count, lists);
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
    ComPtr<ID3D12CommandAllocator> allocator;
    ComPtr<ID3D12GraphicsCommandList> list;
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
              SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))) &&
              SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr,
                                                  IID_PPV_ARGS(&list))) &&
              SUCCEEDED(createFactory(0, IID_PPV_ARGS(&factory))) &&
              SUCCEEDED(factory->CreateSwapChainForHwnd(queue.Get(), window, &swapDesc, nullptr, nullptr, &swapChain));
    if (ok) {
        // IDXGISwapChain::Present, IDXGISwapChain1::Present1, ID3D12CommandQueue::ExecuteCommandLists,
        // ID3D12Device::CreateDepthStencilView, ID3D12GraphicsCommandList::{ResourceBarrier, OMSetRenderTargets}
        ok = Hook(VirtualMethod(swapChain.Get(), 8), reinterpret_cast<void*>(&PresentDetour),
                  reinterpret_cast<void**>(&OriginalPresent), "Present") &&
             Hook(VirtualMethod(swapChain.Get(), 22), reinterpret_cast<void*>(&Present1Detour),
                  reinterpret_cast<void**>(&OriginalPresent1), "Present1") &&
             Hook(VirtualMethod(queue.Get(), 10), reinterpret_cast<void*>(&ExecuteDetour),
                  reinterpret_cast<void**>(&OriginalExecute), "ExecuteCommandLists") &&
             Hook(VirtualMethod(device.Get(), 21), reinterpret_cast<void*>(&CreateDsvDetour),
                  reinterpret_cast<void**>(&OriginalCreateDsv), "CreateDepthStencilView") &&
             Hook(VirtualMethod(list.Get(), 26), reinterpret_cast<void*>(&BarrierDetour),
                  reinterpret_cast<void**>(&OriginalBarrier), "ResourceBarrier") &&
             Hook(VirtualMethod(list.Get(), 46), reinterpret_cast<void*>(&SetTargetsDetour),
                  reinterpret_cast<void**>(&OriginalSetTargets), "OMSetRenderTargets") &&
             Hook(VirtualMethod(device.Get(), 20), reinterpret_cast<void*>(&CreateRtvDetour),
                  reinterpret_cast<void**>(&OriginalCreateRtv), "CreateRenderTargetView");
        // Frame tracing only; ID3D12GraphicsCommandList::{Close, Reset, DrawInstanced, DrawIndexedInstanced, Dispatch,
        // CopyTextureRegion, CopyResource, ClearRenderTargetView, SetMarker, BeginEvent, EndEvent, ExecuteIndirect}
        if (ok) {
            Hook(VirtualMethod(list.Get(), 9), reinterpret_cast<void*>(&CloseDetour),
                 reinterpret_cast<void**>(&OriginalClose), "Close");
            Hook(VirtualMethod(list.Get(), 10), reinterpret_cast<void*>(&ResetDetour),
                 reinterpret_cast<void**>(&OriginalReset), "Reset");
            Hook(VirtualMethod(list.Get(), 12), reinterpret_cast<void*>(&DrawDetour),
                 reinterpret_cast<void**>(&OriginalDraw), "DrawInstanced");
            Hook(VirtualMethod(list.Get(), 13), reinterpret_cast<void*>(&DrawIndexedDetour),
                 reinterpret_cast<void**>(&OriginalDrawIndexed), "DrawIndexedInstanced");
            Hook(VirtualMethod(list.Get(), 14), reinterpret_cast<void*>(&DispatchDetour),
                 reinterpret_cast<void**>(&OriginalDispatch), "Dispatch");
            Hook(VirtualMethod(list.Get(), 16), reinterpret_cast<void*>(&CopyRegionDetour),
                 reinterpret_cast<void**>(&OriginalCopyRegion), "CopyTextureRegion");
            Hook(VirtualMethod(list.Get(), 17), reinterpret_cast<void*>(&CopyResourceDetour),
                 reinterpret_cast<void**>(&OriginalCopyResource), "CopyResource");
            Hook(VirtualMethod(list.Get(), 48), reinterpret_cast<void*>(&ClearRtvDetour),
                 reinterpret_cast<void**>(&OriginalClearRtv), "ClearRenderTargetView");
            Hook(VirtualMethod(list.Get(), 56), reinterpret_cast<void*>(&SetMarkerDetour),
                 reinterpret_cast<void**>(&OriginalSetMarker), "SetMarker");
            Hook(VirtualMethod(list.Get(), 57), reinterpret_cast<void*>(&BeginEventDetour),
                 reinterpret_cast<void**>(&OriginalBeginEvent), "BeginEvent");
            Hook(VirtualMethod(list.Get(), 58), reinterpret_cast<void*>(&EndEventDetour),
                 reinterpret_cast<void**>(&OriginalEndEvent), "EndEvent");
            Hook(VirtualMethod(list.Get(), 59), reinterpret_cast<void*>(&IndirectDetour),
                 reinterpret_cast<void**>(&OriginalIndirect), "ExecuteIndirect");
            ComPtr<ID3D12GraphicsCommandList4> list4;
            if (SUCCEEDED(list.As(&list4)))
                Hook(VirtualMethod(list4.Get(), 68), reinterpret_cast<void*>(&BeginPassDetour),
                     reinterpret_cast<void**>(&OriginalBeginPass), "BeginRenderPass");
        }
    }
    list.Reset();
    allocator.Reset();
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

bool GetSceneDepths(std::vector<SceneDepth>& depths)
{
    std::lock_guard<std::mutex> lock(DepthMutex);
    for (SceneDepth& depth : Candidates) {
        auto it = States.find(depth.resource.Get());
        if (it != States.end())
            depth.state = static_cast<D3D12_RESOURCE_STATES>(it->second);
    }
    depths = Candidates;
    return !depths.empty();
}

bool TraceFrame(std::string& text)
{
    if (!EnsureSwapChainHooks())
        return false;
    std::unique_lock<std::mutex> lock(TraceMutex);
    TraceStage = 1;
    if (!TraceDone.wait_for(lock, std::chrono::seconds(3), [] { return TraceStage == 3; })) {
        TraceStage = 0;
        return false;
    }
    TraceStage = 0;
    text.clear();
    for (std::string const& line : TraceResources)
        text += line + "\n";
    for (std::size_t i = 0; i < TraceLists.size(); ++i) {
        char header[64];
        std::snprintf(header, sizeof(header), "L%zu %p thread %lu\n", i, TraceLists[i].list,
                      static_cast<unsigned long>(TraceLists[i].thread));
        text += header;
        for (std::string const& event : TraceLists[i].events)
            text += "  " + event + "\n";
        if (TraceLists[i].draws)
            text += "  draws " + std::to_string(TraceLists[i].draws) + "\n";
    }
    for (std::string const& line : TraceSubmits)
        text += line + "\n";
    TraceLists.clear();
    TraceCurrent.clear();
    TraceSubmits.clear();
    return true;
}
