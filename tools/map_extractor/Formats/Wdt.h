#ifndef MAP_EXTRACTOR_FORMATS_WDT_H
#define MAP_EXTRACTOR_FORMATS_WDT_H

#include "Client/ClientStorage.h"
#include "Client/Options.h"

#include <bitset>
#include <cstdint>
#include <string>
#include <vector>

struct WdtTileFiles
{
    uint32_t root = 0, obj0 = 0, obj1 = 0, tex0 = 0;
};

struct Wdt
{
    uint32_t mphdFlags = 0;
    std::bitset<TILES_PER_MAP * TILES_PER_MAP> tiles; // index row * 64 + col
    bool hasFileDataIds = false;                       // MAID present
    std::vector<WdtTileFiles> files;                   // same index, only with MAID
    std::vector<uint8_t> modf;                         // global WMO placement (MODF payload)
    std::string globalWmoName;                         // MWMO

    bool Has(int row, int col) const { return tiles[size_t(row * TILES_PER_MAP + col)]; }

    // Source ADT path for a tile of map `directory`.
    std::string AdtPath(std::string const& directory, int row, int col, char const* suffix) const;

    // False if the WDT is missing or has no MAIN chunk.
    static bool Load(ClientStorage const& client, std::string const& directory, Wdt& out);
};

std::string WdtPath(std::string const& directory);

#endif
