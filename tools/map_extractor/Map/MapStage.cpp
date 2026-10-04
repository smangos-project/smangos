#include "Map/MapStage.h"

#include "Cache/Manifest.h"
#include "Client/ChunkedFile.h"
#include "Platform/Define.h"
#include "GridMapDefines.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <memory>

namespace
{
    constexpr char const* STAGE_VERSION = "map-v1";

    constexpr int CELLS = 16;
    constexpr int CELL = 8;
    constexpr int GRID = CELLS * CELL;

    constexpr float USE_MIN_HEIGHT = -2000.0f;
    constexpr float INT8_LIMIT = 2.0f;
    constexpr float INT16_LIMIT = 2048.0f;
    constexpr float FLAT_HEIGHT_DELTA = 0.005f;
    constexpr float FLAT_LIQUID_DELTA = 0.001f;

    constexpr uint32_t MCNK_HIGH_RES_HOLES = 0x10000;
    constexpr uint8_t LIQUID_MATERIAL_VISUAL_ONLY = 0x2;

#pragma pack(push, 1)
    struct McnkHeader
    {
        uint32_t flags, ix, iy, nLayers, nDoodadRefs;
        union
        {
            struct { uint32_t offsMCVT, offsMCNR; } offsets;
            uint8_t highResHoles[8];
        };
        uint32_t offsMCLY, offsMCRF, offsMCAL, sizeMCAL, offsMCSH, sizeMCSH;
        uint32_t areaId, nMapObjRefs;
        uint32_t holes;
        uint16_t s[2];
        uint32_t data1, data2, data3, predTex, nEffectDoodad, offsMCSE, nSndEmitters;
        uint32_t offsMCLQ, sizeMCLQ;
        float zpos, xpos, ypos;
        uint32_t offsMCCV, props, effectId;
    };
    static_assert(sizeof(McnkHeader) == 128);

    struct Mclq
    {
        float height1, height2;
        struct { uint32_t light; float height; } liquid[CELL + 1][CELL + 1];
        uint8_t flags[CELL][CELL];
    };

    struct LiquidInstance
    {
        uint16_t liquidType, vertexFormat;
        float minHeight, maxHeight;
        uint8_t offsetX, offsetY, width, height;
        uint32_t offsetExistsBitmap, offsetVertexData;
    };

    struct LiquidChunkRef { uint32_t offsetInstances, used, offsetAttributes; };
#pragma pack(pop)

    enum VertexFormat { VF_HEIGHT_DEPTH = 0, VF_HEIGHT_TEXCOORD = 1, VF_DEPTH = 2, VF_HEIGHT_DEPTH_TEXCOORD = 3, VF_UNK4 = 4, VF_UNK5 = 5 };

    struct TileData
    {
        uint16_t areaIds[CELLS][CELLS] = {};
        float v9[GRID + 1][GRID + 1] = {};
        float v8[GRID][GRID] = {};
        bool liquidShow[GRID][GRID] = {};
        uint8_t liquidFlags[CELLS][CELLS] = {};
        uint16_t liquidEntry[CELLS][CELLS] = {};
        float liquidHeight[GRID + 1][GRID + 1] = {};
        uint16_t holes[CELLS][CELLS] = {};
    };

    uint16_t HighResToLowRes(uint8_t const (&hi)[8])
    {
        uint16_t lo = 0;
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 8; ++j)
                if (hi[i] & (1 << j))
                    lo |= uint16_t(1 << ((i / 2) * 4 + (j / 2)));
        return lo;
    }

    int VertexFormatOf(Db2Tables const& db2, LiquidInstance const& h)
    {
        if (h.vertexFormat < 42)
            return h.vertexFormat;
        if (h.liquidType == 2)
            return VF_DEPTH;
        auto type = db2.liquidTypes.find(h.liquidType);
        if (type != db2.liquidTypes.end())
        {
            auto material = db2.liquidMaterials.find(type->second.materialId);
            if (material != db2.liquidMaterials.end())
                return material->second.lvf;
        }
        return -1;
    }

    // Bounds checked reader over the MH2O payload.
    struct Mh2o
    {
        uint8_t const* data;
        uint32_t size;

        template <typename T>
        bool Read(uint64_t offset, T& out) const
        {
            if (offset + sizeof(T) > size)
                return false;
            std::memcpy(&out, data + offset, sizeof(T));
            return true;
        }
    };

    void ReadLiquidMh2o(Chunk const& chunk, Db2Tables const& db2, TileData& t, uint32_t mapId, int row, int col)
    {
        Mh2o h2o{ chunk.data, chunk.size };
        for (int i = 0; i < CELLS; ++i)
        {
            for (int j = 0; j < CELLS; ++j)
            {
                LiquidChunkRef ref;
                if (!h2o.Read(uint64_t(i * CELLS + j) * sizeof(ref), ref) || !ref.used || !ref.offsetInstances)
                    continue;
                LiquidInstance h;
                if (!h2o.Read(ref.offsetInstances, h))
                    continue;

                int vf = VertexFormatOf(db2, h);
                uint16_t liquidType = vf == VF_DEPTH ? 2 : h.liquidType;
                t.liquidEntry[i][j] = liquidType;
                auto typeEntry = db2.liquidTypes.find(liquidType);
                if (typeEntry == db2.liquidTypes.end())
                    continue;
                auto material = db2.liquidMaterials.find(typeEntry->second.materialId);
                if (material != db2.liquidMaterials.end() && (material->second.flags & LIQUID_MATERIAL_VISUAL_ONLY))
                    continue;

                bool deep = true;
                if (ref.offsetAttributes)
                {
                    uint64_t fishable = 0, deepMask = 0;
                    h2o.Read(ref.offsetAttributes, fishable);
                    h2o.Read(ref.offsetAttributes + 8, deepMask);
                    deep = deepMask != 0;
                }

                bool narrow = h.vertexFormat < 42;
                int offX = narrow ? h.offsetX : 0, offY = narrow ? h.offsetY : 0;
                int width = narrow ? h.width : CELL, height = narrow ? h.height : CELL;
                if (offX + width > CELL || offY + height > CELL)
                    continue;

                uint64_t exists = ~0ull;
                if (h.offsetExistsBitmap)
                    h2o.Read(h.offsetExistsBitmap, exists);
                for (int y = 0; y < height; ++y)
                {
                    for (int x = 0; x < width; ++x, exists >>= 1)
                        if (exists & 1)
                            t.liquidShow[i * CELL + y + offY][j * CELL + x + offX] = true;
                }

                switch (typeEntry->second.soundBank)
                {
                    case 0: t.liquidFlags[i][j] |= MAP_LIQUID_TYPE_WATER; break;
                    case 1:
                        t.liquidFlags[i][j] |= MAP_LIQUID_TYPE_OCEAN;
                        if (deep)
                            t.liquidFlags[i][j] |= MAP_LIQUID_TYPE_DEEP_WATER;
                        break;
                    case 2: t.liquidFlags[i][j] |= MAP_LIQUID_TYPE_MAGMA; break;
                    case 3: t.liquidFlags[i][j] |= MAP_LIQUID_TYPE_SLIME; break;
                    default:
                        std::fprintf(stderr, "map %u tile %d,%d chunk %d,%d: unknown liquid sound bank %u\n", mapId, row, col, i, j, typeEntry->second.soundBank);
                        break;
                }

                int pos = 0;
                for (int y = 0; y <= height; ++y)
                {
                    for (int x = 0; x <= width; ++x, ++pos)
                    {
                        float v = 0.0f;
                        if (h.offsetVertexData)
                        {
                            switch (vf)
                            {
                                case VF_HEIGHT_DEPTH:
                                case VF_HEIGHT_TEXCOORD:
                                case VF_HEIGHT_DEPTH_TEXCOORD:
                                    h2o.Read(uint64_t(h.offsetVertexData) + uint64_t(pos) * 4, v);
                                    break;
                                case VF_UNK4:
                                case VF_UNK5:
                                    h2o.Read(uint64_t(h.offsetVertexData) + 4 + uint64_t(pos) * 8, v);
                                    break;
                                default:
                                    break;
                            }
                        }
                        t.liquidHeight[i * CELL + y + offY][j * CELL + x + offX] = v;
                    }
                }
            }
        }
    }

    void ReadLiquidMclq(McnkHeader const& mcnk, Chunk const& cell, TileData& t)
    {
        if (mcnk.sizeMCLQ <= 8)
            return;
        Chunk const* sub = cell.Sub("MCLQ");
        Mclq const* liquid = sub ? sub->As<Mclq>() : nullptr;
        if (!liquid)
            return;

        int row = int(mcnk.iy), col = int(mcnk.ix);
        for (int y = 0; y < CELL; ++y)
        {
            for (int x = 0; x < CELL; ++x)
            {
                if (liquid->flags[y][x] == 0x0F)
                    continue;
                t.liquidShow[row * CELL + y][col * CELL + x] = true;
                if (liquid->flags[y][x] & (1 << 7))
                    t.liquidFlags[row][col] |= MAP_LIQUID_TYPE_DEEP_WATER;
            }
        }
        if (mcnk.flags & (1 << 2)) { t.liquidEntry[row][col] = 1; t.liquidFlags[row][col] |= MAP_LIQUID_TYPE_WATER; }
        if (mcnk.flags & (1 << 3)) { t.liquidEntry[row][col] = 2; t.liquidFlags[row][col] |= MAP_LIQUID_TYPE_OCEAN; }
        if (mcnk.flags & (1 << 4)) { t.liquidEntry[row][col] = 3; t.liquidFlags[row][col] |= MAP_LIQUID_TYPE_MAGMA; }
        for (int y = 0; y <= CELL; ++y)
            for (int x = 0; x <= CELL; ++x)
                t.liquidHeight[row * CELL + y][col * CELL + x] = liquid->liquid[y][x].height;
    }

    template <typename T, size_t N>
    void Write2D(T const (&data)[N][N], FILE* f) { std::fwrite(&data[0][0], sizeof(T), N * N, f); }

    bool ConvertAdt(ChunkedFile const& adt, Db2Tables const& db2, uint32_t mapId, int row, int col, std::filesystem::path const& outPath)
    {
        auto data = std::make_unique<TileData>();
        TileData& t = *data;

        for (Chunk const* cell : adt.GetAll("MCNK"))
        {
            McnkHeader const* mcnk = cell->As<McnkHeader>();
            if (!mcnk || mcnk->ix >= CELLS || mcnk->iy >= CELLS)
                continue;
            int iy = int(mcnk->iy), ix = int(mcnk->ix);

            t.areaIds[iy][ix] = uint16_t(mcnk->areaId);
            for (int y = 0; y <= CELL; ++y)
                for (int x = 0; x <= CELL; ++x)
                    t.v9[iy * CELL + y][ix * CELL + x] = mcnk->ypos;
            for (int y = 0; y < CELL; ++y)
                for (int x = 0; x < CELL; ++x)
                    t.v8[iy * CELL + y][ix * CELL + x] = mcnk->ypos;

            if (Chunk const* mcvt = cell->Sub("MCVT"); mcvt && mcvt->size >= sizeof(float) * (9 * 9 + 8 * 8))
            {
                auto const* h = reinterpret_cast<float const*>(mcvt->data);
                for (int y = 0; y <= CELL; ++y)
                    for (int x = 0; x <= CELL; ++x)
                        t.v9[iy * CELL + y][ix * CELL + x] += h[y * (CELL * 2 + 1) + x];
                for (int y = 0; y < CELL; ++y)
                    for (int x = 0; x < CELL; ++x)
                        t.v8[iy * CELL + y][ix * CELL + x] += h[y * (CELL * 2 + 1) + CELL + 1 + x];
            }

            ReadLiquidMclq(*mcnk, *cell, t);

            if (mcnk->flags & MCNK_HIGH_RES_HOLES)
                t.holes[iy][ix] = HighResToLowRes(mcnk->highResHoles);
            else
                t.holes[iy][ix] = uint16_t(mcnk->holes);
        }

        if (Chunk const* mh2o = adt.Get("MH2O"))
            ReadLiquidMh2o(*mh2o, db2, t, mapId, row, col);

        GridMapFileHeader map{};
        map.mapMagic = *reinterpret_cast<uint32_t const*>("MAPS");
        map.versionMagic = *reinterpret_cast<uint32_t const*>("z1.4");

        // area
        uint16_t firstArea = t.areaIds[0][0];
        bool fullArea = false;
        for (auto const& r : t.areaIds)
            for (uint16_t a : r)
                fullArea |= a != firstArea;
        GridMapAreaHeader areaHeader{};
        areaHeader.fourcc = *reinterpret_cast<uint32_t const*>("AREA");
        map.areaMapOffset = sizeof(map);
        map.areaMapSize = sizeof(areaHeader);
        if (fullArea)
            map.areaMapSize += sizeof(t.areaIds);
        else
        {
            areaHeader.flags = MAP_AREA_NO_AREA;
            areaHeader.gridArea = firstArea;
        }

        // height
        float maxHeight = -20000.0f, minHeight = 20000.0f;
        for (auto const& r : t.v8)
            for (float h : r) { maxHeight = std::max(maxHeight, h); minHeight = std::min(minHeight, h); }
        for (auto const& r : t.v9)
            for (float h : r) { maxHeight = std::max(maxHeight, h); minHeight = std::min(minHeight, h); }
        if (minHeight < USE_MIN_HEIGHT)
        {
            for (auto& r : t.v8)
                for (float& h : r) h = std::max(h, USE_MIN_HEIGHT);
            for (auto& r : t.v9)
                for (float& h : r) h = std::max(h, USE_MIN_HEIGHT);
            minHeight = USE_MIN_HEIGHT;
            maxHeight = std::max(maxHeight, USE_MIN_HEIGHT);
        }

        GridMapHeightHeader heightHeader{};
        heightHeader.fourcc = *reinterpret_cast<uint32_t const*>("MHGT");
        heightHeader.gridHeight = minHeight;
        heightHeader.gridMaxHeight = maxHeight;
        map.heightMapOffset = map.areaMapOffset + map.areaMapSize;
        map.heightMapSize = sizeof(heightHeader);
        if (maxHeight == minHeight || maxHeight - minHeight < FLAT_HEIGHT_DELTA)
            heightHeader.flags |= MAP_HEIGHT_NO_HEIGHT;

        thread_local uint16_t tu16v9[GRID + 1][GRID + 1], tu16v8[GRID][GRID];
        thread_local uint8_t tu8v9[GRID + 1][GRID + 1], tu8v8[GRID][GRID];
        if (!(heightHeader.flags & MAP_HEIGHT_NO_HEIGHT))
        {
            float diff = maxHeight - minHeight;
            if (diff < INT8_LIMIT)
            {
                float step = 255.0f / diff;
                heightHeader.flags |= MAP_HEIGHT_AS_INT8;
                for (int y = 0; y < GRID; ++y)
                    for (int x = 0; x < GRID; ++x) tu8v8[y][x] = uint8_t((t.v8[y][x] - minHeight) * step + 0.5f);
                for (int y = 0; y <= GRID; ++y)
                    for (int x = 0; x <= GRID; ++x) tu8v9[y][x] = uint8_t((t.v9[y][x] - minHeight) * step + 0.5f);
                map.heightMapSize += sizeof(tu8v9) + sizeof(tu8v8);
            }
            else if (diff < INT16_LIMIT)
            {
                float step = 65535.0f / diff;
                heightHeader.flags |= MAP_HEIGHT_AS_INT16;
                for (int y = 0; y < GRID; ++y)
                    for (int x = 0; x < GRID; ++x) tu16v8[y][x] = uint16_t((t.v8[y][x] - minHeight) * step + 0.5f);
                for (int y = 0; y <= GRID; ++y)
                    for (int x = 0; x <= GRID; ++x) tu16v9[y][x] = uint16_t((t.v9[y][x] - minHeight) * step + 0.5f);
                map.heightMapSize += sizeof(tu16v9) + sizeof(tu16v8);
            }
            else
                map.heightMapSize += sizeof(t.v9) + sizeof(t.v8);
        }

        // liquid
        uint16_t firstEntry = t.liquidEntry[0][0];
        uint8_t firstFlag = t.liquidFlags[0][0];
        bool fullType = false;
        for (int y = 0; y < CELLS; ++y)
            for (int x = 0; x < CELLS; ++x)
                fullType |= t.liquidEntry[y][x] != firstEntry || t.liquidFlags[y][x] != firstFlag;

        GridMapLiquidHeader liquidHeader{};
        if (firstFlag == 0 && !fullType)
        {
            map.liquidMapOffset = 0;
            map.liquidMapSize = 0;
        }
        else
        {
            int minX = GRID, minY = GRID, maxX = 0, maxY = 0;
            float lmax = -20000.0f, lmin = 20000.0f;
            for (int y = 0; y < GRID; ++y)
            {
                for (int x = 0; x < GRID; ++x)
                {
                    if (t.liquidShow[y][x])
                    {
                        minX = std::min(minX, x); maxX = std::max(maxX, x);
                        minY = std::min(minY, y); maxY = std::max(maxY, y);
                        float h = t.liquidHeight[y][x];
                        lmax = std::max(lmax, h); lmin = std::min(lmin, h);
                    }
                    else
                    {
                        t.liquidHeight[y][x] = USE_MIN_HEIGHT;
                        lmin = std::min(lmin, USE_MIN_HEIGHT);
                    }
                }
            }
            map.liquidMapOffset = map.heightMapOffset + map.heightMapSize;
            map.liquidMapSize = sizeof(liquidHeader);
            liquidHeader.fourcc = *reinterpret_cast<uint32_t const*>("MLIQ");
            liquidHeader.offsetX = uint8_t(minX);
            liquidHeader.offsetY = uint8_t(minY);
            liquidHeader.width = uint8_t(maxX - minX + 2);
            liquidHeader.height = uint8_t(maxY - minY + 2);
            liquidHeader.liquidLevel = lmin;
            if (minY > maxY || minX > maxX || lmax == lmin || lmax - lmin < FLAT_LIQUID_DELTA)
                liquidHeader.flags |= MAP_LIQUID_NO_HEIGHT;
            if (!fullType)
            {
                liquidHeader.flags |= MAP_LIQUID_NO_TYPE;
                liquidHeader.liquidFlags = firstFlag;
                liquidHeader.liquidType = firstEntry;
            }
            else
                map.liquidMapSize += sizeof(t.liquidEntry) + sizeof(t.liquidFlags);
            if (!(liquidHeader.flags & MAP_LIQUID_NO_HEIGHT))
                map.liquidMapSize += sizeof(float) * liquidHeader.width * liquidHeader.height;
        }

        map.holesOffset = map.liquidMapOffset ? map.liquidMapOffset + map.liquidMapSize : map.heightMapOffset + map.heightMapSize;
        map.holesSize = sizeof(t.holes);

        std::filesystem::path tmp = outPath;
        tmp += ".tmp";
        FILE* out = std::fopen(tmp.string().c_str(), "wb");
        if (!out)
        {
            std::fprintf(stderr, "can't create %s\n", tmp.string().c_str());
            return false;
        }
        std::fwrite(&map, sizeof(map), 1, out);
        std::fwrite(&areaHeader, sizeof(areaHeader), 1, out);
        if (!(areaHeader.flags & MAP_AREA_NO_AREA))
            Write2D(t.areaIds, out);
        std::fwrite(&heightHeader, sizeof(heightHeader), 1, out);
        if (!(heightHeader.flags & MAP_HEIGHT_NO_HEIGHT))
        {
            if (heightHeader.flags & MAP_HEIGHT_AS_INT16) { Write2D(tu16v9, out); Write2D(tu16v8, out); }
            else if (heightHeader.flags & MAP_HEIGHT_AS_INT8) { Write2D(tu8v9, out); Write2D(tu8v8, out); }
            else { Write2D(t.v9, out); Write2D(t.v8, out); }
        }
        if (map.liquidMapOffset)
        {
            std::fwrite(&liquidHeader, sizeof(liquidHeader), 1, out);
            if (!(liquidHeader.flags & MAP_LIQUID_NO_TYPE))
            {
                Write2D(t.liquidEntry, out);
                Write2D(t.liquidFlags, out);
            }
            if (!(liquidHeader.flags & MAP_LIQUID_NO_HEIGHT))
                for (int y = 0; y < liquidHeader.height; ++y)
                    std::fwrite(&t.liquidHeight[y + liquidHeader.offsetY][liquidHeader.offsetX], sizeof(float), liquidHeader.width, out);
        }
        Write2D(t.holes, out);
        bool ok = std::fclose(out) == 0;
        std::error_code ec;
        if (ok)
            std::filesystem::rename(tmp, outPath, ec);
        return ok && !ec;
    }

    struct TileJob
    {
        int row, col;
        std::string key;
        std::string fingerprint;
        std::string reason;
    };
}

std::string RootAdtPath(Context const& ctx, KnownMap const& map, int row, int col)
{
    if (map.wdt.hasFileDataIds && (map.wdt.mphdFlags & 0x200))
        return ctx.client.PathOf(map.wdt.files[size_t(row * TILES_PER_MAP + col)].root);
    return map.wdt.AdtPath(map.record.directory, row, col, "");
}

std::filesystem::path Context::MapFile(uint32_t mapId, int row, int col) const
{
    char name[32];
    std::snprintf(name, sizeof(name), "%03u%02d%02d.map", mapId, row, col);
    return MapsDir() / name;
}

StageStats RunMapStage(Context& ctx, KnownMap const& map, TileRect const& region)
{
    StageStats stats;
    Manifest manifest(ctx.CacheDir() / ("map_" + std::to_string(map.record.id) + ".manifest"));

    Fingerprint common;
    common.Add(STAGE_VERSION);
    for (char const* table : { "dbfilesclient/liquidtype.db2", "dbfilesclient/liquidmaterial.db2", "dbfilesclient/liquidobject.db2" })
        common.Add(table, ctx.client.Stat(table));
    common.Add("wdt", ctx.client.Stat(WdtPath(map.record.directory)));

    std::vector<TileJob> jobs;
    for (int row = region.rowMin; row <= region.rowMax; ++row)
    {
        for (int col = region.colMin; col <= region.colMax; ++col)
        {
            if (!map.wdt.Has(row, col))
                continue;
            std::string src = RootAdtPath(ctx, map, row, col);
            FileStamp stamp = ctx.client.Stat(src);
            if (!stamp.exists)
            {
                std::printf("map %u tile %d,%d: missing %s\n", map.record.id, row, col, src.c_str());
                ++stats.missingInput;
                continue;
            }

            Fingerprint fp = common;
            fp.Add(src, stamp);
            TileJob job{ row, col, std::to_string(row) + "," + std::to_string(col), fp.Hex(), {} };
            std::error_code ec;
            if (ctx.options.force)
                job.reason = "forced";
            else if (!std::filesystem::exists(ctx.MapFile(map.record.id, row, col), ec))
                job.reason = "missing output";
            else if (manifest.Get(job.key) != job.fingerprint)
                job.reason = "input changed";
            else
            {
                ++stats.upToDate;
                continue;
            }
            jobs.push_back(std::move(job));
        }
    }

    if (ctx.options.dryRun)
    {
        for (TileJob const& job : jobs)
            std::printf("  map %u tile %d,%d: would redo (%s)\n", map.record.id, job.row, job.col, job.reason.c_str());
        stats.done = unsigned(jobs.size());
        return stats;
    }

    std::error_code ec;
    std::filesystem::create_directories(ctx.MapsDir(), ec);
    std::atomic<unsigned> done{ 0 }, failed{ 0 };
    ParallelFor(jobs.size(), ctx.options.threads, [&](size_t i)
    {
        TileJob const& job = jobs[i];
        std::string src = RootAdtPath(ctx, map, job.row, job.col);
        ChunkedFile adt;
        if (!adt.Parse(ctx.client.Read(src)) ||
            !ConvertAdt(adt, ctx.db2, map.record.id, job.row, job.col, ctx.MapFile(map.record.id, job.row, job.col)))
        {
            std::fprintf(stderr, "map %u tile %d,%d: failed to convert %s\n", map.record.id, job.row, job.col, src.c_str());
            ++failed;
            return;
        }
        manifest.Set(job.key, job.fingerprint);
        ++done;
    });
    manifest.Save();
    stats.done = done;
    stats.failed = failed;
    return stats;
}
