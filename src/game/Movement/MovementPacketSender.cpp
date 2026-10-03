/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MovementPacketSender.h"
#include "Player.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Anticheat.h"
#include "MoveSpline.h"
#include "MovementInfoWire.h"
#include "Packets/Classic/GuidMapping.h"
#include "World.h"

namespace
{
ClassicGuid ToClassic(Unit const* unit)
{
    return ClassicGuidMapping::ToClassic(unit->GetObjectGuid(), realmID);
}

// The three messages a speed change uses: the one that asks the moving client to apply it, the one
// that tells the players around a player-moved unit, and the one for a unit no client is moving.
template<typename Set, typename Update, typename Spline>
struct SpeedMessages
{
    using SetT = Set;
    using UpdateT = Update;
    using SplineT = Spline;
};

template<typename Func>
void WithSpeedMessages(UnitMoveType type, Func&& func)
{
    switch (type)
    {
        case MOVE_WALK:
            func(SpeedMessages<MoveSetWalkSpeed, MoveUpdateWalkSpeed, MoveSplineSetWalkSpeed>());
            break;
        case MOVE_RUN:
            func(SpeedMessages<MoveSetRunSpeed, MoveUpdateRunSpeed, MoveSplineSetRunSpeed>());
            break;
        case MOVE_RUN_BACK:
            func(SpeedMessages<MoveSetRunBackSpeed, MoveUpdateRunBackSpeed, MoveSplineSetRunBackSpeed>());
            break;
        case MOVE_SWIM:
            func(SpeedMessages<MoveSetSwimSpeed, MoveUpdateSwimSpeed, MoveSplineSetSwimSpeed>());
            break;
        case MOVE_SWIM_BACK:
            func(SpeedMessages<MoveSetSwimBackSpeed, MoveUpdateSwimBackSpeed, MoveSplineSetSwimBackSpeed>());
            break;
        case MOVE_TURN_RATE:
            func(SpeedMessages<MoveSetTurnRate, MoveUpdateTurnRate, MoveSplineSetTurnRate>());
            break;
        case MOVE_FLIGHT:
            func(SpeedMessages<MoveSetFlightSpeed, MoveUpdateFlightSpeed, MoveSplineSetFlightSpeed>());
            break;
        case MOVE_FLIGHT_BACK:
            func(SpeedMessages<MoveSetFlightBackSpeed, MoveUpdateFlightBackSpeed, MoveSplineSetFlightBackSpeed>());
            break;
        default:
            break;
    }
}

// The two messages a flag change uses: the one for the moving client and the one for a unit no
// client is moving.
template<typename Set, typename Spline>
struct FlagMessages
{
    using SetT = Set;
    using SplineT = Spline;
};

template<typename Func>
void WithFlagMessages(MovementFlags flag, bool apply, Func&& func)
{
    switch (flag)
    {
        case MOVEFLAG_ROOT:
            if (apply)
                func(FlagMessages<MoveRoot, MoveSplineRoot>());
            else
                func(FlagMessages<MoveUnroot, MoveSplineUnroot>());
            break;
        case MOVEFLAG_WATERWALKING:
            if (apply)
                func(FlagMessages<MoveSetWaterWalk, MoveSplineSetWaterWalk>());
            else
                func(FlagMessages<MoveSetLandWalk, MoveSplineSetLandWalk>());
            break;
        case MOVEFLAG_HOVER:
            if (apply)
                func(FlagMessages<MoveSetHovering, MoveSplineSetHover>());
            else
                func(FlagMessages<MoveUnsetHovering, MoveSplineUnsetHover>());
            break;
        case MOVEFLAG_SAFE_FALL:
            if (apply)
                func(FlagMessages<MoveSetFeatherFall, MoveSplineSetFeatherFall>());
            else
                func(FlagMessages<MoveSetNormalFall, MoveSplineSetNormalFall>());
            break;
        default:
            break;
    }
}

void SendMoveUpdate(Unit* unit, WorldObject const* except)
{
    MoveUpdate update;
    update.Status = MovementInfoToWire(unit->m_movementInfo, unit->GetObjectGuid());
    unit->SendMovementMessageToSet(MakeClassicPacket(update), true, except);
}
}

void MovementPacketSender::AddSpeedChangeToController(Unit* unit, UnitMoveType mtype, float newRate)
{
    Player* mover = unit->GetPlayerMovingMe();
    if (!mover)
    {
        sLog.Out(LOG_MOVEMENT, LOG_LVL_ERROR, "MovementPacketSender::AddSpeedChangeToController: Incorrect use of the function. It was called on a unit controlled by the server! %s", unit->GetGuidStr().c_str());
        return;
    }

    float newSpeedFlat = newRate * baseMoveSpeed[mtype];
    uint32 mCounter = unit->GetMovementCounterAndInc();
    PlayerMovementPendingChange pendingChange;
    pendingChange.movementCounter = mCounter;
    pendingChange.newValue = newSpeedFlat;
    pendingChange.movementChangeType = MovementPacketSender::GetChangeTypeByMoveType(mtype);
    pendingChange.controller = mover->GetObjectGuid();
    unit->PushPendingMovementChange(pendingChange);

    SendSpeedChangeToController(unit, mover, pendingChange);
}

void MovementPacketSender::SendSpeedChangeToController(Unit* unit, Player* mover, PlayerMovementPendingChange& pendingChange)
{
    UnitMoveType mtype = GetMoveTypeByChangeType(pendingChange.movementChangeType);
    WithSpeedMessages(mtype, [&](auto messages)
    {
        typename decltype(messages)::SetT packet;
        packet.Mover = ToClassic(unit);
        packet.SequenceIndex = pendingChange.movementCounter;
        packet.Speed = pendingChange.newValue;

        mover->GetCheatData()->LogMovementPacket(false, MakeClassicPacket(packet));
        mover->GetSession()->SendClassic(packet);
    });
}

MovementChangeType MovementPacketSender::GetChangeTypeByMoveType(UnitMoveType moveType)
{
    switch (moveType)
    {
        case MOVE_WALK:         return SPEED_CHANGE_WALK;
        case MOVE_RUN:          return SPEED_CHANGE_RUN;
        case MOVE_RUN_BACK:     return SPEED_CHANGE_RUN_BACK;
        case MOVE_SWIM:         return SPEED_CHANGE_SWIM;
        case MOVE_SWIM_BACK:    return SPEED_CHANGE_SWIM_BACK;
        case MOVE_TURN_RATE:    return RATE_CHANGE_TURN;
        case MOVE_FLIGHT:       return SPEED_CHANGE_FLIGHT;
        case MOVE_FLIGHT_BACK:  return SPEED_CHANGE_FLIGHT_BACK;
        default:
            ASSERT(false && "MovementPacketSender::GetChangeTypeByMoveType Unsupported UnitMoveType");
    }
}

UnitMoveType MovementPacketSender::GetMoveTypeByChangeType(MovementChangeType moveType)
{
    switch (moveType)
    {
        case SPEED_CHANGE_WALK:         return MOVE_WALK;
        case SPEED_CHANGE_RUN:          return MOVE_RUN;
        case SPEED_CHANGE_RUN_BACK:     return MOVE_RUN_BACK;
        case SPEED_CHANGE_SWIM:         return MOVE_SWIM;
        case SPEED_CHANGE_SWIM_BACK:    return MOVE_SWIM_BACK;
        case RATE_CHANGE_TURN:          return MOVE_TURN_RATE;
        case SPEED_CHANGE_FLIGHT:       return MOVE_FLIGHT;
        case SPEED_CHANGE_FLIGHT_BACK:  return MOVE_FLIGHT_BACK;
        default:
            ASSERT(false && "MovementPacketSender::GetMoveTypeByChangeType Unsupported MovementChangeType");
    }
}

void MovementPacketSender::SendSpeedChangeToObservers(Unit* unit, UnitMoveType mtype, float newSpeed)
{
    Player* mover = unit->GetPlayerMovingMe();
    if (!mover)
    {
        sLog.Out(LOG_MOVEMENT, LOG_LVL_ERROR, "MovementPacketSender::SendSpeedChangeToObservers: Incorrect use of the function. It was called on a unit controlled by the server! %s", unit->GetGuidStr().c_str());
        return;
    }

    WithSpeedMessages(mtype, [&](auto messages)
    {
        if (unit->movespline->Finalized())
        {
            typename decltype(messages)::UpdateT packet;
            packet.Status = MovementInfoToWire(unit->m_movementInfo, unit->GetObjectGuid());
            packet.Speed = newSpeed;
            unit->SendMovementMessageToSet(MakeClassicPacket(packet), true, mover);
        }
        else
        {
            typename decltype(messages)::SplineT packet;
            packet.Mover = ToClassic(unit);
            packet.Speed = newSpeed;
            unit->SendMovementMessageToSet(MakeClassicPacket(packet), true, mover);
        }
    });
}

void MovementPacketSender::SendSpeedChangeToAll(Unit* unit, UnitMoveType mtype, float newRate)
{
    WithSpeedMessages(mtype, [&](auto messages)
    {
        typename decltype(messages)::SplineT packet;
        packet.Mover = ToClassic(unit);
        packet.Speed = newRate * baseMoveSpeed[mtype];
        unit->SendMovementMessageToSet(MakeClassicPacket(packet), true);
    });
}

void MovementPacketSender::SendTeleportToController(Unit* unit, float x, float y, float z, float ang)
{
    Player* mover = unit->GetPlayerMovingMe();
    if (!mover)
    {
        sLog.Out(LOG_MOVEMENT, LOG_LVL_ERROR, "MovementPacketSender::SendTeleportToController: Incorrect use of the function. It was called on a unit controlled by the server! %s", unit->GetGuidStr().c_str());
        return;
    }

    uint32 mCounter = unit->GetMovementCounterAndInc();
    PlayerMovementPendingChange pendingChange;
    pendingChange.movementCounter = mCounter;
    pendingChange.movementChangeType = TELEPORT;
    pendingChange.controller = mover->GetObjectGuid();
    unit->PushPendingMovementChange(pendingChange);

    MoveTeleport packet;
    packet.Mover = ToClassic(unit);
    packet.SequenceIndex = mCounter;
    packet.X = x;
    packet.Y = y;
    packet.Z = z;
    packet.Facing = ang;

    mover->GetCheatData()->LogMovementPacket(false, MakeClassicPacket(packet));
    mover->GetSession()->SendClassic(packet);
}

void MovementPacketSender::SendTeleportToObservers(Unit* unit, float x, float y, float z, float ang)
{
    Player* mover = unit->GetPlayerMovingMe();

    // copy moveinfo to change position to where player is teleporting
    MovementInfo moveInfo = unit->m_movementInfo;
    moveInfo.ChangePosition(x, y, z, ang);

    MoveUpdateTeleport packet;
    packet.Status = MovementInfoToWire(moveInfo, unit->GetObjectGuid());
    WorldPacket data = MakeClassicPacket(packet);

    // not using SendMovementMessageToSet because we want the packet sent now
    unit->SendObjectMessageToSet(&data, true, mover);
}

void MovementPacketSender::SendKnockBackToController(Unit* unit, float vcos, float vsin, float speedXY, float speedZ)
{
    Player* mover = unit->GetPlayerMovingMe();
    if (!mover)
    {
        sLog.Out(LOG_MOVEMENT, LOG_LVL_ERROR, "MovementPacketSender::SendKnockBackToController: Incorrect use of the function. It was called on a unit controlled by the server! %s", unit->GetGuidStr().c_str());
        return;
    }

    uint32 mCounter = unit->GetMovementCounterAndInc();
    PlayerMovementPendingChange pendingChange;
    pendingChange.movementCounter = mCounter;
    pendingChange.movementChangeType = KNOCK_BACK;
    pendingChange.knockbackInfo.vcos = vcos;
    pendingChange.knockbackInfo.vsin = vsin;
    pendingChange.knockbackInfo.speedXY = speedXY;
    pendingChange.knockbackInfo.speedZ = speedZ;
    pendingChange.controller = mover->GetObjectGuid();
    unit->PushPendingMovementChange(pendingChange);

    MoveKnockBack packet;
    packet.Mover = ToClassic(unit);
    packet.SequenceIndex = mCounter;
    packet.DirectionX = vcos;
    packet.DirectionY = vsin;
    packet.HorizontalSpeed = speedXY;
    packet.VerticalSpeed = speedZ;

    mover->GetCheatData()->LogMovementPacket(false, MakeClassicPacket(packet));
    mover->GetSession()->SendClassic(packet);
}

void MovementPacketSender::SendKnockBackToObservers(Unit* unit, float vcos, float vsin, float speedXY, float speedZ)
{
    Player* mover = unit->GetPlayerMovingMe();
    if (!mover)
    {
        sLog.Out(LOG_MOVEMENT, LOG_LVL_ERROR, "MovementPacketSender::SendKnockBackToObservers: Incorrect use of the function. It was called on a unit controlled by the server! %s", unit->GetGuidStr().c_str());
        return;
    }

    // The knock back rides the jump data of the movement info.
    MovementInfo moveInfo = unit->m_movementInfo;
    moveInfo.AddMovementFlag(MOVEFLAG_JUMPING);
    moveInfo.jump.cosAngle = vcos;
    moveInfo.jump.sinAngle = vsin;
    moveInfo.jump.xyspeed = speedXY;
    moveInfo.jump.zspeed = speedZ;

    MoveUpdateKnockBack packet;
    packet.Status = MovementInfoToWire(moveInfo, unit->GetObjectGuid());
    unit->SendMovementMessageToSet(MakeClassicPacket(packet), true, mover);
}

void MovementPacketSender::AddMovementFlagChangeToController(Unit* unit, MovementFlags mFlag, bool apply)
{
    Player* mover = unit->GetPlayerMovingMe();
    if (!mover)
    {
        sLog.Out(LOG_MOVEMENT, LOG_LVL_ERROR, "MovementPacketSender::AddMovementFlagChangeToController: Incorrect use of the function. It was called on a unit controlled by the server! %s", unit->GetGuidStr().c_str());
        return;
    }

    MovementChangeType movementChangeType;
    switch (mFlag)
    {
        case MOVEFLAG_ROOT:                 movementChangeType = ROOT; break;
        case MOVEFLAG_WATERWALKING:         movementChangeType = WATER_WALK; break;
        case MOVEFLAG_HOVER:                movementChangeType = SET_HOVER; break;
        case MOVEFLAG_SAFE_FALL:            movementChangeType = FEATHER_FALL; break;
        default:
            sLog.Out(LOG_MOVEMENT, LOG_LVL_ERROR, "MovementPacketSender::AddMovementFlagChangeToController: Unsupported MovementFlag (%d), data not sent to client. %s", mFlag, unit->GetGuidStr().c_str());
            return;
    }

    uint32 mCounter = unit->GetMovementCounterAndInc();
    PlayerMovementPendingChange pendingChange;
    pendingChange.movementCounter = mCounter;
    pendingChange.movementChangeType = movementChangeType;
    pendingChange.apply = apply;
    pendingChange.controller = mover->GetObjectGuid();
    unit->PushPendingMovementChange(pendingChange);

    SendMovementFlagChangeToController(unit, mover, pendingChange);
}

void MovementPacketSender::SendMovementFlagChangeToController(Unit* unit, Player* mover, PlayerMovementPendingChange& pendingChange)
{
    MovementFlags flag;
    switch (pendingChange.movementChangeType)
    {
        case ROOT:          flag = MOVEFLAG_ROOT; break;
        case WATER_WALK:    flag = MOVEFLAG_WATERWALKING; break;
        case SET_HOVER:     flag = MOVEFLAG_HOVER; break;
        case FEATHER_FALL:  flag = MOVEFLAG_SAFE_FALL; break;
        default:
            sLog.Out(LOG_MOVEMENT, LOG_LVL_ERROR, "MovementPacketSender::SendMovementFlagChangeToController: Unsupported movement change (%u), data not sent to client. %s", pendingChange.movementChangeType, unit->GetGuidStr().c_str());
            return;
    }

    WithFlagMessages(flag, pendingChange.apply, [&](auto messages)
    {
        typename decltype(messages)::SetT packet;
        packet.Mover = ToClassic(unit);
        packet.SequenceIndex = pendingChange.movementCounter;

        mover->GetCheatData()->LogMovementPacket(false, MakeClassicPacket(packet));
        mover->GetSession()->SendClassic(packet);
    });
}

void MovementPacketSender::SendMovementFlagChangeToObservers(Unit* unit, MovementFlags mFlag, bool apply)
{
    Player* mover = unit->GetPlayerMovingMe();
    if (!mover)
    {
        sLog.Out(LOG_MOVEMENT, LOG_LVL_ERROR, "MovementPacketSender::SendMovementFlagChangeToObservers: Incorrect use of the function. It was called on a unit controlled by the server! %s", unit->GetGuidStr().c_str());
        return;
    }

    switch (mFlag)
    {
        case MOVEFLAG_ROOT:
        case MOVEFLAG_WATERWALKING:
        case MOVEFLAG_HOVER:
        case MOVEFLAG_SAFE_FALL:
            break;
        default:
            sLog.Out(LOG_MOVEMENT, LOG_LVL_ERROR, "MovementPacketSender::SendMovementFlagChangeToObservers: Unsupported MovementFlag (%d), data not sent to client. %s", mFlag, unit->GetGuidStr().c_str());
            return;
    }

    SendMoveUpdate(unit, mover);
}

void MovementPacketSender::SendMovementFlagChangeToAll(Unit* unit, MovementFlags mFlag, bool apply)
{
    bool sent = false;
    WithFlagMessages(mFlag, apply, [&](auto messages)
    {
        typename decltype(messages)::SplineT packet;
        packet.Mover = ToClassic(unit);
        unit->SendMovementMessageToSet(MakeClassicPacket(packet), true);
        sent = true;
    });

    if (!sent)
        sLog.Out(LOG_MOVEMENT, LOG_LVL_ERROR, "MovementPacketSender::SendMovementFlagChangeToAll: Unsupported MovementFlag (%d), data not sent to client. %s", mFlag, unit->GetGuidStr().c_str());
}

void MovementPacketSender::SendToggleRunWalkToAll(Unit* unit, bool run)
{
    if (run)
    {
        MoveSplineSetRunMode packet;
        packet.Mover = ToClassic(unit);
        unit->SendMovementMessageToSet(MakeClassicPacket(packet), true);
    }
    else
    {
        MoveSplineSetWalkMode packet;
        packet.Mover = ToClassic(unit);
        unit->SendMovementMessageToSet(MakeClassicPacket(packet), true);
    }
}
