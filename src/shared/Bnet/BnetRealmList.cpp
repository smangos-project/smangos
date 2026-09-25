#include "BnetRealmList.h"
#include "Bnet/BnetClientBuild.h"
#include "Database/DatabaseEnv.h"
#include "IO/Networking/DNS.h"
#include "IO/Networking/Utils.h"
#include "Log.h"

#include <algorithm>

namespace
{
enum BnetPopulationState : uint8
{
    POPULATION_OFFLINE     = 0,
    POPULATION_LOW         = 1,
    POPULATION_MEDIUM      = 2,
    POPULATION_HIGH        = 3,
    POPULATION_NEW         = 4,
    POPULATION_RECOMMENDED = 5,
    POPULATION_FULL        = 6,
    POPULATION_LOCKED      = 7,
};

// Client realm types are 0..13, and the client's config table is 1-based.
constexpr uint8 MAX_CLIENT_REALM_TYPE = 14;
}

IO::Networking::IpAddress const& BnetRealm::GetAddressForClient(IO::Networking::IpAddress const& clientAddress) const
{
    if (clientAddress.GetType() != IO::Networking::IpAddress::Type::IPv4)
        return externalAddress;

    return IO::Networking::IsInSameSubnet(clientAddress, localAddress, localSubnetMaskCidr) ? localAddress : externalAddress;
}

BnetRealmList& BnetRealmList::Instance()
{
    static BnetRealmList instance;
    return instance;
}

void BnetRealmList::Initialize(uint8 region, uint8 battlegroup, uint32 updateIntervalSeconds)
{
    m_region = region;
    m_battlegroup = battlegroup;
    m_updateInterval = updateIntervalSeconds;

    std::lock_guard<std::mutex> lock(m_lock);
    Reload();
}

uint32 BnetRealmList::GetRealmAddress(uint32 realmId) const
{
    return (uint32(m_region) << 24) | (uint32(m_battlegroup) << 16) | (realmId & 0xFFFF);
}

std::string BnetRealmList::GetSubRegionAddress() const
{
    return std::to_string(m_region) + "-" + std::to_string(m_battlegroup) + "-0";
}

std::vector<BnetRealm> BnetRealmList::GetRealms()
{
    std::lock_guard<std::mutex> lock(m_lock);
    UpdateIfNeeded();
    return m_realms;
}

nonstd::optional<BnetRealm> BnetRealmList::FindByAddress(uint32 realmAddress)
{
    std::lock_guard<std::mutex> lock(m_lock);
    UpdateIfNeeded();

    for (BnetRealm const& realm : m_realms)
        if (GetRealmAddress(realm.id) == realmAddress)
            return realm;
    return nonstd::nullopt;
}

nonstd::optional<BnetRealm> BnetRealmList::FindById(uint32 realmId)
{
    std::lock_guard<std::mutex> lock(m_lock);
    UpdateIfNeeded();

    for (BnetRealm const& realm : m_realms)
        if (realm.id == realmId)
            return realm;
    return nonstd::nullopt;
}

void BnetRealmList::UpdateIfNeeded()
{
    if (!m_updateInterval || m_nextUpdate > time(nullptr))
        return;
    Reload();
}

void BnetRealmList::Reload()
{
    m_nextUpdate = time(nullptr) + m_updateInterval;

    std::unique_ptr<QueryResult> result = LoginDatabase.Query(
        //       0     1       2          3               4                  5       6
        "SELECT `id`, `name`, `address`, `localAddress`, `localSubnetMask`, `port`, `icon`, "
        // 7            8           9                       10
        "`realmflags`, `timezone`, `allowedSecurityLevel`, `population` FROM `realmlist` "
        "WHERE (`realmflags` & 1) = 0 ORDER BY `id`");

    std::vector<BnetRealm> realms;
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            BnetRealm realm;
            realm.id = fields[0].GetUInt32();
            realm.name = fields[1].GetCppString();
            std::string const externalString = fields[2].GetCppString();
            std::string const localString = fields[3].GetCppString();
            std::string const subnetString = fields[4].GetCppString();
            realm.port = uint16(fields[5].GetUInt32());
            realm.icon = fields[6].GetUInt8();
            realm.legacyFlags = fields[7].GetUInt8();
            realm.timezone = fields[8].GetUInt8();
            realm.allowedSecurityLevel = AccountTypes(std::min<uint32>(fields[9].GetUInt32(), SEC_ADMINISTRATOR));
            realm.population = fields[10].GetFloat();

            auto external = IO::Networking::DNS::ResolveDomainSingle(externalString, IO::Networking::IpAddress::Type::IPv4);
            if (!external)
            {
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Realm \"%s\" (id %u): cannot resolve address '%s', skipping it", realm.name.c_str(), realm.id, externalString.c_str());
                continue;
            }
            realm.externalAddress = *external;

            auto local = IO::Networking::DNS::ResolveDomainSingle(localString, IO::Networking::IpAddress::Type::IPv4);
            if (!local)
            {
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Realm \"%s\" (id %u): cannot resolve localAddress '%s', skipping it", realm.name.c_str(), realm.id, localString.c_str());
                continue;
            }
            realm.localAddress = *local;

            auto mask = IO::Networking::IpAddress::TryParseFromString(subnetString);
            if (!mask)
            {
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Realm \"%s\" (id %u): bad localSubnetMask '%s', skipping it", realm.name.c_str(), realm.id, subnetString.c_str());
                continue;
            }
            uint32 maskBits = mask->_getInternalIPv4ReprAsUint32();
            if (((~maskBits) & ((~maskBits) + 1)) != 0)
            {
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Realm \"%s\" (id %u): localSubnetMask '%s' has holes, skipping it", realm.name.c_str(), realm.id, subnetString.c_str());
                continue;
            }
            while (maskBits)
            {
                realm.localSubnetMaskCidr += (maskBits & 1);
                maskBits >>= 1;
            }

            realms.push_back(std::move(realm));
        } while (result->NextRow());
    }

    if (m_realms.empty() && !realms.empty())
        for (BnetRealm const& realm : realms)
            sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Added realm \"%s\"", realm.name.c_str());

    m_realms = std::move(realms);
}

uint8 BnetRealmList::PopulationState(BnetRealm const& realm)
{
    if (realm.legacyFlags & REALM_FLAG_OFFLINE)
        return POPULATION_OFFLINE;
    if (realm.legacyFlags & REALM_FLAG_RECOMMENDED)
        return POPULATION_RECOMMENDED;
    if (realm.legacyFlags & REALM_FLAG_NEW_PLAYERS)
        return POPULATION_NEW;
    if ((realm.legacyFlags & REALM_FLAG_FULL) || realm.population > 0.95f)
        return POPULATION_FULL;
    if (realm.population > 0.66f)
        return POPULATION_HIGH;
    if (realm.population > 0.33f)
        return POPULATION_MEDIUM;
    return POPULATION_LOW;
}

nlohmann::json BnetRealmList::BuildRealmEntry(BnetRealm const& realm, AccountTypes security) const
{
    uint8 population = PopulationState(realm);
    if (population != POPULATION_OFFLINE && security < realm.allowedSecurityLevel)
        population = POPULATION_LOCKED;

    return {
        { "wowRealmAddress", GetRealmAddress(realm.id) },
        { "cfgTimezonesID", 1 },
        { "populationState", population },
        { "cfgCategoriesID", realm.timezone },
        { "version", {
            { "versionMajor", BNET_CLIENT_VERSION_MAJOR },
            { "versionMinor", BNET_CLIENT_VERSION_MINOR },
            { "versionRevision", BNET_CLIENT_VERSION_REVISION },
            { "versionBuild", BNET_CLIENT_BUILD },
        } },
        { "cfgRealmsID", realm.id },
        { "flags", 0 },
        { "name", realm.name },
        { "cfgConfigsID", realm.icon < MAX_CLIENT_REALM_TYPE ? realm.icon + 1 : 0 },
        { "cfgLanguagesID", 1 },
    };
}
