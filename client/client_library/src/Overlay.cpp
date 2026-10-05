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

#include <atomic>
#include <cmath>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace {

constexpr std::uintptr_t CameraRoot = 0x1438921a8;
constexpr UINT MaxFrames = 4;
constexpr UINT MaxVertices = 1u << 20;

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
std::atomic<float> TanHalfFovV{0.5f};
std::atomic<int> TestGrid{0};

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
cbuffer C : register(b0) { float4x4 viewProj; };
struct VSIn { float3 pos : POSITION; float4 color : COLOR; };
struct VSOut { float4 pos : SV_POSITION; float4 color : COLOR; };
VSOut VS(VSIn v) { VSOut o; o.pos = mul(float4(v.pos, 1), viewProj); o.color = v.color; return o; }
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
    ComPtr<ID3D12PipelineState> pipeline;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
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

bool CreateRenderer(Renderer& r, ID3D12Device* device, DXGI_FORMAT format)
{
    r.device = device;
    r.format = format;
    ComPtr<ID3DBlob> vs, ps;
    if (!CompileShader("VS", "vs_5_0", vs) || !CompileShader("PS", "ps_5_0", ps))
        return false;

    D3D12_ROOT_PARAMETER parameter = {};
    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    parameter.Constants.Num32BitValues = 16;
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

    D3D12_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature = r.rootSignature.Get();
    pso.VS = {vs->GetBufferPointer(), vs->GetBufferSize()};
    pso.PS = {ps->GetBufferPointer(), ps->GetBufferSize()};
    pso.InputLayout = {layout, 2};
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = format;
    pso.SampleDesc.Count = 1;
    pso.SampleMask = UINT_MAX;
    pso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    pso.RasterizerState.DepthClipEnable = TRUE;
    D3D12_RENDER_TARGET_BLEND_DESC& blend = pso.BlendState.RenderTarget[0];
    blend.BlendEnable = TRUE;
    blend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blend.BlendOp = D3D12_BLEND_OP_ADD;
    blend.SrcBlendAlpha = D3D12_BLEND_ZERO;
    blend.DestBlendAlpha = D3D12_BLEND_ONE;
    blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    if (FAILED(device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&r.pipeline))))
        return false;

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.NumDescriptors = 1;
    if (FAILED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&r.rtvHeap))) ||
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

    std::vector<Vertex> vertices;
    if (TestGrid)
        BuildTestGrid(camera, vertices);
    {
        std::lock_guard<std::mutex> lock(ProviderMutex);
        for (OverlayProvider const& provider : Providers)
            provider(camera.eye, [&](float const* a, float const* b, OverlayColor color) {
                std::uint32_t const packed = Pack(color);
                vertices.push_back({a[0], a[1], a[2], packed});
                vertices.push_back({b[0], b[1], b[2], packed});
            });
    }
    if (vertices.size() > MaxVertices)
        vertices.resize(MaxVertices & ~1u);
    if (vertices.empty())
        return;

    FrameResources& frame = Draw.frames[index];
    if (Draw.fence->GetCompletedValue() < frame.fenceValue) {
        Draw.fence->SetEventOnCompletion(frame.fenceValue, Draw.event);
        WaitForSingleObject(Draw.event, 2000);
    }
    std::memcpy(frame.mapped, vertices.data(), vertices.size() * sizeof(Vertex));
    frame.allocator->Reset();
    Draw.list->Reset(frame.allocator.Get(), Draw.pipeline.Get());

    float const width = static_cast<float>(desc.Width), height = static_cast<float>(desc.Height);
    float const tanV = TanHalfFovV, tanH = tanV * width / height;
    float const nearPlane = 0.2f, farPlane = 5000.f;
    float const a = farPlane / (farPlane - nearPlane), b = -nearPlane * a;
    // Row-vector convention: clip = (x,y,z,1) * M, with M columns computed from the camera basis.
    float m[16] = {};
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

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = backBuffer.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    Draw.list->ResourceBarrier(1, &barrier);
    Draw.list->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    D3D12_VIEWPORT viewport = {0, 0, width, height, 0.f, 1.f};
    D3D12_RECT scissor = {0, 0, static_cast<LONG>(desc.Width), static_cast<LONG>(desc.Height)};
    Draw.list->RSSetViewports(1, &viewport);
    Draw.list->RSSetScissorRects(1, &scissor);
    Draw.list->SetGraphicsRootSignature(Draw.rootSignature.Get());
    Draw.list->SetGraphicsRoot32BitConstants(0, 16, m, 0);
    Draw.list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    D3D12_VERTEX_BUFFER_VIEW view = {frame.vertices->GetGPUVirtualAddress(),
                                     static_cast<UINT>(vertices.size() * sizeof(Vertex)), sizeof(Vertex)};
    Draw.list->IASetVertexBuffers(0, 1, &view);
    Draw.list->DrawInstanced(static_cast<UINT>(vertices.size()), 1, 0, 0);
    std::swap(barrier.Transition.StateBefore, barrier.Transition.StateAfter);
    Draw.list->ResourceBarrier(1, &barrier);
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
    // Hooks need d3d12.dll, which the client loads after custom.dll.
    std::thread([] {
        while (!GetModuleHandleW(L"d3d12.dll") || !GetModuleHandleW(L"dxgi.dll"))
            Sleep(250);
        Sleep(2000);
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
