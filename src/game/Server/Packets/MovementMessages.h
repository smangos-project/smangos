#ifndef MANGOS_PACKETS_MOVEMENTMESSAGES_H
#define MANGOS_PACKETS_MOVEMENTMESSAGES_H

// The modern movement wire: the movement info block and the messages that carry it. Light like
// Packets/PlayerSetup.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <optional>
#include <vector>

// Modern movement flag bits (the first of the three flag words).
enum ClassicMovementFlag : uint32
{
    CLASSIC_MOVEFLAG_FORWARD        = 0x00000001,
    CLASSIC_MOVEFLAG_BACKWARD       = 0x00000002,
    CLASSIC_MOVEFLAG_STRAFE_LEFT    = 0x00000004,
    CLASSIC_MOVEFLAG_STRAFE_RIGHT   = 0x00000008,
    CLASSIC_MOVEFLAG_TURN_LEFT      = 0x00000010,
    CLASSIC_MOVEFLAG_TURN_RIGHT     = 0x00000020,
    CLASSIC_MOVEFLAG_PITCH_UP       = 0x00000040,
    CLASSIC_MOVEFLAG_PITCH_DOWN     = 0x00000080,
    CLASSIC_MOVEFLAG_WALKING        = 0x00000100,
    CLASSIC_MOVEFLAG_DISABLE_GRAVITY = 0x00000200,
    CLASSIC_MOVEFLAG_ROOT           = 0x00000400,
    CLASSIC_MOVEFLAG_FALLING        = 0x00000800,
    CLASSIC_MOVEFLAG_FALLING_FAR    = 0x00001000,
    CLASSIC_MOVEFLAG_SWIMMING       = 0x00100000,
    CLASSIC_MOVEFLAG_FLYING         = 0x01000000,
    CLASSIC_MOVEFLAG_SPLINE_ELEVATION = 0x02000000,
    CLASSIC_MOVEFLAG_WATERWALKING   = 0x04000000,
    CLASSIC_MOVEFLAG_FALLING_SLOW   = 0x08000000,
    CLASSIC_MOVEFLAG_HOVER          = 0x10000000,
};

struct ClassicMovementInfo
{
    struct Transport
    {
        ClassicGuid Guid;
        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;
        float O = 0.0f;
        int8 Seat = -1;
        uint32 Time = 0;
        std::optional<uint32> PrevTime;
        std::optional<uint32> VehicleId;
    };

    struct Inertia
    {
        uint32 Id = 0;
        float ForceX = 0.0f;
        float ForceY = 0.0f;
        float ForceZ = 0.0f;
        uint32 Lifetime = 0;
    };

    struct AdvFlying
    {
        float ForwardVelocity = 0.0f;
        float UpVelocity = 0.0f;
    };

    struct Fall
    {
        uint32 FallTime = 0;
        float ZSpeed = 0.0f;
        // sin, cos and horizontal speed of the jump; present on the wire only while falling
        bool HasDirection = false;
        float SinAngle = 0.0f;
        float CosAngle = 0.0f;
        float XYSpeed = 0.0f;
    };

    ClassicGuid Guid;
    uint32 Flags = 0;
    uint32 Flags2 = 0;
    uint32 Flags3 = 0;
    uint32 Time = 0;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    float O = 0.0f;
    float Pitch = 0.0f;
    float StepUpStartElevation = 0.0f;
    std::vector<ClassicGuid> RemoveForces;
    uint32 MoveIndex = 0;
    bool HeightChangeFailed = false;
    bool RemoteTimeValid = false;
    std::optional<ClassicGuid> StandingOnGameObject;
    std::optional<Transport> TransportInfo;
    std::optional<Inertia> InertiaInfo;
    std::optional<AdvFlying> AdvFlyingInfo;
    std::optional<Fall> FallInfo;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << Flags << Flags2 << Flags3 << Time;
        data << X << Y << Z << O;
        data << Pitch << StepUpStartElevation;
        data << uint32(RemoveForces.size()) << MoveIndex;
        for (ClassicGuid const& guid : RemoveForces)
            guid.Write(data);

        data.WriteBit(StandingOnGameObject.has_value());
        data.WriteBit(TransportInfo.has_value());
        data.WriteBit(FallInfo.has_value());
        data.WriteBit(false); // HasSpline: never sent
        data.WriteBit(HeightChangeFailed);
        data.WriteBit(RemoteTimeValid);
        data.WriteBit(InertiaInfo.has_value());
        data.WriteBit(AdvFlyingInfo.has_value());
        data.FlushBits();

        if (TransportInfo)
        {
            Transport const& t = *TransportInfo;
            t.Guid.Write(data);
            data << t.X << t.Y << t.Z << t.O;
            data << t.Seat << t.Time;
            data.WriteBit(t.PrevTime.has_value());
            data.WriteBit(t.VehicleId.has_value());
            data.FlushBits();
            if (t.PrevTime)
                data << *t.PrevTime;
            if (t.VehicleId)
                data << *t.VehicleId;
        }

        if (StandingOnGameObject)
            StandingOnGameObject->Write(data);

        if (InertiaInfo)
            data << InertiaInfo->Id << InertiaInfo->ForceX << InertiaInfo->ForceY << InertiaInfo->ForceZ << InertiaInfo->Lifetime;

        if (AdvFlyingInfo)
            data << AdvFlyingInfo->ForwardVelocity << AdvFlyingInfo->UpVelocity;

        if (FallInfo)
        {
            data << FallInfo->FallTime << FallInfo->ZSpeed;
            data.WriteBit(FallInfo->HasDirection);
            data.FlushBits();
            if (FallInfo->HasDirection)
                data << FallInfo->SinAngle << FallInfo->CosAngle << FallInfo->XYSpeed;
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < 4 * sizeof(uint32) + 6 * sizeof(float) + 2 * sizeof(uint32))
            return false;
        data >> Flags >> Flags2 >> Flags3 >> Time;
        data >> X >> Y >> Z >> O;
        data >> Pitch >> StepUpStartElevation;
        uint32 removeCount;
        data >> removeCount >> MoveIndex;

        RemoveForces.clear();
        for (uint32 i = 0; i < removeCount; ++i)
        {
            ClassicGuid guid;
            if (!guid.Read(data))
                return false;
            RemoveForces.push_back(guid);
        }

        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasStandingOn = data.ReadBit();
        bool const hasTransport = data.ReadBit();
        bool const hasFall = data.ReadBit();
        bool const hasSpline = data.ReadBit();
        HeightChangeFailed = data.ReadBit();
        RemoteTimeValid = data.ReadBit();
        bool const hasInertia = data.ReadBit();
        bool const hasAdvFlying = data.ReadBit();
        data.ResetBitPos();
        if (hasSpline)
            return false;

        TransportInfo.reset();
        if (hasTransport)
        {
            Transport t;
            if (!t.Guid.Read(data))
                return false;
            if (data.size() - data.rpos() < 4 * sizeof(float) + sizeof(int8) + sizeof(uint32) + 1)
                return false;
            data >> t.X >> t.Y >> t.Z >> t.O >> t.Seat >> t.Time;
            bool const hasPrevTime = data.ReadBit();
            bool const hasVehicleId = data.ReadBit();
            data.ResetBitPos();
            if (data.size() - data.rpos() < (hasPrevTime ? sizeof(uint32) : 0) + (hasVehicleId ? sizeof(uint32) : 0))
                return false;
            if (hasPrevTime)
            {
                uint32 value;
                data >> value;
                t.PrevTime = value;
            }
            if (hasVehicleId)
            {
                uint32 value;
                data >> value;
                t.VehicleId = value;
            }
            TransportInfo = t;
        }

        StandingOnGameObject.reset();
        if (hasStandingOn)
        {
            ClassicGuid guid;
            if (!guid.Read(data))
                return false;
            StandingOnGameObject = guid;
        }

        InertiaInfo.reset();
        if (hasInertia)
        {
            if (data.size() - data.rpos() < 5 * sizeof(uint32))
                return false;
            Inertia inertia;
            data >> inertia.Id >> inertia.ForceX >> inertia.ForceY >> inertia.ForceZ >> inertia.Lifetime;
            InertiaInfo = inertia;
        }

        AdvFlyingInfo.reset();
        if (hasAdvFlying)
        {
            if (data.size() - data.rpos() < 2 * sizeof(float))
                return false;
            AdvFlying advFlying;
            data >> advFlying.ForwardVelocity >> advFlying.UpVelocity;
            AdvFlyingInfo = advFlying;
        }

        FallInfo.reset();
        if (hasFall)
        {
            if (data.size() - data.rpos() < sizeof(uint32) + sizeof(float) + 1)
                return false;
            Fall fall;
            data >> fall.FallTime >> fall.ZSpeed;
            fall.HasDirection = data.ReadBit();
            data.ResetBitPos();
            if (fall.HasDirection)
            {
                if (data.size() - data.rpos() < 3 * sizeof(float))
                    return false;
                data >> fall.SinAngle >> fall.CosAngle >> fall.XYSpeed;
            }
            FallInfo = fall;
        }

        return true;
    }
};

// A movement opcode whose whole body is the mover's movement info (start/stop, jump, swim,
// facing and heartbeat messages). One instantiation per client opcode.
template<ClassicOpcodes Op>
struct ClientPlayerMovement
{
    static constexpr ClassicOpcodes Opcode = Op;

    ClassicMovementInfo Status;

    void Write(ByteBuffer& data) const { Status.Write(data); }
    bool Read(ByteBuffer& data) { return Status.Read(data); }
};

using MoveStartForward = ClientPlayerMovement<CLASSIC_CMSG_MOVE_START_FORWARD>;
using MoveStartBackward = ClientPlayerMovement<CLASSIC_CMSG_MOVE_START_BACKWARD>;
using MoveStop = ClientPlayerMovement<CLASSIC_CMSG_MOVE_STOP>;
using MoveStartStrafeLeft = ClientPlayerMovement<CLASSIC_CMSG_MOVE_START_STRAFE_LEFT>;
using MoveStartStrafeRight = ClientPlayerMovement<CLASSIC_CMSG_MOVE_START_STRAFE_RIGHT>;
using MoveStopStrafe = ClientPlayerMovement<CLASSIC_CMSG_MOVE_STOP_STRAFE>;
using MoveJump = ClientPlayerMovement<CLASSIC_CMSG_MOVE_JUMP>;
using MoveStartTurnLeft = ClientPlayerMovement<CLASSIC_CMSG_MOVE_START_TURN_LEFT>;
using MoveStartTurnRight = ClientPlayerMovement<CLASSIC_CMSG_MOVE_START_TURN_RIGHT>;
using MoveStopTurn = ClientPlayerMovement<CLASSIC_CMSG_MOVE_STOP_TURN>;
using MoveStartPitchUp = ClientPlayerMovement<CLASSIC_CMSG_MOVE_START_PITCH_UP>;
using MoveStartPitchDown = ClientPlayerMovement<CLASSIC_CMSG_MOVE_START_PITCH_DOWN>;
using MoveStopPitch = ClientPlayerMovement<CLASSIC_CMSG_MOVE_STOP_PITCH>;
using MoveSetRunMode = ClientPlayerMovement<CLASSIC_CMSG_MOVE_SET_RUN_MODE>;
using MoveSetWalkMode = ClientPlayerMovement<CLASSIC_CMSG_MOVE_SET_WALK_MODE>;
using MoveFallLand = ClientPlayerMovement<CLASSIC_CMSG_MOVE_FALL_LAND>;
using MoveFallReset = ClientPlayerMovement<CLASSIC_CMSG_MOVE_FALL_RESET>;
using MoveStartSwim = ClientPlayerMovement<CLASSIC_CMSG_MOVE_START_SWIM>;
using MoveStopSwim = ClientPlayerMovement<CLASSIC_CMSG_MOVE_STOP_SWIM>;
using MoveSetFacing = ClientPlayerMovement<CLASSIC_CMSG_MOVE_SET_FACING>;
using MoveSetPitch = ClientPlayerMovement<CLASSIC_CMSG_MOVE_SET_PITCH>;
using MoveHeartbeat = ClientPlayerMovement<CLASSIC_CMSG_MOVE_HEARTBEAT>;

// Another unit's movement, relayed to everyone who sees it.
struct MoveUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_MOVE_UPDATE;

    ClassicMovementInfo Status;

    void Write(ByteBuffer& data) const { Status.Write(data); }
    bool Read(ByteBuffer& data) { return Status.Read(data); }
};

// Tells the client which unit its movement input drives.
struct MoveSetActiveMover
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_MOVE_SET_ACTIVE_MOVER;

    ClassicGuid Mover;

    void Write(ByteBuffer& data) const { Mover.Write(data); }
    bool Read(ByteBuffer& data) { return Mover.Read(data); }
};

// The client confirming which unit it is moving.
struct SetActiveMover
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_ACTIVE_MOVER;

    ClassicGuid Mover;

    void Write(ByteBuffer& data) const { Mover.Write(data); }
    bool Read(ByteBuffer& data) { return Mover.Read(data); }
};

// Sent once the client has applied the active mover; Ticks is the client's clock.
struct MoveInitActiveMoverComplete
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MOVE_INIT_ACTIVE_MOVER_COMPLETE;

    uint32 Ticks = 0;

    void Write(ByteBuffer& data) const { data << Ticks; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Ticks;
        return true;
    }
};

// Whether the client's movement input moves the unit.
struct ControlUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CONTROL_UPDATE;

    ClassicGuid Guid;
    bool On = false;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data.WriteBit(On);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        On = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

#endif // MANGOS_PACKETS_MOVEMENTMESSAGES_H
