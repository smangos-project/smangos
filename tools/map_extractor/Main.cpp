#include "Context.h"

#include "Client/Options.h"
#include "Map/MapStage.h"
#include "World/DoltLiteDatabase.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <set>
#include <thread>

void ParallelFor(size_t count, unsigned threads, std::function<void(size_t)> const& fn)
{
    std::atomic<size_t> next{ 0 };
    auto worker = [&]
    {
        for (size_t i; (i = next++) < count;)
            fn(i);
    };
    std::vector<std::thread> pool;
    for (unsigned i = 1; i < std::min<size_t>(threads, count); ++i)
        pool.emplace_back(worker);
    worker();
    for (std::thread& t : pool)
        t.join();
}

namespace
{
    std::string LoadWorldMapIds(std::filesystem::path const& path, std::set<uint32_t>& ids)
    {
        if (!std::filesystem::exists(path))
            return "world database " + path.string() + " not found";
        try
        {
            DoltLiteDatabase db(path.string());
            DoltLiteStatement stmt = db.Prepare("SELECT ID FROM dbc_map");
            while (stmt.Step())
                ids.insert(uint32_t(stmt.ColumnInt64(0)));
        }
        catch (std::exception const& e)
        {
            return std::string("reading dbc_map: ") + e.what();
        }
        return {};
    }

    // Maps present in map.db2 and in the world database that have a WDT.
    std::string ResolveMaps(Context& ctx)
    {
        std::set<uint32_t> worldMaps;
        if (std::string err = LoadWorldMapIds(ctx.options.world, worldMaps); !err.empty())
            return err;

        std::set<uint32_t> seen;
        for (MapRecord const& rec : ctx.db2.maps)
        {
            if (!worldMaps.count(rec.id) || !seen.insert(rec.id).second)
                continue;
            KnownMap known;
            known.record = rec;
            if (!Wdt::Load(ctx.client, rec.directory, known.wdt))
                continue;
            ctx.maps.push_back(std::move(known));
        }
        std::sort(ctx.maps.begin(), ctx.maps.end(), [](KnownMap const& a, KnownMap const& b) { return a.record.id < b.record.id; });

        for (uint32_t id : ctx.options.maps)
        {
            bool known = std::any_of(ctx.maps.begin(), ctx.maps.end(), [&](KnownMap const& m) { return m.record.id == id; });
            if (known)
                continue;
            if (!worldMaps.count(id))
                return "map " + std::to_string(id) + " is not in dbc_map of the world database";
            return "map " + std::to_string(id) + " has no Map.db2 entry or no WDT in the client files";
        }
        return {};
    }
}

int main(int argc, char** argv)
{
    Options options;
    bool help = false;
    if (std::string err = ParseOptions(argc, argv, options, help); !err.empty())
    {
        std::fprintf(stderr, "map_extractor: %s\n", err.c_str());
        return 2;
    }
    if (help)
    {
        PrintUsage();
        return 0;
    }

    Context ctx(std::move(options));
    if (std::string err = ctx.client.Load(); !err.empty())
    {
        std::fprintf(stderr, "map_extractor: %s\n", err.c_str());
        return 1;
    }
    if (std::string err = ctx.db2.Load(ctx.client); !err.empty())
    {
        std::fprintf(stderr, "map_extractor: %s\n", err.c_str());
        return 1;
    }
    if (std::string err = ResolveMaps(ctx); !err.empty())
    {
        std::fprintf(stderr, "map_extractor: %s\n", err.c_str());
        return 1;
    }

    Options const& o = ctx.options;
    std::printf("%zu known maps\n", ctx.maps.size());
    int exitCode = 0;
    for (KnownMap const& map : ctx.maps)
    {
        if (!o.maps.empty() && std::find(o.maps.begin(), o.maps.end(), map.record.id) == o.maps.end())
            continue;

        TileRect region = ctx.Region();
        size_t tileCount = 0;
        for (int r = region.rowMin; r <= region.rowMax; ++r)
            for (int c = region.colMin; c <= region.colMax; ++c)
                tileCount += map.wdt.Has(r, c);

        if (o.dryRun && !o.stageMap)
        {
            std::printf("map %u (%s): %zu tiles\n", map.record.id, map.record.directory.c_str(), tileCount);
            continue;
        }

        if (o.stageMap)
        {
            StageStats s = RunMapStage(ctx, map, region);
            std::printf("map %u (%s) map stage: %u %s, %u up to date, %u failed, %u missing input\n", map.record.id,
                map.record.directory.c_str(), s.done, o.dryRun ? "to do" : "done", s.upToDate, s.failed, s.missingInput);
            if (s.failed)
                exitCode = 1;
        }
    }
    return exitCode;
}
