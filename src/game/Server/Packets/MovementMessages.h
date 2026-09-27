#ifndef MANGOS_PACKETS_MOVEMENTMESSAGES_H
#define MANGOS_PACKETS_MOVEMENTMESSAGES_H

// The modern movement wire: the movement info block and the messages that carry it. Light like
// Packets/PlayerSetup.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <optional>
#include <utility>
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
using MoveChangeTransport = ClientPlayerMovement<CLASSIC_CMSG_MOVE_CHANGE_TRANSPORT>;
using MoveSetFacingHeartbeat = ClientPlayerMovement<CLASSIC_CMSG_MOVE_SET_FACING_HEARTBEAT>;
using MoveUpdateFallSpeed = ClientPlayerMovement<CLASSIC_CMSG_MOVE_UPDATE_FALL_SPEED>;

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

// A change the server asks the moving client to apply: the client applies it and answers with
// the matching acknowledgement carrying the same sequence index.
template<ClassicOpcodes Op>
struct MoveSetSpeed
{
    static constexpr ClassicOpcodes Opcode = Op;

    ClassicGuid Mover;
    uint32 SequenceIndex = 0;
    float Speed = 0.0f;

    void Write(ByteBuffer& data) const
    {
        Mover.Write(data);
        data << SequenceIndex << Speed;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Mover.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + sizeof(float))
            return false;
        data >> SequenceIndex >> Speed;
        return true;
    }
};

using MoveSetWalkSpeed = MoveSetSpeed<CLASSIC_SMSG_MOVE_SET_WALK_SPEED>;
using MoveSetRunSpeed = MoveSetSpeed<CLASSIC_SMSG_MOVE_SET_RUN_SPEED>;
using MoveSetRunBackSpeed = MoveSetSpeed<CLASSIC_SMSG_MOVE_SET_RUN_BACK_SPEED>;
using MoveSetSwimSpeed = MoveSetSpeed<CLASSIC_SMSG_MOVE_SET_SWIM_SPEED>;
using MoveSetSwimBackSpeed = MoveSetSpeed<CLASSIC_SMSG_MOVE_SET_SWIM_BACK_SPEED>;
using MoveSetTurnRate = MoveSetSpeed<CLASSIC_SMSG_MOVE_SET_TURN_RATE>;

// The same speed change for a unit no client is moving.
template<ClassicOpcodes Op>
struct MoveSplineSetSpeed
{
    static constexpr ClassicOpcodes Opcode = Op;

    ClassicGuid Mover;
    float Speed = 0.0f;

    void Write(ByteBuffer& data) const
    {
        Mover.Write(data);
        data << Speed;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Mover.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(float))
            return false;
        data >> Speed;
        return true;
    }
};

using MoveSplineSetWalkSpeed = MoveSplineSetSpeed<CLASSIC_SMSG_MOVE_SPLINE_SET_WALK_SPEED>;
using MoveSplineSetRunSpeed = MoveSplineSetSpeed<CLASSIC_SMSG_MOVE_SPLINE_SET_RUN_SPEED>;
using MoveSplineSetRunBackSpeed = MoveSplineSetSpeed<CLASSIC_SMSG_MOVE_SPLINE_SET_RUN_BACK_SPEED>;
using MoveSplineSetSwimSpeed = MoveSplineSetSpeed<CLASSIC_SMSG_MOVE_SPLINE_SET_SWIM_SPEED>;
using MoveSplineSetSwimBackSpeed = MoveSplineSetSpeed<CLASSIC_SMSG_MOVE_SPLINE_SET_SWIM_BACK_SPEED>;
using MoveSplineSetTurnRate = MoveSplineSetSpeed<CLASSIC_SMSG_MOVE_SPLINE_SET_TURN_RATE>;

// A player-moved unit's speed change, relayed to the players who see it.
template<ClassicOpcodes Op>
struct MoveUpdateSpeed
{
    static constexpr ClassicOpcodes Opcode = Op;

    ClassicMovementInfo Status;
    float Speed = 0.0f;

    void Write(ByteBuffer& data) const
    {
        Status.Write(data);
        data << Speed;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Status.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(float))
            return false;
        data >> Speed;
        return true;
    }
};

using MoveUpdateWalkSpeed = MoveUpdateSpeed<CLASSIC_SMSG_MOVE_UPDATE_WALK_SPEED>;
using MoveUpdateRunSpeed = MoveUpdateSpeed<CLASSIC_SMSG_MOVE_UPDATE_RUN_SPEED>;
using MoveUpdateRunBackSpeed = MoveUpdateSpeed<CLASSIC_SMSG_MOVE_UPDATE_RUN_BACK_SPEED>;
using MoveUpdateSwimSpeed = MoveUpdateSpeed<CLASSIC_SMSG_MOVE_UPDATE_SWIM_SPEED>;
using MoveUpdateSwimBackSpeed = MoveUpdateSpeed<CLASSIC_SMSG_MOVE_UPDATE_SWIM_BACK_SPEED>;
using MoveUpdateTurnRate = MoveUpdateSpeed<CLASSIC_SMSG_MOVE_UPDATE_TURN_RATE>;

// Root, water walk, hover and feather fall (and their opposites) each have a message of this shape.
template<ClassicOpcodes Op>
struct MoveSetFlag
{
    static constexpr ClassicOpcodes Opcode = Op;

    ClassicGuid Mover;
    uint32 SequenceIndex = 0;

    void Write(ByteBuffer& data) const
    {
        Mover.Write(data);
        data << SequenceIndex;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Mover.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> SequenceIndex;
        return true;
    }
};

using MoveRoot = MoveSetFlag<CLASSIC_SMSG_MOVE_ROOT>;
using MoveUnroot = MoveSetFlag<CLASSIC_SMSG_MOVE_UNROOT>;
using MoveSetWaterWalk = MoveSetFlag<CLASSIC_SMSG_MOVE_SET_WATER_WALK>;
using MoveSetLandWalk = MoveSetFlag<CLASSIC_SMSG_MOVE_SET_LAND_WALK>;
using MoveSetHovering = MoveSetFlag<CLASSIC_SMSG_MOVE_SET_HOVERING>;
using MoveUnsetHovering = MoveSetFlag<CLASSIC_SMSG_MOVE_UNSET_HOVERING>;
using MoveSetFeatherFall = MoveSetFlag<CLASSIC_SMSG_MOVE_SET_FEATHER_FALL>;
using MoveSetNormalFall = MoveSetFlag<CLASSIC_SMSG_MOVE_SET_NORMAL_FALL>;

// The same flag changes for a unit no client is moving, and the run/walk mode toggles.
template<ClassicOpcodes Op>
struct MoveSplineSetFlag
{
    static constexpr ClassicOpcodes Opcode = Op;

    ClassicGuid Mover;

    void Write(ByteBuffer& data) const { Mover.Write(data); }
    bool Read(ByteBuffer& data) { return Mover.Read(data); }
};

using MoveSplineRoot = MoveSplineSetFlag<CLASSIC_SMSG_MOVE_SPLINE_ROOT>;
using MoveSplineUnroot = MoveSplineSetFlag<CLASSIC_SMSG_MOVE_SPLINE_UNROOT>;
using MoveSplineSetWaterWalk = MoveSplineSetFlag<CLASSIC_SMSG_MOVE_SPLINE_SET_WATER_WALK>;
using MoveSplineSetLandWalk = MoveSplineSetFlag<CLASSIC_SMSG_MOVE_SPLINE_SET_LAND_WALK>;
using MoveSplineSetHover = MoveSplineSetFlag<CLASSIC_SMSG_MOVE_SPLINE_SET_HOVER>;
using MoveSplineUnsetHover = MoveSplineSetFlag<CLASSIC_SMSG_MOVE_SPLINE_UNSET_HOVER>;
using MoveSplineSetFeatherFall = MoveSplineSetFlag<CLASSIC_SMSG_MOVE_SPLINE_SET_FEATHER_FALL>;
using MoveSplineSetNormalFall = MoveSplineSetFlag<CLASSIC_SMSG_MOVE_SPLINE_SET_NORMAL_FALL>;
using MoveSplineSetRunMode = MoveSplineSetFlag<CLASSIC_SMSG_MOVE_SPLINE_SET_RUN_MODE>;
using MoveSplineSetWalkMode = MoveSplineSetFlag<CLASSIC_SMSG_MOVE_SPLINE_SET_WALK_MODE>;

// Moves the client's unit to a new position; answered by MoveTeleportAck.
struct MoveTeleport
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_MOVE_TELEPORT;

    ClassicGuid Mover;
    uint32 SequenceIndex = 0;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    float Facing = 0.0f;
    uint8 PreloadWorld = 0;
    std::optional<ClassicGuid> Transport;

    void Write(ByteBuffer& data) const
    {
        Mover.Write(data);
        data << SequenceIndex << X << Y << Z << Facing << PreloadWorld;
        data.WriteBit(Transport.has_value());
        data.WriteBit(false); // no vehicle
        data.FlushBits();
        if (Transport)
            Transport->Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Mover.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + 4 * sizeof(float) + sizeof(uint8) + 1)
            return false;
        data >> SequenceIndex >> X >> Y >> Z >> Facing >> PreloadWorld;
        bool const hasTransport = data.ReadBit();
        bool const hasVehicle = data.ReadBit();
        data.ResetBitPos();
        if (hasVehicle)
            return false;

        Transport.reset();
        if (hasTransport)
        {
            ClassicGuid guid;
            if (!guid.Read(data))
                return false;
            Transport = guid;
        }
        return true;
    }
};

// A teleport, relayed to the players who see the unit. Speeds are only listed when they differ
// from the defaults, in the order walk, run, run back, swim, swim back, flight, flight back, turn
// rate, pitch rate.
struct MoveUpdateTeleport
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_MOVE_UPDATE_TELEPORT;
    static constexpr size_t SpeedCount = 9;

    ClassicMovementInfo Status;
    std::optional<float> Speeds[SpeedCount];

    void Write(ByteBuffer& data) const
    {
        Status.Write(data);
        data << uint32(0); // movement forces
        for (std::optional<float> const& speed : Speeds)
            data.WriteBit(speed.has_value());
        data.FlushBits();
        for (std::optional<float> const& speed : Speeds)
            if (speed)
                data << *speed;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Status.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + 2)
            return false;
        uint32 forces;
        data >> forces;
        if (forces != 0)
            return false;

        bool present[SpeedCount];
        for (bool& bit : present)
            bit = data.ReadBit();
        data.ResetBitPos();

        for (size_t i = 0; i < SpeedCount; ++i)
        {
            Speeds[i].reset();
            if (!present[i])
                continue;
            if (data.size() - data.rpos() < sizeof(float))
                return false;
            float value;
            data >> value;
            Speeds[i] = value;
        }
        return true;
    }
};

// Throws the client's unit through the air: a horizontal direction and two speeds; answered by
// MoveKnockBackAck.
struct MoveKnockBack
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_MOVE_KNOCK_BACK;

    ClassicGuid Mover;
    uint32 SequenceIndex = 0;
    float DirectionX = 0.0f;
    float DirectionY = 0.0f;
    float HorizontalSpeed = 0.0f;
    float VerticalSpeed = 0.0f;

    void Write(ByteBuffer& data) const
    {
        Mover.Write(data);
        data << SequenceIndex << DirectionX << DirectionY << HorizontalSpeed << VerticalSpeed;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Mover.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + 4 * sizeof(float))
            return false;
        data >> SequenceIndex >> DirectionX >> DirectionY >> HorizontalSpeed >> VerticalSpeed;
        return true;
    }
};

// A knock back, relayed to the players who see the unit.
struct MoveUpdateKnockBack
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_MOVE_UPDATE_KNOCK_BACK;

    ClassicMovementInfo Status;

    void Write(ByteBuffer& data) const { Status.Write(data); }
    bool Read(ByteBuffer& data) { return Status.Read(data); }
};

// A movement info block plus the sequence index of the server change it answers.
struct ClassicMovementAck
{
    ClassicMovementInfo Status;
    int32 AckIndex = 0;

    void Write(ByteBuffer& data) const
    {
        Status.Write(data);
        data << AckIndex;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Status.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> AckIndex;
        return true;
    }
};

// The answer to a root/unroot, water walk, hover or feather fall change; the opcode says which one
// and whether it was applied.
template<ClassicOpcodes Op>
struct ClientMovementAck
{
    static constexpr ClassicOpcodes Opcode = Op;

    ClassicMovementAck Ack;

    void Write(ByteBuffer& data) const { Ack.Write(data); }
    bool Read(ByteBuffer& data) { return Ack.Read(data); }
};

using MoveForceRootAck = ClientMovementAck<CLASSIC_CMSG_MOVE_FORCE_ROOT_ACK>;
using MoveForceUnrootAck = ClientMovementAck<CLASSIC_CMSG_MOVE_FORCE_UNROOT_ACK>;
using MoveWaterWalkAck = ClientMovementAck<CLASSIC_CMSG_MOVE_WATER_WALK_ACK>;
using MoveHoverAck = ClientMovementAck<CLASSIC_CMSG_MOVE_HOVER_ACK>;
using MoveFeatherFallAck = ClientMovementAck<CLASSIC_CMSG_MOVE_FEATHER_FALL_ACK>;

// The answer to a speed change, carrying the speed the client applied.
template<ClassicOpcodes Op>
struct ClientMovementSpeedAck
{
    static constexpr ClassicOpcodes Opcode = Op;

    ClassicMovementAck Ack;
    float Speed = 0.0f;

    void Write(ByteBuffer& data) const
    {
        Ack.Write(data);
        data << Speed;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Ack.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(float))
            return false;
        data >> Speed;
        return true;
    }
};

using MoveForceWalkSpeedChangeAck = ClientMovementSpeedAck<CLASSIC_CMSG_MOVE_FORCE_WALK_SPEED_CHANGE_ACK>;
using MoveForceRunSpeedChangeAck = ClientMovementSpeedAck<CLASSIC_CMSG_MOVE_FORCE_RUN_SPEED_CHANGE_ACK>;
using MoveForceRunBackSpeedChangeAck = ClientMovementSpeedAck<CLASSIC_CMSG_MOVE_FORCE_RUN_BACK_SPEED_CHANGE_ACK>;
using MoveForceSwimSpeedChangeAck = ClientMovementSpeedAck<CLASSIC_CMSG_MOVE_FORCE_SWIM_SPEED_CHANGE_ACK>;
using MoveForceSwimBackSpeedChangeAck = ClientMovementSpeedAck<CLASSIC_CMSG_MOVE_FORCE_SWIM_BACK_SPEED_CHANGE_ACK>;
using MoveForceTurnRateChangeAck = ClientMovementSpeedAck<CLASSIC_CMSG_MOVE_FORCE_TURN_RATE_CHANGE_ACK>;

// The answer to a model-scale-driven collision height change (SMSG_MOVE_SET_COLLISION_HEIGHT).
// This engine never sends that SMSG (no scale-driven collision height feature), so nothing ever
// prompts this ack in practice; parsed in full and acknowledged as a no-op, same as SpellClick.
struct MoveSetCollisionHeightAck
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MOVE_SET_COLLISION_HEIGHT_ACK;

    ClassicMovementAck Ack;
    float Height = 1.0f;
    uint32 MountDisplayId = 0;
    uint8 Reason = 0;

    void Write(ByteBuffer& data) const
    {
        Ack.Write(data);
        data << Height;
        data << MountDisplayId;
        data << Reason;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Ack.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(float) + sizeof(uint32) + sizeof(uint8))
            return false;
        data >> Height;
        data >> MountDisplayId;
        data >> Reason;
        return true;
    }
};

struct MoveKnockBackAck
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MOVE_KNOCK_BACK_ACK;

    ClassicMovementAck Ack;
    // The horizontal and vertical speeds the client applied, if it says so.
    std::optional<std::pair<float, float>> Speeds;

    void Write(ByteBuffer& data) const
    {
        Ack.Write(data);
        data.WriteBit(Speeds.has_value());
        data.FlushBits();
        if (Speeds)
            data << Speeds->first << Speeds->second;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Ack.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasSpeeds = data.ReadBit();
        data.ResetBitPos();

        Speeds.reset();
        if (hasSpeeds)
        {
            if (data.size() - data.rpos() < 2 * sizeof(float))
                return false;
            float horizontal, vertical;
            data >> horizontal >> vertical;
            Speeds = std::make_pair(horizontal, vertical);
        }
        return true;
    }
};

struct MoveTeleportAck
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MOVE_TELEPORT_ACK;

    ClassicGuid Mover;
    int32 AckIndex = 0;
    int32 MoveTime = 0;

    void Write(ByteBuffer& data) const
    {
        Mover.Write(data);
        data << AckIndex << MoveTime;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Mover.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(int32))
            return false;
        data >> AckIndex >> MoveTime;
        return true;
    }
};

// Sent when a server-driven spline the client was told about has finished.
struct MoveSplineDone
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MOVE_SPLINE_DONE;

    ClassicMovementInfo Status;
    int32 SplineId = 0;

    void Write(ByteBuffer& data) const
    {
        Status.Write(data);
        data << SplineId;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Status.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> SplineId;
        return true;
    }
};

// Milliseconds of movement time the client lost, and the same told to the players who see it.
template<ClassicOpcodes Op>
struct MoveTimeMessage
{
    static constexpr ClassicOpcodes Opcode = Op;

    ClassicGuid Mover;
    uint32 TimeSkipped = 0;

    void Write(ByteBuffer& data) const
    {
        Mover.Write(data);
        data << TimeSkipped;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Mover.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> TimeSkipped;
        return true;
    }
};

using MoveTimeSkipped = MoveTimeMessage<CLASSIC_CMSG_MOVE_TIME_SKIPPED>;
using MoveSkipTime = MoveTimeMessage<CLASSIC_SMSG_MOVE_SKIP_TIME>;

#endif // MANGOS_PACKETS_MOVEMENTMESSAGES_H
