// BotAccountLease is plain in-memory bookkeeping (no config, no networking), so it's tested
// directly rather than through RealBotMgr (bot-phase-1.md, "Accounts are existing Battle.net
// logins, leased one per bot").
#include "BotAccountLease.h"
#include <catch_amalgamated.hpp>

TEST_CASE("BotAccountLease hands out each account once", "[realbotmgr]")
{
    BotAccountLease lease({ { "bot1", "pass1" }, { "bot2", "pass2" } });
    CHECK(lease.Total() == 2);
    CHECK(lease.Free() == 2);

    std::optional<RealBotAccountCredential> const first = lease.Lease();
    REQUIRE(first);
    CHECK(lease.Free() == 1);

    std::optional<RealBotAccountCredential> const second = lease.Lease();
    REQUIRE(second);
    CHECK(first->Name != second->Name);
    CHECK(lease.Free() == 0);

    CHECK_FALSE(lease.Lease());
}

TEST_CASE("BotAccountLease::Release makes an account leasable again", "[realbotmgr]")
{
    BotAccountLease lease({ { "bot1", "pass1" } });

    std::optional<RealBotAccountCredential> const account = lease.Lease();
    REQUIRE(account);
    CHECK_FALSE(lease.Lease());

    lease.Release(account->Name);
    CHECK(lease.Free() == 1);
    CHECK(lease.Lease());
}

TEST_CASE("BotAccountLease with no accounts always returns nullopt", "[realbotmgr]")
{
    BotAccountLease lease;
    CHECK(lease.Total() == 0);
    CHECK_FALSE(lease.Lease());
}
