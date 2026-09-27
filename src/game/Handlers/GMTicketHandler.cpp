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
#include "Language.h"
#include "WorldPacket.h"
#include "GMTicketMgr.h"
#include "Player.h"
#include "Chat.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Packets/Classic/GuidMapping.h"
#include <sstream>

void WorldSession::HandleGMTicketGetSystemStatusOpcode(GetSystemStatus const& /*packet*/)
{
    SystemStatus response;
    response.Status = sTicketMgr->GetStatus() ? SystemStatus::Enabled : SystemStatus::Disabled;
    SendClassic(response);
}

void WorldSession::HandleGMTicketGetCaseStatusOpcode(GetCaseStatus const& /*packet*/)
{
    // No external web-portal case tracking on this server -- see the class comment (GmTicketMessages.h).
    SendClassic(CaseStatus());
}

void WorldSession::HandleGMTicketAcknowledgeSurveyOpcode(AcknowledgeSurvey const& /*packet*/)
{
    // Nothing to do -- see the class comment (GmTicketMessages.h).
}

void WorldSession::HandleGMTicketSubmitUserFeedbackOpcode(SubmitUserFeedback const& packet)
{
    if (!sTicketMgr->GetStatus())
        return;

    if (sTicketMgr->GetTicketByPlayer(GetPlayer()->GetGUID()))
    {
        ChatHandler(this).SendSysMessage("You already have an open ticket.");
        return;
    }

    GmTicket* ticket = new GmTicket(GetPlayer());
    ticket->SetMessage(packet.Note);
    ticket->SetTicketType(packet.IsSuggestion ? GMTICKET_SUGGESTION : GMTICKET_BUG);
    ticket->SetPosition(uint32(packet.MapId), packet.PosX, packet.PosY, packet.PosZ);
    sTicketMgr->AddTicket(ticket);
    sTicketMgr->UpdateLastChange();

    sWorld.SendGMTicketText(LANG_COMMAND_TICKETNEW, GetPlayer()->GetName(), ticket->GetId());
    ChatHandler(this).SendSysMessage("Your report has been submitted. Thank you.");
}

void WorldSession::HandleGMTicketBugReportOpcode(BugReport const& packet)
{
    if (!sTicketMgr->GetStatus())
        return;

    GmTicket* ticket = new GmTicket(GetPlayer());
    std::string message = packet.Text;
    if (!packet.DiagInfo.empty())
    {
        message += "\n\nDiagnostic info:\n";
        message += packet.DiagInfo;
    }
    ticket->SetMessage(message);
    ticket->SetTicketType(packet.IsSuggestion ? GMTICKET_SUGGESTION : GMTICKET_BUG);
    sTicketMgr->AddTicket(ticket);
    sTicketMgr->UpdateLastChange();

    sWorld.SendGMTicketText(LANG_COMMAND_TICKETNEW, GetPlayer()->GetName(), ticket->GetId());
}

void WorldSession::HandleGMTicketComplaintOpcode(Complaint const& packet)
{
    ComplaintResult result;
    result.ComplaintType = packet.ComplaintType;

    if (!sTicketMgr->GetStatus())
    {
        result.Result = 0; // failure -- ticket system disabled
        SendClassic(result);
        return;
    }

    std::string offenderName;
    nonstd::optional<ObjectGuid> const offenderGuid = ClassicGuidMapping::FromClassic(packet.OffenderGuid);
    if (offenderGuid)
        sObjectMgr.GetPlayerNameByGUID(*offenderGuid, offenderName);

    std::stringstream message;
    message << "Player report against " << (offenderName.empty() ? "an unknown player" : offenderName) << ".\n";
    switch (packet.ComplaintType)
    {
        case Complaint::Mail:
            message << "Type: Mail spam (mail id " << packet.MailId << ")";
            break;
        case Complaint::Chat:
            message << "Type: Chat spam (channel " << packet.ChatChannelId << ")\n";
            message << packet.ChatMessageLog;
            break;
        case Complaint::Calendar:
            message << "Type: Calendar spam";
            break;
        default:
            message << "Type: Unknown (" << uint32(packet.ComplaintType) << ")";
            break;
    }

    GmTicket* ticket = new GmTicket(GetPlayer());
    ticket->SetMessage(message.str());
    ticket->SetTicketType(GMTICKET_COMPLAINT);
    sTicketMgr->AddTicket(ticket);
    sTicketMgr->UpdateLastChange();

    sWorld.SendGMTicketText(LANG_COMMAND_TICKETNEW, GetPlayer()->GetName(), ticket->GetId());

    result.Result = 1; // success
    SendClassic(result);
}
