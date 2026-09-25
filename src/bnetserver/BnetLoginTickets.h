#ifndef BNETSERVER_BNETLOGINTICKETS_H
#define BNETSERVER_BNETLOGINTICKETS_H

#include "Common.h"
#include "nonstd/optional.hpp"

#include <ctime>
#include <mutex>
#include <string>
#include <unordered_map>

// Login tickets the REST portal hands out after a successful login and the RPC port accepts as
// web credentials. In memory, expiring on a TTL and never consumed: a ticket that has not expired
// logs in again, and the client refreshes it while it lives.
class BnetLoginTickets
{
    public:
        static BnetLoginTickets& Instance();

        void SetDuration(uint32 seconds) { m_duration = seconds; }
        uint32 GetDuration() const { return m_duration; }

        // Returns the account's live ticket if it has one, otherwise a new one.
        std::string Issue(uint32 bnetAccountId);

        // The account the ticket belongs to, if it exists and has not expired.
        nonstd::optional<uint32> Verify(std::string const& ticket);

        // Pushes a live ticket's expiry out by one duration and returns the new expiry.
        nonstd::optional<time_t> Refresh(std::string const& ticket);

        void CleanupExpired();

    private:
        struct Entry
        {
            uint32 bnetAccountId = 0;
            time_t expiry = 0;
        };

        std::mutex m_lock;
        std::unordered_map<std::string, Entry> m_tickets;
        std::unordered_map<uint32, std::string> m_ticketByAccount;
        uint32 m_duration = 3600;
};

#define sBnetLoginTickets BnetLoginTickets::Instance()

#endif
