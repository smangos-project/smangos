#include "Database/StatementStats.h"

#include <catch_amalgamated.hpp>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

TEST_CASE("StatementStats::ParseKey extracts verb and table", "[shared][statementstats]")
{
    CHECK(StatementStats::ParseKey("INSERT INTO `characters` (`guid`) VALUES (1)") == "INSERT characters");
    CHECK(StatementStats::ParseKey("insert ignore into character_aura VALUES (?)") == "INSERT character_aura");
    CHECK(StatementStats::ParseKey("REPLACE INTO `item_instance`(guid) VALUES (?)") == "REPLACE item_instance");
    CHECK(StatementStats::ParseKey("  UPDATE `characters` SET `online` = 0") == "UPDATE characters");
    CHECK(StatementStats::ParseKey("DELETE FROM `character_spell` WHERE `guid` = ?") == "DELETE character_spell");
    CHECK(StatementStats::ParseKey("DELETE LOW_PRIORITY QUICK FROM mail WHERE id = 1") == "DELETE mail");
    CHECK(StatementStats::ParseKey("TRUNCATE `x`") == "TRUNCATE");
}

TEST_CASE("StatementStats writes counters and timings to its CSV", "[shared][statementstats]")
{
    std::string const path = (std::filesystem::temp_directory_path() / "statementstats-test.csv").string();
    std::remove(path.c_str());
    {
        StatementStats stats(path, 600000);
        stats.RegisterPrepared(3, "DELETE FROM `character_aura` WHERE `guid` = ?");
        stats.RecordPlain("UPDATE `characters` SET `money` = 5 WHERE `guid` = 2");
        StatementStats::SetInTransaction(true);
        stats.RecordPrepared(3, 4);
        stats.RecordPrepared(3, 4);
        StatementStats::SetInTransaction(false);
        stats.RecordTransaction(2, 150);
        stats.RecordTiming("Player::SaveToDB", 40);
    }
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    std::string const csv = ss.str();
    CHECK(csv.find("unix_ms,kind,key,count,count_in_tx,bytes,p50,p99,max,total\n") == 0);
    CHECK(csv.find(",stmt,DELETE character_aura,2,2,8,,,,") != std::string::npos);
    CHECK(csv.find(",stmt,UPDATE characters,1,0,") != std::string::npos);
    CHECK(csv.find(",timing,transaction,1,2,,150,150,150,150") != std::string::npos);
    CHECK(csv.find(",timing,Player::SaveToDB,1,0,,40,40,40,40") != std::string::npos);
    std::remove(path.c_str());
}
