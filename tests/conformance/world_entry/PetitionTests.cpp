// A GM buys a guild charter from a tabard designer, queries the vendor's price, renames the
// charter and finds a turn-in refused for lack of signatures. Signing and offering a charter are
// wire-tested only (PetitionMessagesTests.cpp): the bot framework doesn't parse an item's
// enchantment field, so it has no way to learn a live petition id the way a real client does.
#include "Packets/ItemMessages.h"
#include "Packets/PetitionMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

namespace
{
constexpr int32_t GUILD_CHARTER_ITEM = 5863;
}

TEST_CASE("petitions: a GM buys a guild charter, queries and renames it, and turn-in refuses without enough signatures", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));

        // Northshire Abbey (the GM's default spawn point) has its own wildlife, so a freshly
        // spawned vendor is not necessarily the only new unit nearby: try every candidate.
        std::vector<ClassicGuid> const before = KnownUnits(gm);
        SayCommand(gm, ".npc spawn add 4974"); // Aldwin Laughlin, a tabard designer
        REQUIRE(co_await TestWaitUntil([&]() { return FindNewUnit(gm, before).has_value(); }, 10000));
        co_await TestDelay(1000);

        std::optional<ClassicGuid> vendor;
        for (ClassicGuid const& guid : KnownUnits(gm))
        {
            if (std::find(before.begin(), before.end(), guid) != before.end())
                continue;
            PetitionShow show;
            show.PetitionerGuid = guid;
            gm.Send(show);
            auto matches = [&](PetitionShowList const& p) { return p.NpcGuid == guid; };
            if (co_await gm.WaitReceived<PetitionShowList>(3000, matches))
            {
                vendor = guid;
                break;
            }
        }
        REQUIRE(vendor);

        auto const showList = gm.FindReceived<PetitionShowList>([&](PetitionShowList const& p) { return p.NpcGuid == *vendor; });
        REQUIRE(showList);
        CHECK(showList->Price == 1000);

        PetitionBuy buy;
        buy.NpcGuid = *vendor;
        buy.Name = "Realbots Test Guild";
        gm.Send(buy);
        auto const push = co_await gm.Packet<ItemPushResult>(5000, [](ItemPushResult const& p) { return p.Item.ItemId == GUILD_CHARTER_ITEM; });
        if (!push)
            SKIP("the GM already holds a guild charter from a previous run of this scenario (GetPetitionByOwnerGuid never clears without a completed turn-in)");

        RenamePetition rename;
        rename.ItemGuid = push->ItemGuid;
        rename.NewName = "Renamed Test Guild";
        gm.Send(rename);
        auto const renamed = co_await gm.Packet<RenamePetitionResult>(5000);
        REQUIRE(renamed);
        CHECK(renamed->ItemGuid == push->ItemGuid);
        CHECK(renamed->NewName == rename.NewName);

        TurnInPetition turnIn;
        turnIn.ItemGuid = push->ItemGuid;
        gm.Send(turnIn);
        auto const result = co_await gm.Packet<TurnInPetitionResult>(5000);
        REQUIRE(result);
        CHECK(result->Result == TurnInPetitionResult::NeedMoreSignatures);

        SayCommand(gm, ".npc despawn");
    });
}
