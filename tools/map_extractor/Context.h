#ifndef MAP_EXTRACTOR_CONTEXT_H
#define MAP_EXTRACTOR_CONTEXT_H

#include "Client/ClientStorage.h"
#include "Client/Db2Tables.h"
#include "Client/Options.h"
#include "Formats/Wdt.h"

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

struct KnownMap
{
    MapRecord record;
    Wdt wdt;
};

struct Context
{
    Options options;
    ClientStorage client;
    Db2Tables db2;
    std::vector<KnownMap> maps;

    explicit Context(Options o) : options(std::move(o)), client(options.input) { }

    std::filesystem::path MapsDir() const { return options.output / "maps"; }
    std::filesystem::path CacheDir() const { return options.output / ".cache"; }
    std::filesystem::path MapFile(uint32_t mapId, int row, int col) const;

    // Tile range of the --tile options, or the whole map.
    TileRect Region() const { return options.tiles.value_or(TileRect{}); }
};

struct StageStats
{
    unsigned done = 0;
    unsigned upToDate = 0;
    unsigned failed = 0;
    unsigned missingInput = 0;
};

// Runs fn(i) for i in [0, count) on `threads` workers.
void ParallelFor(size_t count, unsigned threads, std::function<void(size_t)> const& fn);

#endif
