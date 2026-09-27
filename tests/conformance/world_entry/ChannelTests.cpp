// Two bots join a custom channel, chat in it and leave, between two plain bots (no GM needed).
#include "Packets/ChannelMessages.h"
#include "Packets/ChatMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

TEST_CASE("channel: two bots join a custom channel, chat and leave", "[gameplay]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot first;
        TestBot second;
        REQUIRE(co_await first.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await second.Stage(BotStage::InWorld, 30000));
        // A run that was cut short leaves a bot on the channel from before.
        LeaveChannel leaveStale;
        leaveStale.ChannelName = "Realbotschanneltest";
        first.Send(leaveStale);
        second.Send(leaveStale);
        co_await TestDelay(500);

        ClassicGuid const firstGuid = first.GetCharacterGuid();
        ClassicGuid const secondGuid = second.GetCharacterGuid();

        JoinChannel join;
        join.ChannelName = "Realbotschanneltest";
        first.Send(join);
        auto const firstJoined = co_await first.Packet<ChannelNotifyJoined>(5000);
        REQUIRE(firstJoined);
        CHECK(firstJoined->ChannelName == "Realbotschanneltest");
        CHECK(!firstJoined->ChannelGuid.IsEmpty());

        second.Send(join);
        auto const secondJoined = co_await second.Packet<ChannelNotifyJoined>(5000);
        REQUIRE(secondJoined);
        // A custom channel's guid depends only on the joining player's own map/zone, not on the
        // channel's (always-zero) dbc id, so both bots -- standing in the same starting zone --
        // key their chat lines to the same guid.
        CHECK(secondJoined->ChannelGuid == firstJoined->ChannelGuid);

        // The first bot sees the second arrive: the generic join notice plus a roster update.
        REQUIRE(co_await first.WaitReceived<ChannelNotify>(5000, [&](ChannelNotify const& p)
        {
            return p.Type == 0x00 /* CHAT_JOINED_NOTICE */ && p.SenderGuid == secondGuid;
        }));
        REQUIRE(co_await first.WaitReceived<UserlistUpdate>(5000, [&](UserlistUpdate const& p) { return p.UpdatedUserGuid == secondGuid; }));

        std::string const message = "hello from realbots";
        ChatMessageChannel chat;
        chat.ChannelName = "Realbotschanneltest";
        chat.Message = message;
        chat.Language = 7; // LANG_COMMON
        first.Send(chat);

        auto matchesLine = [&](Chat const& p) { return p.SenderGuid == firstGuid && p.Message == message; };
        auto const heardBySecond = co_await second.Packet<Chat>(5000, matchesLine);
        REQUIRE(heardBySecond);
        CHECK(heardBySecond->ChannelName == "Realbotschanneltest");
        CHECK(heardBySecond->ChannelGuid == firstJoined->ChannelGuid);

        LeaveChannel leave;
        leave.ChannelName = "Realbotschanneltest";
        second.Send(leave);
        auto const secondLeft = co_await second.Packet<ChannelNotifyLeft>(5000);
        REQUIRE(secondLeft);
        CHECK(secondLeft->ChannelName == "Realbotschanneltest");

        REQUIRE(co_await first.WaitReceived<UserlistRemove>(5000, [&](UserlistRemove const& p) { return p.RemovedUserGuid == secondGuid; }));

        first.Send(leave);
        REQUIRE(co_await first.Packet<ChannelNotifyLeft>(5000));
    });
}
