// RealBotTarget::Load reads sConfig directly (bot-phase-1.md, "Targets"), so these write a small
// RealBots.Target.* block to a real file and load it exactly like realbotrunner's -c does -- there
// is no other way to exercise it without a live config.
#include "RealBotTarget.h"
#include "Config/Config.h"
#include <catch_amalgamated.hpp>
#include <filesystem>
#include <fstream>

namespace
{
struct ScopedTargetConfig
{
    std::filesystem::path path = std::filesystem::temp_directory_path() / "realbots_target_test.conf";

    explicit ScopedTargetConfig(std::string const& contents)
    {
        std::ofstream out(path, std::ios::trunc);
        out << contents;
        out.close();
        REQUIRE(sConfig.LoadFromFile(path.string()));
    }

    ~ScopedTargetConfig()
    {
        std::filesystem::remove(path);
    }
};
}

TEST_CASE("RealBotTarget::Load reads every key", "[realbotmgr]")
{
    ScopedTargetConfig const config(
        "RealBots.Target.vibemangos.LoginAddress = \"127.0.0.1\"\n"
        "RealBots.Target.vibemangos.RestPort = 8081\n"
        "RealBots.Target.vibemangos.RpcPort = 1119\n"
        "RealBots.Target.vibemangos.RealmName = \"VibeMangos\"\n"
        "RealBots.Target.vibemangos.ClientBuild = 58797\n"
        "RealBots.Target.vibemangos.Tags = \"protocol gameplay\"\n"
        "RealBots.Target.vibemangos.Accounts = \"bot1@bots:pass1 bot2@bots:pass2\"\n"
        "RealBots.Target.vibemangos.VerifyServerSignature = 0\n"
    );

    RealBotTarget target;
    std::string error;
    REQUIRE(RealBotTarget::Load("vibemangos", target, error));

    CHECK(target.Name == "vibemangos");
    CHECK(target.LoginAddress == "127.0.0.1");
    CHECK(target.RestPort == 8081);
    CHECK(target.RpcPort == 1119);
    CHECK(target.RealmName == "VibeMangos");
    CHECK(target.ClientBuild == 58797);
    CHECK(target.HasTag("protocol"));
    CHECK(target.HasTag("gameplay"));
    CHECK_FALSE(target.HasTag("unknown"));
    CHECK_FALSE(target.VerifyServerSignature);

    REQUIRE(target.Accounts.size() == 2);
    CHECK(target.Accounts[0].Name == "bot1@bots");
    CHECK(target.Accounts[0].Password == "pass1");
    CHECK(target.Accounts[1].Name == "bot2@bots");
    CHECK(target.Accounts[1].Password == "pass2");
}

TEST_CASE("RealBotTarget::Load fails without LoginAddress", "[realbotmgr]")
{
    ScopedTargetConfig const config("RealBots.Target.broken.RestPort = 8081\n");

    RealBotTarget target;
    std::string error;
    CHECK_FALSE(RealBotTarget::Load("broken", target, error));
    CHECK_FALSE(error.empty());
}

TEST_CASE("RealBotTargetList loads every name in RealBots.Targets and skips broken ones", "[realbotmgr]")
{
    ScopedTargetConfig const config(
        "RealBots.Targets = \"vibemangos missing\"\n"
        "RealBots.Target.vibemangos.LoginAddress = \"127.0.0.1\"\n"
    );

    RealBotTargetList targets;
    targets.Load();

    CHECK(targets.All().size() == 1);
    CHECK(targets.Find("vibemangos") != nullptr);
    CHECK(targets.Find("missing") == nullptr);
}
