/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2011 MaNGOSZero <https://github.com/mangos/zero>
 * Copyright (C) 2011-2016 Nostalrius <https://nostalrius.org>
 * Copyright (C) 2016-2017 Elysium Project <https://github.com/elysium-project>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "MapManager.h"
#include "Transport.h"
#include "BattleGround.h"
#include "WaypointMovementGenerator.h"
#include "ConfusedMovementGenerator.h"
#include "MapPersistentStateMgr.h"
#include "World.h"
#include "Anticheat.h"
#include "packet_builder.h"
#include "MovementPacketSender.h"
#include "MoveSpline.h"
#include "Geometry.h"
#include "MovementInfoWire.h"
#include "Packets/Classic/GuidMapping.h"
#include "VMapFactory.h"

void WorldSession::HandleMoveWorldportAckOpcode(WorldPortResponse const& /*packet*/)
{
    HandleMoveWorldportAck();
}

void WorldSession::HandleMoveWorldportAck()
{
    // ignore unexpected far teleports
    if (!GetPlayer()->IsBeingTeleportedFar())
        return;

    // get start teleport coordinates (will used later in fail case)
    WorldLocation oldLoc;
    GetPlayer()->GetPosition(oldLoc);

    // get the teleport destination
    WorldLocation &loc = GetPlayer()->GetTeleportDest();

    // possible errors in the coordinate validity check (only cheating case possible)
    if (!MapManager::IsValidMapCoord(loc))
    {
        sLog.Player(this, LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleMoveWorldportAck: Teleported far to a not valid location "
                      "(map: %u, x: %g, y: %g, z: %g). Porting to homebind instead.",
                      loc.mapId, loc.x, loc.y, loc.z);
        // stop teleportation else we would try this again and again in LogoutPlayer...
        GetPlayer()->SetSemaphoreTeleportFar(false);
        // and teleport the player to a valid place
        GetPlayer()->TeleportToHomebind();
        return;
    }

    // get the destination map entry, not the current one, this will fix homebind and reset greeting
    MapEntry const* mEntry = sMapStorage.LookupEntry<MapEntry>(loc.mapId);

    Map* map = nullptr;

    // prevent crash at attempt landing to not existed battleground instance
    if (mEntry->IsBattleGround())
    {
        if (GetPlayer()->GetBattleGroundId())
            map = sMapMgr.FindMap(loc.mapId, GetPlayer()->GetBattleGroundId());

        if (!map)
        {
            sLog.Player(this, LOG_MOVEMENT, LOG_LVL_DETAIL, "WorldSession::HandleMoveWorldportAck: Teleported far to nonexistent battleground instance "
                       " (map: %u, x: %g, y: %g, z: %g). Trying to port player to previous location.",
                       loc.mapId, loc.x, loc.y, loc.z);

            GetPlayer()->HandleReturnOnTeleportFail(oldLoc);
            return;
        }
    }

    // reset instance validity, except if going to an instance inside an instance
    if (!GetPlayer()->m_instanceValid && !mEntry->IsDungeon())
        GetPlayer()->m_instanceValid = true;

    // relocate the player to the teleport destination
    if (!map)
    {
        if (loc.mapId <= MAX_CONTINENT_ID)
            GetPlayer()->SetLocationInstanceId(sMapMgr.GetContinentInstanceId(loc.mapId, loc.x, loc.y));
        map = sMapMgr.CreateMap(loc.mapId, GetPlayer());
    }

    GetPlayer()->SetMap(map);
    if (GenericTransport* t = GetPlayer()->GetTransport()) // Transport position may have changed while loading
        t->UpdatePassengerPosition(GetPlayer());
    else
        GetPlayer()->Relocate(loc.x, loc.y, loc.z, loc.o);

    GetPlayer()->SendInitialPacketsBeforeAddToMap();
    // the CanEnter checks are done in TeleporTo but conditions may change
    // while the player is in transit, for example the map may get full
    if (!GetPlayer()->GetMap()->Add(GetPlayer()))
    {
        sLog.Player(this, LOG_MOVEMENT, LOG_LVL_DETAIL, "WorldSession::HandleMoveWorldportAckOpcode: Teleported far but couldn't be added to map "
                   " (map: %u, x: %g, y: %g, z: %g). Trying to port player to previous location.",
                   loc.mapId, loc.x, loc.y, loc.z);

        GetPlayer()->HandleReturnOnTeleportFail(oldLoc);
        return;
    }
    GetPlayer()->SetSemaphoreTeleportFar(false);

    // battleground state prepare (in case join to BG), at relogin/tele player not invited
    // only add to bg group and object, if the player was invited (else he entered through command)
    if (_player->InBattleGround())
    {
        // cleanup seting if outdated
        if (!mEntry->IsBattleGround())
        {
            // We're not in BG
            _player->SetBattleGroundId(0, BATTLEGROUND_TYPE_NONE);
            // reset destination bg team
            _player->SetBGTeam(TEAM_NONE);
        }
        // join to bg case
        else if (BattleGround *bg = _player->GetBattleGround())
        {
            if (_player->IsInvitedForBattleGroundInstance(_player->GetBattleGroundId()))
                bg->AddPlayer(_player);
        }
    }

    GetPlayer()->SendInitialPacketsAfterAddToMap();

    // flight fast teleport case
    if (GetPlayer()->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE)
    {
        // the movement could be expiring so check for destination (ritual of summoning / gm teleport)
        if (!_player->InBattleGround() && !_player->m_taxi.empty())
        {
            // short preparations to continue flight
            FlightPathMovementGenerator* flight = (FlightPathMovementGenerator*)(GetPlayer()->GetMotionMaster()->top());
            flight->Reset(*GetPlayer());
            return;
        }

        // battleground state prepare, stop flight
        GetPlayer()->GetMotionMaster()->MovementExpired();
        GetPlayer()->m_taxi.ClearTaxiDestinations();
    }

    if (mEntry->IsRaid())
    {
        time_t timeReset = 0;
        if (DungeonResetScheduler::IsRaidResetSchedulingGlobal())
            timeReset = sMapPersistentStateMgr.GetScheduler().GetResetTimeFor(mEntry->id);
        // before 1.9 each raid instance has its own reset timer
        else if (DungeonPersistentState* state = dynamic_cast<DungeonPersistentState*>(GetPlayer()->GetMap()->GetPersistentState()))
            timeReset = state->GetResetTime();

        if (timeReset)
        {
            uint32 timeleft = uint32(timeReset - time(nullptr));
            GetPlayer()->SendInstanceResetWarning(mEntry->id, timeleft);
        }
    }
    else if (!mEntry->IsDungeon())
    {
        MapEntry const* oldMapEntry = sMapStorage.LookupEntry<MapEntry>(oldLoc.mapId);
        if (oldMapEntry->IsDungeon())
            GetPlayer()->ResetPersonalInstanceOnLeaveDungeon(oldLoc.mapId);
    }

    GetPlayer()->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_ENTER_WORLD_CANCELS);

    // mount allow check
    if (!mEntry->IsMountAllowed())
        _player->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

    // honorless target
    if (!GetPlayer()->pvpInfo.inPvPEnforcedArea)
        GetPlayer()->RemoveDelayedOperation(DELAYED_CAST_HONORLESS_TARGET);

    // resummon pet
    GetPlayer()->ResummonPetTemporaryUnSummonedIfAny();

    // stop drowning if not in water anymore
    GetPlayer()->UpdateTerainEnvironmentFlags();

    //lets process all delayed operations on successful teleport
    GetPlayer()->ProcessDelayedOperations();

    // Let the client know its new position by sending a heartbeat!
    // The Windows client figures this out by itself, but the MacOS one does
    // not.
    //
    // On a successful port, the camera of the MacOS client is facing south and
    // ignores any movement from the transport object. Triggering
    // `SMSG_STANDSTATE_UPDATE' with its current state resets the camera
    // (implemented in `WorldSession::HandleZoneUpdateOpcode').
    if (m_clientOS == CLIENT_OS_MAC && GetPlayer()->m_movementInfo.HasMovementFlag(MOVEFLAG_ONTRANSPORT))
    {
        GetPlayer()->SendHeartBeat(true);
    }
}

void WorldSession::HandleMoveTeleportAckOpcode(MoveTeleportAck const& packet)
{
    uint32 movementCounter = uint32(packet.AckIndex);

    Unit* pMover = _player->GetMover();
    Player* pPlayerMover = pMover->ToPlayer();

    if (!pPlayerMover || !pPlayerMover->IsBeingTeleportedNear())
        return;

    auto const moverGuid = ClassicGuidMapping::FromClassic(packet.Mover);
    if (!moverGuid || *moverGuid != pPlayerMover->GetObjectGuid())
        return;

    if (!pMover->FindPendingMovementTeleportChange(movementCounter))
    {
        sLog.Player(this, LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleMoveTeleportAck: Client sent MSG_MOVE_TELEPORT_ACK with counter %u, but no pending teleport found (current counter is %u).",
            movementCounter, pMover->GetMovementCounter());
    }

    pPlayerMover->SetSplineDonePending(false);
    pPlayerMover->ExecuteTeleportNear();
}

void Player::ExecuteTeleportNear()
{
    if (!IsBeingTeleportedNear())
    {
        sLog.Player(GetSession(), LOG_MOVEMENT, LOG_LVL_ERROR, "Player::ExecuteNearTeleport called without near teleport scheduled!");
        return;
    }

    SetSemaphoreTeleportNear(false);

    // send MSG_MOVE_TELEPORT to observers around both old and new position
    WorldLocation const& dest = GetTeleportDest();
    MovementPacketSender::SendTeleportToObservers(this, dest.x, dest.y, dest.z, dest.o);
    m_movementInfo.UpdateTime(WorldTimer::getMSTime());
    TeleportPositionRelocation(dest);
    MovementPacketSender::SendTeleportToObservers(this, dest.x, dest.y, dest.z, dest.o);

    // if player is sheeped make him wander around new position
    if (GetMotionMaster()->GetCurrentMovementGeneratorType() == CONFUSED_MOTION_TYPE)
    {
        ConfusedMovementGenerator<Player>* confuse = (ConfusedMovementGenerator<Player>*)(GetMotionMaster()->top());
        confuse->SetStartPosition(dest.x, dest.y, dest.z);
    }

    // resummon pet, if the destination is in another continent instance, let Player::SwitchInstance do it
    // because the client will request the name for the old pet guid and receive no answer
    // result would be a pet named "unknown"
    if (GetTemporaryUnsummonedPetNumber())
    {
        if (sWorld.getConfig(CONFIG_BOOL_CONTINENTS_INSTANCIATE) && GetMap()->IsContinent())
        {
            bool transition = false;
            if (sMapMgr.GetContinentInstanceId(GetMap()->GetId(), dest.x, dest.y, &transition) == GetInstanceId())
                ResummonPetTemporaryUnSummonedIfAny();
        }
        else
            ResummonPetTemporaryUnSummonedIfAny();
    }

    // lets process all delayed operations on successful teleport
    ProcessDelayedOperations();
}

// The movement anticheat and fall tracking still key on the 1.12 movement opcode numbers.
static uint32 ToInternalMovementOpcode(ClassicOpcodes opcode)
{
    switch (opcode)
    {
        case CLASSIC_CMSG_MOVE_START_FORWARD:       return MSG_MOVE_START_FORWARD;
        case CLASSIC_CMSG_MOVE_START_BACKWARD:      return MSG_MOVE_START_BACKWARD;
        case CLASSIC_CMSG_MOVE_STOP:                return MSG_MOVE_STOP;
        case CLASSIC_CMSG_MOVE_START_STRAFE_LEFT:   return MSG_MOVE_START_STRAFE_LEFT;
        case CLASSIC_CMSG_MOVE_START_STRAFE_RIGHT:  return MSG_MOVE_START_STRAFE_RIGHT;
        case CLASSIC_CMSG_MOVE_STOP_STRAFE:         return MSG_MOVE_STOP_STRAFE;
        case CLASSIC_CMSG_MOVE_JUMP:                return MSG_MOVE_JUMP;
        case CLASSIC_CMSG_MOVE_START_TURN_LEFT:     return MSG_MOVE_START_TURN_LEFT;
        case CLASSIC_CMSG_MOVE_START_TURN_RIGHT:    return MSG_MOVE_START_TURN_RIGHT;
        case CLASSIC_CMSG_MOVE_STOP_TURN:           return MSG_MOVE_STOP_TURN;
        case CLASSIC_CMSG_MOVE_START_PITCH_UP:      return MSG_MOVE_START_PITCH_UP;
        case CLASSIC_CMSG_MOVE_START_PITCH_DOWN:    return MSG_MOVE_START_PITCH_DOWN;
        case CLASSIC_CMSG_MOVE_STOP_PITCH:          return MSG_MOVE_STOP_PITCH;
        case CLASSIC_CMSG_MOVE_SET_RUN_MODE:        return MSG_MOVE_SET_RUN_MODE;
        case CLASSIC_CMSG_MOVE_SET_WALK_MODE:       return MSG_MOVE_SET_WALK_MODE;
        case CLASSIC_CMSG_MOVE_FALL_LAND:           return MSG_MOVE_FALL_LAND;
        case CLASSIC_CMSG_MOVE_FALL_RESET:          return CMSG_MOVE_FALL_RESET;
        case CLASSIC_CMSG_MOVE_START_SWIM:          return MSG_MOVE_START_SWIM;
        case CLASSIC_CMSG_MOVE_STOP_SWIM:           return MSG_MOVE_STOP_SWIM;
        case CLASSIC_CMSG_MOVE_SET_FACING:          return MSG_MOVE_SET_FACING;
        case CLASSIC_CMSG_MOVE_SET_PITCH:           return MSG_MOVE_SET_PITCH;
        case CLASSIC_CMSG_MOVE_FORCE_WALK_SPEED_CHANGE_ACK:     return CMSG_FORCE_WALK_SPEED_CHANGE_ACK;
        case CLASSIC_CMSG_MOVE_FORCE_RUN_SPEED_CHANGE_ACK:      return CMSG_FORCE_RUN_SPEED_CHANGE_ACK;
        case CLASSIC_CMSG_MOVE_FORCE_RUN_BACK_SPEED_CHANGE_ACK: return CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK;
        case CLASSIC_CMSG_MOVE_FORCE_SWIM_SPEED_CHANGE_ACK:     return CMSG_FORCE_SWIM_SPEED_CHANGE_ACK;
        case CLASSIC_CMSG_MOVE_FORCE_SWIM_BACK_SPEED_CHANGE_ACK: return CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK;
        case CLASSIC_CMSG_MOVE_FORCE_TURN_RATE_CHANGE_ACK:      return CMSG_FORCE_TURN_RATE_CHANGE_ACK;
        case CLASSIC_CMSG_MOVE_FORCE_FLIGHT_SPEED_CHANGE_ACK:   return CMSG_FORCE_RUN_SPEED_CHANGE_ACK;
        case CLASSIC_CMSG_MOVE_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK: return CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK;
        case CLASSIC_CMSG_MOVE_FORCE_ROOT_ACK:      return CMSG_FORCE_MOVE_ROOT_ACK;
        case CLASSIC_CMSG_MOVE_FORCE_UNROOT_ACK:    return CMSG_FORCE_MOVE_UNROOT_ACK;
        case CLASSIC_CMSG_MOVE_WATER_WALK_ACK:      return CMSG_MOVE_WATER_WALK_ACK;
        case CLASSIC_CMSG_MOVE_HOVER_ACK:           return CMSG_MOVE_HOVER_ACK;
        case CLASSIC_CMSG_MOVE_FEATHER_FALL_ACK:    return CMSG_MOVE_FEATHER_FALL_ACK;
        case CLASSIC_CMSG_MOVE_KNOCK_BACK_ACK:      return CMSG_MOVE_KNOCK_BACK_ACK;
        case CLASSIC_CMSG_MOVE_SPLINE_DONE:         return CMSG_MOVE_SPLINE_DONE;
        case CLASSIC_CMSG_MOVE_TELEPORT_ACK:        return MSG_MOVE_TELEPORT_ACK;
        default:                                    return MSG_MOVE_HEARTBEAT;
    }
}

void WorldSession::HandleClassicMovement(ClassicOpcodes classicOpcode, ClassicMovementInfo const& status)
{
    // The mover's own guid rides the block, but the session's confirmed mover is authoritative.
    MovementInfo movementInfo;
    if (!MovementInfoFromWire(status, movementInfo))
        return;

    HandleMovementOpcodes(ToInternalMovementOpcode(classicOpcode), movementInfo);
}

void WorldSession::HandleMovementOpcodes(uint32 opcode, MovementInfo& movementInfo)
{
    // Do not accept packets sent before this time.
    if (movementInfo.stime <= m_moveRejectTime)
        return;

    Unit* pMover = _player->GetConfirmedMover();
    if (!pMover)
        return;

    if (pMover->HasPendingSplineDone())
        return;

    // currently being moved by server
    if (!pMover->movespline->Finalized())
        return;

    Player* pPlayerMover = pMover->ToPlayer();

    // ignore, waiting processing in WorldSession::HandleMoveWorldportAckOpcode and WorldSession::HandleMoveTeleportAck
    if (pPlayerMover && pPlayerMover->IsBeingTeleported())
        return;

    if (!VerifyMovementInfo(movementInfo))
        return;

    if (pPlayerMover)
    {
        if ((m_moveRejectTime = _player->GetCheatData()->HandleFlagTests(pPlayerMover, movementInfo, opcode)) ||
            (m_moveRejectTime = _player->GetCheatData()->HandlePositionTests(pPlayerMover, movementInfo, opcode)))
        {
            return;
        }
    }

    // This is required for proper movement extrapolation
    if (opcode == MSG_MOVE_JUMP)
        pMover->SetJumpInitialSpeed(7.95797334f);
    else if (opcode == MSG_MOVE_FALL_LAND)
        pMover->SetJumpInitialSpeed(-9.645f);

    // fall damage generation (ignore in flight case that can be triggered also at lags in moment teleportation to another map).
    if (opcode == MSG_MOVE_FALL_LAND && pPlayerMover && !pPlayerMover->IsTaxiFlying())
        pPlayerMover->HandleFall(movementInfo);

    // TODO: remove it
    // reset knockback state when fall to ground or water
    if (pPlayerMover)
    {
        if (IsFallEndOpcode(opcode) && pPlayerMover->IsLaunched())
            pPlayerMover->SetLaunched(false);

        pPlayerMover->UpdateFallInformationIfNeed(movementInfo, opcode);
    }

    HandleMoverRelocation(pMover, movementInfo);

    // this is here to accommodate 1.14 client behavior
    // it does not interrupt falling when rooted
    // verify that root is applied after having landed
    if (pMover->HasUnitState(UNIT_STATE_ROOT_ON_LANDING))
    {
        if (movementInfo.HasMovementFlag(MOVEFLAG_ROOT) || !pMover->ShouldBeRooted())
            pMover->ClearUnitState(UNIT_STATE_ROOT_ON_LANDING);
        else if (!movementInfo.HasMovementFlag(MOVEFLAG_JUMPING | MOVEFLAG_FALLINGFAR))
        {
            sLog.Player(this, LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleMovementOpcodes: Has pending root on landing, but sent movement packet with opcode %u not containing root or falling flags!",
                opcode);
            pMover->ClearUnitState(UNIT_STATE_ROOT_ON_LANDING);
            pMover->SetRootedReal(true);
            KickPlayer();
            return;
        }
    }

    // CMSG opcode has no handler in client, should not be sent to others.
    // It is sent by client when you jump and hit something on the way up,
    // thus stopping upward movement and causing you to descend sooner.
    if (opcode == CMSG_MOVE_FALL_RESET)
        return;

    MoveUpdate update;
    update.Status = MovementInfoToWire(movementInfo, pMover->GetObjectGuid());

    pMover->SendMovementMessageToSet(MakeClassicPacket(update), true, _player);
}

/*
handles those packets:
CMSG_FORCE_RUN_SPEED_CHANGE_ACK
CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK
CMSG_FORCE_SWIM_SPEED_CHANGE_ACK
CMSG_FORCE_WALK_SPEED_CHANGE_ACK
CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK
CMSG_FORCE_TURN_RATE_CHANGE_ACK
CMSG_MOVE_FORCE_FLIGHT_SPEED_CHANGE_ACK
CMSG_MOVE_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK
*/
void WorldSession::HandleForceSpeedChangeAck(ClassicOpcodes classicOpcode, ClassicMovementAck const& ack, float newSpeed)
{
    uint32 const opcode = ToInternalMovementOpcode(classicOpcode);
    uint32 const movementCounter = uint32(ack.AckIndex);

    MovementInfo movementInfo;
    auto const moverGuid = ClassicGuidMapping::FromClassic(ack.Status.Guid);
    if (!moverGuid || !MovementInfoFromWire(ack.Status, movementInfo))
        return;

    UnitMoveType move_type;
    switch (classicOpcode)
    {
        case CLASSIC_CMSG_MOVE_FORCE_RUN_SPEED_CHANGE_ACK:
            move_type = MOVE_RUN;
            break;
        case CLASSIC_CMSG_MOVE_FORCE_RUN_BACK_SPEED_CHANGE_ACK:
            move_type = MOVE_RUN_BACK;
            break;
        case CLASSIC_CMSG_MOVE_FORCE_SWIM_SPEED_CHANGE_ACK:
            move_type = MOVE_SWIM;
            break;
        case CLASSIC_CMSG_MOVE_FORCE_WALK_SPEED_CHANGE_ACK:
            move_type = MOVE_WALK;
            break;
        case CLASSIC_CMSG_MOVE_FORCE_SWIM_BACK_SPEED_CHANGE_ACK:
            move_type = MOVE_SWIM_BACK;
            break;
        case CLASSIC_CMSG_MOVE_FORCE_TURN_RATE_CHANGE_ACK:
            move_type = MOVE_TURN_RATE;
            break;
        case CLASSIC_CMSG_MOVE_FORCE_FLIGHT_SPEED_CHANGE_ACK:
            move_type = MOVE_FLIGHT;
            break;
        case CLASSIC_CMSG_MOVE_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK:
            move_type = MOVE_FLIGHT_BACK;
            break;
        default:
            sLog.Out(LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleForceSpeedChangeAck: Unknown move type opcode: %u", uint32(classicOpcode));
            return;
    }

    Unit* pMover = GetMoverFromGuid(*moverGuid);
    if (!pMover)
        return;

    // verify that indeed the client is replying with the changes that were send to him
    if (!pMover->HasPendingMovementChange())
    {
        sLog.Player(this, LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleForceSpeedChangeAck: Client sent opcode %u with counter %u, but no movement change ack was expected from this player (current counter is %u).",
            opcode, movementCounter, pMover->GetMovementCounter());
        if (movementCounter == 0 || movementCounter > pMover->GetMovementCounter())
            _player->GetCheatData()->OnWrongAckData();
        return;
    }

    if (!pMover->FindPendingMovementSpeedChange(newSpeed, movementCounter, move_type))
    {
        sLog.Player(this, LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleForceSpeedChangeAck: Client sent opcode %u with counter %u, but received data does not match pending change (current counter is %u).",
            opcode, movementCounter, pMover->GetMovementCounter());
        if (movementCounter == 0 || movementCounter > pMover->GetMovementCounter())
            _player->GetCheatData()->OnWrongAckData();
        return;
    }

    Player* const pPlayerMover = pMover->ToPlayer();

    // Check if position and movement flags are fine before speed update.
    bool canRelocate = movementInfo.stime > m_moveRejectTime && !pMover->HasPendingSplineDone() && VerifyMovementInfo(movementInfo);
    if (canRelocate && pPlayerMover)
    {
        if ((m_moveRejectTime = _player->GetCheatData()->HandleFlagTests(pPlayerMover, movementInfo, opcode)) ||
            (m_moveRejectTime = _player->GetCheatData()->HandlePositionTests(pPlayerMover, movementInfo, opcode)))
        {
            canRelocate = false;
        }
    }

    // the speed has to be applied before relocation
    // otherwise if the speed update was from an outdoors only aura
    // and after relocation we are now indoors, player will get
    // stuck with the faster speed from the aura after removal
    // because the speed never changed server side
    float const newSpeedRate = newSpeed / baseMoveSpeed[move_type];
    pMover->SetSpeedRateReal(move_type, newSpeedRate);

    if (canRelocate)
    {
        if ((pMover == _player->GetMover()) &&
            (!pPlayerMover || !pPlayerMover->IsBeingTeleported()))
        {
            if (pPlayerMover)
                pPlayerMover->UpdateFallInformationIfNeed(movementInfo, opcode);

            // Update position if it has changed.
            HandleMoverRelocation(pMover, movementInfo);
        }
        else
        {
            // Can only change flags and speed for not current active mover.
            pMover->m_movementInfo.moveFlags = movementInfo.moveFlags;
            pMover->m_movementInfo.CorrectData();
        }
    }

    // send the speed change to others (with updated position if all is fine)
    MovementPacketSender::SendSpeedChangeToObservers(pMover, move_type, newSpeed);
}

/*
handles those packets:
CMSG_MOVE_WATER_WALK_ACK
CMSG_MOVE_HOVER_ACK
CMSG_MOVE_FEATHER_FALL_ACK
*/
void WorldSession::HandleMovementFlagChangeToggleAck(ClassicOpcodes classicOpcode, ClassicMovementAck const& ack)
{
    uint32 const opcode = ToInternalMovementOpcode(classicOpcode);
    uint32 const movementCounter = uint32(ack.AckIndex);

    MovementInfo movementInfo;
    auto const moverGuid = ClassicGuidMapping::FromClassic(ack.Status.Guid);
    if (!moverGuid || !MovementInfoFromWire(ack.Status, movementInfo))
        return;

    // The client reports the change in the flags it now moves with.
    MovementFlags mFlag = MOVEFLAG_NONE;
    switch (classicOpcode)
    {
        case CLASSIC_CMSG_MOVE_WATER_WALK_ACK:      mFlag = MOVEFLAG_WATERWALKING; break;
        case CLASSIC_CMSG_MOVE_HOVER_ACK:           mFlag = MOVEFLAG_HOVER; break;
        case CLASSIC_CMSG_MOVE_FEATHER_FALL_ACK:    mFlag = MOVEFLAG_SAFE_FALL; break;
        default:
            sLog.Out(LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleMovementFlagChangeToggleAck: Unknown move type opcode: %u", uint32(classicOpcode));
            return;
    }
    bool const applyReceived = movementInfo.HasMovementFlag(mFlag);

    Unit* pMover = GetMoverFromGuid(*moverGuid);
    if (!pMover)
        return;

    // verify that indeed the client is replying with the changes that were send to him
    if (!pMover->HasPendingMovementChange())
    {
        sLog.Player(this, LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleMovementFlagChangeToggleAck: Client sent opcode %u with counter %u, but no movement change ack was expected from this player (current counter is %u).",
            opcode, movementCounter, pMover->GetMovementCounter());
        if (movementCounter == 0 || movementCounter > pMover->GetMovementCounter())
            _player->GetCheatData()->OnWrongAckData();
        return;
    }

    MovementChangeType changeTypeReceived;
    switch (mFlag)
    {
        case MOVEFLAG_WATERWALKING: changeTypeReceived = WATER_WALK; break;
        case MOVEFLAG_HOVER:        changeTypeReceived = SET_HOVER; break;
        default:                    changeTypeReceived = FEATHER_FALL; break;
    }

    if (!pMover->FindPendingMovementFlagChange(movementCounter, applyReceived, changeTypeReceived))
    {
        sLog.Player(this, LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleMovementFlagChangeToggleAck: Client sent opcode %u with counter %u, but received data does not match pending change (current counter is %u).",
            opcode, movementCounter, pMover->GetMovementCounter());
        if (movementCounter == 0 || movementCounter > pMover->GetMovementCounter())
            _player->GetCheatData()->OnWrongAckData();
        return;
    }

    if (mFlag == MOVEFLAG_SAFE_FALL)
        pMover->SetJumpInitialSpeed(std::max(pMover->GetJumpInitialSpeed(), 7.0f));

    // Use fake loop here to handle movement position checks separately from change ACK.
    do
    {
        if (pMover->HasPendingSplineDone())
            break;

        // Do not accept packets sent before this time.
        if (movementInfo.stime <= m_moveRejectTime)
            break;

        if (!VerifyMovementInfo(movementInfo))
            break;

        Player* pPlayerMover = pMover->ToPlayer();

        if (pPlayerMover)
        {
            if ((m_moveRejectTime = _player->GetCheatData()->HandleFlagTests(pPlayerMover, movementInfo, opcode)) ||
                (m_moveRejectTime = _player->GetCheatData()->HandlePositionTests(pPlayerMover, movementInfo, opcode)))
            {
                break;
            }
        }

        if ((pMover == _player->GetMover()) &&
            (!pPlayerMover || !pPlayerMover->IsBeingTeleported()))
        {
            if (pPlayerMover)
                pPlayerMover->UpdateFallInformationIfNeed(movementInfo, opcode);

            // Update position if it has changed.
            HandleMoverRelocation(pMover, movementInfo);
        }
        else
        {
            // Can only change flags and speed for not current active mover.
            pMover->m_movementInfo.moveFlags = movementInfo.moveFlags;
            pMover->m_movementInfo.CorrectData();
        }
    } while (false);

    switch (changeTypeReceived)
    {
        case WATER_WALK:            pMover->SetWaterWalkingReal(applyReceived); break;
        case SET_HOVER:             pMover->SetHoverReal(applyReceived); break;
        case FEATHER_FALL:          pMover->SetFeatherFallReal(applyReceived); break;
        default:
            sLog.Out(LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleMovementFlagChangeToggleAck: Unknown move type opcode: %u", opcode);
            return;
    }

    if (mFlag != MOVEFLAG_NONE)
        MovementPacketSender::SendMovementFlagChangeToObservers(pMover, mFlag, applyReceived);
}

/*
handles those packets:
CMSG_FORCE_MOVE_ROOT_ACK
CMSG_FORCE_MOVE_UNROOT_ACK
*/
void WorldSession::HandleMoveRootAck(ClassicOpcodes classicOpcode, ClassicMovementAck const& ack)
{
    uint32 const opcode = ToInternalMovementOpcode(classicOpcode);
    uint32 const movementCounter = uint32(ack.AckIndex);

    MovementInfo movementInfo;
    auto const moverGuid = ClassicGuidMapping::FromClassic(ack.Status.Guid);
    if (!moverGuid || !MovementInfoFromWire(ack.Status, movementInfo))
        return;

    Unit* pMover = GetMoverFromGuid(*moverGuid);
    if (!pMover)
        return;

    // verify that indeed the client is replying with the changes that were send to him
    if (!pMover->HasPendingMovementChange())
    {
        sLog.Player(this, LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleMoveRootAck: Client sent opcode %u with counter %u, but no movement change ack was expected from this player (current counter is %u).",
            opcode, movementCounter, pMover->GetMovementCounter());
        if (movementCounter == 0 || movementCounter > pMover->GetMovementCounter())
            _player->GetCheatData()->OnWrongAckData();
        return;
    }

    bool applyReceived = (classicOpcode == CLASSIC_CMSG_MOVE_FORCE_ROOT_ACK);

    if (!pMover->FindPendingMovementRootChange(movementCounter, applyReceived))
    {
        sLog.Player(this, LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleMoveRootAck: Client sent opcode %u with counter %u, but received data does not match pending change (current counter is %u).",
            opcode, movementCounter, pMover->GetMovementCounter());
        if (movementCounter == 0 || movementCounter > pMover->GetMovementCounter())
            _player->GetCheatData()->OnWrongAckData();
        return;
    }

    // Use fake loop here to handle movement position checks separately from change ACK.
    do
    {
        if (pMover->HasPendingSplineDone())
            break;

        // Do not accept packets sent before this time.
        if (movementInfo.stime <= m_moveRejectTime)
            break;

        if (!VerifyMovementInfo(movementInfo))
            break;

        Player* pPlayerMover = pMover->ToPlayer();

        if (pPlayerMover)
        {
            if ((m_moveRejectTime = _player->GetCheatData()->HandleFlagTests(pPlayerMover, movementInfo, opcode)) ||
                (m_moveRejectTime = _player->GetCheatData()->HandlePositionTests(pPlayerMover, movementInfo, opcode)))
            {
                break;
            }
        }

        if ((pMover == _player->GetMover()) &&
            (!pPlayerMover || !pPlayerMover->IsBeingTeleported()))
        {
            if (pPlayerMover)
                pPlayerMover->UpdateFallInformationIfNeed(movementInfo, opcode);

            // Update position if it has changed.
            HandleMoverRelocation(pMover, movementInfo);
        }
        else
        {
            // Can only change flags and speed for not current active mover.
            pMover->m_movementInfo.moveFlags = movementInfo.moveFlags;
            pMover->m_movementInfo.CorrectData();
        }
    } while (false);

    if (applyReceived && !movementInfo.HasMovementFlag(MOVEFLAG_ROOT))
    {
        // workaround to fix anticheat false positives when using 1.14 client
        // modern client finishes falling to ground before applying the root
        if (!movementInfo.HasMovementFlag(MOVEFLAG_JUMPING | MOVEFLAG_FALLINGFAR))
        {
            sLog.Player(this, LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleMoveRootAck: Client sent root apply ack, but movement info does not have rooted movement flag!");
            KickPlayer();
        }
        else
        {
            sLog.Player(this, LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleMoveRootAck: Client sent root apply ack, but continues falling. Using 1.14 client?");
            pMover->AddUnitState(UNIT_STATE_ROOT_ON_LANDING);
            return;
        }
    }

    // we need to always clear this on root packet for 1.14
    pMover->ClearUnitState(UNIT_STATE_ROOT_ON_LANDING);

    pMover->SetRootedReal(applyReceived);
    MovementPacketSender::SendMovementFlagChangeToObservers(pMover, MOVEFLAG_ROOT, applyReceived);
}

void WorldSession::HandleMoveKnockBackAck(MoveKnockBackAck const& packet)
{
    uint32 const movementCounter = uint32(packet.Ack.AckIndex);

    MovementInfo movementInfo;
    auto const moverGuid = ClassicGuidMapping::FromClassic(packet.Ack.Status.Guid);
    if (!moverGuid || !MovementInfoFromWire(packet.Ack.Status, movementInfo))
        return;

    uint32 const opcode = ToInternalMovementOpcode(packet.Opcode);

    Unit* pMover = GetMoverFromGuid(*moverGuid);
    if (!pMover)
        return;

    // verify that indeed the client is replying with the changes that were send to him
    if (!pMover->HasPendingMovementChange())
    {
        sLog.Player(this, LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleMoveKnockBackAck: Client sent opcode %u with counter %u, but no movement change ack was expected from this player (current counter is %u).",
            opcode, movementCounter, pMover->GetMovementCounter());
        if (movementCounter == 0 || movementCounter > pMover->GetMovementCounter())
            _player->GetCheatData()->OnWrongAckData();
        return;
    }

    if (!pMover->FindPendingMovementKnockbackChange(movementInfo, movementCounter))
    {
        sLog.Player(this, LOG_MOVEMENT, LOG_LVL_ERROR, "WorldSession::HandleMoveKnockBackAck: Client sent opcode %u with counter %u, but received data does not match pending change (current counter is %u).",
            opcode, movementCounter, pMover->GetMovementCounter());
        if (movementCounter == 0 || movementCounter > pMover->GetMovementCounter())
            _player->GetCheatData()->OnWrongAckData();
        return;
    }

    if (pMover->HasPendingSplineDone())
        return;

    if (!VerifyMovementInfo(movementInfo))
        return;

    // ignore, waiting processing in WorldSession::HandleMoveWorldportAckOpcode and WorldSession::HandleMoveTeleportAck
    if (pMover->IsPlayer() && static_cast<Player*>(pMover)->IsBeingTeleported())
        return;

    if (Player* pPlayerMover = pMover->ToPlayer())
    {
        if ((m_moveRejectTime = _player->GetCheatData()->HandleFlagTests(pPlayerMover, movementInfo, opcode)) ||
            (m_moveRejectTime = _player->GetCheatData()->HandlePositionTests(pPlayerMover, movementInfo, opcode)))
        {
            return;
        }

        pPlayerMover->SetFallInformation(0);
    }

    HandleMoverRelocation(pMover, movementInfo);

    MovementPacketSender::SendKnockBackToObservers(pMover, movementInfo.jump.cosAngle, movementInfo.jump.sinAngle, movementInfo.jump.xyspeed, movementInfo.jump.zspeed);
}

void WorldSession::HandleMoveSplineDoneOpcode(MoveSplineDone const& packet)
{
    MovementInfo movementInfo;
    if (!MovementInfoFromWire(packet.Status, movementInfo))
        return;

    if (!VerifyMovementInfo(movementInfo))
        return;

    Unit* pMover = _player->GetMover();

    if (pMover->GetObjectGuid() != m_clientMoverGuid)
        return;

    if (pMover->movespline->GetId() != uint32(packet.SplineId))
        return;

    // must be after checking this is the newest spline id
    if (pMover->HasPendingSplineDone())
        pMover->SetSplineDonePending(false);
    else
        return;

    if (Player* pPlayerMover = pMover->ToPlayer())
    {
        // ignore, waiting processing in WorldSession::HandleMoveWorldportAckOpcode and WorldSession::HandleMoveTeleportAck
        if (pPlayerMover->IsBeingTeleported())
            return;

        // no need to reject future packets in this case
        if (!_player->GetCheatData()->HandleSplineDone(pPlayerMover, movementInfo, uint32(packet.SplineId)))
            return;

        if (m_moveRejectTime = _player->GetCheatData()->HandleFlagTests(pPlayerMover, movementInfo, CMSG_MOVE_SPLINE_DONE))
            return;
    }

    HandleMoverRelocation(pMover, movementInfo);

    MoveUpdate update;
    update.Status = MovementInfoToWire(movementInfo, pMover->GetObjectGuid());

    pMover->SendMovementMessageToSet(MakeClassicPacket(update), true, _player);
}

void WorldSession::HandleSetCollisionHeightAckOpcode(MoveSetCollisionHeightAck const& /*packet*/)
{
    // nothing to do -- this engine never sends SMSG_MOVE_SET_COLLISION_HEIGHT for a client to ack
}

void WorldSession::HandleSetActiveMoverOpcode(SetActiveMover const& packet)
{
    ObjectGuid moverGuid;
    if (auto const guid = ClassicGuidMapping::FromClassic(packet.Mover))
        moverGuid = *guid;

    if (!moverGuid.IsEmpty())
    {
        Unit* pMover = _player->GetMover();
        if (pMover->GetObjectGuid() != moverGuid)
        {
            sLog.Player(this, LOG_MOVEMENT, LOG_LVL_ERROR, "HandleSetActiveMover: Incorrect mover guid. Mover is %s and should be %s.",
                pMover->GetGuidStr().c_str(), moverGuid.GetString().c_str());
            m_clientMoverGuid = pMover->GetObjectGuid();
            return;
        }

        if (pMover->IsCreature())
        {
            if (pMover->IsRooted())
                MovementPacketSender::AddMovementFlagChangeToController(pMover, MOVEFLAG_ROOT, true);

            // Older clients do not send spline done opcode for splines that started before they took control.
        }

        // mover swap after Eyes of the Beast, PetAI::UpdateAI handle the pet's return
        // Check if we actually have a pet before looking up
        if (_player->GetPetGuid() && _player->GetPetGuid() == m_clientMoverGuid)
        {
            if (Pet* pet = _player->GetPet())
            {
                pet->ClearUnitState(UNIT_STATE_POSSESSED);
                pet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED);
                // out of range pet dismissed
                if (!pet->IsWithinDistInMap(_player, pet->GetMap()->GetGridActivationDistance()))
                    _player->RemovePet(PET_SAVE_REAGENTS);
            }
        }
    }

    m_clientMoverGuid = moverGuid;
}

void WorldSession::HandleMoveInitActiveMoverCompleteOpcode(MoveInitActiveMoverComplete const& /*packet*/)
{
}

void WorldSession::SendActiveMover(Unit const* mover)
{
    // The server decides what the client moves; the client's own confirmation only re-checks it.
    m_clientMoverGuid = mover->GetObjectGuid();

    MoveSetActiveMover packet;
    packet.Mover = ClassicGuidMapping::ToClassic(mover->GetObjectGuid(), realmID);
    SendClassic(packet);
}

void WorldSession::HandleMountSpecialAnimOpcode(MountSpecial const& packet)
{
    SpecialMountAnim anim;
    anim.UnitGuid = ClassicGuidMapping::ToClassic(GetPlayer()->GetObjectGuid(), realmID);
    anim.SpellVisualKitIDs = packet.SpellVisualKitIDs;
    anim.SequenceVariation = packet.SequenceVariation;
    GetPlayer()->SendMovementMessageToSet(MakeClassicPacket(anim), false);
}

void WorldSession::HandleSummonResponseOpcode(SummonResponse const& packet)
{
    if (!_player->IsAlive() || _player->IsInCombat())
        return;

    _player->SummonIfPossible(packet.Accept);
}

void WorldSession::HandleMoveTimeSkippedOpcode(MoveTimeSkipped const& packet)
{
    auto const moverGuid = ClassicGuidMapping::FromClassic(packet.Mover);
    if (!moverGuid)
        return;

    Unit* pMover = GetMoverFromGuid(*moverGuid);
    if (!pMover)
        return;

    if (pMover->m_movementInfo.ctime)
    {
        pMover->m_movementInfo.stime += packet.TimeSkipped;
        pMover->m_movementInfo.ctime += packet.TimeSkipped;
    }

    // fix an 1.12 client problem with transports
    if (_player->HasJustBoarded())
    {
        _player->SetJustBoarded(false);
        if (GenericTransport* pTransport = pMover->GetTransport())
        {
            pTransport->SendOutOfRangeUpdateToPlayer(_player);
            pTransport->SendCreateUpdateToPlayer(_player);
        }
    }
    else
    {
        MoveSkipTime skip;
        skip.Mover = ClassicGuidMapping::ToClassic(pMover->GetObjectGuid(), realmID);
        skip.TimeSkipped = packet.TimeSkipped;
        pMover->SendMovementMessageToSet(MakeClassicPacket(skip), true, _player);
    }
}

// make sure this client is allowed to control the unit which guid is provided
Unit* WorldSession::GetMoverFromGuid(ObjectGuid const& guid) const
{
    if (guid == _player->GetMover()->GetObjectGuid())
        return _player->GetMover();
    if (guid == _player->GetObjectGuid())
        return _player;
    if (guid == m_clientMoverGuid)
        return _player->GetMap()->GetUnit(guid);

    return nullptr;
}

void WorldSession::RejectMovementPacketsFor(uint32 ms)
{
    uint32 timeout = WorldTimer::getMSTime() + ms;
    if (m_moveRejectTime < timeout)
        m_moveRejectTime = timeout;
}

bool WorldSession::VerifyMovementInfo(MovementInfo const& movementInfo) const
{
    if (!MaNGOS::IsValidMapCoord(movementInfo.GetPos().x, movementInfo.GetPos().y, movementInfo.GetPos().z, movementInfo.GetPos().o))
        return false;

    if (movementInfo.HasMovementFlag(MOVEFLAG_ONTRANSPORT))
    {
        // transports size limited
        // (also received at zeppelin/lift leave by some reason with t_* as absolute in continent coordinates, can be safely skipped)
        if (std::fabs(movementInfo.GetTransportPos().x) > 250 || std::fabs(movementInfo.GetTransportPos().y) > 250 || std::fabs(movementInfo.GetTransportPos().z) > 100)
            return false;

        if (!MaNGOS::IsValidMapCoord(movementInfo.GetPos().x + movementInfo.GetTransportPos().x, movementInfo.GetPos().y + movementInfo.GetTransportPos().y,
                                     movementInfo.GetPos().z + movementInfo.GetTransportPos().z, movementInfo.GetPos().o + movementInfo.GetTransportPos().o))
            return false;
    }

    return true;
}

void WorldSession::HandleMoverRelocation(Unit* pMover, MovementInfo& movementInfo)
{
    Player* const pPlayerMover = pMover->ToPlayer();

    movementInfo.sourceSessionGuid = GetGUID();
    movementInfo.CorrectData();

    // Prevent client from removing root flag.
    if (pMover->HasUnitMovementFlag(MOVEFLAG_ROOT) && !movementInfo.HasMovementFlag(MOVEFLAG_ROOT))
        movementInfo.AddMovementFlag(MOVEFLAG_ROOT);

    pMover->m_movementInfo = movementInfo;

    if (pMover->m_movementInfo.HasMovementFlag(MOVEFLAG_ONTRANSPORT))
    {
        if (!pMover->GetTransport())
        {
            if (GenericTransport* transport = pMover->GetMap()->GetTransport(pMover->m_movementInfo.GetTransportGuid()))
            {
                transport->AddPassenger(pMover);
                if (pPlayerMover)
                    pPlayerMover->SetJustBoarded(true); // fix a 1.12 client problem with transports
            }
        }
        else
        {
            if (pPlayerMover)
                pPlayerMover->SetJustBoarded(false);
            pMover->m_movementInfo.pos.x = pMover->m_movementInfo.GetTransportPos().x;
            pMover->m_movementInfo.pos.y = pMover->m_movementInfo.GetTransportPos().y;
            pMover->m_movementInfo.pos.z = pMover->m_movementInfo.GetTransportPos().z;
            pMover->m_movementInfo.pos.o = pMover->m_movementInfo.GetTransportPos().o;
            pMover->GetTransport()->CalculatePassengerPosition(pMover->m_movementInfo.pos.x, pMover->m_movementInfo.pos.y, pMover->m_movementInfo.pos.z, &pMover->m_movementInfo.pos.o);
        }
    }
    else if (pMover->GetTransport()) // if we were on a transport, leave
    {
        pMover->m_transport->RemovePassenger(pMover);
        pMover->m_transport = nullptr;
        pMover->m_movementInfo.ClearTransportData();
    }

    if (pPlayerMover)
    {
        if (pMover->m_movementInfo.HasMovementFlag(MOVEFLAG_MASK_MOVING))
        {
            if (ObjectGuid const& lootGuid = pPlayerMover->GetLootGuid())
                if (!lootGuid.IsItem())
                    pPlayerMover->GetSession()->DoLootRelease(lootGuid);
        }

        pPlayerMover->SetPosition(pMover->m_movementInfo.GetPos().x, pMover->m_movementInfo.GetPos().y, pMover->m_movementInfo.GetPos().z, pMover->m_movementInfo.GetPos().o);

        // Nostalrius - antiundermap1
        // terrain-only heights are meaningless below WMOs such as Undercity
        if (pMover->m_movementInfo.HasMovementFlag(MOVEFLAG_FALLINGFAR))
        {
            bool const vmapHeight = VMAP::VMapFactory::createOrGetVMapManager()->isHeightCalcEnabled();
            float hauteur = pPlayerMover->GetMap()->GetHeight(pPlayerMover->GetPositionX(), pPlayerMover->GetPositionY(), pPlayerMover->GetPositionZ(), true);
            bool undermap = false;
            // Undermap
            if (vmapHeight && (pPlayerMover->GetPositionZ() + 100.0f) < hauteur)
                undermap = true;
            if (pPlayerMover->GetPositionZ() < 250.0f && pPlayerMover->GetMapId() == MAP_WARSONG_GULCH)
                undermap = true;

            if (undermap)
                if (pPlayerMover->UndermapRecall())
                    sLog.Player(this, LOG_MOVEMENT, LOG_LVL_MINIMAL, "[UNDERMAP] Player: %s (GUID: %u) Teleporting to safe position. MapId: %u Pos: %g %g %g", pPlayerMover->GetName(), pPlayerMover->GetGUIDLow(), pPlayerMover->GetMapId(), pPlayerMover->GetPositionX(), pPlayerMover->GetPositionY(), pPlayerMover->GetPositionZ());
        }
        else if (pPlayerMover->CanFreeMove())
            pPlayerMover->SaveNoUndermapPosition(pMover->m_movementInfo.GetPos().x, pMover->m_movementInfo.GetPos().y, pMover->m_movementInfo.GetPos().z + 3.0f, pMover->m_movementInfo.GetPos().o);

        // Antiundermap2: teleport to graveyard
        if (pMover->m_movementInfo.GetPos().z < -500.0f && !pPlayerMover->IsGameMaster())
        {
            // NOTE: this is actually called many times while falling
            // even after the player has been teleported away
            // TODO: discard movement packets after the player is rooted
            if (pPlayerMover->IsAlive())
            {
                if (pPlayerMover->InBattleGround())
                    pPlayerMover->EnvironmentalDamage(DAMAGE_FALL_TO_VOID, pPlayerMover->GetHealth());
                else
                    pPlayerMover->EnvironmentalDamage(DAMAGE_FALL_TO_VOID, pPlayerMover->GetHealth() / 2);

                // player can be alive if GM and God
                if (!pPlayerMover->IsAlive())
                {
                    // change the death state to CORPSE to prevent the death timer from
                    // starting in the next player update
                    pPlayerMover->KillPlayer();
                    pPlayerMover->BuildPlayerRepop();
                }
            }

            // cancel the death timer here if started
            sLog.Player(this, LOG_MOVEMENT, LOG_LVL_MINIMAL, "[UNDERMAP/Teleport] Teleporting to graveyard.");
            pPlayerMover->RepopAtGraveyard();
        }
    }
    else
    {
        pMover->HandleInterruptsOnMovement(pMover->m_movementInfo.HasMovementFlag(MOVEFLAG_MASK_MOVING)); // called inside SetPosition for players
        pMover->GetMap()->CreatureRelocation((Creature*)pMover, pMover->m_movementInfo.GetPos().x, pMover->m_movementInfo.GetPos().y, pMover->m_movementInfo.GetPos().z, pMover->m_movementInfo.GetPos().o);
    }
}
