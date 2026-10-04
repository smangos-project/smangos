#include "Client/Options.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <thread>

#ifndef MAP_EXTRACTOR_SOURCE_DIR
#define MAP_EXTRACTOR_SOURCE_DIR "."
#endif

TileRect TileRect::Grown(int by) const
{
    TileRect r;
    r.rowMin = std::max(0, rowMin - by);
    r.colMin = std::max(0, colMin - by);
    r.rowMax = std::min(TILES_PER_MAP - 1, rowMax + by);
    r.colMax = std::min(TILES_PER_MAP - 1, colMax + by);
    return r;
}

namespace
{
    std::vector<std::string> Split(std::string const& s, char sep)
    {
        std::vector<std::string> parts;
        size_t start = 0;
        for (;;)
        {
            size_t at = s.find(sep, start);
            parts.push_back(s.substr(start, at == std::string::npos ? at : at - start));
            if (at == std::string::npos)
                return parts;
            start = at + 1;
        }
    }

    bool ParseInt(std::string const& s, long& out)
    {
        char* end = nullptr;
        out = std::strtol(s.c_str(), &end, 10);
        return !s.empty() && end && !*end;
    }

    bool ParseFloat(std::string const& s, double& out)
    {
        char* end = nullptr;
        out = std::strtod(s.c_str(), &end);
        return !s.empty() && end && !*end;
    }

    int TileFromWorld(double w) { return int(32.0 - w / (1600.0 / 3.0)); }
}

void PrintUsage()
{
    std::printf(
        "Usage: map_extractor [options]\n"
        "  --input DIR          extracted client (default .client/Content)\n"
        "  --output DIR         server data directory (default .server/data)\n"
        "  --world FILE         world database listing the known maps (default .world/world.doltlite)\n"
        "  --stages LIST        map,vmap,mmap (default all)\n"
        "  --map ID[,ID...]     restrict to maps\n"
        "  --tile R,C           restrict to one tile (numbers as in MMMRRCC.map)\n"
        "  --tiles R1,C1,R2,C2  restrict to a tile rectangle\n"
        "  --around X,Y,RADIUS  restrict to tiles within RADIUS yards of world position\n"
        "  --threads N          worker threads (default: all cores)\n"
        "  --force              redo the selected region regardless of the cache\n"
        "  --dry-run            print what would be redone and why\n"
        "  --precise-vmaps      keep every vmap model detail (no triangle pruning)\n"
        "  --mmap-config FILE   per map/tile mmap overrides (json)\n"
        "  --offmesh FILE       off-mesh connections for mmaps\n"
        "  --max-angle DEG      mmap walkable slope (default 60)\n"
        "  --skip-liquid        mmaps without liquid\n"
        "  --debug-mmap         write debug obj meshes next to mmaps\n");
}

std::string ParseOptions(int argc, char** argv, Options& o, bool& help)
{
    help = false;
    std::filesystem::path root = MAP_EXTRACTOR_SOURCE_DIR;
    o.input = root / ".client" / "Content";
    o.output = root / ".server" / "data";
    o.world = root / ".world" / "world.doltlite";
    o.threads = std::max(1u, std::thread::hardware_concurrency());
    std::string stages = "map,vmap,mmap";

    for (int i = 1; i < argc; ++i)
    {
        std::string a = argv[i];
        auto value = [&](std::string& v) -> bool
        {
            if (i + 1 >= argc)
                return false;
            v = argv[++i];
            return true;
        };
        std::string v;

        if (a == "-h" || a == "--help") { help = true; return {}; }
        else if (a == "--force") o.force = true;
        else if (a == "--dry-run") o.dryRun = true;
        else if (a == "--precise-vmaps") o.preciseVmaps = true;
        else if (a == "--skip-liquid") o.skipLiquid = true;
        else if (a == "--debug-mmap") o.debugMmap = true;
        else if (a == "--input" || a == "--output" || a == "--world" || a == "--stages" || a == "--map" || a == "--tile" ||
                 a == "--tiles" || a == "--around" || a == "--threads" || a == "--mmap-config" || a == "--offmesh" || a == "--max-angle")
        {
            if (!value(v))
                return a + " needs a value";
            auto parts = Split(v, ',');
            std::vector<long> n;
            auto numbers = [&](size_t count) -> bool
            {
                if (parts.size() != count)
                    return false;
                for (auto const& p : parts)
                {
                    long x;
                    if (!ParseInt(p, x))
                        return false;
                    n.push_back(x);
                }
                return true;
            };

            if (a == "--input") o.input = v;
            else if (a == "--output") o.output = v;
            else if (a == "--world") o.world = v;
            else if (a == "--mmap-config") o.mmapConfig = v;
            else if (a == "--offmesh") o.offmesh = v;
            else if (a == "--stages") stages = v;
            else if (a == "--map")
            {
                for (auto const& p : parts)
                {
                    long id;
                    if (!ParseInt(p, id) || id < 0)
                        return "bad map id '" + p + "'";
                    o.maps.push_back(uint32_t(id));
                }
            }
            else if (a == "--threads")
            {
                long t;
                if (!ParseInt(v, t) || t < 1)
                    return "bad --threads";
                o.threads = unsigned(t);
            }
            else if (a == "--max-angle")
            {
                double d;
                if (!ParseFloat(v, d))
                    return "bad --max-angle";
                o.maxAngle = float(d);
            }
            else if (a == "--tile")
            {
                if (!numbers(2))
                    return "--tile needs R,C";
                o.tiles = TileRect{ int(n[0]), int(n[1]), int(n[0]), int(n[1]) };
            }
            else if (a == "--tiles")
            {
                if (!numbers(4))
                    return "--tiles needs R1,C1,R2,C2";
                o.tiles = TileRect{ int(std::min(n[0], n[2])), int(std::min(n[1], n[3])), int(std::max(n[0], n[2])), int(std::max(n[1], n[3])) };
            }
            else if (a == "--around")
            {
                double x, y, r;
                if (parts.size() != 3 || !ParseFloat(parts[0], x) || !ParseFloat(parts[1], y) || !ParseFloat(parts[2], r) || r < 0)
                    return "--around needs X,Y,RADIUS";
                int r0 = TileFromWorld(x + r), r1 = TileFromWorld(x - r);
                int c0 = TileFromWorld(y + r), c1 = TileFromWorld(y - r);
                o.tiles = TileRect{ std::min(r0, r1), std::min(c0, c1), std::max(r0, r1), std::max(c0, c1) };
            }
        }
        else
            return "unknown option " + a;
    }

    for (auto const& s : Split(stages, ','))
    {
        if (s == "map") o.stageMap = true;
        else if (s == "vmap") o.stageVmap = true;
        else if (s == "mmap") o.stageMmap = true;
        else return "unknown stage '" + s + "'";
    }

    if (o.tiles)
    {
        TileRect& t = *o.tiles;
        if (t.rowMin < 0 || t.colMin < 0 || t.rowMax >= TILES_PER_MAP || t.colMax >= TILES_PER_MAP)
            return "tile outside 0..63";
        if (o.maps.size() != 1)
            return "--tile/--tiles/--around need exactly one --map";
    }
    return {};
}
