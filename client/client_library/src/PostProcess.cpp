#define NOMINMAX
#include "PostProcess.hpp"

#include "SwapChainCapture.hpp"

#include <windows.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include "hook_lib/Log.hpp"

#include <algorithm>
#include <cstring>
#include <map>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace {

constexpr UINT MaxFrames = 4;
constexpr UINT DescriptorsPerFrame = 4;

char const* const VertexShader = R"(
struct VSOut { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
VSOut main(uint id : SV_VertexID)
{
    VSOut o;
    o.uv = float2((id << 1) & 2, id & 2);
    o.position = float4(o.uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return o;
}
)";

struct Effect {
    PostProcessEffect info;
    ComPtr<ID3DBlob> shader;
    std::map<DXGI_FORMAT, ComPtr<ID3D12PipelineState>> pipelines;
    bool failed = false;
};

std::mutex EffectMutex;

std::vector<Effect>& Effects()
{
    static std::vector<Effect> effects;
    return effects;
}

std::mutex ParameterMutex;
std::unordered_map<std::string, float> Parameters;

struct Renderer {
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3D12DescriptorHeap> srvHeap;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    UINT srvStep = 0;
    UINT rtvStep = 0;
    ComPtr<ID3D12Resource> textures[2];
    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    UINT64 width = 0;
    UINT height = 0;
    ComPtr<ID3D12CommandAllocator> allocators[MaxFrames];
    UINT64 fenceValues[MaxFrames] = {};
    ComPtr<ID3D12GraphicsCommandList> list;
    ComPtr<ID3D12Fence> fence;
    HANDLE event = nullptr;
    UINT64 nextFence = 0;
    UINT slot = 0;
    bool failed = false;
};

Renderer Post;

bool Compile(char const* source, char const* target, ComPtr<ID3DBlob>& blob, char const* name)
{
    HMODULE module = LoadLibraryW(L"d3dcompiler_47.dll");
    auto compile = module ? reinterpret_cast<pD3DCompile>(GetProcAddress(module, "D3DCompile")) : nullptr;
    if (!compile)
        return false;
    ComPtr<ID3DBlob> errors;
    HRESULT const hr = compile(source, std::strlen(source), name, nullptr, nullptr, "main", target, 0, 0, &blob, &errors);
    if (FAILED(hr))
        HOOK_LOG_ERROR("postprocess: %s: %s", name, errors ? static_cast<char const*>(errors->GetBufferPointer()) : "compile failed");
    return SUCCEEDED(hr);
}

DXGI_FORMAT ColorViewFormat(DXGI_FORMAT format)
{
    switch (format) {
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        return DXGI_FORMAT_B8G8R8A8_UNORM;
    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        return DXGI_FORMAT_R10G10B10A2_UNORM;
    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    default:
        return format;
    }
}

DXGI_FORMAT DepthViewFormat(DXGI_FORMAT format)
{
    switch (format) {
    case DXGI_FORMAT_R32_TYPELESS:
        return DXGI_FORMAT_R32_FLOAT;
    case DXGI_FORMAT_R24G8_TYPELESS:
        return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    case DXGI_FORMAT_R16_TYPELESS:
        return DXGI_FORMAT_R16_UNORM;
    case DXGI_FORMAT_R32G8X24_TYPELESS:
        return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
    default:
        return DXGI_FORMAT_UNKNOWN;
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

struct Barriers {
    D3D12_RESOURCE_BARRIER items[4];
    UINT count = 0;

    void Add(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
    {
        if (resource && before != after)
            items[count++] = Transition(resource, before, after);
    }

    void Submit(ID3D12GraphicsCommandList* list)
    {
        if (count)
            list->ResourceBarrier(count, items);
        count = 0;
    }
};

bool CreateRenderer(Renderer& r, ID3D12Device* device)
{
    r.device = device;
    if (!Compile(VertexShader, "vs_5_0", r.vertexShader, "postprocess vertex shader"))
        return false;

    D3D12_DESCRIPTOR_RANGE range = {};
    range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range.NumDescriptors = 2;
    D3D12_ROOT_PARAMETER parameters[2] = {};
    parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    parameters[0].Constants.Num32BitValues = 20;
    parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    parameters[1].DescriptorTable.NumDescriptorRanges = 1;
    parameters[1].DescriptorTable.pDescriptorRanges = &range;
    parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    D3D12_STATIC_SAMPLER_DESC samplers[2] = {};
    for (UINT i = 0; i < 2; ++i) {
        samplers[i].Filter = i ? D3D12_FILTER_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        samplers[i].AddressU = samplers[i].AddressV = samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplers[i].MaxLOD = D3D12_FLOAT32_MAX;
        samplers[i].ShaderRegister = i;
        samplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    }
    D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
    rootDesc.NumParameters = 2;
    rootDesc.pParameters = parameters;
    rootDesc.NumStaticSamplers = 2;
    rootDesc.pStaticSamplers = samplers;
    ComPtr<ID3DBlob> serialized, errors;
    auto serialize = reinterpret_cast<PFN_D3D12_SERIALIZE_ROOT_SIGNATURE>(
        GetProcAddress(GetModuleHandleW(L"d3d12.dll"), "D3D12SerializeRootSignature"));
    if (!serialize || FAILED(serialize(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &errors)) ||
        FAILED(device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
                                           IID_PPV_ARGS(&r.rootSignature))))
        return false;

    D3D12_DESCRIPTOR_HEAP_DESC srvDesc = {};
    srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvDesc.NumDescriptors = MaxFrames * DescriptorsPerFrame;
    srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDesc.NumDescriptors = 3;
    if (FAILED(device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&r.srvHeap))) ||
        FAILED(device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&r.rtvHeap))) ||
        FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&r.fence))))
        return false;
    r.srvStep = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    r.rtvStep = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    r.event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    for (ComPtr<ID3D12CommandAllocator>& allocator : r.allocators)
        if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))))
            return false;
    if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, r.allocators[0].Get(), nullptr,
                                         IID_PPV_ARGS(&r.list))))
        return false;
    r.list->Close();
    return true;
}

void WaitFor(Renderer& r, UINT64 value)
{
    if (r.fence->GetCompletedValue() < value) {
        r.fence->SetEventOnCompletion(value, r.event);
        WaitForSingleObject(r.event, 2000);
    }
}

// The ping-pong textures match the scene colour and rest in PIXEL_SHADER_RESOURCE.
bool EnsureTextures(Renderer& r, D3D12_RESOURCE_DESC const& scene)
{
    if (r.textures[0] && r.format == scene.Format && r.width == scene.Width && r.height == scene.Height)
        return true;
    WaitFor(r, r.nextFence);
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width = scene.Width;
    desc.Height = scene.Height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = scene.Format;
    desc.SampleDesc.Count = 1;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type = D3D12_HEAP_TYPE_DEFAULT;
    for (ComPtr<ID3D12Resource>& texture : r.textures) {
        texture.Reset();
        if (FAILED(r.device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,
                                                     D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr,
                                                     IID_PPV_ARGS(&texture))))
            return false;
    }
    r.format = scene.Format;
    r.width = scene.Width;
    r.height = scene.Height;
    return true;
}

ID3D12PipelineState* GetPipeline(Renderer& r, Effect& effect, DXGI_FORMAT format)
{
    ComPtr<ID3D12PipelineState>& pipeline = effect.pipelines[format];
    if (pipeline)
        return pipeline.Get();
    if (!effect.shader && !Compile(effect.info.pixelShaderHlsl, "ps_5_0", effect.shader, effect.info.name)) {
        effect.failed = true;
        return nullptr;
    }
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature = r.rootSignature.Get();
    pso.VS = {r.vertexShader->GetBufferPointer(), r.vertexShader->GetBufferSize()};
    pso.PS = {effect.shader->GetBufferPointer(), effect.shader->GetBufferSize()};
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = format;
    pso.SampleDesc.Count = 1;
    pso.SampleMask = UINT_MAX;
    pso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    pso.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    if (FAILED(r.device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pipeline)))) {
        HOOK_LOG_ERROR("postprocess: %s: pipeline creation failed (format %d)", effect.info.name, static_cast<int>(format));
        effect.failed = true;
    }
    return pipeline.Get();
}

struct Pass {
    Effect* effect;
    float params[16];
};

void Run(ID3D12CommandQueue* queue, SceneTargets const& scene, ExecuteLists execute)
{
    if (Post.failed)
        return;
    std::lock_guard<std::mutex> lock(EffectMutex);
    std::vector<Pass> passes;
    for (Effect& effect : Effects()) {
        if (effect.failed || (effect.info.isEnabled && !effect.info.isEnabled()))
            continue;
        Pass pass = {&effect, {}};
        if (!effect.info.setup || effect.info.setup(pass.params))
            passes.push_back(pass);
    }
    if (passes.empty())
        return;

    ComPtr<ID3D12Device> device;
    if (FAILED(scene.color->GetDevice(IID_PPV_ARGS(&device))))
        return;
    if (!Post.device) {
        if (!CreateRenderer(Post, device.Get())) {
            HOOK_LOG_ERROR("postprocess: renderer creation failed");
            Post.failed = true;
            return;
        }
        HOOK_LOG_INFO("postprocess: renderer ready");
    }
    if (Post.device.Get() != device.Get())
        return;
    D3D12_RESOURCE_DESC const sceneDesc = scene.color->GetDesc();
    if (!EnsureTextures(Post, sceneDesc)) {
        HOOK_LOG_ERROR("postprocess: texture creation failed");
        Post.failed = true;
        return;
    }
    DXGI_FORMAT const viewFormat = ColorViewFormat(sceneDesc.Format);
    for (Pass const& pass : passes)
        if (!GetPipeline(Post, *pass.effect, viewFormat))
            return;

    UINT const slot = Post.slot;
    Post.slot = (Post.slot + 1) % MaxFrames;
    WaitFor(Post, Post.fenceValues[slot]);

    // Per frame: (texture 0, depth), (texture 1, depth).
    D3D12_CPU_DESCRIPTOR_HANDLE srv = Post.srvHeap->GetCPUDescriptorHandleForHeapStart();
    srv.ptr += static_cast<SIZE_T>(slot) * DescriptorsPerFrame * Post.srvStep;
    DXGI_FORMAT const depthFormat =
        scene.hasDepth ? DepthViewFormat(scene.depth.resource->GetDesc().Format) : DXGI_FORMAT_UNKNOWN;
    bool const haveDepth = depthFormat != DXGI_FORMAT_UNKNOWN;
    for (UINT i = 0; i < 2; ++i) {
        D3D12_SHADER_RESOURCE_VIEW_DESC view = {};
        view.Format = viewFormat;
        view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        view.Texture2D.MipLevels = 1;
        D3D12_CPU_DESCRIPTOR_HANDLE handle = {srv.ptr + (i * 2) * Post.srvStep};
        Post.device->CreateShaderResourceView(Post.textures[i].Get(), &view, handle);
        view.Format = haveDepth ? depthFormat : DXGI_FORMAT_R32_FLOAT;
        handle.ptr += Post.srvStep;
        Post.device->CreateShaderResourceView(haveDepth ? scene.depth.resource.Get() : nullptr, &view, handle);
    }
    D3D12_CPU_DESCRIPTOR_HANDLE rtv[3];
    for (UINT i = 0; i < 3; ++i) {
        rtv[i] = Post.rtvHeap->GetCPUDescriptorHandleForHeapStart();
        rtv[i].ptr += i * Post.rtvStep;
        D3D12_RENDER_TARGET_VIEW_DESC view = {};
        view.Format = viewFormat;
        view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        Post.device->CreateRenderTargetView(i < 2 ? Post.textures[i].Get() : scene.color.Get(), &view, rtv[i]);
    }

    ID3D12GraphicsCommandList* list = Post.list.Get();
    Post.allocators[slot]->Reset();
    list->Reset(Post.allocators[slot].Get(), nullptr);

    D3D12_RESOURCE_STATES const depthState = scene.depth.state;
    bool const depthReadable = (depthState & D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) != 0;
    Barriers barriers;
    barriers.Add(scene.color.Get(), scene.colorState, D3D12_RESOURCE_STATE_COPY_SOURCE);
    barriers.Add(Post.textures[0].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    if (haveDepth && !depthReadable)
        barriers.Add(scene.depth.resource.Get(), depthState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    barriers.Submit(list);
    list->CopyResource(Post.textures[0].Get(), scene.color.Get());
    barriers.Add(Post.textures[0].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    barriers.Add(scene.color.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    barriers.Submit(list);

    float const width = static_cast<float>(sceneDesc.Width), height = static_cast<float>(sceneDesc.Height);
    D3D12_VIEWPORT viewport = {0, 0, width, height, 0.f, 1.f};
    D3D12_RECT scissor = {0, 0, static_cast<LONG>(sceneDesc.Width), static_cast<LONG>(sceneDesc.Height)};
    ID3D12DescriptorHeap* heaps[] = {Post.srvHeap.Get()};
    list->SetDescriptorHeaps(1, heaps);
    list->SetGraphicsRootSignature(Post.rootSignature.Get());
    list->RSSetViewports(1, &viewport);
    list->RSSetScissorRects(1, &scissor);
    list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    float const frame[4] = {width, height, 1.f / width, 1.f / height};
    list->SetGraphicsRoot32BitConstants(0, 4, frame, 0);

    D3D12_GPU_DESCRIPTOR_HANDLE const tables = Post.srvHeap->GetGPUDescriptorHandleForHeapStart();
    for (std::size_t i = 0; i < passes.size(); ++i) {
        UINT const input = i % 2;
        bool const last = i + 1 == passes.size();
        UINT const output = last ? 2 : 1 - input;
        if (!last) {
            barriers.Add(Post.textures[output].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                         D3D12_RESOURCE_STATE_RENDER_TARGET);
            barriers.Submit(list);
        }
        list->OMSetRenderTargets(1, &rtv[output], FALSE, nullptr);
        list->SetPipelineState(GetPipeline(Post, *passes[i].effect, viewFormat));
        list->SetGraphicsRoot32BitConstants(0, 16, passes[i].params, 4);
        list->SetGraphicsRootDescriptorTable(
            1, {tables.ptr + (static_cast<UINT64>(slot) * DescriptorsPerFrame + input * 2) * Post.srvStep});
        list->DrawInstanced(3, 1, 0, 0);
        if (!last) {
            barriers.Add(Post.textures[output].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
                         D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            barriers.Submit(list);
        }
    }

    barriers.Add(scene.color.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, scene.colorState);
    if (haveDepth && !depthReadable)
        barriers.Add(scene.depth.resource.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, depthState);
    barriers.Submit(list);
    if (FAILED(list->Close()))
        return;
    ID3D12CommandList* lists[] = {list};
    execute(queue, 1, lists);
    Post.fenceValues[slot] = ++Post.nextFence;
    queue->Signal(Post.fence.Get(), Post.fenceValues[slot]);
}

} // namespace

void RegisterPostProcessEffect(PostProcessEffect effect)
{
    std::lock_guard<std::mutex> lock(EffectMutex);
    std::vector<Effect>& effects = Effects();
    auto at = std::upper_bound(effects.begin(), effects.end(), effect.order,
                               [](int order, Effect const& e) { return order < e.info.order; });
    effects.insert(at, Effect{effect});
}

void StartPostProcess()
{
    std::thread([] {
        LoadLibraryW(L"d3d12.dll");
        LoadLibraryW(L"dxgi.dll");
        SetSceneCallback(&Run);
        EnsureSwapChainHooks();
    }).detach();
}

void SetPostProcessParameter(std::string const& name, float value)
{
    std::lock_guard<std::mutex> lock(ParameterMutex);
    Parameters[name] = value;
}

float GetPostProcessParameter(std::string const& name, float fallback)
{
    std::lock_guard<std::mutex> lock(ParameterMutex);
    auto it = Parameters.find(name);
    return it == Parameters.end() ? fallback : it->second;
}
