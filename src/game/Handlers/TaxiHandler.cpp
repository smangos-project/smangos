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
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Path.h"
#include "WaypointMovementGenerator.h"
#include "Database/DBCStores.h"
#include "Server/Packets/Classic/GuidMapping.h"

namespace
{
ClassicTaxiMask ToClassicTaxiMask(ByteBuffer const& mask)
{
    ClassicTaxiMask classic{};
    memcpy(classic.data(), mask.contents(), classic.size());
    return classic;
}
}

void WorldSession::HandleTaxiNodeStatusQueryOpcode(TaxiNodeStatusQuery const& packet)
{
    auto const guid = ClassicGuidMapping::FromClassic(packet.UnitGuid);
    if (!guid)
        return;
    SendTaxiStatus(*guid);
}

void WorldSession::SendTaxiStatus(ObjectGuid guid)
{
    // cheating checks
    Creature* unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WorldSession::SendTaxiStatus - %s not found or you can't interact with it.", guid.GetString().c_str());
        return;
    }

    uint32 curloc = sObjectMgr.GetNearestTaxiNode(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), unit->GetMapId(), GetPlayer()->GetTeam());

    // not found nearest
    if (curloc == 0)
        return;

    TaxiNodeStatus status;
    status.UnitGuid = ClassicGuidMapping::ToClassic(guid, realmID);
    status.Status = GetPlayer()->m_taxi.IsTaximaskNodeKnown(curloc) ? CLASSIC_TAXISTATUS_LEARNED : CLASSIC_TAXISTATUS_UNLEARNED;
    SendClassic(status);
}

void WorldSession::HandleTaxiQueryAvailableNodes(TaxiQueryAvailableNodes const& packet)
{
    auto const guid = ClassicGuidMapping::FromClassic(packet.UnitGuid);
    if (!guid)
        return;

    // cheating checks
    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(*guid, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!unit)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WORLD: HandleTaxiQueryAvailableNodes - %s not found or you can't interact with him.", guid->GetString().c_str());
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // unknown taxi node case
    if (SendLearnNewTaxiNode(unit))
        return;

    // known taxi node case
    SendTaxiMenu(unit);
}

void WorldSession::HandleEnableTaxiNodeOpcode(EnableTaxiNode const& packet)
{
    auto const guid = ClassicGuidMapping::FromClassic(packet.UnitGuid);
    if (!guid)
        return;

    if (Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(*guid, UNIT_NPC_FLAG_FLIGHTMASTER))
        SendLearnNewTaxiNode(unit);
}

void WorldSession::SendTaxiMenu(Creature* unit)
{
    // find current node
    uint32 curloc = sObjectMgr.GetNearestTaxiNode(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), unit->GetMapId(), GetPlayer()->GetTeam());

    if (curloc == 0)
        return;

    ShowTaxiNodes packet;
    packet.UnitGuid = ClassicGuidMapping::ToClassic(unit->GetObjectGuid(), realmID);
    packet.CurrentNode = int32(curloc);

    ByteBuffer mask;
    GetPlayer()->m_taxi.AppendTaximaskTo(mask, GetPlayer()->IsTaxiCheater());
    packet.CanLandNodes = ToClassicTaxiMask(mask);
    packet.CanUseNodes = packet.CanLandNodes;

    SendClassic(packet);
}

void WorldSession::SendDoFlight(uint32 mountDisplayId, uint32 path, uint32 pathNode)
{
    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    while (GetPlayer()->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE)
        GetPlayer()->GetMotionMaster()->MovementExpired(false);

    if (mountDisplayId)
        GetPlayer()->Mount(mountDisplayId);

    // Check if it's a multi path
    if (!GetPlayer()->m_taxi.GetTaxiPath().empty())
        GetPlayer()->GetMotionMaster()->MoveTaxiFlight();
    else
        GetPlayer()->GetMotionMaster()->MoveTaxiFlight(path, pathNode);
}

bool WorldSession::SendLearnNewTaxiNode(Creature* unit)
{
    // find current node
    uint32 curloc = sObjectMgr.GetNearestTaxiNode(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), unit->GetMapId(), GetPlayer()->GetTeam());

    if (curloc == 0)
        return true;                                        // `true` send to avoid WorldSession::SendTaxiMenu call with one more curlock seartch with same false result.

    if (GetPlayer()->m_taxi.SetTaximaskNode(curloc))
    {
        NewTaxiPath newPath;
        newPath.TaxiNodesId = int32(curloc);
        SendClassic(newPath);

        TaxiNodeStatus status;
        status.UnitGuid = ClassicGuidMapping::ToClassic(unit->GetObjectGuid(), realmID);
        status.Status = CLASSIC_TAXISTATUS_LEARNED;
        SendClassic(status);

        return true;
    }
    else
        return false;
}

void WorldSession::SendActivateTaxiReply(uint32 replyCode)
{
    ActivateTaxiReply reply;
    reply.Reply = uint8(replyCode);
    SendClassic(reply);
}

void WorldSession::HandleActivateTaxiOpcode(ActivateTaxi const& packet)
{
    auto const vendorGuid = ClassicGuidMapping::FromClassic(packet.Vendor);
    if (!vendorGuid)
        return;

    Creature* npc = GetPlayer()->GetNPCIfCanInteractWith(*vendorGuid, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!npc)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WORLD: HandleActivateTaxiOpcode - %s not found or you can't interact with it.", vendorGuid->GetString().c_str());
        SendActivateTaxiReply(ERR_TAXITOOFARAWAY);
        return;
    }

    uint32 curloc = sObjectMgr.GetNearestTaxiNode(npc->GetPositionX(), npc->GetPositionY(), npc->GetPositionZ(), npc->GetMapId(), GetPlayer()->GetTeam());
    if (curloc == 0)
        return;

    std::vector<uint32> nodes { curloc, packet.Node };
    GetPlayer()->ActivateTaxiPathTo(nodes, npc);
}
