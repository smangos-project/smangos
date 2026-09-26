// What a bot answers when the server changes how it moves.
#include "Packets/MovementMessages.h"
#include "TestFakeBot.h"
#include <catch_amalgamated.hpp>

namespace
{
ClassicGuid TestMover()
{
    ClassicGuid guid;
    guid.Low = 0x42;
    guid.High = 0x0400000000000000ULL;
    return guid;
}

template<typename T>
std::optional<T> LastSent(TestFakeBot const& bot)
{
    for (auto it = bot.GetSentPackets().rbegin(); it != bot.GetSentPackets().rend(); ++it)
    {
        if (it->Opcode != T::Opcode)
            continue;
        ByteBuffer body = it->Data;
        body.rpos(0);
        T value;
        if (value.Read(body))
            return value;
    }
    return std::nullopt;
}
}

TEST_CASE("bot movement: a root is acknowledged with the root flag set, an unroot with it cleared", "[movement]")
{
    TestFakeBot bot;

    MoveRoot root;
    root.Mover = TestMover();
    root.SequenceIndex = 5;
    bot.InjectIncoming(root);
    bot.Tick(10);

    std::optional<MoveForceRootAck> const rootAck = LastSent<MoveForceRootAck>(bot);
    REQUIRE(rootAck);
    CHECK(rootAck->Ack.AckIndex == 5);
    CHECK(rootAck->Ack.Status.Guid == TestMover());
    CHECK((rootAck->Ack.Status.Flags & CLASSIC_MOVEFLAG_ROOT) != 0);
    CHECK((bot.GetMovementFlags() & CLASSIC_MOVEFLAG_ROOT) != 0);

    MoveUnroot unroot;
    unroot.Mover = TestMover();
    unroot.SequenceIndex = 6;
    bot.InjectIncoming(unroot);
    bot.Tick(10);

    std::optional<MoveForceUnrootAck> const unrootAck = LastSent<MoveForceUnrootAck>(bot);
    REQUIRE(unrootAck);
    CHECK(unrootAck->Ack.AckIndex == 6);
    CHECK((unrootAck->Ack.Status.Flags & CLASSIC_MOVEFLAG_ROOT) == 0);
    CHECK(bot.GetMovementFlags() == 0);
}

TEST_CASE("bot movement: hover, water walk and feather fall are acknowledged with their flag", "[movement]")
{
    TestFakeBot bot;

    MoveSetHovering hover;
    hover.Mover = TestMover();
    hover.SequenceIndex = 1;
    bot.InjectIncoming(hover);

    MoveSetWaterWalk waterWalk;
    waterWalk.Mover = TestMover();
    waterWalk.SequenceIndex = 2;
    bot.InjectIncoming(waterWalk);

    MoveSetFeatherFall featherFall;
    featherFall.Mover = TestMover();
    featherFall.SequenceIndex = 3;
    bot.InjectIncoming(featherFall);
    bot.Tick(10);

    std::optional<MoveHoverAck> const hoverAck = LastSent<MoveHoverAck>(bot);
    REQUIRE(hoverAck);
    CHECK(hoverAck->Ack.AckIndex == 1);
    CHECK((hoverAck->Ack.Status.Flags & CLASSIC_MOVEFLAG_HOVER) != 0);

    std::optional<MoveWaterWalkAck> const waterWalkAck = LastSent<MoveWaterWalkAck>(bot);
    REQUIRE(waterWalkAck);
    CHECK(waterWalkAck->Ack.AckIndex == 2);
    CHECK((waterWalkAck->Ack.Status.Flags & CLASSIC_MOVEFLAG_WATERWALKING) != 0);

    std::optional<MoveFeatherFallAck> const featherFallAck = LastSent<MoveFeatherFallAck>(bot);
    REQUIRE(featherFallAck);
    CHECK(featherFallAck->Ack.AckIndex == 3);
    CHECK((featherFallAck->Ack.Status.Flags & CLASSIC_MOVEFLAG_FALLING_SLOW) != 0);

    MoveUnsetHovering unhover;
    unhover.Mover = TestMover();
    unhover.SequenceIndex = 4;
    bot.InjectIncoming(unhover);
    bot.Tick(10);
    CHECK((bot.GetMovementFlags() & CLASSIC_MOVEFLAG_HOVER) == 0);
    CHECK((bot.GetMovementFlags() & CLASSIC_MOVEFLAG_WATERWALKING) != 0);
}

TEST_CASE("bot movement: a speed change is acknowledged with the speed the server set", "[movement]")
{
    TestFakeBot bot;

    MoveSetRunSpeed run;
    run.Mover = TestMover();
    run.SequenceIndex = 9;
    run.Speed = 11.5f;
    bot.InjectIncoming(run);

    MoveSetSwimSpeed swim;
    swim.Mover = TestMover();
    swim.SequenceIndex = 10;
    swim.Speed = 6.25f;
    bot.InjectIncoming(swim);
    bot.Tick(10);

    std::optional<MoveForceRunSpeedChangeAck> const runAck = LastSent<MoveForceRunSpeedChangeAck>(bot);
    REQUIRE(runAck);
    CHECK(runAck->Ack.AckIndex == 9);
    CHECK(runAck->Speed == 11.5f);

    std::optional<MoveForceSwimSpeedChangeAck> const swimAck = LastSent<MoveForceSwimSpeedChangeAck>(bot);
    REQUIRE(swimAck);
    CHECK(swimAck->Ack.AckIndex == 10);
    CHECK(swimAck->Speed == 6.25f);
}

TEST_CASE("bot movement: a teleport is acknowledged with its sequence index", "[movement]")
{
    TestFakeBot bot;

    MoveTeleport teleport;
    teleport.Mover = TestMover();
    teleport.SequenceIndex = 12;
    teleport.X = 10.0f;
    teleport.Y = 20.0f;
    teleport.Z = 30.0f;
    bot.InjectIncoming(teleport);
    bot.Tick(10);

    std::optional<MoveTeleportAck> const ack = LastSent<MoveTeleportAck>(bot);
    REQUIRE(ack);
    CHECK(ack->Mover == TestMover());
    CHECK(ack->AckIndex == 12);
}

TEST_CASE("bot movement: a knock back is acknowledged with the jump it describes", "[movement]")
{
    TestFakeBot bot;

    MoveKnockBack knock;
    knock.Mover = TestMover();
    knock.SequenceIndex = 14;
    knock.DirectionX = 0.6f;
    knock.DirectionY = 0.8f;
    knock.HorizontalSpeed = 12.0f;
    knock.VerticalSpeed = 5.0f;
    bot.InjectIncoming(knock);
    bot.Tick(10);

    std::optional<MoveKnockBackAck> const ack = LastSent<MoveKnockBackAck>(bot);
    REQUIRE(ack);
    CHECK(ack->Ack.AckIndex == 14);
    REQUIRE(ack->Ack.Status.FallInfo);
    CHECK(ack->Ack.Status.FallInfo->CosAngle == 0.6f);
    CHECK(ack->Ack.Status.FallInfo->SinAngle == 0.8f);
    CHECK(ack->Ack.Status.FallInfo->XYSpeed == 12.0f);
    CHECK(ack->Ack.Status.FallInfo->ZSpeed == 5.0f);
    CHECK((ack->Ack.Status.Flags & CLASSIC_MOVEFLAG_FALLING) != 0);
}
