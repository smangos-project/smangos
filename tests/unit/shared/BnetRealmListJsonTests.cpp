#include "Bnet/BnetRealmListJson.h"

#include <catch_amalgamated.hpp>

TEST_CASE("BnetRealmListJson round-trips a plain named JSON attribute", "[bnet]")
{
    nlohmann::json identity;
    identity["gameAccountID"] = 7;
    identity["gameAccountRegion"] = 1;

    std::string const text = BnetRealmListJson::EncodePlain("RealmListTicketIdentity", identity);
    CHECK(text.rfind("RealmListTicketIdentity:", 0) == 0);

    nonstd::optional<nlohmann::json> decoded = BnetRealmListJson::DecodePlain(text);
    REQUIRE(decoded.has_value());
    CHECK(*decoded == identity);
}

TEST_CASE("BnetRealmListJson round-trips a zlib-compressed named JSON attribute", "[bnet]")
{
    nlohmann::json realmList;
    realmList["updates"] = nlohmann::json::array();
    realmList["updates"].push_back({ { "update", { { "name", "VibeMangos" }, { "wowRealmAddress", 1u } } } });

    nonstd::optional<std::string> blob = BnetRealmListJson::EncodeCompressed("RealmListUpdates", realmList);
    REQUIRE(blob.has_value());

    nonstd::optional<nlohmann::json> decoded = BnetRealmListJson::DecodeCompressed(*blob);
    REQUIRE(decoded.has_value());
    CHECK(*decoded == realmList);
}

TEST_CASE("BnetRealmListJson::DecodeCompressed rejects a too-short or garbage blob", "[bnet]")
{
    CHECK_FALSE(BnetRealmListJson::DecodeCompressed("").has_value());
    CHECK_FALSE(BnetRealmListJson::DecodeCompressed(std::string(3, '\0')).has_value());

    std::string garbage(8, '\0');
    garbage[4] = char(0xFF);
    garbage[5] = char(0xFF);
    garbage[6] = char(0xFF);
    garbage[7] = char(0xFF);
    CHECK_FALSE(BnetRealmListJson::DecodeCompressed(garbage).has_value());
}

TEST_CASE("BnetRealmListJson::DecodePlain rejects a line with no colon", "[bnet]")
{
    CHECK_FALSE(BnetRealmListJson::DecodePlain("NoColonHere").has_value());
}
