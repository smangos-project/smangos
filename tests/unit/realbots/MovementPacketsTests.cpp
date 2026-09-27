// Round trips for the movement info block and the modern movement messages.
#include "Packets/MonsterMove.h"
#include "Packets/MovementMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

namespace
{
ClassicGuid TestGuid(uint64 low)
{
    ClassicGuid guid;
    guid.Low = low;
    guid.High = 0x0400000000000000ULL;
    return guid;
}

void CheckSame(ClassicMovementInfo const& a, ClassicMovementInfo const& b)
{
    CHECK(a.Guid == b.Guid);
    CHECK(a.Flags == b.Flags);
    CHECK(a.Flags2 == b.Flags2);
    CHECK(a.Flags3 == b.Flags3);
    CHECK(a.Time == b.Time);
    CHECK(a.X == b.X);
    CHECK(a.Y == b.Y);
    CHECK(a.Z == b.Z);
    CHECK(a.O == b.O);
    CHECK(a.Pitch == b.Pitch);
    CHECK(a.StepUpStartElevation == b.StepUpStartElevation);
    CHECK(a.RemoveForces == b.RemoveForces);
    CHECK(a.MoveIndex == b.MoveIndex);
    CHECK(a.HeightChangeFailed == b.HeightChangeFailed);
    CHECK(a.RemoteTimeValid == b.RemoteTimeValid);
    CHECK(a.StandingOnGameObject == b.StandingOnGameObject);

    REQUIRE(a.TransportInfo.has_value() == b.TransportInfo.has_value());
    if (a.TransportInfo)
    {
        CHECK(a.TransportInfo->Guid == b.TransportInfo->Guid);
        CHECK(a.TransportInfo->X == b.TransportInfo->X);
        CHECK(a.TransportInfo->Y == b.TransportInfo->Y);
        CHECK(a.TransportInfo->Z == b.TransportInfo->Z);
        CHECK(a.TransportInfo->O == b.TransportInfo->O);
        CHECK(a.TransportInfo->Seat == b.TransportInfo->Seat);
        CHECK(a.TransportInfo->Time == b.TransportInfo->Time);
        CHECK(a.TransportInfo->PrevTime == b.TransportInfo->PrevTime);
        CHECK(a.TransportInfo->VehicleId == b.TransportInfo->VehicleId);
    }

    REQUIRE(a.InertiaInfo.has_value() == b.InertiaInfo.has_value());
    if (a.InertiaInfo)
    {
        CHECK(a.InertiaInfo->Id == b.InertiaInfo->Id);
        CHECK(a.InertiaInfo->ForceX == b.InertiaInfo->ForceX);
        CHECK(a.InertiaInfo->ForceY == b.InertiaInfo->ForceY);
        CHECK(a.InertiaInfo->ForceZ == b.InertiaInfo->ForceZ);
        CHECK(a.InertiaInfo->Lifetime == b.InertiaInfo->Lifetime);
    }

    REQUIRE(a.AdvFlyingInfo.has_value() == b.AdvFlyingInfo.has_value());
    if (a.AdvFlyingInfo)
    {
        CHECK(a.AdvFlyingInfo->ForwardVelocity == b.AdvFlyingInfo->ForwardVelocity);
        CHECK(a.AdvFlyingInfo->UpVelocity == b.AdvFlyingInfo->UpVelocity);
    }

    REQUIRE(a.FallInfo.has_value() == b.FallInfo.has_value());
    if (a.FallInfo)
    {
        CHECK(a.FallInfo->FallTime == b.FallInfo->FallTime);
        CHECK(a.FallInfo->ZSpeed == b.FallInfo->ZSpeed);
        CHECK(a.FallInfo->HasDirection == b.FallInfo->HasDirection);
        CHECK(a.FallInfo->SinAngle == b.FallInfo->SinAngle);
        CHECK(a.FallInfo->CosAngle == b.FallInfo->CosAngle);
        CHECK(a.FallInfo->XYSpeed == b.FallInfo->XYSpeed);
    }
}

ClassicMovementInfo Standing()
{
    ClassicMovementInfo info;
    info.Guid = TestGuid(0x1234);
    info.Flags = CLASSIC_MOVEFLAG_FORWARD;
    info.Time = 987654;
    info.X = -8949.95f;
    info.Y = -132.493f;
    info.Z = 83.5312f;
    info.O = 1.5f;
    info.Pitch = 0.25f;
    return info;
}
}

TEST_CASE("ClassicMovementInfo round-trips a plain running mover", "[packets]")
{
    ClassicMovementInfo out = Standing();
    RoundTrip(out, CheckSame);
    RejectsEveryTruncation(out);
}

TEST_CASE("ClassicMovementInfo round-trips every optional section", "[packets]")
{
    ClassicMovementInfo out = Standing();
    out.Flags |= CLASSIC_MOVEFLAG_FALLING;
    out.RemoveForces = { TestGuid(1), TestGuid(2) };
    out.MoveIndex = 9;
    out.HeightChangeFailed = true;
    out.RemoteTimeValid = true;
    out.StandingOnGameObject = TestGuid(77);

    ClassicMovementInfo::Transport transport;
    transport.Guid = TestGuid(500);
    transport.X = 1.0f;
    transport.Y = 2.0f;
    transport.Z = 3.0f;
    transport.O = 4.0f;
    transport.Seat = 2;
    transport.Time = 1500;
    transport.PrevTime = 1400;
    transport.VehicleId = 33;
    out.TransportInfo = transport;

    ClassicMovementInfo::Inertia inertia;
    inertia.Id = 5;
    inertia.ForceX = 0.5f;
    inertia.ForceY = 1.5f;
    inertia.ForceZ = 2.5f;
    inertia.Lifetime = 800;
    out.InertiaInfo = inertia;

    ClassicMovementInfo::AdvFlying advFlying;
    advFlying.ForwardVelocity = 12.0f;
    advFlying.UpVelocity = -1.0f;
    out.AdvFlyingInfo = advFlying;

    ClassicMovementInfo::Fall fall;
    fall.FallTime = 420;
    fall.ZSpeed = -9.5f;
    fall.HasDirection = true;
    fall.SinAngle = 0.5f;
    fall.CosAngle = 0.75f;
    fall.XYSpeed = 7.0f;
    out.FallInfo = fall;

    RoundTrip(out, CheckSame);
    RejectsEveryTruncation(out);
}

TEST_CASE("ClassicMovementInfo round-trips a fall without a direction", "[packets]")
{
    ClassicMovementInfo out = Standing();
    ClassicMovementInfo::Fall fall;
    fall.FallTime = 100;
    out.FallInfo = fall;

    RoundTrip(out, CheckSame);
    RejectsEveryTruncation(out);
}

TEST_CASE("ClassicMovementInfo rejects a block that announces a spline", "[packets]")
{
    ByteBuffer data;
    Standing().Write(data);

    // The eight flag bits follow the fixed fields and the (empty) remove-forces list; the fourth
    // bit is HasSpline.
    size_t const bitsOffset = data.size() - 1;
    data.put<uint8>(bitsOffset, uint8(1 << 4));

    ClassicMovementInfo in;
    CHECK_FALSE(in.Read(data));
}

TEST_CASE("ClientPlayerMovement and MoveUpdate carry the block unchanged", "[packets]")
{
    MoveStartForward start;
    start.Status = Standing();
    RoundTrip(start, [](MoveStartForward const& a, MoveStartForward const& b) { CheckSame(a.Status, b.Status); });
    static_assert(MoveStartForward::Opcode == CLASSIC_CMSG_MOVE_START_FORWARD);
    static_assert(MoveHeartbeat::Opcode == CLASSIC_CMSG_MOVE_HEARTBEAT);

    MoveUpdate update;
    update.Status = Standing();
    RoundTrip(update, [](MoveUpdate const& a, MoveUpdate const& b) { CheckSame(a.Status, b.Status); });
}

TEST_CASE("The active mover messages round-trip", "[packets]")
{
    MoveSetActiveMover set;
    set.Mover = TestGuid(42);
    RoundTrip(set, [](MoveSetActiveMover const& a, MoveSetActiveMover const& b) { CHECK(a.Mover == b.Mover); });
    RejectsEveryTruncation(set);

    SetActiveMover confirm;
    confirm.Mover = TestGuid(42);
    RoundTrip(confirm, [](SetActiveMover const& a, SetActiveMover const& b) { CHECK(a.Mover == b.Mover); });

    MoveInitActiveMoverComplete init;
    init.Ticks = 123456;
    RoundTrip(init, [](MoveInitActiveMoverComplete const& a, MoveInitActiveMoverComplete const& b) { CHECK(a.Ticks == b.Ticks); });
    RejectsEveryTruncation(init);

    ControlUpdate control;
    control.Guid = TestGuid(42);
    control.On = true;
    RoundTrip(control, [](ControlUpdate const& a, ControlUpdate const& b)
    {
        CHECK(a.Guid == b.Guid);
        CHECK(a.On == b.On);
    });
    RejectsEveryTruncation(control);
}

TEST_CASE("Speed change messages round-trip", "[packets]")
{
    MoveSetRunSpeed set;
    set.Mover = TestGuid(9);
    set.SequenceIndex = 4;
    set.Speed = 8.5f;
    RoundTrip(set, [](MoveSetRunSpeed const& a, MoveSetRunSpeed const& b)
    {
        CHECK(a.Mover == b.Mover);
        CHECK(a.SequenceIndex == b.SequenceIndex);
        CHECK(a.Speed == b.Speed);
    });
    RejectsEveryTruncation(set);
    static_assert(MoveSetTurnRate::Opcode == CLASSIC_SMSG_MOVE_SET_TURN_RATE);

    MoveSplineSetSwimSpeed spline;
    spline.Mover = TestGuid(9);
    spline.Speed = 4.75f;
    RoundTrip(spline, [](MoveSplineSetSwimSpeed const& a, MoveSplineSetSwimSpeed const& b)
    {
        CHECK(a.Mover == b.Mover);
        CHECK(a.Speed == b.Speed);
    });
    RejectsEveryTruncation(spline);

    MoveUpdateRunBackSpeed update;
    update.Status = Standing();
    update.Speed = 4.5f;
    RoundTrip(update, [](MoveUpdateRunBackSpeed const& a, MoveUpdateRunBackSpeed const& b)
    {
        CheckSame(a.Status, b.Status);
        CHECK(a.Speed == b.Speed);
    });
    RejectsEveryTruncation(update);

    MoveForceRunSpeedChangeAck ack;
    ack.Ack.Status = Standing();
    ack.Ack.AckIndex = 4;
    ack.Speed = 8.5f;
    RoundTrip(ack, [](MoveForceRunSpeedChangeAck const& a, MoveForceRunSpeedChangeAck const& b)
    {
        CheckSame(a.Ack.Status, b.Ack.Status);
        CHECK(a.Ack.AckIndex == b.Ack.AckIndex);
        CHECK(a.Speed == b.Speed);
    });
    RejectsEveryTruncation(ack);
}

TEST_CASE("Flag change messages round-trip", "[packets]")
{
    MoveRoot root;
    root.Mover = TestGuid(9);
    root.SequenceIndex = 11;
    RoundTrip(root, [](MoveRoot const& a, MoveRoot const& b)
    {
        CHECK(a.Mover == b.Mover);
        CHECK(a.SequenceIndex == b.SequenceIndex);
    });
    RejectsEveryTruncation(root);
    static_assert(MoveSetHovering::Opcode == CLASSIC_SMSG_MOVE_SET_HOVERING);

    MoveSplineUnroot spline;
    spline.Mover = TestGuid(9);
    RoundTrip(spline, [](MoveSplineUnroot const& a, MoveSplineUnroot const& b) { CHECK(a.Mover == b.Mover); });
    RejectsEveryTruncation(spline);

    MoveHoverAck ack;
    ack.Ack.Status = Standing();
    ack.Ack.AckIndex = 11;
    RoundTrip(ack, [](MoveHoverAck const& a, MoveHoverAck const& b)
    {
        CheckSame(a.Ack.Status, b.Ack.Status);
        CHECK(a.Ack.AckIndex == b.Ack.AckIndex);
    });
    RejectsEveryTruncation(ack);
}

TEST_CASE("Teleport messages round-trip", "[packets]")
{
    MoveTeleport teleport;
    teleport.Mover = TestGuid(9);
    teleport.SequenceIndex = 3;
    teleport.X = 1.0f;
    teleport.Y = 2.0f;
    teleport.Z = 3.0f;
    teleport.Facing = 4.0f;
    teleport.Transport = TestGuid(500);
    RoundTrip(teleport, [](MoveTeleport const& a, MoveTeleport const& b)
    {
        CHECK(a.Mover == b.Mover);
        CHECK(a.SequenceIndex == b.SequenceIndex);
        CHECK(a.X == b.X);
        CHECK(a.Y == b.Y);
        CHECK(a.Z == b.Z);
        CHECK(a.Facing == b.Facing);
        CHECK(a.Transport == b.Transport);
    });
    RejectsEveryTruncation(teleport);

    teleport.Transport.reset();
    RoundTrip(teleport, [](MoveTeleport const& a, MoveTeleport const& b) { CHECK(a.Transport == b.Transport); });

    MoveUpdateTeleport update;
    update.Status = Standing();
    update.Speeds[1] = 9.0f;
    update.Speeds[7] = 3.14f;
    RoundTrip(update, [](MoveUpdateTeleport const& a, MoveUpdateTeleport const& b)
    {
        CheckSame(a.Status, b.Status);
        for (size_t i = 0; i < MoveUpdateTeleport::SpeedCount; ++i)
            CHECK(a.Speeds[i] == b.Speeds[i]);
    });
    RejectsEveryTruncation(update);

    MoveTeleportAck ack;
    ack.Mover = TestGuid(9);
    ack.AckIndex = 3;
    ack.MoveTime = 555;
    RoundTrip(ack, [](MoveTeleportAck const& a, MoveTeleportAck const& b)
    {
        CHECK(a.Mover == b.Mover);
        CHECK(a.AckIndex == b.AckIndex);
        CHECK(a.MoveTime == b.MoveTime);
    });
    RejectsEveryTruncation(ack);
}

TEST_CASE("Knock back messages round-trip", "[packets]")
{
    MoveKnockBack knock;
    knock.Mover = TestGuid(9);
    knock.SequenceIndex = 6;
    knock.DirectionX = 0.6f;
    knock.DirectionY = 0.8f;
    knock.HorizontalSpeed = 12.0f;
    knock.VerticalSpeed = 5.0f;
    RoundTrip(knock, [](MoveKnockBack const& a, MoveKnockBack const& b)
    {
        CHECK(a.Mover == b.Mover);
        CHECK(a.SequenceIndex == b.SequenceIndex);
        CHECK(a.DirectionX == b.DirectionX);
        CHECK(a.DirectionY == b.DirectionY);
        CHECK(a.HorizontalSpeed == b.HorizontalSpeed);
        CHECK(a.VerticalSpeed == b.VerticalSpeed);
    });
    RejectsEveryTruncation(knock);

    MoveUpdateKnockBack update;
    update.Status = Standing();
    RoundTrip(update, [](MoveUpdateKnockBack const& a, MoveUpdateKnockBack const& b) { CheckSame(a.Status, b.Status); });

    MoveKnockBackAck ack;
    ack.Ack.Status = Standing();
    ack.Ack.AckIndex = 6;
    ack.Speeds = std::make_pair(12.0f, 5.0f);
    RoundTrip(ack, [](MoveKnockBackAck const& a, MoveKnockBackAck const& b)
    {
        CheckSame(a.Ack.Status, b.Ack.Status);
        CHECK(a.Ack.AckIndex == b.Ack.AckIndex);
        CHECK(a.Speeds == b.Speeds);
    });
    RejectsEveryTruncation(ack);

    ack.Speeds.reset();
    RoundTrip(ack, [](MoveKnockBackAck const& a, MoveKnockBackAck const& b) { CHECK(a.Speeds == b.Speeds); });
    RejectsEveryTruncation(ack);
}

TEST_CASE("MoveChangeTransport, MoveSetFacingHeartbeat and MoveUpdateFallSpeed share the movement block", "[packets]")
{
    static_assert(MoveChangeTransport::Opcode == CLASSIC_CMSG_MOVE_CHANGE_TRANSPORT);
    static_assert(MoveSetFacingHeartbeat::Opcode == CLASSIC_CMSG_MOVE_SET_FACING_HEARTBEAT);
    static_assert(MoveUpdateFallSpeed::Opcode == CLASSIC_CMSG_MOVE_UPDATE_FALL_SPEED);

    MoveChangeTransport packet;
    packet.Status = Standing();
    RoundTrip(packet, [](MoveChangeTransport const& a, MoveChangeTransport const& b) { CheckSame(a.Status, b.Status); });
}

TEST_CASE("MoveSetCollisionHeightAck round-trips", "[packets]")
{
    MoveSetCollisionHeightAck ack;
    ack.Ack.Status = Standing();
    ack.Ack.AckIndex = 3;
    ack.Height = 2.5f;
    ack.MountDisplayId = 1234;
    ack.Reason = 1;
    RoundTrip(ack, [](MoveSetCollisionHeightAck const& a, MoveSetCollisionHeightAck const& b)
    {
        CheckSame(a.Ack.Status, b.Ack.Status);
        CHECK(a.Ack.AckIndex == b.Ack.AckIndex);
        CHECK(a.Height == b.Height);
        CHECK(a.MountDisplayId == b.MountDisplayId);
        CHECK(a.Reason == b.Reason);
    });
    RejectsEveryTruncation(ack);
}

TEST_CASE("Spline done and time messages round-trip", "[packets]")
{
    MoveSplineDone done;
    done.Status = Standing();
    done.SplineId = 77;
    RoundTrip(done, [](MoveSplineDone const& a, MoveSplineDone const& b)
    {
        CheckSame(a.Status, b.Status);
        CHECK(a.SplineId == b.SplineId);
    });
    RejectsEveryTruncation(done);

    MoveTimeSkipped skipped;
    skipped.Mover = TestGuid(9);
    skipped.TimeSkipped = 250;
    RoundTrip(skipped, [](MoveTimeSkipped const& a, MoveTimeSkipped const& b)
    {
        CHECK(a.Mover == b.Mover);
        CHECK(a.TimeSkipped == b.TimeSkipped);
    });
    RejectsEveryTruncation(skipped);
    static_assert(MoveSkipTime::Opcode == CLASSIC_SMSG_MOVE_SKIP_TIME);
}

TEST_CASE("MonsterMove round-trips a path with each kind of facing", "[packets]")
{
    MonsterMove out;
    out.Mover = TestGuid(21);
    out.Start = { 1.0f, 2.0f, 3.0f };
    out.SplineId = 8;
    out.Flags = CLASSIC_SPLINEFLAG_UNCOMPRESSED_PATH | CLASSIC_SPLINEFLAG_FLYING;
    out.Elapsed = 120;
    out.MoveTime = 4500;
    out.Points = { { 4.0f, 5.0f, 6.0f }, { 7.0f, 8.0f, 9.0f } };

    auto check = [](MonsterMove const& a, MonsterMove const& b)
    {
        CHECK(a.Mover == b.Mover);
        CHECK(a.Start.X == b.Start.X);
        CHECK(a.Start.Y == b.Start.Y);
        CHECK(a.Start.Z == b.Start.Z);
        CHECK(a.SplineId == b.SplineId);
        CHECK(a.Flags == b.Flags);
        CHECK(a.Elapsed == b.Elapsed);
        CHECK(a.MoveTime == b.MoveTime);
        CHECK(a.Transport == b.Transport);
        CHECK(a.Face == b.Face);
        CHECK(a.FaceDirection == b.FaceDirection);
        CHECK(a.FaceTarget == b.FaceTarget);
        CHECK(a.PackedDeltas == b.PackedDeltas);
        REQUIRE(a.Points.size() == b.Points.size());
        for (size_t i = 0; i < a.Points.size(); ++i)
        {
            CHECK(a.Points[i].X == b.Points[i].X);
            CHECK(a.Points[i].Y == b.Points[i].Y);
            CHECK(a.Points[i].Z == b.Points[i].Z);
        }
    };

    RoundTrip(out, check);
    RejectsEveryTruncation(out);

    out.Face = CLASSIC_MONSTERMOVE_FACING_ANGLE;
    out.FaceDirection = 1.25f;
    RoundTrip(out, check);
    RejectsEveryTruncation(out);

    out.Face = CLASSIC_MONSTERMOVE_FACING_TARGET;
    out.FaceTarget = TestGuid(99);
    RoundTrip(out, check);
    RejectsEveryTruncation(out);

    out.Face = CLASSIC_MONSTERMOVE_FACING_SPOT;
    out.FaceDirection = 0.0f;
    out.FaceTarget = ClassicGuid();
    out.FaceSpot = { 10.0f, 11.0f, 12.0f };
    RoundTrip(out, [&](MonsterMove const& a, MonsterMove const& b)
    {
        check(a, b);
        CHECK(a.FaceSpot.X == b.FaceSpot.X);
        CHECK(a.FaceSpot.Y == b.FaceSpot.Y);
        CHECK(a.FaceSpot.Z == b.FaceSpot.Z);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("MonsterMove round-trips a stop, a transport mover and packed deltas", "[packets]")
{
    MonsterMove stop;
    stop.Mover = TestGuid(21);
    stop.Start = { 1.0f, 2.0f, 3.0f };
    stop.SplineId = 9;
    RoundTrip(stop, [](MonsterMove const& a, MonsterMove const& b)
    {
        CHECK(a.SplineId == b.SplineId);
        CHECK(b.Points.empty());
    });
    RejectsEveryTruncation(stop);

    MonsterMove packed = stop;
    packed.Transport = TestGuid(500);
    packed.VehicleSeat = 3;
    packed.Points = { { 4.0f, 5.0f, 6.0f } };
    packed.PackedDeltas = { 0x11223344u, 0x55667788u };
    RoundTrip(packed, [](MonsterMove const& a, MonsterMove const& b)
    {
        CHECK(a.Transport == b.Transport);
        CHECK(a.VehicleSeat == b.VehicleSeat);
        CHECK(a.PackedDeltas == b.PackedDeltas);
    });
    RejectsEveryTruncation(packed);
    static_assert(MonsterMove::Opcode == CLASSIC_SMSG_ON_MONSTER_MOVE);
}
