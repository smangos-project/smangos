// Round trips for the modern query wire.
#include "Packets/QueryMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("QueryPlayerNames round-trips and caps the count", "[packets]")
{
    QueryPlayerNames query;
    query.Players = { { 11, 0x0800000000000000ULL }, { 22, 0 } };
    RoundTrip(query, [](QueryPlayerNames const& a, QueryPlayerNames const& b)
    {
        CHECK(a.Players == b.Players);
    });
    RejectsEveryTruncation(query);

    ByteBuffer tooMany;
    tooMany << uint32(CLASSIC_MAX_NAME_QUERIES + 1);
    QueryPlayerNames in;
    CHECK_FALSE(in.Read(tooMany));
}

TEST_CASE("QueryPlayerNamesResponse round-trips known and unknown players", "[packets]")
{
    QueryPlayerNamesResponse response;

    PlayerNameLookup& known = response.Players.emplace_back();
    known.Player = { 5, 0x0800000000000000ULL };
    PlayerNameData& data = known.Data.emplace();
    data.Name = "Botone";
    data.AccountId = { 9, 0x7400000000000000ULL };
    data.BnetAccountId = { 9, 0x7800000000000000ULL };
    data.PlayerGuid = known.Player;
    data.Race = 1;
    data.Sex = 1;
    data.ClassId = 3;
    data.Level = 60;
    data.IsDeleted = true;
    data.DeclinedNames[2] = "Botthree";

    PlayerNameLookup& unknown = response.Players.emplace_back();
    unknown.Result = 1;
    unknown.Player = { 6, 0x0800000000000000ULL };

    RoundTrip(response, [](QueryPlayerNamesResponse const& a, QueryPlayerNamesResponse const& b)
    {
        REQUIRE(a.Players.size() == b.Players.size());
        for (size_t i = 0; i < a.Players.size(); ++i)
        {
            CHECK(a.Players[i].Result == b.Players[i].Result);
            CHECK(a.Players[i].Player == b.Players[i].Player);
            REQUIRE(a.Players[i].Data.has_value() == b.Players[i].Data.has_value());
            if (!a.Players[i].Data)
                continue;
            PlayerNameData const& x = *a.Players[i].Data;
            PlayerNameData const& y = *b.Players[i].Data;
            CHECK(x.Name == y.Name);
            CHECK(x.DeclinedNames == y.DeclinedNames);
            CHECK(x.IsDeleted == y.IsDeleted);
            CHECK(x.AccountId == y.AccountId);
            CHECK(x.BnetAccountId == y.BnetAccountId);
            CHECK(x.PlayerGuid == y.PlayerGuid);
            CHECK(x.Race == y.Race);
            CHECK(x.Sex == y.Sex);
            CHECK(x.ClassId == y.ClassId);
            CHECK(x.Level == y.Level);
        }
    });
    RejectsEveryTruncation(response);
}

TEST_CASE("QueryCreature round-trips", "[packets]")
{
    QueryCreature query;
    query.Entry = 6;
    RoundTrip(query, [](QueryCreature const& a, QueryCreature const& b)
    {
        CHECK(a.Entry == b.Entry);
    });
    RejectsEveryTruncation(query);
}

TEST_CASE("QueryCreatureResponse round-trips an allowed and a missing entry", "[packets]")
{
    QueryCreatureResponse found;
    found.Entry = 6;
    found.Allow = true;
    found.Name = "Kobold Vermin";
    found.Title = "Stable Master";
    found.Civilian = true;
    found.TypeFlags = 0x1000;
    found.CreatureType = 7;
    found.Family = 2;
    found.Classification = 1;
    found.PetSpellDataId = 4;
    found.TotalProbability = 3.0f;
    found.Displays = { { 100, 1.5f, 1.0f }, { 101, 0.5f, 2.0f } };
    found.HealthMultiplier = 2.0f;
    found.EnergyMultiplier = 0.5f;
    found.UnitClass = 4;
    RoundTrip(found, [](QueryCreatureResponse const& a, QueryCreatureResponse const& b)
    {
        CHECK(a.Entry == b.Entry);
        CHECK(a.Allow == b.Allow);
        CHECK(a.Name == b.Name);
        CHECK(a.Title == b.Title);
        CHECK(a.Civilian == b.Civilian);
        CHECK(a.Leader == b.Leader);
        CHECK(a.TypeFlags == b.TypeFlags);
        CHECK(a.CreatureType == b.CreatureType);
        CHECK(a.Family == b.Family);
        CHECK(a.Classification == b.Classification);
        CHECK(a.PetSpellDataId == b.PetSpellDataId);
        CHECK(a.TotalProbability == b.TotalProbability);
        REQUIRE(a.Displays.size() == b.Displays.size());
        for (size_t i = 0; i < a.Displays.size(); ++i)
        {
            CHECK(a.Displays[i].DisplayId == b.Displays[i].DisplayId);
            CHECK(a.Displays[i].Scale == b.Displays[i].Scale);
            CHECK(a.Displays[i].Probability == b.Displays[i].Probability);
        }
        CHECK(a.HealthMultiplier == b.HealthMultiplier);
        CHECK(a.EnergyMultiplier == b.EnergyMultiplier);
        CHECK(a.UnitClass == b.UnitClass);
    });
    RejectsEveryTruncation(found);

    QueryCreatureResponse untitled = found;
    untitled.Title.clear();
    RoundTrip(untitled, [](QueryCreatureResponse const& a, QueryCreatureResponse const& b)
    {
        CHECK(b.Title.empty());
        CHECK(a.Name == b.Name);
    });
    RejectsEveryTruncation(untitled);

    QueryCreatureResponse missing;
    missing.Entry = 999;
    RoundTrip(missing, [](QueryCreatureResponse const& a, QueryCreatureResponse const& b)
    {
        CHECK(b.Entry == a.Entry);
        CHECK_FALSE(b.Allow);
    });
    RejectsEveryTruncation(missing);
}

TEST_CASE("QueryGameObject round-trips", "[packets]")
{
    QueryGameObject query;
    query.Entry = 1731;
    query.Guid = { 3, 0x5800000000000000ULL };
    RoundTrip(query, [](QueryGameObject const& a, QueryGameObject const& b)
    {
        CHECK(a.Entry == b.Entry);
        CHECK(a.Guid == b.Guid);
    });
    RejectsEveryTruncation(query);
}

TEST_CASE("QueryGameObjectResponse round-trips an allowed and a missing entry", "[packets]")
{
    QueryGameObjectResponse found;
    found.Entry = 1731;
    found.Guid = { 3, 0x5800000000000000ULL };
    found.Allow = true;
    found.Type = 3;
    found.DisplayId = 311;
    found.Name = "Copper Vein";
    found.IconName = "Mine";
    found.Data[0] = 43;
    found.Data[23] = -7;
    found.Size = 1.25f;
    RoundTrip(found, [](QueryGameObjectResponse const& a, QueryGameObjectResponse const& b)
    {
        CHECK(a.Entry == b.Entry);
        CHECK(a.Guid == b.Guid);
        CHECK(b.Allow);
        CHECK(a.Type == b.Type);
        CHECK(a.DisplayId == b.DisplayId);
        CHECK(a.Name == b.Name);
        CHECK(a.IconName == b.IconName);
        CHECK(a.Data == b.Data);
        CHECK(a.Size == b.Size);
    });
    RejectsEveryTruncation(found);

    QueryGameObjectResponse missing;
    missing.Entry = 5;
    RoundTrip(missing, [](QueryGameObjectResponse const& a, QueryGameObjectResponse const& b)
    {
        CHECK(a.Entry == b.Entry);
        CHECK_FALSE(b.Allow);
    });
    RejectsEveryTruncation(missing);
}

TEST_CASE("QueryNpcText round-trips", "[packets]")
{
    QueryNpcText query;
    query.TextId = 68;
    query.Guid = { 12, 0x2000000000000000ULL };
    RoundTrip(query, [](QueryNpcText const& a, QueryNpcText const& b)
    {
        CHECK(a.TextId == b.TextId);
        CHECK(a.Guid == b.Guid);
    });
    RejectsEveryTruncation(query);
}

TEST_CASE("QueryNpcTextResponse round-trips an allowed and a missing text", "[packets]")
{
    QueryNpcTextResponse found;
    found.TextId = 68;
    found.Allow = true;
    found.Probabilities[0] = 1.0f;
    found.Probabilities[3] = 0.25f;
    found.BroadcastTextIds[0] = 4000;
    found.BroadcastTextIds[3] = 4003;
    RoundTrip(found, [](QueryNpcTextResponse const& a, QueryNpcTextResponse const& b)
    {
        CHECK(a.TextId == b.TextId);
        CHECK(b.Allow);
        CHECK(a.Probabilities == b.Probabilities);
        CHECK(a.BroadcastTextIds == b.BroadcastTextIds);
    });
    RejectsEveryTruncation(found);

    QueryNpcTextResponse missing;
    missing.TextId = 9;
    RoundTrip(missing, [](QueryNpcTextResponse const& a, QueryNpcTextResponse const& b)
    {
        CHECK(a.TextId == b.TextId);
        CHECK_FALSE(b.Allow);
    });
    RejectsEveryTruncation(missing);
}

TEST_CASE("QueryPageText round-trips", "[packets]")
{
    QueryPageText query;
    query.PageTextId = 2;
    query.ItemGuid = { 8, 0x0C00000000000000ULL };
    RoundTrip(query, [](QueryPageText const& a, QueryPageText const& b)
    {
        CHECK(a.PageTextId == b.PageTextId);
        CHECK(a.ItemGuid == b.ItemGuid);
    });
    RejectsEveryTruncation(query);
}

TEST_CASE("QueryPageTextResponse round-trips a chain of pages", "[packets]")
{
    QueryPageTextResponse response;
    response.PageTextId = 2;
    response.Allow = true;
    response.Pages = { { 2, 3, "First page." }, { 3, 0, std::string(300, 'x') } };
    RoundTrip(response, [](QueryPageTextResponse const& a, QueryPageTextResponse const& b)
    {
        CHECK(a.PageTextId == b.PageTextId);
        REQUIRE(a.Pages.size() == b.Pages.size());
        for (size_t i = 0; i < a.Pages.size(); ++i)
        {
            CHECK(a.Pages[i].Id == b.Pages[i].Id);
            CHECK(a.Pages[i].NextPageId == b.Pages[i].NextPageId);
            CHECK(a.Pages[i].Text == b.Pages[i].Text);
        }
    });
    RejectsEveryTruncation(response);
}

TEST_CASE("QueryTime and its response round-trip", "[packets]")
{
    RoundTrip(QueryTime(), [](QueryTime const&, QueryTime const&) {});

    QueryTimeResponse response;
    response.CurrentTime = 1'700'000'000;
    RoundTrip(response, [](QueryTimeResponse const& a, QueryTimeResponse const& b)
    {
        CHECK(a.CurrentTime == b.CurrentTime);
    });
    RejectsEveryTruncation(response);
}

TEST_CASE("QueryCorpseLocationFromClient round-trips", "[packets]")
{
    QueryCorpseLocationFromClient packet;
    packet.Player = { 0x1234, 0 };
    RoundTrip(packet, [](QueryCorpseLocationFromClient const& a, QueryCorpseLocationFromClient const& b) { CHECK(a.Player == b.Player); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("CorpseLocation round-trips valid and not found", "[packets]")
{
    for (bool valid : { false, true })
    {
        CorpseLocation packet;
        packet.Valid = valid;
        packet.Player = { 0x1234, 0 };
        packet.ActualMapId = 1;
        packet.MapId = 0;
        packet.PositionX = 100.0f;
        packet.PositionY = -200.0f;
        packet.PositionZ = 50.0f;
        RoundTrip(packet, [](CorpseLocation const& a, CorpseLocation const& b)
        {
            CHECK(a.Valid == b.Valid);
            CHECK(a.Player == b.Player);
            CHECK(a.ActualMapId == b.ActualMapId);
            CHECK(a.MapId == b.MapId);
            CHECK(a.Transport == b.Transport);
            CHECK(a.PositionX == b.PositionX);
            CHECK(a.PositionY == b.PositionY);
            CHECK(a.PositionZ == b.PositionZ);
        });
        RejectsEveryTruncation(packet);
    }
}
