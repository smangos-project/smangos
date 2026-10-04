#include "Formats/Wdt.h"

#include "Client/ChunkedFile.h"

#include <cstdio>
#include <cstring>

namespace
{
#pragma pack(push, 1)
    struct MainEntry { uint32_t flag, data; };
    struct MaidEntry { uint32_t root, obj0, obj1, tex0, lod, mapTexture, mapTextureN, minimap; };
#pragma pack(pop)

    constexpr uint32_t MPHD_FLAG_FILEDATAIDS = 0x200;
}

std::string WdtPath(std::string const& directory)
{
    return ClientStorage::Normalize("world/maps/" + directory + "/" + directory + ".wdt");
}

std::string Wdt::AdtPath(std::string const& directory, int row, int col, char const* suffix) const
{
    char buf[64];
    std::snprintf(buf, sizeof(buf), "_%d_%d%s.adt", col, row, suffix);
    return ClientStorage::Normalize("world/maps/" + directory + "/" + directory + buf);
}

bool Wdt::Load(ClientStorage const& client, std::string const& directory, Wdt& out)
{
    ChunkedFile file;
    if (!file.Parse(client.Read(WdtPath(directory))))
        return false;

    Chunk const* main = file.Get("MAIN");
    if (!main || main->size < sizeof(MainEntry) * TILES_PER_MAP * TILES_PER_MAP)
        return false;

    out = {};
    if (Chunk const* mphd = file.Get("MPHD"); mphd && mphd->size >= 4)
        std::memcpy(&out.mphdFlags, mphd->data, 4);

    auto const* entries = reinterpret_cast<MainEntry const*>(main->data);
    for (int i = 0; i < TILES_PER_MAP * TILES_PER_MAP; ++i)
        if (entries[i].flag & 1)
            out.tiles[size_t(i)] = true;

    if (Chunk const* maid = file.Get("MAID"); maid && maid->size >= sizeof(MaidEntry) * TILES_PER_MAP * TILES_PER_MAP)
    {
        out.hasFileDataIds = true;
        out.files.resize(TILES_PER_MAP * TILES_PER_MAP);
        auto const* ids = reinterpret_cast<MaidEntry const*>(maid->data);
        for (int i = 0; i < TILES_PER_MAP * TILES_PER_MAP; ++i)
            out.files[size_t(i)] = { ids[i].root, ids[i].obj0, ids[i].obj1, ids[i].tex0 };
    }

    if (Chunk const* modf = file.Get("MODF"))
        out.modf.assign(modf->data, modf->data + modf->size);
    if (Chunk const* mwmo = file.Get("MWMO"); mwmo && mwmo->size > 0)
        out.globalWmoName.assign(reinterpret_cast<char const*>(mwmo->data), strnlen(reinterpret_cast<char const*>(mwmo->data), mwmo->size));
    return true;
}
