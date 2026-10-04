#include "Mmap/MmapStage.h"

#include "Cache/Manifest.h"
#include "MapBuilder.h"
#include "MapTree.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <memory>
#include <set>
#include <sstream>

namespace
{
    constexpr char const* STAGE_VERSION = "mmap-v1";

    namespace fs = std::filesystem;

    std::string StampOf(fs::path const& file)
    {
        std::error_code ec;
        auto size = fs::file_size(file, ec);
        if (ec)
            return "-";
        auto time = fs::last_write_time(file, ec);
        return std::to_string(size) + ":" + std::to_string(ec ? 0 : time.time_since_epoch().count());
    }

    std::string FileText(fs::path const& file)
    {
        std::ifstream in(file, std::ios::binary);
        std::ostringstream out;
        out << in.rdbuf();
        return out.str();
    }

    struct Job
    {
        int row, col;
        std::string key, fingerprint, reason;
    };

    // The 3x3 block of inputs around a tile: Recast samples its neighbours' borders.
    void AddTileInputs(Fingerprint& fp, Context const& ctx, uint32_t mapId, int row, int col)
    {
        for (int r = row - 1; r <= row + 1; ++r)
        {
            for (int c = col - 1; c <= col + 1; ++c)
            {
                if (r < 0 || c < 0 || r >= TILES_PER_MAP || c >= TILES_PER_MAP)
                    continue;
                char name[48];
                std::snprintf(name, sizeof(name), "%03u_%02d_%02d.vmtile", mapId, c, r);
                fp.Add(std::string("map") + std::to_string(r) + "," + std::to_string(c) + StampOf(ctx.MapFile(mapId, r, c)));
                fp.Add(std::string(name) + StampOf(ctx.options.output / "vmaps" / name));
            }
        }
    }

    bool HasMmtile(fs::path const& mmaps, uint32_t mapId, int row, int col)
    {
        char name[32];
        std::snprintf(name, sizeof(name), "%03u%02d%02d.mmtile", mapId, row, col);
        std::error_code ec;
        return fs::exists(mmaps / name, ec);
    }
}

StageStats RunMmapStage(Context& ctx, std::vector<KnownMap const*> const& maps, TileRect const& region)
{
    StageStats stats;
    Options const& o = ctx.options;
    fs::path output = fs::absolute(o.output);
    fs::path mmaps = output / "mmaps";

    fs::path configPath = o.mmapConfig.empty() ? fs::path(MAP_EXTRACTOR_SOURCE_DIR) / "contrib/mmap/config.json" : fs::absolute(o.mmapConfig);
    fs::path offmeshPath = o.offmesh.empty() ? fs::path(MAP_EXTRACTOR_SOURCE_DIR) / "contrib/mmap/offmesh.txt" : fs::absolute(o.offmesh);

    Fingerprint common;
    common.Add(STAGE_VERSION);
    common.Add(FileText(configPath));
    common.Add(FileText(offmeshPath));
    common.Add(o.skipLiquid ? "noliquid" : "liquid");

    std::vector<std::vector<Job>> jobsPerMap(maps.size());
    unsigned total = 0;
    for (size_t i = 0; i < maps.size(); ++i)
    {
        KnownMap const& map = *maps[i];
        uint32_t id = map.record.id;
        if ((o.skipContinents && id <= 1) || (o.skipJunkMaps && (id == 42 || id == 169)) ||
            (o.skipBattlegrounds && (id == 30 || id == 37 || id == 489 || id == 529)))
            continue;

        char tree[16];
        std::snprintf(tree, sizeof(tree), "%03u.vmtree", id);
        Manifest manifest(ctx.CacheDir() / ("mmap_" + std::to_string(id) + ".manifest"));
        Fingerprint mapCommon = common;
        mapCommon.Add(std::string(tree) + StampOf(output / "vmaps" / tree));

        std::vector<std::pair<int, int>> tiles;
        for (int row = region.rowMin; row <= region.rowMax; ++row)
            for (int col = region.colMin; col <= region.colMax; ++col)
                if (map.wdt.Has(row, col))
                    tiles.emplace_back(row, col);

        for (auto [row, col] : tiles)
        {
            if (!std::filesystem::exists(ctx.MapFile(id, row, col)))
            {
                ++stats.missingInput;
                continue;
            }
            Fingerprint fp = mapCommon;
            AddTileInputs(fp, ctx, id, row, col);
            Job job{ row, col, std::to_string(row) + "," + std::to_string(col), fp.Hex(), {} };
            if (o.force)
                job.reason = "forced";
            else if (!HasMmtile(mmaps, id, row, col))
                job.reason = "missing output";
            else if (manifest.Get(job.key) != job.fingerprint)
                job.reason = "input changed";
            else
            {
                ++stats.upToDate;
                continue;
            }
            jobsPerMap[i].push_back(std::move(job));
        }
        total += unsigned(jobsPerMap[i].size());
    }

    if (o.dryRun)
    {
        for (size_t i = 0; i < maps.size(); ++i)
            for (Job const& job : jobsPerMap[i])
                std::printf("  map %u tile %d,%d: would redo (%s)\n", maps[i]->record.id, job.row, job.col, job.reason.c_str());
        stats.done = total;
        return stats;
    }
    if (!total)
        return stats;

    std::error_code ec;
    fs::create_directories(mmaps, ec);
    fs::path previous = fs::current_path(ec);
    fs::current_path(output, ec);
    if (ec)
    {
        std::fprintf(stderr, "mmap: cannot enter %s\n", output.c_str());
        stats.failed = total;
        return stats;
    }

    unsigned threads = o.threads ? o.threads : std::max(1u, std::thread::hardware_concurrency());
    std::string config = configPath.string(), offmesh = offmeshPath.string();
    {
        MMAP::MapBuilder builder(config.c_str(), o.skipLiquid, false, false, false, o.debugMmap, false, offmesh.c_str(), uint8_t(std::min(threads, 255u)));
        for (size_t i = 0; i < maps.size(); ++i)
        {
            if (jobsPerMap[i].empty())
                continue;
            KnownMap const& map = *maps[i];
            // Packed as (col << 16 | row): the .mmap header is derived from the whole map, not from the range.
            std::set<uint32_t> all;
            for (int row = 0; row < TILES_PER_MAP; ++row)
                for (int col = 0; col < TILES_PER_MAP; ++col)
                    if (map.wdt.Has(row, col))
                        all.insert(VMAP::StaticMapTree::packTileID(col, row));
            builder.setTileList(map.record.id, std::move(all));

            std::vector<uint32_t> packed;
            for (Job const& job : jobsPerMap[i])
                packed.push_back(VMAP::StaticMapTree::packTileID(job.col, job.row));
            bool wasFailing = builder.HasErrors();
            builder.buildTiles(map.record.id, packed);
            if (builder.HasErrors() && !wasFailing)
            {
                stats.failed += unsigned(packed.size());
                continue;
            }
            Manifest manifest(ctx.CacheDir() / ("mmap_" + std::to_string(map.record.id) + ".manifest"));
            for (Job const& job : jobsPerMap[i])
                manifest.Set(job.key, job.fingerprint);
            manifest.Save();
            stats.done += unsigned(packed.size());
        }
    }
    fs::current_path(previous, ec);
    return stats;
}
