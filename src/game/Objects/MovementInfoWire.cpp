#include "MovementInfoWire.h"
#include "Server/Packets/Classic/GuidMapping.h"
#include "World.h"

namespace
{
struct FlagPair
{
    uint32 legacy;
    uint32 modern;
};

FlagPair const flagPairs[] =
{
    { MOVEFLAG_FORWARD,          CLASSIC_MOVEFLAG_FORWARD },
    { MOVEFLAG_BACKWARD,         CLASSIC_MOVEFLAG_BACKWARD },
    { MOVEFLAG_STRAFE_LEFT,      CLASSIC_MOVEFLAG_STRAFE_LEFT },
    { MOVEFLAG_STRAFE_RIGHT,     CLASSIC_MOVEFLAG_STRAFE_RIGHT },
    { MOVEFLAG_TURN_LEFT,        CLASSIC_MOVEFLAG_TURN_LEFT },
    { MOVEFLAG_TURN_RIGHT,       CLASSIC_MOVEFLAG_TURN_RIGHT },
    { MOVEFLAG_PITCH_UP,         CLASSIC_MOVEFLAG_PITCH_UP },
    { MOVEFLAG_PITCH_DOWN,       CLASSIC_MOVEFLAG_PITCH_DOWN },
    { MOVEFLAG_WALK_MODE,        CLASSIC_MOVEFLAG_WALKING },
    { MOVEFLAG_LEVITATING,       CLASSIC_MOVEFLAG_DISABLE_GRAVITY },
    { MOVEFLAG_ROOT,             CLASSIC_MOVEFLAG_ROOT },
    { MOVEFLAG_JUMPING,          CLASSIC_MOVEFLAG_FALLING },
    { MOVEFLAG_FALLINGFAR,       CLASSIC_MOVEFLAG_FALLING_FAR },
    { MOVEFLAG_SWIMMING,         CLASSIC_MOVEFLAG_SWIMMING },
    { MOVEFLAG_FLYING,           CLASSIC_MOVEFLAG_FLYING },
    { MOVEFLAG_SPLINE_ELEVATION, CLASSIC_MOVEFLAG_SPLINE_ELEVATION },
    { MOVEFLAG_WATERWALKING,     CLASSIC_MOVEFLAG_WATERWALKING },
    { MOVEFLAG_SAFE_FALL,        CLASSIC_MOVEFLAG_FALLING_SLOW },
    { MOVEFLAG_HOVER,            CLASSIC_MOVEFLAG_HOVER },
};
}

uint32 ToModernMovementFlags(uint32 moveFlags)
{
    uint32 flags = 0;
    for (FlagPair const& pair : flagPairs)
        if (moveFlags & pair.legacy)
            flags |= pair.modern;
    return flags;
}

uint32 FromModernMovementFlags(uint32 flags)
{
    uint32 moveFlags = 0;
    for (FlagPair const& pair : flagPairs)
        if (flags & pair.modern)
            moveFlags |= pair.legacy;
    return moveFlags;
}

bool MovementInfoFromWire(ClassicMovementInfo const& wire, MovementInfo& info)
{
    info.stime = WorldTimer::getMSTime();
    info.ctime = wire.Time;
    info.moveFlags = FromModernMovementFlags(wire.Flags);
    info.pos.x = wire.X;
    info.pos.y = wire.Y;
    info.pos.z = wire.Z;
    info.pos.o = wire.O;
    info.s_pitch = wire.Pitch;

    info.ClearTransportData();
    info.RemoveMovementFlag(MOVEFLAG_ONTRANSPORT);
    if (wire.TransportInfo)
    {
        auto const guid = ClassicGuidMapping::FromClassic(wire.TransportInfo->Guid);
        if (!guid)
            return false;
        info.SetTransportData(*guid, wire.TransportInfo->X, wire.TransportInfo->Y, wire.TransportInfo->Z, wire.TransportInfo->O);
        info.AddMovementFlag(MOVEFLAG_ONTRANSPORT);
    }

    info.fallTime = 0;
    if (wire.FallInfo)
    {
        info.fallTime = wire.FallInfo->FallTime;
        info.jump.zspeed = wire.FallInfo->ZSpeed;
        info.jump.sinAngle = wire.FallInfo->SinAngle;
        info.jump.cosAngle = wire.FallInfo->CosAngle;
        info.jump.xyspeed = wire.FallInfo->XYSpeed;
    }

    if (info.HasMovementFlag(MOVEFLAG_JUMPING))
    {
        if (!info.jump.startClientTime)
        {
            info.jump.startClientTime = info.ctime;
            info.jump.start = info.pos;
        }
    }
    else
        info.jump.startClientTime = 0;

    return true;
}

ClassicMovementInfo MovementInfoToWire(MovementInfo const& info, ObjectGuid const& mover)
{
    ClassicMovementInfo wire;
    wire.Guid = ClassicGuidMapping::ToClassic(mover, realmID);
    wire.Flags = ToModernMovementFlags(info.moveFlags);
    wire.Time = info.stime;
    wire.X = info.pos.x;
    wire.Y = info.pos.y;
    wire.Z = info.pos.z;
    wire.O = info.pos.o;
    wire.Pitch = info.s_pitch;

    if (info.HasMovementFlag(MOVEFLAG_ONTRANSPORT) && !info.t_guid.IsEmpty())
    {
        ClassicMovementInfo::Transport transport;
        transport.Guid = ClassicGuidMapping::ToClassic(info.t_guid, realmID);
        transport.X = info.t_pos.x;
        transport.Y = info.t_pos.y;
        transport.Z = info.t_pos.z;
        transport.O = info.t_pos.o;
        wire.TransportInfo = transport;
    }

    bool const falling = info.HasMovementFlag(MOVEFLAG_JUMPING | MOVEFLAG_FALLINGFAR);
    if (falling || info.fallTime != 0)
    {
        ClassicMovementInfo::Fall fall;
        fall.FallTime = info.fallTime;
        fall.ZSpeed = info.jump.zspeed;
        if (falling)
        {
            fall.HasDirection = true;
            fall.SinAngle = info.jump.sinAngle;
            fall.CosAngle = info.jump.cosAngle;
            fall.XYSpeed = info.jump.xyspeed;
        }
        wire.FallInfo = fall;
    }

    return wire;
}
