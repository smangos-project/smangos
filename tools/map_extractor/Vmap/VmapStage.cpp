#include "Vmap/VmapStage.h"

#include "Cache/Manifest.h"
#include "Client/ChunkedFile.h"
#include "Map/MapStage.h"
#include "Vmap/RawModels.h"

#include "TileAssembler.h"
#include "VMapDefinitions.h"

#include <G3D/Matrix3.h>
#include <G3D/Quat.h>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <mutex>
#include <set>
#include <tuple>
#include <unordered_map>

namespace
{
    namespace fs = std::filesystem;

    constexpr char const* STAGE_VERSION = "vmap-v1";
    constexpr float WORLD_OFFSET = 533.33333f * 32;
    constexpr int GLOBAL_TILE = 65;

    constexpr uint32_t SPAWN_M2 = 1;
    constexpr uint32_t SPAWN_WORLDSPAWN = 2;
    constexpr uint32_t SPAWN_HAS_BOUND = 4;

    constexpr uint16_t MDDF_FILEDATAID = 0x40;
    constexpr uint16_t MODF_DESTRUCTIBLE = 0x1;
    constexpr uint16_t MODF_SCALED = 0x4;
    constexpr uint16_t MODF_FILEDATAID = 0x8;

#pragma pack(push, 1)
    struct Mddf
    {
        uint32_t id, uniqueId;
        float position[3], rotation[3];
        uint16_t scale, flags;
    };

    struct Modf
    {
        uint32_t id, uniqueId;
        float position[3], rotation[3];
        float boundsMin[3], boundsMax[3];
        uint16_t flags, doodadSet, nameSet, scale;
    };
#pragma pack(pop)
    static_assert(sizeof(Mddf) == 36);
    static_assert(sizeof(Modf) == 64);

    // An ADT/WDT object as stored in the file.
    struct Placement
    {
        uint32_t fileDataId = 0;
        bool wmo = false;
        Modf modf{};
        Mddf mddf{};
    };

    struct TilePlacements
    {
        int row = 0, col = 0;
        std::vector<Placement> items;
    };

    using SpawnKey = std::tuple<bool, uint32_t, uint32_t>; // isWmo, client unique id, doodad number

    // A model instance on a tile, with the world transform of the vmap format.
    struct Candidate
    {
        SpawnKey key;
        uint32_t fileDataId = 0;
        bool wmo = false;
        uint32_t flags = 0;
        uint16_t adtId = 0;
        float pos[3] = {}, rot[3] = {}, scale = 1.0f;
        float bound[6] = {};
        int tileX = 0, tileY = 0;
    };

    struct ModelResult
    {
        ModelState state = ModelState::Failed;
        std::string fingerprint;
        WmoDoodads doodads;
    };

    using ModelKey = std::pair<uint32_t, bool>; // FileDataID, isWmo

    struct Models
    {
        fs::path dir;
        fs::path rawFile(uint32_t id, bool wmo) const { return dir / RawModelName(id, wmo); }
        fs::path doodadFile(uint32_t id) const { return dir / (RawModelName(id, true) + ".doodads"); }
        std::map<ModelKey, ModelResult> results;
    };

    bool WriteFile(fs::path const& path, void const* data, size_t size)
    {
        fs::path tmp = path;
        tmp += ".tmp";
        {
            std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
            out.write(static_cast<char const*>(data), std::streamsize(size));
            if (!out)
                return false;
        }
        std::error_code ec;
        fs::rename(tmp, path, ec);
        return !ec;
    }

    bool WriteFile(fs::path const& path, std::vector<uint8_t> const& bytes) { return WriteFile(path, bytes.data(), bytes.size()); }

    std::vector<uint8_t> ReadFile(fs::path const& path)
    {
        std::ifstream in(path, std::ios::binary);
        return { std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
    }

    std::vector<std::string> Strings(Chunk const* chunk)
    {
        std::vector<std::string> out;
        if (!chunk)
            return out;
        auto const* base = reinterpret_cast<char const*>(chunk->data);
        for (uint32_t off = 0; off < chunk->size;)
        {
            size_t len = strnlen(base + off, chunk->size - off);
            out.emplace_back(base + off, len);
            off += uint32_t(len) + 1;
        }
        return out;
    }

    template <typename T>
    void ForEachEntry(Chunk const* chunk, std::function<void(T const&)> const& fn)
    {
        if (!chunk)
            return;
        for (uint32_t i = 0; (i + 1) * sizeof(T) <= chunk->size; ++i)
        {
            T value;
            std::memcpy(&value, chunk->data + i * sizeof(T), sizeof(T));
            fn(value);
        }
    }

    void ParseModf(Chunk const* modf, std::vector<std::string> const& names, ClientStorage const& client, std::vector<Placement>& out)
    {
        ForEachEntry<Modf>(modf, [&](Modf const& m)
        {
            Placement p;
            p.wmo = true;
            p.modf = m;
            if (m.flags & MODF_FILEDATAID)
                p.fileDataId = m.id;
            else if (m.id < names.size())
                p.fileDataId = client.FileDataIdOf(names[m.id]);
            if (p.fileDataId)
                out.push_back(p);
        });
    }

    bool ParseObj0(ClientStorage const& client, std::vector<uint8_t> data, std::vector<Placement>& out)
    {
        ChunkedFile adt;
        if (!adt.Parse(std::move(data)))
            return false;
        std::vector<std::string> modelNames = Strings(adt.Get("MMDX"));
        std::vector<std::string> wmoNames = Strings(adt.Get("MWMO"));
        ForEachEntry<Mddf>(adt.Get("MDDF"), [&](Mddf const& d)
        {
            Placement p;
            p.mddf = d;
            if (d.flags & MDDF_FILEDATAID)
                p.fileDataId = d.id;
            else if (d.id < modelNames.size())
                p.fileDataId = client.FileDataIdOf(modelNames[d.id]);
            if (p.fileDataId)
                out.push_back(p);
        });
        ParseModf(adt.Get("MODF"), wmoNames, client, out);
        return true;
    }

    std::string Obj0Path(Context const& ctx, KnownMap const& map, int row, int col)
    {
        if (map.wdt.hasFileDataIds && (map.wdt.mphdFlags & 0x200))
            return ctx.client.PathOf(map.wdt.files[size_t(row * TILES_PER_MAP + col)].obj0);
        return map.wdt.AdtPath(map.record.directory, row, col, "_obj0");
    }

    std::vector<Placement> GlobalPlacements(Context const& ctx, KnownMap const& map)
    {
        std::vector<Placement> out;
        if (map.wdt.modf.empty())
            return out;
        // Only the first MWMO name is kept by the WDT reader, which is all a global WMO map has.
        std::vector<std::string> names;
        if (!map.wdt.globalWmoName.empty())
            names.push_back(map.wdt.globalWmoName);
        Chunk chunk;
        chunk.data = map.wdt.modf.data();
        chunk.size = uint32_t(map.wdt.modf.size());
        ParseModf(&chunk, names, ctx.client, out);
        return out;
    }

    void Fix(float const in[3], float out[3])
    {
        out[0] = in[2];
        out[1] = in[0];
        out[2] = in[1];
    }

    void ExpandPlacement(Placement const& p, Models const& models, bool global, int tileX, int tileY, std::vector<Candidate>& out)
    {
        if (!p.wmo)
        {
            Candidate c;
            c.key = { false, p.mddf.uniqueId, 0 };
            c.fileDataId = p.fileDataId;
            c.flags = SPAWN_M2;
            Fix(p.mddf.position, c.pos);
            std::memcpy(c.rot, p.mddf.rotation, 12);
            c.scale = p.mddf.scale / 1024.0f;
            c.tileX = tileX;
            c.tileY = tileY;
            out.push_back(c);
            return;
        }

        Modf const& m = p.modf;
        auto itr = models.results.find({ p.fileDataId, true });
        if (itr == models.results.end())
            return;
        ModelResult const& model = itr->second;
        uint32_t worldFlag = global ? SPAWN_WORLDSPAWN : 0;

        if (model.state == ModelState::Ok && !(m.flags & MODF_DESTRUCTIBLE))
        {
            Candidate c;
            c.key = { true, m.uniqueId, 0 };
            c.fileDataId = p.fileDataId;
            c.wmo = true;
            c.flags = SPAWN_HAS_BOUND | worldFlag;
            c.adtId = m.nameSet;
            Fix(m.position, c.pos);
            if (global)
            {
                c.pos[0] += WORLD_OFFSET;
                c.pos[1] += WORLD_OFFSET;
            }
            std::memcpy(c.rot, m.rotation, 12);
            c.scale = (m.flags & MODF_SCALED) ? m.scale / 1024.0f : 1.0f;
            Fix(m.boundsMin, c.bound);
            Fix(m.boundsMax, c.bound + 3);
            c.tileX = tileX;
            c.tileY = tileY;
            out.push_back(c);
        }

        WmoDoodads const& d = model.doodads;
        if (d.sets.empty())
            return;

        G3D::Vector3 wmoPosition(m.position[2], m.position[0], m.position[1]);
        G3D::Matrix3 wmoRotation = G3D::Matrix3::fromEulerAnglesZYX(
            G3D::toRadians(m.rotation[1]), G3D::toRadians(m.rotation[0]), G3D::toRadians(m.rotation[2]));
        if (global)
            wmoPosition += G3D::Vector3(WORLD_OFFSET, WORLD_OFFSET, 0.0f);

        uint32_t doodadId = 0;
        auto addSet = [&](WmoDoodadSet const& set)
        {
            for (uint16_t index : d.references)
            {
                if (index < set.startIndex || index >= set.startIndex + set.count)
                    continue;
                WmoDoodadSpawn const& s = d.spawns[index];

                ++doodadId;
                G3D::Vector3 position = wmoPosition + wmoRotation * G3D::Vector3(s.position[0], s.position[1], s.position[2]);
                float rz, rx, ry;
                (G3D::Quat(s.rotation[0], s.rotation[1], s.rotation[2], s.rotation[3]).toRotationMatrix() * wmoRotation)
                    .toEulerAnglesXYZ(rz, rx, ry);

                Candidate c;
                c.key = { false, m.uniqueId, doodadId };
                c.fileDataId = s.fileDataId;
                c.flags = SPAWN_M2 | worldFlag;
                c.pos[0] = position.x;
                c.pos[1] = position.y;
                c.pos[2] = position.z;
                c.rot[0] = G3D::toDegrees(rx);
                c.rot[1] = G3D::toDegrees(ry);
                c.rot[2] = G3D::toDegrees(rz);
                c.scale = s.scale;
                c.tileX = tileX;
                c.tileY = tileY;
                out.push_back(c);
            }
        };
        addSet(d.sets[0]);
        if (m.doodadSet != 0 && m.doodadSet < d.sets.size())
            addSet(d.sets[m.doodadSet]);
    }

    std::string ModelKeyName(ModelKey const& k) { return RawModelName(k.first, k.second); }

    class ModelConverter
    {
    public:
        ModelConverter(Context& ctx, Manifest& manifest, Models& models) : m_ctx(ctx), m_manifest(manifest), m_models(models) { }

        // Brings the raw model of every key up to date; results are stored in models.results.
        void Ensure(std::set<ModelKey> const& keys, StageStats& stats)
        {
            std::vector<ModelKey> todo;
            for (ModelKey const& k : keys)
                if (!m_models.results.count(k))
                    todo.push_back(k);
            for (ModelKey const& k : todo)
                m_models.results[k]; // slots are filled concurrently below

            std::atomic<unsigned> done{ 0 }, upToDate{ 0 }, failed{ 0 };
            ParallelFor(todo.size(), m_ctx.options.threads, [&](size_t i)
            {
                ModelKey const& k = todo[i];
                ModelResult& result = m_models.results.find(k)->second;
                if (LoadCached(k, result))
                {
                    ++upToDate;
                    return;
                }
                if (m_ctx.options.dryRun)
                {
                    std::printf("  model %s: would redo\n", ModelKeyName(k).c_str());
                    ++done;
                    return;
                }
                if (Convert(k, result))
                    ++done;
                else
                    ++failed;
            });
            stats.done += done;
            stats.upToDate += upToDate;
            stats.missingInput += failed;
        }

    private:
        std::string Value(ModelState state, std::string const& fp) const
        {
            return fp + (state == ModelState::Ok ? " ok" : " empty");
        }

        std::string Fingerprint_(std::vector<uint32_t> const& inputs) const
        {
            Fingerprint fp;
            fp.Add(STAGE_VERSION);
            fp.Add(m_ctx.options.preciseVmaps ? "precise" : "pruned");
            for (uint32_t id : inputs)
                fp.Add(m_ctx.client.PathOf(id), m_ctx.client.Stat(id));
            return fp.Hex();
        }

        static std::vector<uint32_t> ParseIds(std::string const& s)
        {
            std::vector<uint32_t> ids;
            size_t pos = 0;
            while (pos < s.size())
            {
                size_t end = s.find(',', pos);
                if (end == std::string::npos)
                    end = s.size();
                ids.push_back(uint32_t(std::stoul(s.substr(pos, end - pos))));
                pos = end + 1;
            }
            return ids;
        }

        bool LoadCached(ModelKey const& k, ModelResult& out)
        {
            if (m_ctx.options.force)
                return false;
            std::string name = ModelKeyName(k);
            std::string value = m_manifest.Get("model:" + name);
            std::string inputs = m_manifest.Get("inputs:" + name);
            size_t space = value.find(' ');
            if (space == std::string::npos || inputs.empty())
                return false;
            std::string fp = value.substr(0, space);
            ModelState state = value.substr(space + 1) == "ok" ? ModelState::Ok : ModelState::NoCollision;
            if (fp != Fingerprint_(ParseIds(inputs)))
                return false;
            std::error_code ec;
            if (state == ModelState::Ok && !fs::exists(m_models.rawFile(k.first, k.second), ec))
                return false;
            if (k.second && !WmoDoodads::Deserialize(ReadFile(m_models.doodadFile(k.first)), out.doodads))
                return false;
            out.state = state;
            out.fingerprint = fp;
            return true;
        }

        bool Convert(ModelKey const& k, ModelResult& out)
        {
            ClientStorage const& client = m_ctx.client;
            std::vector<uint8_t> raw;
            std::vector<uint32_t> inputs;
            ModelState state;
            if (k.second)
            {
                state = ConvertWmo(client, k.first, raw, out.doodads, inputs);
            }
            else
            {
                std::vector<uint8_t> file = client.Read(k.first);
                state = file.empty() ? ModelState::Failed : ConvertM2(file, raw);
                inputs.push_back(k.first);
            }
            std::string name = ModelKeyName(k);
            if (state == ModelState::Failed)
            {
                std::fprintf(stderr, "model %s (%s): cannot convert\n", name.c_str(), client.PathOf(k.first).c_str());
                return false;
            }

            bool ok = true;
            if (state == ModelState::Ok)
                ok = WriteFile(m_models.rawFile(k.first, k.second), raw);
            else
            {
                std::error_code ec;
                fs::remove(m_models.rawFile(k.first, k.second), ec);
            }
            if (ok && k.second)
                ok = WriteFile(m_models.doodadFile(k.first), out.doodads.Serialize());
            if (!ok)
            {
                std::fprintf(stderr, "model %s: cannot write output\n", name.c_str());
                return false;
            }

            std::string ids;
            for (uint32_t id : inputs)
                ids += (ids.empty() ? "" : ",") + std::to_string(id);
            out.state = state;
            out.fingerprint = Fingerprint_(inputs);
            m_manifest.Set("inputs:" + name, ids);
            m_manifest.Set("model:" + name, Value(state, out.fingerprint));
            return true;
        }

        Context& m_ctx;
        Manifest& m_manifest;
        Models& m_models;
    };

    void Put(std::vector<uint8_t>& out, void const* data, size_t size)
    {
        auto const* p = static_cast<uint8_t const*>(data);
        out.insert(out.end(), p, p + size);
    }

    template <typename T>
    void Put(std::vector<uint8_t>& out, T const& value) { Put(out, &value, sizeof(T)); }

    struct MapSpawn
    {
        Candidate c;
        uint32_t id = 0;
        std::string name;
        std::set<std::pair<int, int>> tiles;
    };

    // dir_bin as read by TileAssembler: mapId, tileX, tileY, then the spawn.
    std::vector<uint8_t> BuildDirBin(uint32_t mapId, std::vector<MapSpawn> const& spawns)
    {
        std::vector<uint8_t> out;
        for (MapSpawn const& s : spawns)
        {
            for (auto const& [tx, ty] : s.tiles)
            {
                Put(out, mapId);
                Put(out, uint32_t(tx));
                Put(out, uint32_t(ty));
                Put(out, s.c.flags);
                Put(out, s.c.adtId);
                Put(out, s.id);
                Put(out, s.c.pos, 12);
                Put(out, s.c.rot, 12);
                Put(out, s.c.scale);
                if (s.c.flags & SPAWN_HAS_BOUND)
                    Put(out, s.c.bound, 24);
                Put(out, uint32_t(s.name.size()));
                Put(out, s.name.data(), s.name.size());
            }
        }
        return out;
    }

    bool IsWmoDisplay(std::string const& modelName)
    {
        return modelName.size() >= 4 && !strcasecmp(modelName.c_str() + modelName.size() - 4, ".wmo");
    }
}

StageStats RunVmapStage(Context& ctx, std::vector<KnownMap const*> const& maps, TileRect const& region)
{
    StageStats stats;
    fs::path buildings = ctx.options.output / "Buildings";
    fs::path vmaps = ctx.options.output / "vmaps";
    fs::path spawnDir = ctx.CacheDir() / "vmap";
    Manifest manifest(ctx.CacheDir() / "vmap.manifest");
    if (!ctx.options.dryRun)
    {
        std::error_code ec;
        fs::create_directories(buildings, ec);
        fs::create_directories(vmaps, ec);
        fs::create_directories(spawnDir, ec);
    }

    // Placements per map and tile.
    struct MapWork
    {
        KnownMap const* map = nullptr;
        std::vector<Placement> global;
        std::vector<TilePlacements> tiles;
        std::vector<Candidate> candidates;
    };
    std::vector<MapWork> work(maps.size());
    struct TileRef { size_t map; int row, col; };
    std::vector<TileRef> refs;
    for (size_t i = 0; i < maps.size(); ++i)
    {
        work[i].map = maps[i];
        work[i].global = GlobalPlacements(ctx, *maps[i]);
        for (int row = region.rowMin; row <= region.rowMax; ++row)
            for (int col = region.colMin; col <= region.colMax; ++col)
                if (maps[i]->wdt.Has(row, col))
                    refs.push_back({ i, row, col });
    }

    std::vector<TilePlacements> parsed(refs.size());
    ParallelFor(refs.size(), ctx.options.threads, [&](size_t i)
    {
        TileRef const& r = refs[i];
        parsed[i].row = r.row;
        parsed[i].col = r.col;
        ParseObj0(ctx.client, ctx.client.Read(Obj0Path(ctx, *maps[r.map], r.row, r.col)), parsed[i].items);
    });
    for (size_t i = 0; i < refs.size(); ++i)
        work[refs[i].map].tiles.push_back(std::move(parsed[i]));
    parsed.clear();

    Models models;
    models.dir = buildings;
    ModelConverter converter(ctx, manifest, models);

    std::set<ModelKey> wmoKeys;
    for (MapWork const& w : work)
    {
        for (Placement const& p : w.global)
            wmoKeys.insert({ p.fileDataId, true });
        for (TilePlacements const& t : w.tiles)
            for (Placement const& p : t.items)
                if (p.wmo)
                    wmoKeys.insert({ p.fileDataId, true });
    }
    std::printf("vmap: converting %zu wmo models\n", wmoKeys.size());
    converter.Ensure(wmoKeys, stats);

    std::set<ModelKey> m2Keys;
    for (MapWork& w : work)
    {
        for (Placement const& p : w.global)
            ExpandPlacement(p, models, true, GLOBAL_TILE, GLOBAL_TILE, w.candidates);
        for (TilePlacements const& t : w.tiles)
            for (Placement const& p : t.items)
                ExpandPlacement(p, models, false, t.col, t.row, w.candidates);
        for (Candidate const& c : w.candidates)
            if (!c.wmo && c.fileDataId)
                m2Keys.insert({ c.fileDataId, false });
    }
    std::printf("vmap: converting %zu m2 models\n", m2Keys.size());
    converter.Ensure(m2Keys, stats);

    // Gameobject models.
    std::set<ModelKey> goKeys;
    for (auto const& [displayId, rec] : ctx.db2.gameObjectDisplays)
        if (rec.fileDataId)
            goKeys.insert({ rec.fileDataId, IsWmoDisplay(rec.modelName) });
    std::printf("vmap: converting %zu gameobject models\n", goKeys.size());
    converter.Ensure(goKeys, stats);

    // Spawns per map.
    struct Assembly
    {
        uint32_t mapId;
        std::vector<MapSpawn> spawns;
        std::vector<uint8_t> dirBin;
        std::string fingerprint;
        std::string reason;
    };
    std::vector<Assembly> assemblies;
    std::set<ModelKey> usedModels;
    for (MapWork& w : work)
    {
        std::map<SpawnKey, MapSpawn> unique;
        for (Candidate const& c : w.candidates)
        {
            if (!c.fileDataId)
                continue;
            auto model = models.results.find({ c.fileDataId, c.wmo });
            if (model == models.results.end() || model->second.state != ModelState::Ok)
                continue;
            MapSpawn& s = unique[c.key];
            if (s.name.empty())
            {
                s.c = c;
                s.name = RawModelName(c.fileDataId, c.wmo);
            }
            s.tiles.insert({ c.tileX, c.tileY });
        }

        Assembly a;
        a.mapId = w.map->record.id;
        uint32_t nextId = 1;
        Fingerprint fp;
        fp.Add(STAGE_VERSION);
        for (auto& [key, spawn] : unique)
        {
            spawn.id = nextId++;
            usedModels.insert({ spawn.c.fileDataId, spawn.c.wmo });
            fp.Add(models.results[{ spawn.c.fileDataId, spawn.c.wmo }].fingerprint);
            a.spawns.push_back(std::move(spawn));
        }
        a.dirBin = BuildDirBin(a.mapId, a.spawns);
        fp.Add(std::string(a.dirBin.begin(), a.dirBin.end()));
        a.fingerprint = fp.Hex();
        assemblies.push_back(std::move(a));
    }

    for (ModelKey const& k : goKeys)
        if (auto itr = models.results.find(k); itr != models.results.end() && itr->second.state == ModelState::Ok)
            usedModels.insert(k);

    // Gameobject model list in raw form, then with bounds.
    std::vector<uint8_t> goList;
    Fingerprint goFp;
    goFp.Add(STAGE_VERSION);
    for (auto const& [displayId, rec] : ctx.db2.gameObjectDisplays)
    {
        if (!rec.fileDataId)
            continue;
        auto itr = models.results.find({ rec.fileDataId, IsWmoDisplay(rec.modelName) });
        if (itr == models.results.end() || itr->second.state != ModelState::Ok)
            continue;
        std::string name = RawModelName(rec.fileDataId, IsWmoDisplay(rec.modelName));
        Put(goList, displayId);
        Put(goList, uint32_t(name.size()));
        Put(goList, name.data(), name.size());
        goFp.Add(std::to_string(displayId) + name + itr->second.fingerprint);
    }

    if (ctx.options.dryRun)
    {
        for (Assembly const& a : assemblies)
        {
            char tree[32];
            std::snprintf(tree, sizeof(tree), "%03u.vmtree", a.mapId);
            std::error_code ec;
            if (ctx.options.force || manifest.Get("map:" + std::to_string(a.mapId)) != a.fingerprint || !fs::exists(vmaps / tree, ec))
            {
                std::printf("  map %u: would reassemble\n", a.mapId);
                ++stats.done;
            }
            else
                ++stats.upToDate;
        }
        return stats;
    }

    // vmo files
    std::vector<ModelKey> vmoTodo;
    for (ModelKey const& k : usedModels)
    {
        std::string name = ModelKeyName(k);
        std::error_code ec;
        if (!ctx.options.force && manifest.Get("vmo:" + name) == models.results[k].fingerprint && fs::exists(vmaps / (name + ".vmo"), ec))
            continue;
        vmoTodo.push_back(k);
    }
    std::printf("vmap: building %zu of %zu vmo files\n", vmoTodo.size(), usedModels.size());
    std::atomic<unsigned> vmoFailed{ 0 };
    ParallelFor(vmoTodo.size(), ctx.options.threads, [&](size_t i)
    {
        std::string name = ModelKeyName(vmoTodo[i]);
        VMAP::TileAssembler assembler(buildings.string(), vmaps.string());
        if (!assembler.convertRawFile(name))
        {
            std::fprintf(stderr, "model %s: vmo conversion failed\n", name.c_str());
            ++vmoFailed;
            return;
        }
        manifest.Set("vmo:" + name, models.results[vmoTodo[i]].fingerprint);
    });
    stats.failed += vmoFailed;
    stats.done += unsigned(vmoTodo.size()) - vmoFailed;

    // Gameobject model list
    std::string goFile = (vmaps / VMAP::GAMEOBJECT_MODELS).string();
    std::error_code ec;
    if (ctx.options.force || manifest.Get("gameobjects") != goFp.Hex() || !fs::exists(goFile, ec))
    {
        WriteFile(buildings / VMAP::GAMEOBJECT_MODELS, goList);
        VMAP::TileAssembler assembler(buildings.string(), vmaps.string());
        assembler.exportGameobjectModels();
        manifest.Set("gameobjects", goFp.Hex());
        ++stats.done;
    }

    // vmtree and vmtile files
    std::atomic<unsigned> assembled{ 0 }, failed{ 0 }, current{ 0 };
    ParallelFor(assemblies.size(), ctx.options.threads, [&](size_t i)
    {
        Assembly const& a = assemblies[i];
        char tree[32];
        std::snprintf(tree, sizeof(tree), "%03u.vmtree", a.mapId);
        std::string key = "map:" + std::to_string(a.mapId);
        std::error_code localEc;
        if (!ctx.options.force && manifest.Get(key) == a.fingerprint && fs::exists(vmaps / tree, localEc))
        {
            ++current;
            return;
        }

        char prefix[16];
        std::snprintf(prefix, sizeof(prefix), "%03u", a.mapId);
        for (fs::directory_entry const& e : fs::directory_iterator(vmaps, localEc))
        {
            std::string file = e.path().filename().string();
            if ((file.size() > 3 && file.compare(0, 3, prefix) == 0 && file[3] == '_' && file.ends_with(".vmtile")) || file == tree)
                fs::remove(e.path(), localEc);
        }
        manifest.Erase(key);
        if (a.spawns.empty())
        {
            manifest.Set(key, a.fingerprint);
            ++assembled;
            return;
        }

        fs::path dirBin = spawnDir / (std::to_string(a.mapId) + ".dir_bin");
        VMAP::TileAssembler assembler(buildings.string(), vmaps.string());
        assembler.setSpawnFile(dirBin.string());
        if (!WriteFile(dirBin, a.dirBin) || !assembler.convertWorld2(false))
        {
            std::fprintf(stderr, "map %u: vmap assembly failed\n", a.mapId);
            ++failed;
            return;
        }
        manifest.Set(key, a.fingerprint);
        ++assembled;
    });
    stats.done += assembled;
    stats.upToDate += current;
    stats.failed += failed;
    manifest.Save();
    return stats;
}
