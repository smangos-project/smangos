#ifndef SHARED_BNETREALMLIST_H
#define SHARED_BNETREALMLIST_H

#include "Common.h"
#include "IO/Networking/IpAddress.h"
#include "nonstd/optional.hpp"

#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

// A `realmlist` row, resolved.
struct BnetRealm
{
    uint32 id = 0;
    std::string name;
    IO::Networking::IpAddress externalAddress;
    IO::Networking::IpAddress localAddress;
    uint8 localSubnetMaskCidr = 0;
    uint16 port = 0;
    uint8 icon = 0;
    uint8 legacyFlags = 0;
    uint8 timezone = 0;
    AccountTypes allowedSecurityLevel = SEC_PLAYER;
    float population = 0.0f;

    bool IsOffline() const { return (legacyFlags & REALM_FLAG_OFFLINE) != 0; }
    IO::Networking::IpAddress const& GetAddressForClient(IO::Networking::IpAddress const& clientAddress) const;
};

// The realms in `realmlist`, re-read after the update interval so a realm going offline or coming
// back shows up without a restart. Realm addresses are (region << 24) | (battlegroup << 16) | id,
// with region and battlegroup coming from configuration. Shared between bnetserver (which needs
// the full realm list) and mangosd (which only needs its own row's address, to redirect a session
// to the Instance connection without re-querying the database on every redirect).
class BnetRealmList
{
    public:
        static BnetRealmList& Instance();

        void Initialize(uint8 region, uint8 battlegroup, uint32 updateIntervalSeconds);

        uint8 GetRegion() const { return m_region; }
        uint8 GetBattlegroup() const { return m_battlegroup; }

        uint32 GetRealmAddress(uint32 realmId) const;
        uint32 GetRealmIdFromAddress(uint32 realmAddress) const { return realmAddress & 0xFFFF; }
        std::string GetSubRegionAddress() const;

        // Refreshed if due; a copy, so callers hold no lock.
        std::vector<BnetRealm> GetRealms();
        nonstd::optional<BnetRealm> FindByAddress(uint32 realmAddress);
        nonstd::optional<BnetRealm> FindById(uint32 realmId);

        // The JSON objects the client reads; `security` is the asking account's level on that realm.
        nlohmann::json BuildRealmEntry(BnetRealm const& realm, AccountTypes security) const;

        static uint8 PopulationState(BnetRealm const& realm);

    private:
        void UpdateIfNeeded();
        void Reload();

        std::mutex m_lock;
        std::vector<BnetRealm> m_realms;
        uint8 m_region = 1;
        uint8 m_battlegroup = 1;
        uint32 m_updateInterval = 0;
        time_t m_nextUpdate = 0;
};

#define sBnetRealmList BnetRealmList::Instance()

#endif
