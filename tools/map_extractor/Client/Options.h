#ifndef MAP_EXTRACTOR_CLIENT_OPTIONS_H
#define MAP_EXTRACTOR_CLIENT_OPTIONS_H

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

constexpr int TILES_PER_MAP = 64;

// Tile as numbered in output file names: `row` follows world x, `col` follows world y
// (MMMRRCC.map, source file <dir>_<col>_<row>.adt).
struct TileRect
{
    int rowMin = 0, colMin = 0, rowMax = TILES_PER_MAP - 1, colMax = TILES_PER_MAP - 1;

    bool Contains(int row, int col) const { return row >= rowMin && row <= rowMax && col >= colMin && col <= colMax; }
    TileRect Grown(int by) const;
};

struct Options
{
    std::filesystem::path input;
    std::filesystem::path output;
    std::filesystem::path world;
    bool stageMap = false, stageVmap = false, stageMmap = false;
    std::vector<uint32_t> maps;
    std::optional<TileRect> tiles;
    unsigned threads = 0;
    bool force = false;
    bool dryRun = false;
    bool preciseVmaps = false;
    std::filesystem::path mmapConfig;
    std::filesystem::path offmesh;
    bool debugMmap = false;
    bool skipLiquid = false;
    bool skipContinents = false;
    bool skipJunkMaps = true;
    bool skipBattlegrounds = false;
};

// Returns an empty string on success, a message otherwise; `help` is set when usage was requested.
std::string ParseOptions(int argc, char** argv, Options& out, bool& help);
void PrintUsage();

#endif
