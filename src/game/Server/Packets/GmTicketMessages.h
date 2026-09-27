#ifndef MANGOS_PACKETS_GMTICKETMESSAGES_H
#define MANGOS_PACKETS_GMTICKETMESSAGES_H

// The modern support wire: the Help UI's status polls, its two submission forms (feedback and the
// legacy bug report), and the right-click player-report form. Light like Packets/DuelMessages.h,
// so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <string>

// --- Client packets ---

struct GetSystemStatus
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GM_TICKET_GET_SYSTEM_STATUS;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// No external web-portal case tracking exists on this server (see CaseStatus below) -- this
// request is accepted and always answered with zero cases.
struct GetCaseStatus
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GM_TICKET_GET_CASE_STATUS;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// Sent after the client shows its post-resolution CSAT survey. Nothing consumes survey results.
struct AcknowledgeSurvey
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GM_TICKET_ACKNOWLEDGE_SURVEY;

    int32 CaseId = 0;

    void Write(ByteBuffer& data) const { data << CaseId; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> CaseId;
        return true;
    }
};

// The Help UI's "Submit Feedback" form: a free-text note, where it was sent from, and a
// suggestion/bug toggle. No category selector.
struct SubmitUserFeedback
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SUBMIT_USER_FEEDBACK;

    int32 MapId = 0;
    float PosX = 0.0f;
    float PosY = 0.0f;
    float PosZ = 0.0f;
    float Facing = 0.0f;
    std::string Note;
    bool IsSuggestion = false;

    void Write(ByteBuffer& data) const
    {
        data << MapId << PosX << PosY << PosZ << Facing;
        data.WriteBits(uint32(Note.size() + 1), 24);
        data.WriteBit(IsSuggestion);
        data.FlushBits();
        if (!Note.empty())
            data.WriteString(Note);
        data << uint8(0);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32) + 4 * sizeof(float))
            return false;
        data >> MapId >> PosX >> PosY >> PosZ >> Facing;
        if (data.size() - data.rpos() < 4) // 24 + 1 bits, flushed to four bytes
            return false;
        uint32 const noteLength = data.ReadBits(24);
        IsSuggestion = data.ReadBit();
        data.ResetBitPos();
        Note.clear();
        if (noteLength == 0)
            return true;
        if (data.size() - data.rpos() < noteLength)
            return false;
        if (noteLength > 1)
            Note = data.ReadString(noteLength - 1);
        data.read_skip<uint8>(); // trailing NUL
        return true;
    }
};

// The right-click "Report Player" form (chat spam / mail spam / calendar spam). The much larger
// modern SupportTicketSubmitComplaint (LFG-list/club-finder/community-voice reports) is not
// implemented: those report sources don't exist on this server.
struct Complaint
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_COMPLAINT;

    static constexpr uint8 Mail = 0;
    static constexpr uint8 Chat = 1;
    static constexpr uint8 Calendar = 2;

    uint8 ComplaintType = 0;
    ClassicGuid OffenderGuid;
    uint32 OffenderRealmAddress = 0;
    uint32 TimeSinceOffence = 0;
    uint64 MailId = 0;                // ComplaintType == Mail
    uint32 ChatCommand = 0;           // ComplaintType == Chat
    uint32 ChatChannelId = 0;         // ComplaintType == Chat
    std::string ChatMessageLog;       // ComplaintType == Chat
    uint64 CalendarEventGuid = 0;     // ComplaintType == Calendar
    uint64 CalendarInviteGuid = 0;    // ComplaintType == Calendar

    void Write(ByteBuffer& data) const
    {
        data << ComplaintType;
        OffenderGuid.Write(data);
        data << OffenderRealmAddress << TimeSinceOffence;
        switch (ComplaintType)
        {
            case Mail:
                data << MailId;
                break;
            case Chat:
                data << ChatCommand << ChatChannelId;
                data.WriteBits(uint32(ChatMessageLog.size()), 12);
                data.FlushBits();
                data.WriteString(ChatMessageLog);
                break;
            case Calendar:
                data << CalendarEventGuid << CalendarInviteGuid;
                break;
            default:
                break;
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data >> ComplaintType;
        if (!OffenderGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32))
            return false;
        data >> OffenderRealmAddress >> TimeSinceOffence;

        switch (ComplaintType)
        {
            case Mail:
                if (data.size() - data.rpos() < sizeof(uint64))
                    return false;
                data >> MailId;
                return true;
            case Chat:
                if (data.size() - data.rpos() < 2 * sizeof(uint32) + 2) // + 12 bits, flushed to two bytes
                    return false;
                data >> ChatCommand >> ChatChannelId;
                {
                    uint32 const logLength = data.ReadBits(12);
                    data.ResetBitPos();
                    if (data.size() - data.rpos() < logLength)
                        return false;
                    ChatMessageLog = data.ReadString(logLength);
                }
                return true;
            case Calendar:
                if (data.size() - data.rpos() < 2 * sizeof(uint64))
                    return false;
                data >> CalendarEventGuid >> CalendarInviteGuid;
                return true;
            default:
                return true;
        }
    }
};

// The legacy standalone bug report, only reachable client-side via `/script ReportBug("text")` --
// SubmitUserFeedback is what the normal Help UI sends.
struct BugReport
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_BUG_REPORT;

    bool IsSuggestion = false;
    std::string DiagInfo;
    std::string Text;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(IsSuggestion);
        data.WriteBits(uint32(DiagInfo.size()), 12);
        data.WriteBits(uint32(Text.size()), 10);
        data.FlushBits();
        data.WriteString(DiagInfo);
        data.WriteString(Text);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 3) // 1 + 12 + 10 bits, flushed to three bytes
            return false;
        IsSuggestion = data.ReadBit();
        uint32 const diagLength = data.ReadBits(12);
        uint32 const textLength = data.ReadBits(10);
        data.ResetBitPos();
        if (data.size() - data.rpos() < diagLength + textLength)
            return false;
        DiagInfo = data.ReadString(diagLength);
        Text = data.ReadString(textLength);
        return true;
    }
};

// --- Server packets ---

struct SystemStatus
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GM_TICKET_SYSTEM_STATUS;

    static constexpr int32 Disabled = 0;
    static constexpr int32 Enabled = 1;

    int32 Status = Disabled;

    void Write(ByteBuffer& data) const { data << Status; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> Status;
        return true;
    }
};

// Always reports zero cases: this server has no external web ticketing portal for the client's
// Cases panel to link to. Player-facing ticket status instead goes through plain chat text (GM
// `.ticket` commands), not this packet.
struct CaseStatus
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GM_TICKET_CASE_STATUS;

    void Write(ByteBuffer& data) const { data << int32(0); }

    // A body naming a case is rejected on read: nothing ever builds one.
    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        int32 caseCount;
        data >> caseCount;
        return caseCount == 0;
    }
};

struct ComplaintResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_COMPLAINT_RESULT;

    uint32 ComplaintType = 0;
    uint8 Result = 0;

    void Write(ByteBuffer& data) const { data << ComplaintType << Result; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) + 1)
            return false;
        data >> ComplaintType >> Result;
        return true;
    }
};

#endif
