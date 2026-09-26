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
#include "Log.h"
#include "Opcodes.h"
#include "UpdateData.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "Packets/Classic/GuidMapping.h"

void WorldSession::HandleCanDuelOpcode(CanDuel const& packet)
{
    nonstd::optional<ObjectGuid> const targetGuid = ClassicGuidMapping::FromClassic(packet.Target);
    Player* target = targetGuid ? sObjectMgr.GetPlayer(*targetGuid) : nullptr;
    if (!target)
        return;

    CanDuelResult response;
    response.Target = packet.Target;
    response.Result = !target->m_duel;
    SendClassic(response);
}

void WorldSession::HandleDuelResponseOpcode(DuelResponse const& packet)
{
    if (packet.Accepted && !packet.Forfeited)
        HandleDuelAccepted(packet.Arbiter);
    else
        HandleDuelCancelled();
}

void WorldSession::HandleDuelAccepted(ClassicGuid const& arbiter)
{
    Player* pl = GetPlayer();
    if (!pl->m_duel)                                          // ignore accept from duel-sender
        return;

    Player* plTarget = pl->m_duel->opponent;

    if (pl == pl->m_duel->initiator || !plTarget || !plTarget->m_duel || pl == plTarget || pl->m_duel->startTime != 0 || plTarget->m_duel->startTime != 0)
        return;

    nonstd::optional<ObjectGuid> const arbiterGuid = ClassicGuidMapping::FromClassic(arbiter);
    if (!arbiterGuid || pl->GetGuidValue(PLAYER_DUEL_ARBITER) != *arbiterGuid)
        return;

    time_t now = time(nullptr);
    pl->m_duel->startTimer = now;
    plTarget->m_duel->startTimer = now;

    pl->SendDuelCountdown(3000);
    plTarget->SendDuelCountdown(3000);
}

void WorldSession::HandleDuelCancelled()
{
    auto pPlayer = GetPlayer();
    // no duel requested
    if (!pPlayer->m_duel)
        return;

    // player surrendered in a duel using /forfeit
    if (pPlayer->m_duel->startTime != 0)
    {
        pPlayer->CombatStopWithPets(true);
        if (pPlayer->m_duel->opponent)
            pPlayer->m_duel->opponent->CombatStopWithPets(true);

        pPlayer->CastSpell(GetPlayer(), 7267, true);    // beg
        pPlayer->DuelComplete(DUEL_WON);
        return;
    }

    // player either discarded the duel using the "discard button"
    // or used "/forfeit" before countdown reached 0
    pPlayer->DuelComplete(DUEL_INTERRUPTED);
}
