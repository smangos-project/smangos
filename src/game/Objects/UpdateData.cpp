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
#include "UpdateData.h"
#include "ByteBuffer.h"
#include "WorldPacket.h"
#include "Log.h"
#include "Opcodes.h"
#include "Player.h"
#include "World.h"
#include "ObjectGuid.h"
#include "Packets/Classic/GuidMapping.h"
#include "PacketFraming.h"
#include "Errors.h"
#include <cstring>
#include <zlib.h>

// Splits a session's blocks across several packets once one gets this big, well under
// MAX_CLASSIC_PACKET_SIZE (PacketFraming.h) to leave room for the block that pushed it over plus
// this packet's own header (map id, block count, destroy/out-of-range lists).
#define MAX_UNCOMPRESSED_PACKET_SIZE (MAX_CLASSIC_PACKET_SIZE / 2)

UpdateData::UpdateData()
{
}

UpdateData::~UpdateData()
{
    Clear();
}

void UpdateData::AddOutOfRangeGUID(ObjectGuidSet& guids)
{
    m_outOfRangeGUIDs.insert(guids.begin(), guids.end());
}

void UpdateData::AddOutOfRangeGUID(ObjectGuid const& guid)
{
    m_outOfRangeGUIDs.insert(guid);
}

void UpdateData::AddDestroyObject(ObjectGuid const& guid)
{
    m_destroyGUIDs.insert(guid);
}

ByteBuffer& UpdateData::AddUpdateBlockAndGetBuffer()
{
    if (m_datas.empty())
        m_datas.emplace_back();
    std::list<UpdateDataBlock>::iterator it = m_datas.end();
    --it;
    if (it->data.wpos() > MAX_UNCOMPRESSED_PACKET_SIZE)
    {
        m_datas.emplace_back();
        it = m_datas.end();
        --it;
    }
    ++it->blockCount;
    return it->data;
}

void UpdateData::BuildPacket(UpdateObject& packet, uint32 mapId, UpdateDataBlock const* block)
{
    packet.MapId = uint16(mapId);
    packet.BlockCount = block ? block->blockCount : 0;
    if (block && block->data.wpos())
    {
        packet.BlockData.resize(block->data.wpos());
        std::memcpy(packet.BlockData.data(), block->data.contents(), block->data.wpos());
    }

    // Only the first physical packet of a split send carries the destroy/out-of-range lists.
    for (ObjectGuid const& guid : m_destroyGUIDs)
        packet.DestroyGUIDs.push_back(ClassicGuidMapping::ToClassic(guid, realmID));
    for (ObjectGuid const& guid : m_outOfRangeGUIDs)
        packet.OutOfRangeGUIDs.push_back(ClassicGuidMapping::ToClassic(guid, realmID));
    m_destroyGUIDs.clear();
    m_outOfRangeGUIDs.clear();
}

void UpdateData::Send(WorldSession* session)
{
    if (!HasData())
        return;

    uint32 mapId = 0;
    if (Player const* player = session->GetPlayer())
        mapId = player->GetMapId();

    if (m_datas.empty())
    {
        UpdateObject packet;
        BuildPacket(packet, mapId);
        session->SendClassic(packet);
        return;
    }

    for (UpdateDataBlock& block : m_datas)
    {
        UpdateObject packet;
        BuildPacket(packet, mapId, &block);
        session->SendClassic(packet);
    }
    m_datas.clear();

    for (AuraUpdate const& auras : m_auraUpdates)
        session->SendClassic(auras);
    m_auraUpdates.clear();
}

void UpdateData::Clear()
{
    m_datas.clear();
    m_auraUpdates.clear();
    m_outOfRangeGUIDs.clear();
    m_destroyGUIDs.clear();
}

