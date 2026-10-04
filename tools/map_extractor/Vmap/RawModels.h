#ifndef MAP_EXTRACTOR_VMAP_RAWMODELS_H
#define MAP_EXTRACTOR_VMAP_RAWMODELS_H

#include "Client/ClientStorage.h"

#include <cstdint>
#include <string>
#include <vector>

enum class ModelState
{
    Failed,      // unreadable or inconsistent input
    NoCollision, // valid file without collision geometry
    Ok,
};

// MODD entry of a WMO doodad set, with the model resolved to a FileDataID (0 if unknown).
struct WmoDoodadSpawn
{
    uint32_t fileDataId = 0;
    float position[3] = {};
    float rotation[4] = {}; // x, y, z, w
    float scale = 1.0f;
};

struct WmoDoodadSet
{
    uint32_t startIndex = 0;
    uint32_t count = 0;
};

// Doodads placed by a root WMO; stored next to the raw model so ADT placement doesn't reparse the WMO.
struct WmoDoodads
{
    std::vector<WmoDoodadSet> sets;
    std::vector<WmoDoodadSpawn> spawns;
    std::vector<uint16_t> references; // sorted spawn indices used by any group

    std::vector<uint8_t> Serialize() const;
    static bool Deserialize(std::vector<uint8_t> const& data, WmoDoodads& out);
};

// Raw model names are derived from the FileDataID so models from different folders never collide.
std::string RawModelName(uint32_t fileDataId, bool wmo);

// Writes the "VMAPs05" raw M2 collision model into `out`.
ModelState ConvertM2(std::vector<uint8_t> const& file, std::vector<uint8_t>& out);

// Writes the raw WMO (root + collision groups) and its doodad data. Files read are appended to `inputs`.
ModelState ConvertWmo(ClientStorage const& client, uint32_t rootFileDataId, std::vector<uint8_t>& out,
    WmoDoodads& doodads, std::vector<uint32_t>& inputs);

#endif
