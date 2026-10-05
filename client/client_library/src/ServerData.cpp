#define NOMINMAX
#include "ServerData.hpp"

#include "Overlay.hpp"

#include <windows.h>

#include "hook_lib/Log.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

constexpr double TileSize = 533.33333333;
constexpr double MapHalf = 32 * TileSize;
constexpr std::size_t MaxModelCache = 3000;
constexpr std::size_t MaxTileSegments = 600000;
constexpr float CollisionRadius = 250.f;
constexpr float NavmeshRadius = 200.f;

struct Segment {
    float a[3], b[3];
};

struct Tile {
    std::vector<Segment> collision;
    std::vector<Segment> navmesh;
};

std::atomic<int> MapId{0};
std::atomic<bool> ShowCollision{true};
std::atomic<bool> ShowNavmesh{true};
std::atomic<float> EyeX{0}, EyeY{0};
std::atomic<bool> HaveEye{false};

std::mutex TilesMutex;
std::map<std::pair<int, int>, std::shared_ptr<Tile const>> Tiles;
int TilesMap = -1;

std::string DataDir;

bool ReadFile(std::string const& path, std::vector<std::uint8_t>& bytes)
{
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in)
        return false;
    bytes.resize(static_cast<std::size_t>(in.tellg()));
    in.seekg(0);
    in.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return true;
}

struct Cursor {
    std::uint8_t const* data;
    std::size_t size, at = 0;
    bool failed = false;

    template <typename T> T Read()
    {
        T value{};
        if (failed || sizeof(T) > size - at) {
            failed = true;
            return value;
        }
        std::memcpy(&value, data + at, sizeof(T));
        at += sizeof(T);
        return value;
    }
    void Skip(std::size_t n)
    {
        if (failed || n > size - at)
            failed = true;
        else
            at += n;
    }
    bool Tag(char const* tag, std::size_t n)
    {
        if (failed || n > size - at || std::memcmp(data + at, tag, n) != 0)
            return failed = true, false;
        at += n;
        return true;
    }
};

struct Wireframe {
    std::vector<float> vertices;
    std::vector<std::uint32_t> edges;
};

std::shared_ptr<Wireframe const> ParseModel(std::vector<std::uint8_t> const& bytes)
{
    Cursor in{bytes.data(), bytes.size()};
    if (!in.Tag("VMAP_7.0", 8) || !in.Tag("WMOD", 4))
        return nullptr;
    in.Skip(8);
    if (!in.Tag("GMOD", 4))
        return nullptr;
    std::uint32_t const groups = in.Read<std::uint32_t>();
    auto wire = std::make_shared<Wireframe>();
    std::unordered_set<std::uint64_t> seen;
    for (std::uint32_t g = 0; g < groups && !in.failed; ++g) {
        in.Skip(24 + 8);
        if (!in.Tag("VERT", 4))
            return nullptr;
        in.Skip(4);
        std::uint32_t const vertexCount = in.Read<std::uint32_t>();
        if (vertexCount == 0)
            continue;
        std::uint32_t const base = static_cast<std::uint32_t>(wire->vertices.size() / 3);
        std::size_t const at = wire->vertices.size();
        wire->vertices.resize(at + std::size_t(vertexCount) * 3);
        for (std::size_t i = 0; i < std::size_t(vertexCount) * 3; ++i)
            wire->vertices[at + i] = in.Read<float>();
        if (!in.Tag("TRIM", 4))
            return nullptr;
        in.Skip(4);
        std::uint32_t const triangleCount = in.Read<std::uint32_t>();
        for (std::uint32_t t = 0; t < triangleCount && !in.failed; ++t) {
            std::uint32_t idx[3] = {in.Read<std::uint32_t>(), in.Read<std::uint32_t>(), in.Read<std::uint32_t>()};
            if (idx[0] >= vertexCount || idx[1] >= vertexCount || idx[2] >= vertexCount)
                return nullptr;
            for (int c = 0; c < 3; ++c) {
                std::uint32_t const from = base + idx[c], to = base + idx[(c + 1) % 3];
                if (seen.insert((std::uint64_t(std::min(from, to)) << 32) | std::max(from, to)).second) {
                    wire->edges.push_back(from);
                    wire->edges.push_back(to);
                }
            }
        }
        if (!in.Tag("MBIH", 4))
            return nullptr;
        in.Skip(24);
        in.Skip(std::size_t(in.Read<std::uint32_t>()) * 4);
        in.Skip(std::size_t(in.Read<std::uint32_t>()) * 4);
        if (!in.Tag("LIQU", 4))
            return nullptr;
        std::uint32_t const liquid = in.Read<std::uint32_t>();
        in.Skip(liquid);
    }
    return in.failed ? nullptr : wire;
}

std::mutex ModelMutex;
std::unordered_map<std::string, std::shared_ptr<Wireframe const>> Models;

std::shared_ptr<Wireframe const> LoadModel(std::string const& name)
{
    {
        std::lock_guard<std::mutex> lock(ModelMutex);
        auto it = Models.find(name);
        if (it != Models.end())
            return it->second;
    }
    std::vector<std::uint8_t> bytes;
    std::shared_ptr<Wireframe const> model;
    if (ReadFile(DataDir + "/vmaps/" + name + ".vmo", bytes))
        model = ParseModel(bytes);
    std::lock_guard<std::mutex> lock(ModelMutex);
    if (Models.size() >= MaxModelCache)
        Models.clear();
    Models[name] = model;
    return model;
}

// Mirrors ModelInstance: Z(yaw)*Y(pitch)*X(roll) from the spawn's (Y, X, Z) degrees, scale, translate.
struct Transform {
    double m[3][3];
    double scale, position[3];

    Transform(float const* pos, float const* rot, float s) : scale(s)
    {
        constexpr double Deg = 3.14159265358979323846 / 180.0;
        double const yaw = rot[1] * Deg, pitch = rot[0] * Deg, roll = rot[2] * Deg;
        double const sz = std::sin(yaw), cz = std::cos(yaw), sy = std::sin(pitch), cy = std::cos(pitch);
        double const sx = std::sin(roll), cx = std::cos(roll);
        double const a[3][3] = {{cy, sy * sx, sy * cx}, {0.0, cx, -sx}, {-sy, cy * sx, cy * cx}};
        for (int c = 0; c < 3; ++c) {
            m[0][c] = cz * a[0][c] - sz * a[1][c];
            m[1][c] = sz * a[0][c] + cz * a[1][c];
            m[2][c] = a[2][c];
        }
        for (int i = 0; i < 3; ++i)
            position[i] = pos[i];
    }

    void ToServer(float const* p, float* out) const
    {
        double const x = p[0] * scale, y = p[1] * scale, z = p[2] * scale;
        double v[3];
        for (int r = 0; r < 3; ++r)
            v[r] = m[r][0] * x + m[r][1] * y + m[r][2] * z + position[r];
        out[0] = static_cast<float>(MapHalf - v[0]);
        out[1] = static_cast<float>(MapHalf - v[1]);
        out[2] = static_cast<float>(v[2]);
    }
};

void BuildCollision(int map, int tx, int ty, std::vector<Segment>& out)
{
    char name[48];
    std::snprintf(name, sizeof(name), "/vmaps/%03d_%02d_%02d.vmtile", map, ty, tx);
    std::vector<std::uint8_t> bytes;
    if (!ReadFile(DataDir + name, bytes))
        return;
    Cursor in{bytes.data(), bytes.size()};
    if (!in.Tag("VMAP_7.0", 8))
        return;
    std::uint32_t const count = in.Read<std::uint32_t>();
    for (std::uint32_t i = 0; i < count && !in.failed; ++i) {
        std::uint32_t const flags = in.Read<std::uint32_t>();
        in.Skip(2 + 4);
        float pos[3], rot[3];
        for (float& f : pos)
            f = in.Read<float>();
        for (float& f : rot)
            f = in.Read<float>();
        float const scale = in.Read<float>();
        if (flags & 4)
            in.Skip(24);
        std::uint32_t const nameLength = in.Read<std::uint32_t>();
        if (in.failed || nameLength > 500 || nameLength > in.size - in.at)
            return;
        std::string const modelName(reinterpret_cast<char const*>(in.data + in.at), nameLength);
        in.Skip(nameLength + 4);
        std::shared_ptr<Wireframe const> model = LoadModel(modelName);
        if (!model || out.size() + model->edges.size() / 2 > MaxTileSegments)
            continue;
        Transform transform(pos, rot, scale);
        std::vector<float> world(model->vertices.size());
        for (std::size_t v = 0; v + 2 < world.size(); v += 3)
            transform.ToServer(&model->vertices[v], &world[v]);
        for (std::size_t e = 0; e + 1 < model->edges.size(); e += 2) {
            Segment s;
            std::memcpy(s.a, &world[model->edges[e] * 3], 12);
            std::memcpy(s.b, &world[model->edges[e + 1] * 3], 12);
            out.push_back(s);
        }
    }
}

void BuildNavmesh(int map, int tx, int ty, std::vector<Segment>& out)
{
    char name[48];
    std::snprintf(name, sizeof(name), "/mmaps/%03d%02d%02d.mmtile", map, tx, ty);
    std::vector<std::uint8_t> bytes;
    if (!ReadFile(DataDir + name, bytes) || bytes.size() < 120)
        return;
    std::uint32_t header[5];
    std::memcpy(header, bytes.data(), sizeof(header));
    if (header[0] != 0x4d4d4150 || header[1] != 7 || header[3] != bytes.size() - 20)
        return;
    std::uint8_t const* data = bytes.data() + 20;
    auto const i32 = [&](std::size_t offset) {
        std::int32_t v;
        std::memcpy(&v, data + offset, 4);
        return v;
    };
    std::int32_t const polyCount = i32(24), vertCount = i32(28), detailMeshCount = i32(36), detailVertCount = i32(40);
    std::int32_t const detailTriCount = i32(44);
    if (polyCount < 0 || vertCount < 0 || detailMeshCount < 0 || detailVertCount < 0 || detailTriCount < 0)
        return;
    auto const align4 = [](std::size_t n) { return (n + 3) & ~std::size_t(3); };
    std::size_t const vertsAt = 100;
    std::size_t const polysAt = vertsAt + align4(12 * std::size_t(vertCount));
    // dtLink is 16 bytes with a 64-bit dtPolyRef, 12 with a 32-bit one.
    std::size_t const bv = 16, off = 36;
    std::int32_t const maxLinks = i32(32), bvNodes = i32(48), offMesh = i32(52);
    std::size_t linkSize = 0;
    for (std::size_t candidate : {16, 12}) {
        std::size_t total = 100 + align4(12 * std::size_t(vertCount)) + align4(32 * std::size_t(polyCount)) +
                            align4(candidate * std::size_t(maxLinks)) + align4(12 * std::size_t(detailMeshCount)) +
                            align4(12 * std::size_t(detailVertCount)) + align4(4 * std::size_t(detailTriCount)) +
                            align4(bv * std::size_t(bvNodes)) + align4(off * std::size_t(offMesh));
        if (total == bytes.size() - 20)
            linkSize = candidate;
    }
    if (!linkSize)
        return;
    std::size_t const detailMeshesAt = polysAt + align4(32 * std::size_t(polyCount)) +
                                       align4(linkSize * std::size_t(maxLinks));
    std::size_t const detailVertsAt = detailMeshesAt + align4(12 * std::size_t(detailMeshCount));
    std::size_t const detailTrisAt = detailVertsAt + align4(12 * std::size_t(detailVertCount));

    auto const vertex = [&](float const* recast, float* server) {
        server[0] = recast[2];
        server[1] = recast[0];
        server[2] = recast[1] + 0.15f;
    };
    float const* verts = reinterpret_cast<float const*>(data + vertsAt);
    float const* detailVerts = reinterpret_cast<float const*>(data + detailVertsAt);
    std::uint8_t const* detailTris = data + detailTrisAt;
    for (std::int32_t p = 0; p < polyCount && p < detailMeshCount; ++p) {
        std::uint8_t const* poly = data + polysAt + 32 * std::size_t(p);
        if (poly[31] >> 6)
            continue;
        std::uint8_t const vertexCount = poly[30];
        std::uint16_t indices[6];
        std::memcpy(indices, poly + 4, sizeof(indices));
        std::uint8_t const* mesh = data + detailMeshesAt + 12 * std::size_t(p);
        std::uint32_t vertBase, triBase;
        std::memcpy(&vertBase, mesh, 4);
        std::memcpy(&triBase, mesh + 4, 4);
        std::uint8_t const detailVertexCount = mesh[8], triangleCount = mesh[9];
        if (vertexCount > 6 || vertBase + detailVertexCount > std::uint32_t(detailVertCount) ||
            triBase + triangleCount > std::uint32_t(detailTriCount))
            continue;
        auto const corner = [&](int index, float* out) {
            if (index < vertexCount) {
                if (indices[index] >= vertCount)
                    return false;
                vertex(verts + indices[index] * 3, out);
            } else {
                vertex(detailVerts + (vertBase + index - vertexCount) * 3, out);
            }
            return true;
        };
        for (int t = 0; t < triangleCount; ++t) {
            std::uint8_t const* tri = detailTris + (triBase + t) * 4;
            for (int e = 0; e < 3; ++e) {
                if (((tri[3] >> (e * 2)) & 3) == 0)
                    continue;
                Segment s;
                if (tri[e] >= vertexCount + detailVertexCount || tri[(e + 1) % 3] >= vertexCount + detailVertexCount)
                    continue;
                if (!corner(tri[e], s.a) || !corner(tri[(e + 1) % 3], s.b))
                    continue;
                out.push_back(s);
            }
        }
    }
}

std::shared_ptr<Tile const> BuildTile(int map, int tx, int ty)
{
    auto tile = std::make_shared<Tile>();
    BuildCollision(map, tx, ty, tile->collision);
    BuildNavmesh(map, tx, ty, tile->navmesh);
    return tile;
}

void StreamLoop()
{
    int lastTx = -1000, lastTy = -1000, lastMap = -1;
    bool toggled[2] = {};
    for (;; Sleep(100)) {
        bool const ctrlShift = (GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(VK_SHIFT) & 0x8000);
        int const keys[2] = {'C', 'N'};
        for (int i = 0; i < 2; ++i) {
            bool const down = ctrlShift && (GetAsyncKeyState(keys[i]) & 0x8000);
            if (down && !toggled[i]) {
                std::atomic<bool>& flag = i == 0 ? ShowCollision : ShowNavmesh;
                flag = !flag.load();
                HOOK_LOG_INFO("overlay: collision %d navmesh %d", ShowCollision.load(), ShowNavmesh.load());
            }
            toggled[i] = down;
        }
        if (!HaveEye || DataDir.empty())
            continue;
        int const map = MapId;
        int const tx = static_cast<int>(std::floor(32.0 - EyeX / TileSize));
        int const ty = static_cast<int>(std::floor(32.0 - EyeY / TileSize));
        if (map == lastMap && tx == lastTx && ty == lastTy)
            continue;
        lastMap = map;
        lastTx = tx;
        lastTy = ty;
        std::map<std::pair<int, int>, std::shared_ptr<Tile const>> next;
        {
            std::lock_guard<std::mutex> lock(TilesMutex);
            if (TilesMap == map)
                for (int dx = -1; dx <= 1; ++dx)
                    for (int dy = -1; dy <= 1; ++dy) {
                        auto it = Tiles.find({tx + dx, ty + dy});
                        if (it != Tiles.end())
                            next[it->first] = it->second;
                    }
        }
        for (int dx = -1; dx <= 1; ++dx)
            for (int dy = -1; dy <= 1; ++dy)
                if (!next.count({tx + dx, ty + dy}) && tx + dx >= 0 && ty + dy >= 0 && tx + dx < 64 && ty + dy < 64)
                    next[{tx + dx, ty + dy}] = BuildTile(map, tx + dx, ty + dy);
        std::lock_guard<std::mutex> lock(TilesMutex);
        Tiles = std::move(next);
        TilesMap = map;
    }
}

void Emit(float const* eye, OverlayLineSink const& line)
{
    EyeX = eye[0];
    EyeY = eye[1];
    HaveEye = true;
    std::vector<std::shared_ptr<Tile const>> tiles;
    {
        std::lock_guard<std::mutex> lock(TilesMutex);
        if (TilesMap != MapId)
            return;
        for (auto const& entry : Tiles)
            tiles.push_back(entry.second);
    }
    auto const within = [&](Segment const& s, float radius) {
        float const dx = s.a[0] - eye[0], dy = s.a[1] - eye[1], dz = s.a[2] - eye[2];
        return dx * dx + dy * dy + dz * dz < radius * radius;
    };
    for (auto const& tile : tiles) {
        if (ShowCollision)
            for (Segment const& s : tile->collision)
                if (within(s, CollisionRadius))
                    line(s.a, s.b, {255, 160, 40, 180});
        if (ShowNavmesh)
            for (Segment const& s : tile->navmesh)
                if (within(s, NavmeshRadius))
                    line(s.a, s.b, {60, 255, 90, 160});
    }
}

} // namespace

void StartServerData()
{
    if (char const* dir = std::getenv("SMANGOS_SERVER_DATA"))
        DataDir = dir;
    if (DataDir.empty()) {
        HOOK_LOG_INFO("overlay: SMANGOS_SERVER_DATA not set, collision and navmesh disabled");
        return;
    }
    AddOverlayProvider(&Emit);
    std::thread(StreamLoop).detach();
}

void SetServerDataMap(int map)
{
    MapId = map;
}
