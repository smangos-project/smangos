#include "MovementInfoWire.h"
#include "Server/Packets/Classic/GuidMapping.h"
#include "World.h"
#include "Geometry.h"

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
    wire.O = Geometry::NormalizeOrientation(info.pos.o);
    wire.Pitch = info.s_pitch;

    if (info.HasMovementFlag(MOVEFLAG_ONTRANSPORT) && !info.t_guid.IsEmpty())
    {
        ClassicMovementInfo::Transport transport;
        transport.Guid = ClassicGuidMapping::ToClassic(info.t_guid, realmID);
        transport.X = info.t_pos.x;
        transport.Y = info.t_pos.y;
        transport.Z = info.t_pos.z;
        transport.O = Geometry::NormalizeOrientation(info.t_pos.o);
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
