#define NOMINMAX
#include "Overlay.hpp"

#include "ServerData.hpp"
#include "SwapChainCapture.hpp"

#include <windows.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include "hook_lib/Log.hpp"
#include "hook_lib/ProcessImage.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace {

constexpr std::uintptr_t CameraRoot = 0x1438921a8;
constexpr UINT MaxFrames = 4;
constexpr UINT MaxVertices = 1u << 21;

struct Vertex {
    float x, y, z;
    std::uint32_t color;
};

struct Camera {
    float eye[3];
    float forward[3];
    float right[3];
    float up[3];
    float player[3];
};

std::atomic<bool> Enabled{false};
std::atomic<float> TanHalfFovV{0.57735f};
std::atomic<int> TestGrid{0};
std::atomic<float> NearPlane{0.146f};
std::atomic<float> FarPlane{1e7f};
std::atomic<bool> ReversedDepth{true};
std::atomic<float> LineBias{0.004f};
std::atomic<float> FaceBias{0.003f};
std::atomic<float> HeightOffset{2.f};
std::atomic<bool> UseDepth{true};
std::atomic<bool> DumpDepth{false};
std::atomic<int> DepthPick{0};

std::mutex ProviderMutex;
std::vector<OverlayProvider> Providers;

template <typename T> bool ReadAt(std::uintptr_t address, T& out)
{
    return ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<void const*>(address), &out, sizeof(T), nullptr);
}

std::uintptr_t Rebase(std::uintptr_t ghidraAddress)
{
    return reinterpret_cast<std::uintptr_t>(GetProcessImage().BaseAddress) + (ghidraAddress - 0x140000000ull);
}

bool ReadCamera(Camera& camera)
{
    std::uintptr_t pointer = 0;
    if (!ReadAt(Rebase(CameraRoot), pointer) || !pointer || !ReadAt(pointer + 0x3a58, pointer) || !pointer)
        return false;
    float block[0x40];
    if (!ReadAt(pointer + 0x10, block))
        return false;
    std::memcpy(camera.eye, block, 12);
    std::memcpy(camera.forward, block + 3, 12);
    std::memcpy(camera.right, block + 6, 12);
    std::memcpy(camera.up, block + 9, 12);
    return ReadAt(pointer + 0x224, camera.player);
}

char const* const ShaderSource = R"(
#pragma pack_matrix(row_major)
cbuffer C : register(b0) { float4x4 viewProj; float bias; };
struct VSIn { float3 pos : POSITION; float4 color : COLOR; };
struct VSOut { float4 pos : SV_POSITION; float4 color : COLOR; };
VSOut VS(VSIn v) { VSOut o; o.pos = mul(float4(v.pos, 1), viewProj); o.pos.z *= 1 + bias; o.color = v.color; return o; }
float4 PS(VSOut v) : SV_TARGET { return v.color; }
)";

struct FrameResources {
    ComPtr<ID3D12CommandAllocator> allocator;
    ComPtr<ID3D12Resource> vertices;
    Vertex* mapped = nullptr;
    UINT64 fenceValue = 0;
};

struct Renderer {
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3DBlob> vs, ps;
    std::map<std::pair<DXGI_FORMAT, int>, ComPtr<ID3D12PipelineState>> pipelines;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12DescriptorHeap> dsvHeap;
    ComPtr<ID3D12GraphicsCommandList> list;
    ComPtr<ID3D12Fence> fence;
    HANDLE event = nullptr;
    FrameResources frames[MaxFrames];
    UINT64 nextFence = 0;
    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    bool failed = false;
};

Renderer Draw;

bool CompileShader(char const* entry, char const* target, ComPtr<ID3DBlob>& blob)
{
    HMODULE module = LoadLibraryW(L"d3dcompiler_47.dll");
    auto compile = module ? reinterpret_cast<pD3DCompile>(GetProcAddress(module, "D3DCompile")) : nullptr;
    if (!compile)
        return false;
    ComPtr<ID3DBlob> errors;
    HRESULT hr = compile(ShaderSource, std::strlen(ShaderSource), nullptr, nullptr, nullptr, entry, target, 0, 0, &blob, &errors);
    if (FAILED(hr) && errors)
        HOOK_LOG_ERROR("overlay shader: %s", static_cast<char const*>(errors->GetBufferPointer()));
    return SUCCEEDED(hr);
}

// kind 0: lines, 1: faces. `depthFormat` is DXGI_FORMAT_UNKNOWN when drawing without a depth test.
ID3D12PipelineState* GetPipeline(Renderer& r, int kind, DXGI_FORMAT depthFormat)
{
    kind &= 1;
    ComPtr<ID3D12PipelineState>& pipeline = r.pipelines[{depthFormat, kind | (ReversedDepth ? 2 : 0)}];
    if (pipeline)
        return pipeline.Get();
    D3D12_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature = r.rootSignature.Get();
    pso.VS = {r.vs->GetBufferPointer(), r.vs->GetBufferSize()};
    pso.PS = {r.ps->GetBufferPointer(), r.ps->GetBufferSize()};
    pso.InputLayout = {layout, 2};
    pso.PrimitiveTopologyType = kind ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE : D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = r.format;
    pso.DSVFormat = depthFormat;
    pso.SampleDesc.Count = 1;
    pso.SampleMask = UINT_MAX;
    pso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    pso.RasterizerState.DepthClipEnable = TRUE;
    pso.DepthStencilState.DepthEnable = depthFormat != DXGI_FORMAT_UNKNOWN;
    pso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    pso.DepthStencilState.DepthFunc = ReversedDepth ? D3D12_COMPARISON_FUNC_GREATER_EQUAL
                                                    : D3D12_COMPARISON_FUNC_LESS_EQUAL;
    D3D12_RENDER_TARGET_BLEND_DESC& blend = pso.BlendState.RenderTarget[0];
    blend.BlendEnable = TRUE;
    blend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blend.BlendOp = D3D12_BLEND_OP_ADD;
    blend.SrcBlendAlpha = D3D12_BLEND_ZERO;
    blend.DestBlendAlpha = D3D12_BLEND_ONE;
    blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    if (FAILED(r.device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pipeline))))
        HOOK_LOG_ERROR("overlay: pipeline creation failed (kind %d, depth format %d)", kind, static_cast<int>(depthFormat));
    return pipeline.Get();
}

bool CreateRenderer(Renderer& r, ID3D12Device* device, DXGI_FORMAT format)
{
    r.device = device;
    r.format = format;
    if (!CompileShader("VS", "vs_5_0", r.vs) || !CompileShader("PS", "ps_5_0", r.ps))
        return false;

    D3D12_ROOT_PARAMETER parameter = {};
    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    parameter.Constants.Num32BitValues = 20;
    parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
    rootDesc.NumParameters = 1;
    rootDesc.pParameters = &parameter;
    rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    ComPtr<ID3DBlob> serialized, errors;
    auto serialize = reinterpret_cast<PFN_D3D12_SERIALIZE_ROOT_SIGNATURE>(
        GetProcAddress(GetModuleHandleW(L"d3d12.dll"), "D3D12SerializeRootSignature"));
    if (!serialize || FAILED(serialize(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &errors)) ||
        FAILED(device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
                                           IID_PPV_ARGS(&r.rootSignature))))
        return false;

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.NumDescriptors = 1;
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = heapDesc;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    if (FAILED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&r.rtvHeap))) ||
        FAILED(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&r.dsvHeap))) ||
        FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&r.fence))))
        return false;
    r.event = CreateEventW(nullptr, FALSE, FALSE, nullptr);

    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = sizeof(Vertex) * MaxVertices;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    for (FrameResources& frame : r.frames) {
        if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame.allocator))) ||
            FAILED(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                                   D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                   IID_PPV_ARGS(&frame.vertices))) ||
            FAILED(frame.vertices->Map(0, nullptr, reinterpret_cast<void**>(&frame.mapped))))
            return false;
    }
    if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, r.frames[0].allocator.Get(), nullptr,
                                         IID_PPV_ARGS(&r.list))))
        return false;
    r.list->Close();
    return true;
}

std::uint32_t Pack(OverlayColor c)
{
    return c.r | (c.g << 8) | (c.b << 16) | (c.a << 24);
}

void BuildTestGrid(Camera const& camera, std::vector<Vertex>& out)
{
    float const px = camera.player[0], py = camera.player[1], pz = camera.player[2];
    std::uint32_t const grid = Pack({255, 255, 0, 200});
    for (int i = -10; i <= 10; ++i) {
        out.push_back({px + i, py - 10, pz, grid});
        out.push_back({px + i, py + 10, pz, grid});
        out.push_back({px - 10, py + i, pz, grid});
        out.push_back({px + 10, py + i, pz, grid});
    }
    std::uint32_t const pole = Pack({255, 0, 0, 255});
    out.push_back({px, py, pz, pole});
    out.push_back({px, py, pz + 10, pole});
}

DXGI_FORMAT DepthViewFormat(SceneDepth const& depth)
{
    if (depth.hasView && depth.view.Format != DXGI_FORMAT_UNKNOWN)
        return depth.view.Format;
    switch (depth.resource->GetDesc().Format) {
    case DXGI_FORMAT_R32_TYPELESS:
        return DXGI_FORMAT_D32_FLOAT;
    case DXGI_FORMAT_R24G8_TYPELESS:
        return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case DXGI_FORMAT_R16_TYPELESS:
        return DXGI_FORMAT_D16_UNORM;
    case DXGI_FORMAT_R32G8X24_TYPELESS:
        return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    default:
        return depth.resource->GetDesc().Format;
    }
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

// Writes the depth plane and the camera to a file for fitting the client's projection offline.
void WriteDepthDump(ID3D12Device* device, ID3D12CommandQueue* queue, ExecuteLists execute, SceneDepth const& depth,
                    Camera const& camera, int slot)
{
    D3D12_RESOURCE_DESC const desc = depth.resource->GetDesc();
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
        return;
    D3D12_TEXTURE_COPY_LOCATION destination = {};
    destination.pResource = readback.Get();
    destination.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    destination.PlacedFootprint = footprint;
    D3D12_TEXTURE_COPY_LOCATION source = {};
    source.pResource = depth.resource.Get();
    source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    D3D12_RESOURCE_BARRIER toCopy = Transition(depth.resource.Get(), depth.state, D3D12_RESOURCE_STATE_COPY_SOURCE);
    D3D12_RESOURCE_BARRIER back = Transition(depth.resource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, depth.state);
    list->ResourceBarrier(1, &toCopy);
    list->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
    list->ResourceBarrier(1, &back);
    if (FAILED(list->Close()))
        return;
    ID3D12CommandList* lists[] = {list.Get()};
    execute(queue, 1, lists);
    HANDLE event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    bool ok = event && SUCCEEDED(queue->Signal(fence.Get(), 1)) && SUCCEEDED(fence->SetEventOnCompletion(1, event)) &&
              WaitForSingleObject(event, 2000) == WAIT_OBJECT_0;
    if (event)
        CloseHandle(event);
    void* mapped = nullptr;
    D3D12_RANGE range = {0, static_cast<SIZE_T>(total)};
    if (!ok || FAILED(readback->Map(0, &range, &mapped)))
        return;
    UINT const width = static_cast<UINT>(desc.Width), height = desc.Height;
    std::vector<float> values(static_cast<std::size_t>(width) * height);
    bool const unorm24 = desc.Format == DXGI_FORMAT_R24G8_TYPELESS;
    for (UINT y = 0; y < height; ++y) {
        std::uint8_t const* row = static_cast<std::uint8_t const*>(mapped) + footprint.Offset +
                                  static_cast<std::size_t>(y) * footprint.Footprint.RowPitch;
        for (UINT x = 0; x < width; ++x) {
            std::uint32_t word;
            std::memcpy(&word, row + x * 4, 4);
            float value;
            if (unorm24)
                value = static_cast<float>(word & 0xffffff) / 16777215.f;
            else
                std::memcpy(&value, &word, 4);
            values[static_cast<std::size_t>(y) * width + x] = value;
        }
    }
    D3D12_RANGE none = {0, 0};
    readback->Unmap(0, &none);
    char path[48];
    std::snprintf(path, sizeof(path), "Z:\\tmp\\depth%d.bin", slot);
    if (FILE* file = std::fopen(path, "wb")) {
        std::uint32_t const header[2] = {width, height};
        std::fwrite(header, sizeof(header), 1, file);
        std::fwrite(&camera, sizeof(camera), 1, file);
        std::fwrite(values.data(), sizeof(float), values.size(), file);
        std::fclose(file);
    }
    HOOK_LOG_INFO("overlay: depth dump %ux%u format %d", width, height, static_cast<int>(desc.Format));
}

void Render(IDXGISwapChain* swapChain, ID3D12CommandQueue* queue, ExecuteLists execute)
{
    if (!Enabled || Draw.failed)
        return;
    ComPtr<IDXGISwapChain3> swapChain3;
    ComPtr<ID3D12Resource> backBuffer;
    ComPtr<ID3D12Device> device;
    if (FAILED(swapChain->QueryInterface(IID_PPV_ARGS(&swapChain3))))
        return;
    UINT const index = swapChain3->GetCurrentBackBufferIndex();
    if (index >= MaxFrames || FAILED(swapChain3->GetBuffer(index, IID_PPV_ARGS(&backBuffer))) ||
        FAILED(backBuffer->GetDevice(IID_PPV_ARGS(&device))))
        return;
    D3D12_RESOURCE_DESC const desc = backBuffer->GetDesc();

    if (!Draw.device) {
        if (!CreateRenderer(Draw, device.Get(), desc.Format)) {
            HOOK_LOG_ERROR("overlay: renderer creation failed");
            Draw.failed = true;
            return;
        }
        HOOK_LOG_INFO("overlay: renderer ready (format %d)", static_cast<int>(desc.Format));
    }
    if (Draw.device.Get() != device.Get() || Draw.format != desc.Format)
        return;

    Camera camera;
    if (!ReadCamera(camera))
        return;

    SceneDepth depth;
    std::vector<SceneDepth> depths;
    GetSceneDepths(depths);
    depths.erase(std::remove_if(depths.begin(), depths.end(),
                                [&](SceneDepth const& d) { return d.width != desc.Width || d.height != desc.Height; }),
                 depths.end());
    if (DumpDepth.exchange(false))
        for (std::size_t i = 0; i < depths.size(); ++i)
            WriteDepthDump(device.Get(), queue, execute, depths[i], camera, static_cast<int>(i));
    int const pick = DepthPick;
    bool const haveDepth = UseDepth && static_cast<int>(depths.size()) > pick;
    if (haveDepth)
        depth = depths[pick];

    std::vector<Vertex> lines, faces;
    if (TestGrid)
        BuildTestGrid(camera, lines);
    {
        std::lock_guard<std::mutex> lock(ProviderMutex);
        for (OverlayProvider const& provider : Providers)
            provider(
                camera.eye,
                [&](float const* a, float const* b, OverlayColor color) {
                    std::uint32_t const packed = Pack(color);
                    lines.push_back({a[0], a[1], a[2], packed});
                    lines.push_back({b[0], b[1], b[2], packed});
                },
                [&](float const* a, float const* b, float const* c, OverlayColor color) {
                    std::uint32_t const packed = Pack(color);
                    faces.push_back({a[0], a[1], a[2], packed});
                    faces.push_back({b[0], b[1], b[2], packed});
                    faces.push_back({c[0], c[1], c[2], packed});
                });
    }
    if (lines.size() + faces.size() > MaxVertices) {
        faces.resize(std::min<std::size_t>(faces.size(), MaxVertices - std::min<std::size_t>(lines.size(), MaxVertices / 2)) / 3 * 3);
        lines.resize(std::min<std::size_t>(lines.size(), MaxVertices - faces.size()) & ~std::size_t(1));
    }
    if (lines.empty() && faces.empty())
        return;

    FrameResources& frame = Draw.frames[index];
    if (Draw.fence->GetCompletedValue() < frame.fenceValue) {
        Draw.fence->SetEventOnCompletion(frame.fenceValue, Draw.event);
        WaitForSingleObject(Draw.event, 2000);
    }
    std::memcpy(frame.mapped, faces.data(), faces.size() * sizeof(Vertex));
    std::memcpy(frame.mapped + faces.size(), lines.data(), lines.size() * sizeof(Vertex));
    frame.allocator->Reset();
    Draw.list->Reset(frame.allocator.Get(), nullptr);

    float const width = static_cast<float>(desc.Width), height = static_cast<float>(desc.Height);
    float const tanV = TanHalfFovV, tanH = tanV * width / height;
    float const nearPlane = NearPlane, farPlane = FarPlane;
    float a, b;
    if (ReversedDepth) {
        b = nearPlane * farPlane / (farPlane - nearPlane);
        a = -nearPlane / (farPlane - nearPlane);
    } else {
        a = farPlane / (farPlane - nearPlane);
        b = -nearPlane * a;
    }
    // Row-vector convention: clip = (x,y,z,1) * M, with M columns computed from the camera basis.
    float m[20] = {};
    camera.eye[2] -= HeightOffset;
    auto const column = [&](int c, float const* axis, float scale, float constant) {
        for (int row = 0; row < 3; ++row)
            m[row * 4 + c] = axis[row] * scale;
        m[3 * 4 + c] = -(axis[0] * camera.eye[0] + axis[1] * camera.eye[1] + axis[2] * camera.eye[2]) * scale + constant;
    };
    column(0, camera.right, 1.f / tanH, 0.f);
    column(1, camera.up, 1.f / tanV, 0.f);
    column(2, camera.forward, a, b);
    column(3, camera.forward, 1.f, 0.f);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = Draw.rtvHeap->GetCPUDescriptorHandleForHeapStart();
    Draw.device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtv);
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = Draw.dsvHeap->GetCPUDescriptorHandleForHeapStart();
    DXGI_FORMAT depthFormat = DXGI_FORMAT_UNKNOWN;
    if (haveDepth) {
        depthFormat = DepthViewFormat(depth);
        D3D12_DEPTH_STENCIL_VIEW_DESC view = depth.view;
        if (!depth.hasView) {
            view = {};
            view.Format = depthFormat;
            view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        }
        view.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
        Draw.device->CreateDepthStencilView(depth.resource.Get(), &view, dsv);
    }

    D3D12_RESOURCE_BARRIER barriers[2];
    UINT barrierCount = 0;
    barriers[barrierCount++] = Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    if (haveDepth && depth.state != D3D12_RESOURCE_STATE_DEPTH_READ)
        barriers[barrierCount++] = Transition(depth.resource.Get(), depth.state, D3D12_RESOURCE_STATE_DEPTH_READ);
    Draw.list->ResourceBarrier(barrierCount, barriers);
    Draw.list->OMSetRenderTargets(1, &rtv, FALSE, haveDepth ? &dsv : nullptr);
    D3D12_VIEWPORT viewport = {0, 0, width, height, 0.f, 1.f};
    D3D12_RECT scissor = {0, 0, static_cast<LONG>(desc.Width), static_cast<LONG>(desc.Height)};
    Draw.list->RSSetViewports(1, &viewport);
    Draw.list->RSSetScissorRects(1, &scissor);
    Draw.list->SetGraphicsRootSignature(Draw.rootSignature.Get());
    Draw.list->SetGraphicsRoot32BitConstants(0, 16, m, 0);
    D3D12_VERTEX_BUFFER_VIEW view = {frame.vertices->GetGPUVirtualAddress(),
                                     static_cast<UINT>((faces.size() + lines.size()) * sizeof(Vertex)), sizeof(Vertex)};
    Draw.list->IASetVertexBuffers(0, 1, &view);
    if (!faces.empty()) {
        float const bias = FaceBias;
        Draw.list->SetPipelineState(GetPipeline(Draw, 1, depthFormat));
        Draw.list->SetGraphicsRoot32BitConstants(0, 1, &bias, 16);
        Draw.list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        Draw.list->DrawInstanced(static_cast<UINT>(faces.size()), 1, 0, 0);
    }
    if (!lines.empty()) {
        float const bias = LineBias;
        Draw.list->SetPipelineState(GetPipeline(Draw, 0, depthFormat));
        Draw.list->SetGraphicsRoot32BitConstants(0, 1, &bias, 16);
        Draw.list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        Draw.list->DrawInstanced(static_cast<UINT>(lines.size()), 1, static_cast<UINT>(faces.size()), 0);
    }
    for (UINT i = 0; i < barrierCount; ++i)
        std::swap(barriers[i].Transition.StateBefore, barriers[i].Transition.StateAfter);
    Draw.list->ResourceBarrier(barrierCount, barriers);
    if (FAILED(Draw.list->Close()))
        return;

    ID3D12CommandList* lists[] = {Draw.list.Get()};
    execute(queue, 1, lists);
    frame.fenceValue = ++Draw.nextFence;
    queue->Signal(Draw.fence.Get(), frame.fenceValue);
}

} // namespace

void StartOverlay()
{
    // Hooked before the client creates its device, so every depth view it makes is seen.
    std::thread([] {
        LoadLibraryW(L"d3d12.dll");
        LoadLibraryW(L"dxgi.dll");
        SetPresentCallback(&Render);
        if (EnsureSwapChainHooks())
            Enabled = true;
    }).detach();
}

void SetOverlayEnabled(bool enabled)
{
    Enabled = enabled;
}

void SetOverlayParameter(std::string const& name, float value)
{
    if (name == "tanv")
        TanHalfFovV = value;
    else if (name == "near")
        NearPlane = value;
    else if (name == "far")
        FarPlane = value;
    else if (name == "revz")
        ReversedDepth = value != 0.f;
    else if (name == "linebias")
        LineBias = value;
    else if (name == "facebias")
        FaceBias = value;
    else if (name == "zoff")
        HeightOffset = value;
    else if (name == "depth")
        UseDepth = value != 0.f;
    else if (name == "depthpick")
        DepthPick = static_cast<int>(value);
    else if (name == "dumpdepth")
        DumpDepth = value != 0.f;
    else if (name == "grid")
        TestGrid = static_cast<int>(value);
    else if (name == "map")
        SetServerDataMap(static_cast<int>(value));
    else if (name == "enabled")
        Enabled = value != 0.f;
}

void AddOverlayProvider(OverlayProvider provider)
{
    std::lock_guard<std::mutex> lock(ProviderMutex);
    Providers.push_back(std::move(provider));
}
