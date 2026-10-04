#ifndef MAP_EXTRACTOR_CLIENT_DB2TABLES_H
#define MAP_EXTRACTOR_CLIENT_DB2TABLES_H

#include "Client/ClientStorage.h"

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

struct MapRecord
{
    uint32_t id = 0;
    std::string directory;
    uint32_t parentMapId = 0;
};

struct LiquidTypeRecord
{
    uint8_t soundBank = 0;
    uint8_t materialId = 0;
};

struct LiquidMaterialRecord
{
    uint8_t flags = 0;
    uint8_t lvf = 0;
};

struct GameObjectDisplayRecord
{
    std::string modelName;
    uint32_t fileDataId = 0;
};

struct Db2Tables
{
    std::vector<MapRecord> maps;
    std::unordered_map<uint32_t, LiquidTypeRecord> liquidTypes;
    std::unordered_map<uint32_t, LiquidMaterialRecord> liquidMaterials;
    std::unordered_map<uint32_t, uint16_t> liquidObjects; // id -> LiquidTypeID
    std::map<uint32_t, GameObjectDisplayRecord> gameObjectDisplays;

    // Reads the tables from dbfilesclient. Returns an empty string on success.
    std::string Load(ClientStorage const& client);
};

#endif
