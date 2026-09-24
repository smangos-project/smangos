#include "BotAccountLease.h"
#include <algorithm>

BotAccountLease::BotAccountLease(std::vector<RealBotAccountCredential> accounts)
    : m_accounts(std::move(accounts)), m_leased(m_accounts.size(), false)
{
}

std::optional<RealBotAccountCredential> BotAccountLease::Lease()
{
    std::lock_guard<std::mutex> lock(m_lock);
    for (size_t i = 0; i < m_accounts.size(); ++i)
    {
        if (!m_leased[i])
        {
            m_leased[i] = true;
            return m_accounts[i];
        }
    }
    return std::nullopt;
}

void BotAccountLease::Release(std::string const& accountName)
{
    std::lock_guard<std::mutex> lock(m_lock);
    for (size_t i = 0; i < m_accounts.size(); ++i)
        if (m_accounts[i].Name == accountName)
            m_leased[i] = false;
}

size_t BotAccountLease::Free() const
{
    std::lock_guard<std::mutex> lock(m_lock);
    return size_t(std::count(m_leased.begin(), m_leased.end(), false));
}

size_t BotAccountLease::Total() const
{
    std::lock_guard<std::mutex> lock(m_lock);
    return m_accounts.size();
}
