// SMSG_CHAT and the CMSG_CHAT_MESSAGE_* opcodes that replaced SMSG_MESSAGECHAT/CMSG_MESSAGECHAT
// (bot-phase-4.md, slice B).
#include "Packets/ChatMessages.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>
#include <optional>

namespace
{
// TrinityCore's ChatMsg wire values (see ToModernChatMsgType, Chat/Chat.cpp), not vmangos's own.
constexpr uint8_t kChatMsgSystem = 0x00;
constexpr uint8_t kChatMsgWhisper = 0x07;
}

TEST_CASE("world_entry: the login burst's MOTD arrives as a system message", "[protocol]")
{
    // WaitReceived + LastReceived, not Packet<T>: the MOTD is part of the login burst, sent before
    // the bot reaches InWorld, so a fresh Packet<T> watch set up after that stage would miss it.
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await bot.WaitReceived(Chat::Opcode, 10000));

        std::optional<BotPacket> raw = bot.LastReceived(Chat::Opcode);
        REQUIRE(raw);
        CHECK(raw->Connection == BotConnectionKind::Realm);

        raw->Data.rpos(0);
        Chat motd;
        REQUIRE(motd.Read(raw->Data));
        CHECK(raw->Data.rpos() == raw->Data.size());
        CHECK(motd.MsgType == kChatMsgSystem);
        CHECK(motd.SenderGuid.IsEmpty());
        CHECK(motd.SenderName.empty());
        CHECK_FALSE(motd.Message.empty());
    });
}

TEST_CASE("chat: a whisper between two bots arrives with the sender's name and the text", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot botA;
        TestBot botB;
        REQUIRE(co_await botA.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await botB.Stage(BotStage::InWorld, 30000));

        ChatMessageWhisper whisper;
        whisper.Language = 7; // LANG_COMMON: LANG_UNIVERSAL (0) is only valid for AFK/DND
        whisper.TargetName = botB.GetCharacterName();
        whisper.Message = "hey, got a minute?";
        botA.Send(whisper);

        auto received = co_await botB.Packet<Chat>(10000, [](Chat const& c) { return c.MsgType == kChatMsgWhisper; });
        REQUIRE(received);
        CHECK(received->SenderName == botA.GetCharacterName());
        CHECK(received->Message == "hey, got a minute?");
        CHECK(received->SenderGuid.Low == botA.GetCharacterGuid().Low);
        CHECK(received->SenderGuid.High == botA.GetCharacterGuid().High);
    });
}
