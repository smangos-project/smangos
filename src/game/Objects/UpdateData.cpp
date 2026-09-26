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

void PacketCompressor::Compress(void* dst, uint32* dst_size, void* src, int src_size)
{
    z_stream c_stream;

    c_stream.zalloc = (alloc_func)0;
    c_stream.zfree = (free_func)0;
    c_stream.opaque = (voidpf)0;

    // default Z_BEST_SPEED (1)
    int z_res = deflateInit(&c_stream, sWorld.getConfig(CONFIG_UINT32_COMPRESSION_LEVEL));
    if (z_res != Z_OK)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Can't compress update packet (zlib: deflateInit) Error code: %i (%s)", z_res, zError(z_res));
        *dst_size = 0;
        return;
    }

    c_stream.next_out = (Bytef*)dst;
    c_stream.avail_out = *dst_size;
    c_stream.next_in = (Bytef*)src;
    c_stream.avail_in = (uInt)src_size;

    z_res = deflate(&c_stream, Z_NO_FLUSH);
    if (z_res != Z_OK)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Can't compress update packet (zlib: deflate) Error code: %i (%s)", z_res, zError(z_res));
        *dst_size = 0;
        return;
    }

    if (c_stream.avail_in != 0)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Can't compress update packet (zlib: deflate not greedy)");
        *dst_size = 0;
        return;
    }

    z_res = deflate(&c_stream, Z_FINISH);
    if (z_res != Z_STREAM_END)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Can't compress update packet (zlib: deflate should report Z_STREAM_END instead %i (%s)", z_res, zError(z_res));
        *dst_size = 0;
        return;
    }

    z_res = deflateEnd(&c_stream);
    if (z_res != Z_OK)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Can't compress update packet (zlib: deflateEnd) Error code: %i (%s)", z_res, zError(z_res));
        *dst_size = 0;
        return;
    }

    *dst_size = c_stream.total_out;
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
}

void UpdateData::Clear()
{
    m_datas.clear();
    m_outOfRangeGUIDs.clear();
    m_destroyGUIDs.clear();
}

#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_7_1
bool MovementData::CanAddPacket(WorldPacket const& data)
{
    // Since packet size is stored with an uint8, packet size is limited for compressed packets
    if ((data.wpos() + 2) > 0xFF)
        return false;

    if ((m_buffer.wpos() + (data.wpos() + 2)) >= 900000)
        return false;

    return true;
}

void MovementData::AddPacket(WorldPacket const& data)
{
    ASSERT(data.wpos() + 2 <= 0xFF); // Max packet size to be stored on uint8. Client crash else.
    m_buffer << uint8(data.wpos() + 2); // Packet + opcode size
    m_buffer << uint16(data.GetOpcode());
    m_buffer.append(data.contents(), data.wpos());
}

bool MovementData::BuildPacket(WorldPacket& packet)
{
    MANGOS_ASSERT(packet.empty()); // We want a clean packet !

    size_t pSize = m_buffer.wpos();                              // use real used data size

    if (pSize >= 900000)
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "[CRASH-CLIENT] Too large packet size %u (SMSG_COMPRESSED_MOVES)", pSize);

    uint32 destsize = compressBound(pSize);
    packet.resize(destsize + sizeof(uint32));
    packet.put<uint32>(0, pSize);
    PacketCompressor::Compress(const_cast<uint8*>(packet.contents()) + sizeof(uint32), &destsize, (void*)m_buffer.contents(), pSize);
    if (destsize == 0)
        return false;

    packet.resize(destsize + sizeof(uint32));
    packet.SetOpcode(SMSG_COMPRESSED_MOVES);
    return true;
}
#endif
