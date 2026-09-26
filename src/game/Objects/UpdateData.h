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

#ifndef __UPDATEDATA_H
#define __UPDATEDATA_H

#include <list>
#include <memory>
#include <vector>
#include "ByteBuffer.h"
#include "ObjectGuid.h"
#include "Packets/Auras.h"
#include "Packets/ObjectUpdate.h"

class WorldPacket;
class WorldSession;
class WorldObject;

enum ObjectUpdateType
{
    UPDATETYPE_VALUES               = 0,
    UPDATETYPE_CREATE_OBJECT        = 2,
    UPDATETYPE_CREATE_OBJECT2       = 3,
};

// One physical SMSG_UPDATE_OBJECT's worth of already-serialized create/values/movement blocks.
struct UpdateDataBlock
{
    ByteBuffer data;
    uint32 blockCount = 0;
};

class UpdateData
{
    public:
        UpdateData();
        ~UpdateData();

        void AddOutOfRangeGUID(ObjectGuidSet& guids);
        void AddOutOfRangeGUID(ObjectGuid const& guid);
        void AddDestroyObject(ObjectGuid const& guid);
        ByteBuffer& AddUpdateBlockAndGetBuffer();
        // Aura lists of the units whose create blocks are in this update; sent right after the blocks,
        // since the client drops aura updates for a unit it has not been told about yet.
        void AddAuraUpdate(AuraUpdate&& packet) { m_auraUpdates.push_back(std::move(packet)); }
        // Fills `packet` with this update's first (or only) block, if any, and the destroy/out-of-
        // range guid lists, which are then cleared. For a caller that wants to build the wire
        // packet once and reuse the same bytes across several sessions (Send() below rebuilds one
        // per session/split instead).
        void BuildPacket(UpdateObject& packet, uint32 mapId, UpdateDataBlock const* block = nullptr);
        // Sends to `session`'s player's current map; splits into more than one physical packet if
        // the blocks built up don't fit in one (see AddUpdateBlockAndGetBuffer).
        void Send(WorldSession* session);
        bool HasData() const { return !m_datas.empty() || !m_outOfRangeGUIDs.empty() || !m_destroyGUIDs.empty(); }
        void Clear();

        ObjectGuidSet const& GetOutOfRangeGUIDs() const { return m_outOfRangeGUIDs; }

    protected:
        ObjectGuidSet m_outOfRangeGUIDs;
        ObjectGuidSet m_destroyGUIDs;
        std::list<UpdateDataBlock> m_datas;
        std::vector<AuraUpdate> m_auraUpdates;
};

#endif
