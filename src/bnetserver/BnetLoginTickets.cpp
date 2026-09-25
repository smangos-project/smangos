#include "BnetLoginTickets.h"
#include "Crypto/BigNumber.h"
#include "Util.h"

namespace
{
std::string NewTicket()
{
    BigNumber random;
    random.SetRand(20 * 8);
    return "BN-" + ByteArrayToHexStr(random.AsByteArray(20, false).data(), 20);
}
}

BnetLoginTickets& BnetLoginTickets::Instance()
{
    static BnetLoginTickets instance;
    return instance;
}

std::string BnetLoginTickets::Issue(uint32 bnetAccountId)
{
    time_t const now = time(nullptr);

    std::lock_guard<std::mutex> lock(m_lock);

    auto existing = m_ticketByAccount.find(bnetAccountId);
    if (existing != m_ticketByAccount.end())
    {
        auto entry = m_tickets.find(existing->second);
        if (entry != m_tickets.end() && entry->second.expiry > now)
        {
            entry->second.expiry = now + m_duration;
            return existing->second;
        }
        if (entry != m_tickets.end())
            m_tickets.erase(entry);
    }

    std::string ticket = NewTicket();
    m_tickets[ticket] = Entry{ bnetAccountId, now + m_duration };
    m_ticketByAccount[bnetAccountId] = ticket;
    return ticket;
}

nonstd::optional<uint32> BnetLoginTickets::Verify(std::string const& ticket)
{
    std::lock_guard<std::mutex> lock(m_lock);

    auto entry = m_tickets.find(ticket);
    if (entry == m_tickets.end() || entry->second.expiry <= time(nullptr))
        return nonstd::nullopt;
    return entry->second.bnetAccountId;
}

nonstd::optional<time_t> BnetLoginTickets::Refresh(std::string const& ticket)
{
    time_t const now = time(nullptr);

    std::lock_guard<std::mutex> lock(m_lock);

    auto entry = m_tickets.find(ticket);
    if (entry == m_tickets.end() || entry->second.expiry <= now)
        return nonstd::nullopt;

    entry->second.expiry = now + m_duration;
    return entry->second.expiry;
}

void BnetLoginTickets::CleanupExpired()
{
    time_t const now = time(nullptr);

    std::lock_guard<std::mutex> lock(m_lock);

    for (auto entry = m_tickets.begin(); entry != m_tickets.end();)
    {
        if (entry->second.expiry <= now)
        {
            auto byAccount = m_ticketByAccount.find(entry->second.bnetAccountId);
            if (byAccount != m_ticketByAccount.end() && byAccount->second == entry->first)
                m_ticketByAccount.erase(byAccount);
            entry = m_tickets.erase(entry);
        }
        else
            ++entry;
    }
}
