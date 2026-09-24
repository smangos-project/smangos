// RealBotMgr's leasing, sharing, reaping and relog (bot-phase-2.md, "Reconnect", "Sharing an account
// on purpose"), against targets that can never log in: a refused port fails a bot within
// milliseconds, a TEST-NET-3 address (RFC 5737, never routable) keeps it in LoginPortal for as long
// as ConnectTimeoutMs says, so both "already failed" and "still trying" bots are on tap without a
// server.
#include "Config/Config.h"
#include "RealBotCommands.h"
#include "RealBotMgr.h"
#include <catch_amalgamated.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

namespace
{
// The targets every test here uses. "refused" fails at once, "slow" takes ConnectTimeoutMs (1.5s).
constexpr char const* MgrConfig =
    "RealBots.Targets = \"refused slow\"\n"
    "RealBots.WorkerThreads = 1\n"
    "RealBots.TickIntervalMs = 20\n"
    "RealBots.ReapAfterMs = 300\n"
    "RealBots.Target.refused.LoginAddress = \"127.0.0.1\"\n"
    "RealBots.Target.refused.RestPort = 1\n"
    "RealBots.Target.refused.RpcPort = 1\n"
    "RealBots.Target.refused.Accounts = \"a@x:pa b@x:pb\"\n"
    "RealBots.Target.refused.ConnectTimeoutMs = 500\n"
    "RealBots.Target.slow.LoginAddress = \"203.0.113.1\"\n"
    "RealBots.Target.slow.RestPort = 8081\n"
    "RealBots.Target.slow.RpcPort = 1119\n"
    "RealBots.Target.slow.Accounts = \"c@x:pc d@x:pd\"\n"
    "RealBots.Target.slow.ConnectTimeoutMs = 1500\n"
    "RealBots.Target.slow.StageTimeoutMs = 30000\n";

// Loads MgrConfig and starts the manager; stops it (removing every bot, releasing every lease)
// on the way out.
struct MgrFixture
{
    std::filesystem::path Path = std::filesystem::temp_directory_path() / "realbots_mgr_test.conf";

    MgrFixture()
    {
        {
            std::ofstream out(Path, std::ios::trunc);
            out << MgrConfig;
        }
        REQUIRE(sConfig.LoadFromFile(Path.string()));
        sRealBotMgr.Start();
    }

    ~MgrFixture()
    {
        sRealBotMgr.Stop();
        std::filesystem::remove(Path);
    }

    static BotStage StageOf(std::string const& name)
    {
        std::optional<RealBotInfo> info = sRealBotMgr.GetInfo(name);
        return info ? info->Stage : BotStage::Disconnected;
    }

    static bool WaitForStage(std::string const& name, BotStage stage, std::chrono::milliseconds timeout = std::chrono::milliseconds(5000))
    {
        auto const deadline = std::chrono::steady_clock::now() + timeout;
        while (StageOf(name) != stage)
        {
            if (std::chrono::steady_clock::now() >= deadline)
                return false;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        return true;
    }

    static size_t FreeAccounts(std::string const& target)
    {
        return sRealBotMgr.GetLeaseCounts(target)->first;
    }
};
}

TEST_CASE("RealBotMgr: a bot sharing an account takes no second lease, and the account frees with the last user", "[realbotmgr]")
{
    MgrFixture fixture;
    REQUIRE(MgrFixture::FreeAccounts("slow") == 2);

    std::string error;
    std::string const first = sRealBotMgr.AddWithOverrides("slow", RealBotAddOverrides{}, "", &error);
    REQUIRE_FALSE(first.empty());
    CHECK(MgrFixture::FreeAccounts("slow") == 1);

    RealBotAddOverrides share;
    share.ShareAccountWith = first;
    std::string const second = sRealBotMgr.AddWithOverrides("slow", share, "", &error);
    INFO("error: " << error);
    REQUIRE_FALSE(second.empty());
    CHECK(second != first);
    CHECK(MgrFixture::FreeAccounts("slow") == 1); // still only the one lease

    // Both are logging in as the same account (the sharer got the first bot's credential).
    std::shared_ptr<RealBot> a = sRealBotMgr.GetBot(first);
    std::shared_ptr<RealBot> b = sRealBotMgr.GetBot(second);
    REQUIRE(a);
    REQUIRE(b);
    CHECK(a->GetLoginOptions().AccountName == b->GetLoginOptions().AccountName);
    CHECK(a->GetLoginOptions().Password == b->GetLoginOptions().Password);

    sRealBotMgr.Remove(first);
    CHECK(MgrFixture::FreeAccounts("slow") == 1); // the sharer is still on it
    sRealBotMgr.Remove(second);
    CHECK(MgrFixture::FreeAccounts("slow") == 2);
}

TEST_CASE("RealBotMgr: sharing an account with a bot that does not exist, or on another target, fails", "[realbotmgr]")
{
    MgrFixture fixture;

    RealBotAddOverrides share;
    share.ShareAccountWith = "nobody";
    std::string error;
    CHECK(sRealBotMgr.AddWithOverrides("slow", share, "", &error).empty());
    CHECK(error.find("nobody") != std::string::npos);
    CHECK(MgrFixture::FreeAccounts("slow") == 2); // nothing was leased by the failed attempt

    std::string const other = sRealBotMgr.AddWithOverrides("refused", RealBotAddOverrides{}, "", &error);
    REQUIRE_FALSE(other.empty());
    share.ShareAccountWith = other;
    CHECK(sRealBotMgr.AddWithOverrides("slow", share, "", &error).empty()); // `other` is on a different target
    CHECK(MgrFixture::FreeAccounts("slow") == 2);
}

TEST_CASE("RealBotMgr: Update reaps a terminal bot unless its owner asked to keep it", "[realbotmgr]")
{
    MgrFixture fixture;

    RealBotAddOverrides keep;
    keep.Keep = true;
    std::string const kept = sRealBotMgr.AddWithOverrides("refused", keep, "", nullptr);
    std::string const reaped = sRealBotMgr.AddWithOverrides("refused", RealBotAddOverrides{}, "", nullptr);
    REQUIRE_FALSE(kept.empty());
    REQUIRE_FALSE(reaped.empty());

    REQUIRE(MgrFixture::WaitForStage(kept, BotStage::Failed));
    REQUIRE(MgrFixture::WaitForStage(reaped, BotStage::Failed));

    // ReapAfterMs is 300: three 200ms updates put both well past it.
    for (int i = 0; i < 3; ++i)
        sRealBotMgr.Update(200);

    CHECK(sRealBotMgr.GetInfo(kept).has_value());
    CHECK_FALSE(sRealBotMgr.GetInfo(reaped).has_value());
    CHECK(MgrFixture::FreeAccounts("refused") == 1); // the reaped bot's account came back
}

TEST_CASE("RealBotMgr: Reconnect logs a failed bot in again and clears what the last attempt left", "[realbotmgr]")
{
    MgrFixture fixture;

    RealBotAddOverrides keep;
    keep.Keep = true;
    std::string const name = sRealBotMgr.AddWithOverrides("slow", keep, "", nullptr);
    REQUIRE_FALSE(name.empty());

    // The connect to a TEST-NET address takes 1.5s to time out: Failed at LoginPortal.
    REQUIRE(MgrFixture::WaitForStage(name, BotStage::Failed, std::chrono::milliseconds(8000)));
    std::optional<RealBotInfo> failed = sRealBotMgr.GetInfo(name);
    REQUIRE(failed);
    CHECK(failed->FailedStage == BotStage::LoginPortal);
    CHECK_FALSE(failed->LastError.empty());

    REQUIRE(sRealBotMgr.Reconnect(name));

    // Back to trying: LoginPortal again, with the old failure gone.
    REQUIRE(MgrFixture::WaitForStage(name, BotStage::LoginPortal, std::chrono::milliseconds(1000)));
    std::optional<RealBotInfo> again = sRealBotMgr.GetInfo(name);
    REQUIRE(again);
    CHECK(again->LastError.empty());
    CHECK(again->FailedStage == BotStage::Disconnected);
    CHECK(again->TaskStack.size() == 1);

    // ... and it fails the same way in time, proving it really ran a whole new login.
    REQUIRE(MgrFixture::WaitForStage(name, BotStage::Failed, std::chrono::milliseconds(8000)));

    CHECK_FALSE(sRealBotMgr.Reconnect("nobody"));
}

TEST_CASE("RealBotCommands: relog and add ... keep=1", "[realbotmgr]")
{
    MgrFixture fixture;

    std::vector<std::string> output;
    RealBotCommands::Sink sink = [&](std::string const& line) { output.push_back(line); };

    RealBotCommands::Handle("relog nobody", sink);
    REQUIRE(output.size() == 1);
    CHECK(output[0] == "no such bot 'nobody'");

    output.clear();
    RealBotCommands::Handle("add refused 1 keep=1", sink);
    REQUIRE(output.size() == 1);
    std::vector<std::string> const names = sRealBotMgr.List();
    REQUIRE(names.size() == 1);
    REQUIRE(MgrFixture::WaitForStage(names[0], BotStage::Failed));

    for (int i = 0; i < 3; ++i)
        sRealBotMgr.Update(200);
    CHECK(sRealBotMgr.GetInfo(names[0]).has_value()); // kept

    output.clear();
    RealBotCommands::Handle("relog " + names[0], sink);
    REQUIRE(output.size() == 1);
    CHECK(output[0] == "bot '" + names[0] + "' is logging in again");
}
