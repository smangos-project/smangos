#include "TerrainLayers.hpp"

#include "hook_lib/HookManager.hpp"
#include "hook_lib/Log.hpp"
#include "hook_lib/ProcessImage.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <unordered_map>
#include <vector>

// Addresses are Ghidra addresses (image base 0x140000000) for WowClassic.exe 1.15.6.58797.
//
// Map chunk (CMapChunk): +0x18 map area, +0xa8 MCNK flags, +0xb4 u16 layer count,
// +0xc8 MCLY entries (texture id already turned into a file id), +0x170 MCAL.
// Map area: +0x4b0 alpha LOD shift, +0x4b8 u32* MDID file ids.
// Render chunk: +0x8 flags (0x20: 4 bytes per blend texel, else 2), +0x58 chunks[4],
// +0xc8 first batch.
// Batch: +0 flags (1: two chunks tall, 2: two chunks wide), +1 mask of covered chunks,
// +2 layer count, +0x10 render chunk, +0x18 next batch, +0x28 blend texture.

namespace {

constexpr std::uintptr_t ParseChunkAddress = 0x141934760;
constexpr std::uintptr_t BuildBlendTexturesAddress = 0x14193be00;
constexpr std::uintptr_t BindBatchAddress = 0x1418bfb40;
constexpr std::uintptr_t CreateTextureAddress = 0x14179b990;
constexpr std::uintptr_t ReleaseTextureAddress = 0x14179e660;
constexpr std::uintptr_t TextureHandleAddress = 0x14179d3c0;
constexpr std::uintptr_t TrivialFunctionManagerAddress = 0x14022bf90;
constexpr std::uintptr_t FindTerrainTextureAddress = 0x1418d6980;
constexpr std::uintptr_t TerrainTextureDiffuseAddress = 0x1418d6710;

constexpr std::uint32_t MclyMagic = 0x4d434c59;
constexpr std::uint32_t MclyEntrySize = 16;
constexpr std::uint32_t ClientLayers = 4;
constexpr std::uint32_t ExtraLayers = 4;
constexpr std::uint32_t BlendSlot = 22;
constexpr std::uint32_t FirstDiffuseSlot = 23;
constexpr std::uint32_t LayerUseAlpha = 0x100;
constexpr std::uint32_t LayerCompressedAlpha = 0x200;
constexpr std::uint32_t ChunkDoNotFixAlpha = 0x8000;

using ParseChunkFn = void (*)(std::uintptr_t chunk, std::uintptr_t data, int isRoot);
using BuildBlendTexturesFn = void (*)(std::uintptr_t renderChunk);
using BindBatchFn = void (*)(void* ctx, std::uint8_t* batch, void* params, float a, float b);
using CreateTextureFn = void* (*)(std::uint8_t type, std::uint32_t width, std::uint32_t height, std::uint32_t depth,
                                  std::uint8_t format, std::uint8_t format2, std::uint64_t flags, void* source,
                                  char const* name, char unique);
using ReleaseTextureFn = void (*)(void** texture);
using TextureHandleFn = void* (*)(void* texture);
using FindTerrainTextureFn = void* (*)(std::uint32_t fileId);
using TerrainTextureDiffuseFn = void* (*)(void* record);
using SetTextureFn = void (*)(void* ctx, int stage, std::uint32_t slot, void* handle);

ParseChunkFn OriginalParseChunk = nullptr;
BuildBlendTexturesFn OriginalBuildBlendTextures = nullptr;
BindBatchFn OriginalBindBatch = nullptr;
CreateTextureFn CreateTexture = nullptr;
ReleaseTextureFn ReleaseTexture = nullptr;
TextureHandleFn TextureHandle = nullptr;
void* TrivialFunctionManager = nullptr;
FindTerrainTextureFn FindTerrainTexture = nullptr;
TerrainTextureDiffuseFn TerrainTextureDiffuse = nullptr;

struct ExtraLayer {
    std::uint32_t textureIndex = 0;
    std::uint32_t flags = 0;
    std::uint32_t alphaOffset = 0;
};

struct ChunkLayers {
    std::uint32_t count = 0;
    std::array<ExtraLayer, ExtraLayers> layers;
};

// Pixels handed to the client's texture loader; lives as long as its texture.
struct BlendPixels {
    std::uint32_t pitch = 0;
    std::vector<std::uint8_t> data;
};

struct BatchLayers {
    void* blend = nullptr;
    BlendPixels* pixels = nullptr;
    std::array<std::uint32_t, ExtraLayers> fileIds = {};
};

// Layout of the client's std::function-like texture source.
struct TextureSource {
    void* invoke = nullptr;
    void* manager = nullptr;
    std::uintptr_t storage[2] = {};
};

std::mutex Mutex;
std::unordered_map<std::uintptr_t, ChunkLayers> Chunks;
std::unordered_map<std::uintptr_t, BatchLayers> Batches;
void* ZeroBlend = nullptr;

std::uintptr_t Rebase(std::uintptr_t ghidraAddress)
{
    return reinterpret_cast<std::uintptr_t>(GetProcessImage().BaseAddress) + (ghidraAddress - 0x140000000ull);
}

template <typename T> T At(std::uintptr_t address)
{
    T value;
    std::memcpy(&value, reinterpret_cast<void const*>(address), sizeof(T));
    return value;
}

void InvokeBlendSource(char op, int* width, int* pitch, void** data, std::uintptr_t* storage)
{
    auto* pixels = reinterpret_cast<BlendPixels*>(storage[0]);
    if (op == 0) {
        void* copy = std::malloc(pixels->data.size());
        std::memcpy(copy, pixels->data.data(), pixels->data.size());
        *data = copy;
    } else if (op == 1) {
        *pitch = static_cast<int>(pixels->pitch) * *width;
    } else if (op == 2) {
        std::free(*data);
    }
}

void* CreateBlendTexture(BlendPixels* pixels, std::uint32_t width, std::uint32_t height, std::uint8_t wrapFormat,
                         std::uint8_t format)
{
    TextureSource source;
    source.invoke = reinterpret_cast<void*>(&InvokeBlendSource);
    source.manager = TrivialFunctionManager;
    source.storage[0] = reinterpret_cast<std::uintptr_t>(pixels);
    return CreateTexture(0, width, height, 1, wrapFormat, format, 0x82001, &source, "TerrainBlend2", 0);
}

// One chunk's 64x64 alpha map for a layer, 8 bits per texel.
void DecodeAlpha(std::uint8_t const* mcal, ExtraLayer const& layer, bool bigAlpha, bool fixAlpha,
                 std::uint8_t* out)
{
    std::memset(out, 0, 64 * 64);
    if (!mcal || !(layer.flags & LayerUseAlpha))
        return;
    std::uint8_t const* src = mcal + layer.alphaOffset;
    if (layer.flags & LayerCompressedAlpha) {
        for (std::uint32_t n = 0; n < 64 * 64;) {
            std::uint8_t control = *src++;
            std::uint32_t count = control & 0x7f;
            bool fill = control & 0x80;
            for (std::uint32_t i = 0; i < count && n < 64 * 64; ++i, ++n)
                out[n] = fill ? *src : src[i];
            src += fill ? 1 : count;
        }
    } else if (bigAlpha) {
        std::memcpy(out, src, 64 * 64);
    } else {
        for (std::uint32_t n = 0; n < 64 * 64; ++n) {
            std::uint8_t nibble = (src[n / 2] >> ((n & 1) * 4)) & 0xf;
            out[n] = static_cast<std::uint8_t>(nibble * 17);
        }
        if (fixAlpha) {
            for (std::uint32_t y = 0; y < 64; ++y)
                out[y * 64 + 63] = out[y * 64 + 62];
            std::memcpy(out + 63 * 64, out + 62 * 64, 64);
        }
    }
}

void ReleaseBatch(BatchLayers& batch)
{
    // The pixels stay allocated: a load the client queued earlier may still read them.
    if (batch.blend)
        ReleaseTexture(&batch.blend);
}

// Mirrors the client's own blend texture for this batch, with layers 5-8 in r, g, b, a.
void BuildBatchLayers(std::uintptr_t renderChunk, std::uint8_t const* batch)
{
    auto const key = reinterpret_cast<std::uintptr_t>(batch);
    if (auto it = Batches.find(key); it != Batches.end()) {
        ReleaseBatch(it->second);
        Batches.erase(it);
    }
    if (batch[2] == 0)
        return;

    std::uint8_t const flags = batch[0];
    std::uint8_t const covered = batch[1];
    std::uintptr_t const firstChunk = At<std::uintptr_t>(renderChunk + 0x58);
    std::uint32_t const size = 64u >> (At<std::uint32_t>(At<std::uintptr_t>(firstChunk + 0x18) + 0x4b0) & 0x1f);
    std::uint32_t const width = (flags & 2) ? size * 2 : size;
    std::uint32_t const height = (flags & 1) ? size * 2 : size;
    std::uint32_t const renderFlags = At<std::uint32_t>(renderChunk + 8);
    bool const bigAlpha = renderFlags & 0x20;

    std::uint32_t offsets[4] = { 0, 0, 0, 0 };
    if ((flags & 3) == 3) {
        offsets[1] = size;
        offsets[2] = width * size;
        offsets[3] = width * size + size;
    } else if (flags & 2) {
        offsets[1] = size;
    } else if (flags & 1) {
        offsets[1] = size * size;
    }

    BatchLayers result;
    auto* pixels = new BlendPixels;
    pixels->pitch = bigAlpha ? 4 : 2;
    pixels->data.assign(width * height * pixels->pitch, 0);

    bool any = false;
    std::uint32_t coveredIndex = 0;
    std::uint8_t alpha[ExtraLayers][64 * 64];
    for (std::uint32_t i = 0; i < 4; ++i) {
        if (!(covered & (1u << i)))
            continue;
        std::uint32_t const offset = offsets[coveredIndex++];
        std::uintptr_t const chunk = At<std::uintptr_t>(renderChunk + 0x58 + i * 8);
        auto it = chunk ? Chunks.find(chunk) : Chunks.end();
        if (it == Chunks.end())
            continue;

        ChunkLayers const& extra = it->second;
        auto const* mcal = At<std::uint8_t const*>(chunk + 0x170);
        bool const fixAlpha = !(At<std::uint32_t>(chunk + 0xa8) & ChunkDoNotFixAlpha);
        auto const* fileIds = At<std::uint32_t const*>(At<std::uintptr_t>(chunk + 0x18) + 0x4b8);
        for (std::uint32_t layer = 0; layer < ExtraLayers; ++layer) {
            if (layer < extra.count) {
                DecodeAlpha(mcal, extra.layers[layer], bigAlpha, fixAlpha, alpha[layer]);
                if (!any)
                    result.fileIds[layer] = fileIds[extra.layers[layer].textureIndex];
            } else {
                std::memset(alpha[layer], 0, sizeof(alpha[layer]));
            }
        }
        any = true;

        std::uint32_t const step = 64 / size;
        for (std::uint32_t y = 0; y < size; ++y) {
            for (std::uint32_t x = 0; x < size; ++x) {
                std::uint32_t const src = y * step * 64 + x * step;
                std::uint32_t const texel = offset + y * width + x;
                std::uint8_t const r = alpha[0][src], g = alpha[1][src], b = alpha[2][src], a = alpha[3][src];
                if (bigAlpha) {
                    std::uint8_t* out = &pixels->data[texel * 4];
                    out[0] = b;
                    out[1] = g;
                    out[2] = r;
                    out[3] = a;
                } else {
                    std::uint16_t const packed = static_cast<std::uint16_t>((a >> 4) << 12 | (r >> 4) << 8 |
                                                                            (g >> 4) << 4 | (b >> 4));
                    std::memcpy(&pixels->data[texel * 2], &packed, 2);
                }
            }
        }
    }

    if (!any) {
        delete pixels;
        return;
    }

    std::uint8_t const wrapFormat = (~(renderFlags >> 6) & 1) | 2;
    std::uint8_t const format = (~(renderFlags >> 5) & 1) | 2;
    result.pixels = pixels;
    result.blend = CreateBlendTexture(pixels, width, height, wrapFormat, format);
    Batches.emplace(key, result);
}

void DetourParseChunk(std::uintptr_t chunk, std::uintptr_t data, int isRoot)
{
    if (isRoot) {
        {
            std::lock_guard lock(Mutex);
            Chunks.erase(chunk);
        }
        OriginalParseChunk(chunk, data, isRoot);
        return;
    }

    auto* sub = reinterpret_cast<std::uint8_t*>(data + 8);
    auto* end = sub + At<std::int32_t>(data + 4);
    while (sub + 8 <= end) {
        std::uint32_t const magic = At<std::uint32_t>(reinterpret_cast<std::uintptr_t>(sub));
        std::uint32_t const size = At<std::uint32_t>(reinterpret_cast<std::uintptr_t>(sub) + 4);
        if (magic == MclyMagic && size > ClientLayers * MclyEntrySize) {
            ChunkLayers extra;
            std::uint8_t* entries = sub + 8;
            std::uint32_t const total = size / MclyEntrySize;
            extra.count = std::min(total - ClientLayers, ExtraLayers);
            for (std::uint32_t i = 0; i < extra.count; ++i) {
                std::uint8_t const* entry = entries + (ClientLayers + i) * MclyEntrySize;
                std::memcpy(&extra.layers[i].textureIndex, entry, 4);
                std::memcpy(&extra.layers[i].flags, entry + 4, 4);
                std::memcpy(&extra.layers[i].alphaOffset, entry + 8, 4);
            }

            // Show the client 4 layers and hide the rest behind an unknown sub-chunk header for the
            // duration of the parse; the file data is left as it was.
            std::uint8_t* hidden = entries + ClientLayers * MclyEntrySize;
            std::uint8_t saved[8];
            std::memcpy(saved, hidden, 8);
            std::uint32_t const clientSize = ClientLayers * MclyEntrySize;
            std::uint32_t const filler[2] = { 0, size - clientSize - 8 };
            std::memcpy(hidden, filler, 8);
            std::memcpy(sub + 4, &clientSize, 4);
            OriginalParseChunk(chunk, data, isRoot);
            std::memcpy(sub + 4, &size, 4);
            std::memcpy(hidden, saved, 8);

            std::lock_guard lock(Mutex);
            Chunks[chunk] = extra;
            return;
        }
        sub += 8 + size;
    }
    OriginalParseChunk(chunk, data, isRoot);
}

void DetourBuildBlendTextures(std::uintptr_t renderChunk)
{
    OriginalBuildBlendTextures(renderChunk);

    std::lock_guard lock(Mutex);
    if (!ZeroBlend) {
        static BlendPixels zero{ 4, std::vector<std::uint8_t>(4 * 4 * 4, 0) };
        ZeroBlend = CreateBlendTexture(&zero, 4, 4, 2, 2);
    }
    if (Chunks.empty() && Batches.empty())
        return;
    for (auto* batch = At<std::uint8_t*>(renderChunk + 0xc8); batch;
         batch = At<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(batch) + 0x18))
        BuildBatchLayers(renderChunk, batch);
}

void DetourBindBatch(void* ctx, std::uint8_t* batch, void* params, float a, float b)
{
    OriginalBindBatch(ctx, batch, params, a, b);

    auto const setTexture = At<SetTextureFn>(At<std::uintptr_t>(reinterpret_cast<std::uintptr_t>(ctx)) + 0x38);
    std::lock_guard lock(Mutex);
    auto it = Batches.find(reinterpret_cast<std::uintptr_t>(batch));
    if (it == Batches.end()) {
        setTexture(ctx, 2, BlendSlot, TextureHandle(ZeroBlend));
        return;
    }
    setTexture(ctx, 2, BlendSlot, TextureHandle(it->second.blend));
    for (std::uint32_t i = 0; i < ExtraLayers; ++i) {
        std::uint32_t const fileId = it->second.fileIds[i];
        void* record = fileId ? FindTerrainTexture(fileId) : nullptr;
        void* texture = record ? TerrainTextureDiffuse(record) : nullptr;
        setTexture(ctx, 2, FirstDiffuseSlot + i, TextureHandle(texture));
    }
}

bool Hook(char const* name, std::uintptr_t address, void* detour, void** original)
{
    HookRegistration hook;
    hook.Name = name;
    hook.Target = Rebase(address);
    hook.Detour = detour;
    hook.Original = original;
    return InstallHook(hook);
}

} // namespace

bool InstallTerrainLayerHooks()
{
    CreateTexture = reinterpret_cast<CreateTextureFn>(Rebase(CreateTextureAddress));
    ReleaseTexture = reinterpret_cast<ReleaseTextureFn>(Rebase(ReleaseTextureAddress));
    TextureHandle = reinterpret_cast<TextureHandleFn>(Rebase(TextureHandleAddress));
    TrivialFunctionManager = reinterpret_cast<void*>(Rebase(TrivialFunctionManagerAddress));
    FindTerrainTexture = reinterpret_cast<FindTerrainTextureFn>(Rebase(FindTerrainTextureAddress));
    TerrainTextureDiffuse = reinterpret_cast<TerrainTextureDiffuseFn>(Rebase(TerrainTextureDiffuseAddress));

    bool ok = Hook("TerrainParseChunk", ParseChunkAddress, reinterpret_cast<void*>(&DetourParseChunk),
                   reinterpret_cast<void**>(&OriginalParseChunk));
    ok = Hook("TerrainBuildBlendTextures", BuildBlendTexturesAddress,
              reinterpret_cast<void*>(&DetourBuildBlendTextures),
              reinterpret_cast<void**>(&OriginalBuildBlendTextures)) && ok;
    ok = Hook("TerrainBindBatch", BindBatchAddress, reinterpret_cast<void*>(&DetourBindBatch),
              reinterpret_cast<void**>(&OriginalBindBatch)) && ok;
    if (ok)
        HOOK_LOG_INFO("terrain layers: hooks installed (up to %u layers per chunk)", ClientLayers + ExtraLayers);
    return ok;
}
