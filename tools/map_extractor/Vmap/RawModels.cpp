#include "Vmap/RawModels.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstddef>
#include <map>
#include <set>

namespace
{
    constexpr char RawMagic[8] = { 'V', 'M', 'A', 'P', 's', '0', '5', '\0' };

    constexpr uint32_t MopyDetail = 0x04;
    constexpr uint32_t MopyCollision = 0x08;
    constexpr uint32_t MopyRender = 0x20;

    struct Writer
    {
        std::vector<uint8_t> bytes;

        void Put(void const* data, size_t size)
        {
            auto const* p = static_cast<uint8_t const*>(data);
            bytes.insert(bytes.end(), p, p + size);
        }
        template <typename T>
        void Put(T const& value) { Put(&value, sizeof(T)); }
        void Tag(char const* fourcc) { Put(fourcc, 4); }
        void PutAt(size_t offset, uint32_t value) { std::memcpy(bytes.data() + offset, &value, 4); }
    };

    struct Chunk
    {
        std::string id; // as written in the format docs, e.g. "MOGP"
        uint8_t const* data = nullptr;
        uint32_t size = 0;
    };

    template <typename T>
    bool ReadAt(Chunk const& c, size_t offset, T& out)
    {
        if (offset + sizeof(T) > c.size)
            return false;
        std::memcpy(&out, c.data + offset, sizeof(T));
        return true;
    }

    // MOGP's size field covers the sub-chunks that follow it, so only its 68 byte header is its payload.
    template <typename Fn>
    void ForEachChunk(std::vector<uint8_t> const& file, Fn&& fn)
    {
        size_t pos = 0;
        while (pos + 8 <= file.size())
        {
            Chunk c;
            c.id = { char(file[pos + 3]), char(file[pos + 2]), char(file[pos + 1]), char(file[pos]) };
            std::memcpy(&c.size, &file[pos + 4], 4);
            if (c.id == "MOGP")
                c.size = 68;
            c.data = &file[pos + 8];
            if (pos + 8 + size_t(c.size) > file.size())
                break;
            fn(c);
            pos += 8 + size_t(c.size);
        }
    }

#pragma pack(push, 1)
    struct LiquidHeader
    {
        int32_t xverts, yverts, xtiles, ytiles;
        float pos[3];
        int16_t type;
    };

    struct LiquidVert
    {
        uint16_t unk1, unk2;
        float height;
    };

    struct M2Header
    {
        char id[4];
        uint8_t version[4];
        uint32_t fields0[2];       // nameLength, nameOfs
        uint32_t type;
        uint32_t fields1[35];      // everything up to texture animation lookup
        float boundingBox[6];
        float boundingSphereRadius;
        float collisionBox[6];
        float collisionSphereRadius;
        uint32_t nBoundingTriangles;
        uint32_t ofsBoundingTriangles;
        uint32_t nBoundingVertices;
        uint32_t ofsBoundingVertices;
    };

    struct Modd
    {
        uint32_t nameAndFlags;
        float position[3];
        float rotation[4];
        float scale;
        uint32_t color;
    };

    struct Mods
    {
        char name[20];
        uint32_t startIndex, count, pad;
    };
#pragma pack(pop)
    static_assert(sizeof(LiquidHeader) == 30);
    static_assert(sizeof(Modd) == 40);
    static_assert(sizeof(Mods) == 32);
    static_assert(offsetof(M2Header, boundingBox) == 160);

    struct Group
    {
        int32_t nameIndex = 0;
        uint32_t mogpFlags = 0, flags2 = 0, liquidType = 0, wmoId = 0;
        float boxMin[3] = {}, boxMax[3] = {};
        int16_t parentSplit = 0;
        std::vector<uint16_t> mopy;     // flags per triangle
        std::vector<uint32_t> indices;
        std::vector<float> vertices;    // xyz
        std::vector<uint16_t> batches;  // MOBA
        std::vector<uint16_t> doodadRefs;
        bool hasLiquid = false;
        LiquidHeader liquidHeader{};
        std::vector<float> liquidHeights;
        std::vector<uint8_t> liquidFlags;
    };

    bool ReadGroup(std::vector<uint8_t> const& file, Group& g)
    {
        bool sawHeader = false;
        ForEachChunk(file, [&](Chunk const& c)
        {
            if (c.id == "MOGP")
            {
                sawHeader = c.size >= 68;
                if (!sawHeader)
                    return;
                std::memcpy(&g.nameIndex, c.data, 4);
                std::memcpy(&g.mogpFlags, c.data + 8, 4);
                std::memcpy(g.boxMin, c.data + 12, 12);
                std::memcpy(g.boxMax, c.data + 24, 12);
                std::memcpy(&g.liquidType, c.data + 52, 4);
                std::memcpy(&g.wmoId, c.data + 56, 4);
                std::memcpy(&g.flags2, c.data + 60, 4);
                std::memcpy(&g.parentSplit, c.data + 64, 2);
            }
            else if (c.id == "MOPY")
            {
                g.mopy.resize(c.size / 2);
                for (size_t i = 0; i < g.mopy.size(); ++i)
                    g.mopy[i] = c.data[2 * i];
            }
            else if (c.id == "MPY2")
            {
                g.mopy.resize(c.size / 4);
                for (size_t i = 0; i < g.mopy.size(); ++i)
                    std::memcpy(&g.mopy[i], c.data + 4 * i, 2);
            }
            else if (c.id == "MOVI")
            {
                g.indices.resize(c.size / 2);
                for (size_t i = 0; i < g.indices.size(); ++i)
                {
                    uint16_t v;
                    std::memcpy(&v, c.data + 2 * i, 2);
                    g.indices[i] = v;
                }
            }
            else if (c.id == "MOVX")
            {
                g.indices.resize(c.size / 4);
                std::memcpy(g.indices.data(), c.data, g.indices.size() * 4);
            }
            else if (c.id == "MOVT")
            {
                g.vertices.resize(c.size / 12 * 3);
                std::memcpy(g.vertices.data(), c.data, g.vertices.size() * 4);
            }
            else if (c.id == "MOBA")
            {
                g.batches.resize(c.size / 2);
                std::memcpy(g.batches.data(), c.data, g.batches.size() * 2);
            }
            else if (c.id == "MODR")
            {
                g.doodadRefs.resize(c.size / 2);
                std::memcpy(g.doodadRefs.data(), c.data, g.doodadRefs.size() * 2);
            }
            else if (c.id == "MLIQ")
            {
                LiquidHeader h;
                if (!ReadAt(c, 0, h) || h.xverts <= 0 || h.yverts <= 0 || h.xtiles < 0 || h.ytiles < 0)
                    return;
                size_t vertCount = size_t(h.xverts) * size_t(h.yverts);
                size_t tileCount = size_t(h.xtiles) * size_t(h.ytiles);
                if (sizeof(h) + vertCount * sizeof(LiquidVert) + tileCount > c.size)
                    return;
                g.hasLiquid = true;
                g.liquidHeader = h;
                g.liquidHeights.resize(vertCount);
                for (size_t i = 0; i < vertCount; ++i)
                {
                    LiquidVert v;
                    std::memcpy(&v, c.data + sizeof(h) + i * sizeof(v), sizeof(v));
                    g.liquidHeights[i] = v.height;
                }
                g.liquidFlags.assign(c.data + sizeof(h) + vertCount * sizeof(LiquidVert),
                    c.data + sizeof(h) + vertCount * sizeof(LiquidVert) + tileCount);
            }
        });
        return sawHeader;
    }

    // Vanilla liquid type ids the server's WMO liquid lookup expects.
    uint32_t LegacyLiquidType(Group const& g, uint16_t rootFlags, uint32_t rootWmoId)
    {
        uint32_t entry;
        if (rootFlags & 4)
            entry = g.liquidType;
        else if (g.liquidType == 15)
            entry = 0;
        else
            entry = g.liquidType + 1;

        if (!entry)
        {
            for (uint8_t b : g.liquidFlags)
            {
                if ((b & 0xF) != 15)
                {
                    entry = (b & 0xF) + 1;
                    break;
                }
            }
        }

        if (entry && entry < 21)
        {
            switch ((entry - 1) & 3)
            {
                case 0: entry = ((g.mogpFlags & 0x80000) != 0) + 1; break;
                case 1: entry = 2; break;
                case 2: entry = 3; break;
                default: entry = rootWmoId == 4489 ? 21 : 4; break;
            }
        }
        return entry;
    }

    // Returns the number of collision vertices written, or -1 if the group can't be represented.
    int WriteGroup(Writer& w, Group const& g, uint16_t rootFlags, uint32_t rootWmoId)
    {
        size_t triangles = std::min(g.mopy.size(), g.indices.size() / 3);
        size_t vertexCount = g.vertices.size() / 3;

        std::vector<int32_t> renumber(vertexCount, -1);
        std::vector<uint16_t> colIndices;
        for (size_t i = 0; i < triangles; ++i)
        {
            bool render = (g.mopy[i] & MopyRender) && !(g.mopy[i] & MopyDetail);
            if (!(g.mopy[i] & MopyCollision) && !render)
                continue;
            for (size_t j = 0; j < 3; ++j)
                if (g.indices[3 * i + j] >= vertexCount)
                    return -1;
            for (size_t j = 0; j < 3; ++j)
                renumber[g.indices[3 * i + j]] = 1;
        }
        uint32_t colVertices = 0;
        for (int32_t& r : renumber)
            if (r == 1)
                r = int32_t(colVertices++);
        if (colVertices > 0xFFFF)
            return -1;
        for (size_t i = 0; i < triangles; ++i)
        {
            bool render = (g.mopy[i] & MopyRender) && !(g.mopy[i] & MopyDetail);
            if (!(g.mopy[i] & MopyCollision) && !render)
                continue;
            for (size_t j = 0; j < 3; ++j)
                colIndices.push_back(uint16_t(renumber[g.indices[3 * i + j]]));
        }

        uint32_t liquidFlags = 0;
        if (g.hasLiquid)
            liquidFlags = 1;

        w.Put(g.mogpFlags);
        w.Put(g.wmoId);
        w.Put(g.boxMin, 12);
        w.Put(g.boxMax, 12);
        w.Put(liquidFlags);

        // MOBA: one value per batch, read at the same offset the stock extractor uses
        int32_t batchCount = int32_t(g.batches.size() / 12);
        w.Tag("GRP ");
        w.Put(int32_t(batchCount * 4 + 4));
        w.Put(batchCount);
        for (int32_t b = 0; b < batchCount; ++b)
            w.Put(int32_t(g.batches[size_t(b) * 12 + 8]));

        w.Tag("INDX");
        w.Put(int32_t(colIndices.size() * 2 + 4));
        w.Put(uint32_t(colIndices.size()));
        w.Put(colIndices.data(), colIndices.size() * 2);

        w.Tag("VERT");
        w.Put(int32_t(colVertices * 12 + 4));
        w.Put(colVertices);
        for (size_t i = 0; i < vertexCount; ++i)
            if (renumber[i] >= 0)
                w.Put(&g.vertices[3 * i], 12);

        if (g.hasLiquid)
        {
            LiquidHeader h = g.liquidHeader;
            h.type = int16_t(LegacyLiquidType(g, rootFlags, rootWmoId));
            w.Tag("LIQU");
            w.Put(int32_t(sizeof(h) + g.liquidHeights.size() * 4 + g.liquidFlags.size()));
            w.Put(h);
            w.Put(g.liquidHeights.data(), g.liquidHeights.size() * 4);
            w.Put(g.liquidFlags.data(), g.liquidFlags.size());
        }
        return int(colVertices);
    }

    std::string TrimmedString(char const* p, char const* end)
    {
        return std::string(p, static_cast<char const*>(memchr(p, 0, size_t(end - p))
            ? static_cast<char const*>(memchr(p, 0, size_t(end - p))) : end));
    }

    uint32_t ResolveDoodadPath(ClientStorage const& client, std::string path)
    {
        std::string n = ClientStorage::Normalize(std::move(path));
        if (n.size() > 4 && (n.ends_with(".mdx") || n.ends_with(".mdl")))
            n.replace(n.size() - 3, 3, "m2");
        return client.FileDataIdOf(n);
    }
}

std::string RawModelName(uint32_t fileDataId, bool wmo)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "FILE%08X.%s", fileDataId, wmo ? "wmo" : "m2");
    return buf;
}

std::vector<uint8_t> WmoDoodads::Serialize() const
{
    Writer w;
    w.Put(uint32_t(sets.size()));
    for (WmoDoodadSet const& s : sets)
        w.Put(s);
    w.Put(uint32_t(spawns.size()));
    for (WmoDoodadSpawn const& s : spawns)
        w.Put(s);
    w.Put(uint32_t(references.size()));
    w.Put(references.data(), references.size() * 2);
    return std::move(w.bytes);
}

bool WmoDoodads::Deserialize(std::vector<uint8_t> const& data, WmoDoodads& out)
{
    size_t pos = 0;
    auto take = [&](void* dst, size_t n)
    {
        if (pos + n > data.size())
            return false;
        std::memcpy(dst, data.data() + pos, n);
        pos += n;
        return true;
    };
    out = {};
    uint32_t n;
    if (!take(&n, 4))
        return false;
    out.sets.resize(n);
    if (!take(out.sets.data(), n * sizeof(WmoDoodadSet)) || !take(&n, 4))
        return false;
    out.spawns.resize(n);
    if (!take(out.spawns.data(), n * sizeof(WmoDoodadSpawn)) || !take(&n, 4))
        return false;
    out.references.resize(n);
    return take(out.references.data(), n * 2);
}

ModelState ConvertM2(std::vector<uint8_t> const& file, std::vector<uint8_t>& out)
{
    size_t start = 0;
    while (start + 4 <= file.size() && std::memcmp(&file[start], "MD20", 4) != 0)
        ++start;
    if (start + sizeof(M2Header) > file.size())
        return ModelState::Failed;

    M2Header h;
    std::memcpy(&h, &file[start], sizeof(h));
    if (!h.nBoundingTriangles)
        return ModelState::NoCollision;

    size_t indexEnd = start + size_t(h.ofsBoundingTriangles) + size_t(h.nBoundingTriangles) * 2;
    size_t vertexEnd = start + size_t(h.ofsBoundingVertices) + size_t(h.nBoundingVertices) * 12;
    if (indexEnd > file.size() || vertexEnd > file.size())
        return ModelState::Failed;

    std::vector<uint16_t> indices(h.nBoundingTriangles);
    std::memcpy(indices.data(), &file[start + h.ofsBoundingTriangles], indices.size() * 2);
    for (size_t i = 0; i + 1 < indices.size(); ++i)
        if (i % 3 == 1)
            std::swap(indices[i], indices[i + 1]);

    Writer w;
    w.Put(RawMagic, 8);
    w.Put(h.nBoundingVertices);
    w.Put(uint32_t(1));            // groups
    w.Put(uint32_t(0));            // root wmo id
    w.Put(uint32_t(0));            // mogp flags
    w.Put(uint32_t(0));            // group wmo id
    w.Put(h.collisionBox, 24);
    w.Put(uint32_t(0));            // liquid flags
    w.Tag("GRP ");
    w.Put(int32_t(8));
    w.Put(uint32_t(1));
    w.Put(h.nBoundingTriangles);
    w.Tag("INDX");
    w.Put(int32_t(h.nBoundingTriangles * 2 + 4));
    w.Put(h.nBoundingTriangles);
    w.Put(indices.data(), indices.size() * 2);
    w.Tag("VERT");
    w.Put(int32_t(h.nBoundingVertices * 12 + 4));
    w.Put(h.nBoundingVertices);
    w.Put(&file[start + h.ofsBoundingVertices], size_t(h.nBoundingVertices) * 12);
    out = std::move(w.bytes);
    return ModelState::Ok;
}

ModelState ConvertWmo(ClientStorage const& client, uint32_t rootFileDataId, std::vector<uint8_t>& out,
    WmoDoodads& doodads, std::vector<uint32_t>& inputs)
{
    std::vector<uint8_t> root = client.Read(rootFileDataId);
    if (root.empty())
        return ModelState::Failed;
    inputs.push_back(rootFileDataId);

    uint32_t groupCount = 0, rootWmoId = 0;
    uint16_t rootFlags = 0;
    std::vector<uint32_t> groupIds;
    std::vector<char> groupNames;
    std::vector<Modd> modd;
    std::vector<uint32_t> doodadFileIds; // by NameIndex (MODI) or by byte offset (MODN)
    std::map<uint32_t, uint32_t> pathOffsetToId;
    bool sawHeader = false;

    doodads = {};
    ForEachChunk(root, [&](Chunk const& c)
    {
        if (c.id == "MOHD")
        {
            sawHeader = ReadAt(c, 4, groupCount) && ReadAt(c, 32, rootWmoId) && ReadAt(c, 60, rootFlags);
        }
        else if (c.id == "MODS")
        {
            for (size_t i = 0; i + sizeof(Mods) <= c.size; i += sizeof(Mods))
            {
                Mods m;
                std::memcpy(&m, c.data + i, sizeof(m));
                doodads.sets.push_back({ m.startIndex, m.count });
            }
        }
        else if (c.id == "MODN")
        {
            char const* base = reinterpret_cast<char const*>(c.data);
            for (uint32_t off = 0; off < c.size;)
            {
                std::string path = TrimmedString(base + off, base + c.size);
                pathOffsetToId[off] = ResolveDoodadPath(client, path);
                off += uint32_t(path.size()) + 1;
            }
        }
        else if (c.id == "MODI")
        {
            doodadFileIds.resize(c.size / 4);
            std::memcpy(doodadFileIds.data(), c.data, doodadFileIds.size() * 4);
        }
        else if (c.id == "MODD")
        {
            modd.resize(c.size / sizeof(Modd));
            std::memcpy(modd.data(), c.data, modd.size() * sizeof(Modd));
        }
        else if (c.id == "MOGN")
        {
            groupNames.assign(c.data, c.data + c.size);
        }
        else if (c.id == "GFID")
        {
            for (uint32_t i = 0; i < groupCount && (i + 1) * 4 <= c.size; ++i)
            {
                uint32_t id;
                std::memcpy(&id, c.data + 4 * i, 4);
                if (id)
                    groupIds.push_back(id);
            }
        }
    });
    if (!sawHeader)
        return ModelState::Failed;

    for (Modd const& d : modd)
    {
        WmoDoodadSpawn s;
        uint32_t nameIndex = d.nameAndFlags & 0xFFFFFF;
        if (!doodadFileIds.empty())
            s.fileDataId = nameIndex < doodadFileIds.size() ? doodadFileIds[nameIndex] : 0;
        else if (auto itr = pathOffsetToId.find(nameIndex); itr != pathOffsetToId.end())
            s.fileDataId = itr->second;
        std::memcpy(s.position, d.position, 12);
        std::memcpy(s.rotation, d.rotation, 16);
        s.scale = d.scale;
        doodads.spawns.push_back(s);
    }

    std::vector<Group> groups;
    groups.reserve(groupIds.size());
    for (uint32_t id : groupIds)
    {
        std::vector<uint8_t> file = client.Read(id);
        Group g;
        if (file.empty() || !ReadGroup(file, g))
            return ModelState::Failed;
        inputs.push_back(id);
        groups.push_back(std::move(g));
    }

    Writer w;
    w.Put(RawMagic, 8);
    w.Put(uint32_t(0));    // vertex total, patched below
    w.Put(uint32_t(0));    // group count, patched below
    w.Put(rootWmoId);

    uint32_t written = 0;
    int64_t totalVertices = 0;
    std::set<uint16_t> references;
    for (Group& g : groups)
    {
        // antiportals, unreachable groups
        bool skip = (g.mogpFlags & 0x80) || (g.mogpFlags & 0x4000000);
        if (!skip && g.nameIndex >= 0 && size_t(g.nameIndex) < groupNames.size())
            skip = !std::strcmp(&groupNames[size_t(g.nameIndex)], "antiportal");
        if (skip)
            continue;

        if ((g.flags2 & 0x80) && g.parentSplit >= 0 && size_t(g.parentSplit) < groups.size())
            g.wmoId = groups[size_t(g.parentSplit)].wmoId;

        size_t mark = w.bytes.size();
        int vertices = WriteGroup(w, g, rootFlags, rootWmoId);
        if (vertices < 0)
        {
            w.bytes.resize(mark);
            continue;
        }
        totalVertices += vertices;
        ++written;
        for (uint16_t ref : g.doodadRefs)
            if (ref < doodads.spawns.size() && doodads.spawns[ref].fileDataId)
                references.insert(ref);
    }
    doodads.references.assign(references.begin(), references.end());

    w.PutAt(8, uint32_t(totalVertices));
    w.PutAt(12, written);
    out = std::move(w.bytes);
    return totalVertices ? ModelState::Ok : ModelState::NoCollision;
}
