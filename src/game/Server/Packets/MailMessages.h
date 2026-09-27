#ifndef MANGOS_PACKETS_MAILMESSAGES_H
#define MANGOS_PACKETS_MAILMESSAGES_H

// The modern mail wire: send, list, take item/money, mark read, return to sender, delete,
// create-text-item (copy a mail's body to a readable item), next-mail-time, and the item-text
// query a readable item's own text (mail body items included) answers through. Light like
// Packets/GuildMessages.h. Confirmed against TrinityCore's Server/Packets/MailPackets.{h,cpp} and
// QueryPackets.{h,cpp} (ItemTextQuery/QueryItemTextResponse) -- see the RealBots README "Mail"
// dispute bullet for where vibemangos's own reference diverges from its credited TC source.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include "Packets/ItemMessages.h"
#include <string>
#include <vector>

// --- Client packets ---

struct SendMail
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SEND_MAIL;

    struct Attachment
    {
        uint8 Position = 0;
        ClassicGuid ItemGuid;
    };

    ClassicGuid MailboxGuid;
    int32 StationeryId = 0;
    int64 SendMoney = 0;
    int64 Cod = 0;
    std::string Target;
    std::string Subject;
    std::string Body;
    std::vector<Attachment> Attachments;

    void Write(ByteBuffer& data) const
    {
        MailboxGuid.Write(data);
        data << StationeryId << SendMoney << Cod;
        data.WriteBits(uint32(Target.length()), 9);
        data.WriteBits(uint32(Subject.length()), 9);
        data.WriteBits(uint32(Body.length()), 11);
        data.WriteBits(uint32(Attachments.size()), 5);
        data.FlushBits();
        data.WriteString(Target);
        data.WriteString(Subject);
        data.WriteString(Body);
        for (Attachment const& att : Attachments)
        {
            data << att.Position;
            att.ItemGuid.Write(data);
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (!MailboxGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32) + 2 * sizeof(int64))
            return false;
        data >> StationeryId >> SendMoney >> Cod;

        if (data.size() - data.rpos() < 5) // 9 + 9 + 11 + 5 = 34 bits
            return false;
        uint32 const targetLength = data.ReadBits(9);
        uint32 const subjectLength = data.ReadBits(9);
        uint32 const bodyLength = data.ReadBits(11);
        uint32 const attachCount = data.ReadBits(5);
        data.ResetBitPos();

        if (data.size() - data.rpos() < targetLength)
            return false;
        Target = data.ReadString(targetLength);
        if (data.size() - data.rpos() < subjectLength)
            return false;
        Subject = data.ReadString(subjectLength);
        if (data.size() - data.rpos() < bodyLength)
            return false;
        Body = data.ReadString(bodyLength);

        Attachments.clear();
        for (uint32 i = 0; i < attachCount; ++i)
        {
            if (data.size() - data.rpos() < 1)
                return false;
            Attachment att;
            data >> att.Position;
            if (!att.ItemGuid.Read(data))
                return false;
            Attachments.push_back(att);
        }
        return true;
    }
};

// No mailbox guid on the real wire format -- unlike TrinityCore, which re-checks proximity via a
// generic "last interaction target" the client and server both track for every gossip/vendor/
// mailbox talk (PlayerTalkClass::GetInteractionData().SourceGuid), a tracker this engine doesn't
// have. WorldSession::m_currentMailboxGuid (set by a successful GetMailList) stands in for it.
// SenderGuid is the mail's claimed sender, used as an extra ownership sanity check against the
// stored Mail::sender, matching TrinityCore's own HandleMailReturnToSender.
struct MailReturnToSender
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MAIL_RETURN_TO_SENDER;

    uint64 MailId = 0;
    ClassicGuid SenderGuid;

    void Write(ByteBuffer& data) const
    {
        data << MailId;
        SenderGuid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint64))
            return false;
        data >> MailId;
        return SenderGuid.Read(data);
    }
};

struct MailMarkAsRead
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MAIL_MARK_AS_READ;

    ClassicGuid MailboxGuid;
    uint64 MailId = 0;

    void Write(ByteBuffer& data) const
    {
        MailboxGuid.Write(data);
        data << MailId;
    }

    bool Read(ByteBuffer& data)
    {
        if (!MailboxGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint64))
            return false;
        data >> MailId;
        return true;
    }
};

struct MailTakeItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MAIL_TAKE_ITEM;

    ClassicGuid MailboxGuid;
    uint64 MailId = 0;
    // The low guid of the specific attached item to take -- this engine only ever attaches at most
    // one item per mail (MAX_MAIL_ITEMS), but the wire itself names the attachment explicitly.
    uint64 AttachId = 0;

    void Write(ByteBuffer& data) const
    {
        MailboxGuid.Write(data);
        data << MailId << AttachId;
    }

    bool Read(ByteBuffer& data)
    {
        if (!MailboxGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint64))
            return false;
        data >> MailId >> AttachId;
        return true;
    }
};

struct MailTakeMoney
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MAIL_TAKE_MONEY;

    ClassicGuid MailboxGuid;
    uint64 MailId = 0;
    uint64 Money = 0;

    void Write(ByteBuffer& data) const
    {
        MailboxGuid.Write(data);
        data << MailId << Money;
    }

    bool Read(ByteBuffer& data)
    {
        if (!MailboxGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint64))
            return false;
        data >> MailId >> Money;
        return true;
    }
};

struct GetMailList
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MAIL_GET_LIST;

    ClassicGuid MailboxGuid;

    void Write(ByteBuffer& data) const { MailboxGuid.Write(data); }
    bool Read(ByteBuffer& data) { return MailboxGuid.Read(data); }
};

// No mailbox guid on the real wire format, same story as MailReturnToSender above -- unlike that
// one, this engine has no proximity check for a delete at all (matches TrinityCore's own
// HandleMailDelete, which never calls CanOpenMailBox either). DeleteReason is parsed but unused --
// no reason-specific behavior implemented, same scope as the equivalent vibemangos field.
struct MailDelete
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MAIL_DELETE;

    uint64 MailId = 0;
    int32 DeleteReason = 0;

    void Write(ByteBuffer& data) const { data << MailId << DeleteReason; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint64) + sizeof(int32))
            return false;
        data >> MailId >> DeleteReason;
        return true;
    }
};

struct MailCreateTextItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MAIL_CREATE_TEXT_ITEM;

    ClassicGuid MailboxGuid;
    uint64 MailId = 0;

    void Write(ByteBuffer& data) const
    {
        MailboxGuid.Write(data);
        data << MailId;
    }

    bool Read(ByteBuffer& data)
    {
        if (!MailboxGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint64))
            return false;
        data >> MailId;
        return true;
    }
};

struct QueryNextMailTime
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUERY_NEXT_MAIL_TIME;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// The request side of the readable-item text flow: a mail body item, a quest scroll, a book --
// anything with ITEM_FIELD_ITEM_TEXT_ID set. Id is the item's own guid (the client already knows
// it from the mail list or its inventory), not a mail id or a text id -- confirmed against
// TrinityCore's WorldPackets::Query::ItemTextQuery / WorldSession::HandleItemTextQuery, which
// looks the item up by guid rather than by the numeric text id this engine stores on it.
struct ItemTextQuery
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ITEM_TEXT_QUERY;

    ClassicGuid Id;

    void Write(ByteBuffer& data) const { Id.Write(data); }
    bool Read(ByteBuffer& data) { return Id.Read(data); }
};

// --- Server packets ---

// Named after TrinityCore's own MailCommandResult, not vanilla's SendMailResult -- this codebase's
// WorldSession::SendMailResult() (the function that builds and sends one) would otherwise collide
// with the wire struct's name now that light packets carry no namespace to disambiguate with.
// Always writes all 6 fields unconditionally (confirmed against TrinityCore's
// MailCommandResult::Write()) -- BagResult is only meaningful when ErrorCode is the equip-error
// code, and AttachId/QtyInInventory only when Command is the item-taken action, but the wire
// carries all of them every time regardless.
struct MailCommandResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_MAIL_COMMAND_RESULT;

    uint64 MailId = 0;
    int32 Command = 0;
    int32 ErrorCode = 0;
    int32 BagResult = 0;
    uint64 AttachId = 0;
    int32 QtyInInventory = 0;

    void Write(ByteBuffer& data) const
    {
        data << MailId << Command << ErrorCode << BagResult << AttachId << QtyInInventory;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2 * sizeof(uint64) + 4 * sizeof(int32))
            return false;
        data >> MailId >> Command >> ErrorCode >> BagResult >> AttachId >> QtyInInventory;
        return true;
    }
};

struct ReceivedMail
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_NOTIFY_RECEIVED_MAIL;

    float Delay = 0.0f;

    void Write(ByteBuffer& data) const { data << Delay; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(float))
            return false;
        data >> Delay;
        return true;
    }
};

// Confirmed against TrinityCore's QueryItemTextResponse::Write() / operator<<(ItemTextCache
// const&): WriteBit(Valid), FlushBits, then a bits<13>-length-prefixed string, then the item guid
// -- the guid rides last on the wire despite being declared first in TrinityCore's own struct.
struct ItemTextQueryResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUERY_ITEM_TEXT_RESPONSE;

    ClassicGuid Id;
    bool Valid = false;
    std::string Text;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(Valid);
        data.FlushBits();
        data.WriteBits(uint32(Text.length()), 13);
        data.FlushBits();
        data.WriteString(Text);
        Id.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        Valid = data.ReadBit();
        data.ResetBitPos();
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const length = data.ReadBits(13);
        data.ResetBitPos();
        if (data.size() - data.rpos() < length)
            return false;
        Text = data.ReadString(length);
        return Id.Read(data);
    }
};

// Populated from up to 2 unread, already-delivered mails (one per distinct sender), matching
// TrinityCore's WorldSession::HandleQueryNextMailTime. Every entry field is written unconditionally
// (confirmed against TrinityCore's MailQueryNextTimeResult::Write()) -- SenderGuid/AltSenderId are
// both present on the wire even though only one is meaningful for a given AltSenderType.
struct MailQueryNextTimeResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_MAIL_QUERY_NEXT_TIME_RESULT;

    struct Entry
    {
        ClassicGuid SenderGuid;  // set when the sender is a player (MAIL_NORMAL)
        float TimeLeft = 0.0f;
        int32 AltSenderId = 0;   // set when the sender is a creature/gameobject/auction
        int8 AltSenderType = 0;
        int32 StationeryId = 0;
    };

    float NextMailTime = 0.0f;
    std::vector<Entry> Mails;

    void Write(ByteBuffer& data) const
    {
        data << NextMailTime;
        data << int32(Mails.size());
        for (Entry const& entry : Mails)
        {
            entry.SenderGuid.Write(data);
            data << entry.TimeLeft << entry.AltSenderId << entry.AltSenderType << entry.StationeryId;
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(float) + sizeof(int32))
            return false;
        data >> NextMailTime;
        int32 count;
        data >> count;

        Mails.clear();
        for (int32 i = 0; i < count; ++i)
        {
            Entry entry;
            if (!entry.SenderGuid.Read(data))
                return false;
            if (data.size() - data.rpos() < sizeof(float) + sizeof(int32) + sizeof(int8) + sizeof(int32))
                return false;
            data >> entry.TimeLeft >> entry.AltSenderId >> entry.AltSenderType >> entry.StationeryId;
            Mails.push_back(entry);
        }
        return true;
    }
};

// An item attached to a mail. This engine attaches at most one per mail (MAX_MAIL_ITEMS) and never
// tracks enchants or gems on a mail attachment, so those two lists are always sent empty.
struct MailAttachedItem
{
    uint8 Position = 0;
    uint64 AttachId = 0; // the attached item's own low guid
    int32 Count = 0;
    int32 Charges = 0;
    uint32 MaxDurability = 0;
    int32 Durability = 0;
    bool Unlocked = false;
    ClassicItemInstance Item;

    void Write(ByteBuffer& data) const
    {
        data << Position << AttachId << Count << Charges << MaxDurability << Durability;
        Item.Write(data);
        data.WriteBits(0, 4); // Enchants.Count
        data.WriteBits(0, 2); // Gems.Count
        data.WriteBit(Unlocked);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint8) + sizeof(uint64) + 2 * sizeof(int32) + sizeof(uint32) + sizeof(int32))
            return false;
        data >> Position >> AttachId >> Count >> Charges >> MaxDurability >> Durability;
        if (!Item.Read(data))
            return false;
        if (data.size() - data.rpos() < 2) // 4 + 2 + 1 bits
            return false;
        uint32 const enchantCount = data.ReadBits(4);
        uint32 const gemCount = data.ReadBits(2);
        Unlocked = data.ReadBit();
        data.ResetBitPos();
        return enchantCount == 0 && gemCount == 0; // this engine never sends either
    }
};

// Full rewrite from vanilla's single-item, null-terminated-string shape. Confirmed against
// TrinityCore's MailListEntry / operator<<(MailListEntry const&) (Server/Packets/MailPackets.cpp).
// SenderGuid is written only for MAIL_NORMAL and AltSenderId only for MAIL_AUCTION/CREATURE/
// GAMEOBJECT (this engine's own MAIL_ITEM has no modern counterpart, matching the legacy handler's
// own "NYI" treatment) -- there is no bit flag on the wire selecting which, unlike an earlier
// reference implementation of this struct (vibemangos), which added two such bits with no
// counterpart in its own credited TrinityCore source; see the RealBots README "Mail" bullet.
struct MailListEntry
{
    uint64 MailId = 0;
    uint8 SenderType = 0;
    ClassicGuid SenderGuid;
    int32 AltSenderId = 0;
    uint64 Cod = 0;
    int32 StationeryId = 0;
    uint64 SentMoney = 0;
    int32 Flags = 0;
    float DaysLeft = 0.0f;
    int32 MailTemplateId = 0;
    std::string Subject;
    std::string Body;
    std::vector<MailAttachedItem> Attachments;

    void Write(ByteBuffer& data) const
    {
        data << MailId << uint32(SenderType) << Cod << StationeryId << SentMoney << Flags << DaysLeft << MailTemplateId;
        data << uint32(Attachments.size());

        switch (SenderType)
        {
            case 0: // MAIL_NORMAL
                SenderGuid.Write(data);
                break;
            case 2: case 3: case 4: // MAIL_AUCTION, MAIL_CREATURE, MAIL_GAMEOBJECT
                data << AltSenderId;
                break;
            default:
                break;
        }

        data.WriteBits(uint32(Subject.length()), 8);
        data.WriteBits(uint32(Body.length()), 13);
        data.FlushBits();

        for (MailAttachedItem const& att : Attachments)
            att.Write(data);

        data.WriteString(Subject);
        data.WriteString(Body);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint64) + sizeof(uint32) + sizeof(uint64) + 3 * sizeof(int32) + sizeof(uint64) + sizeof(float))
            return false;
        uint32 senderType;
        data >> MailId >> senderType;
        SenderType = uint8(senderType);
        data >> Cod >> StationeryId >> SentMoney >> Flags >> DaysLeft >> MailTemplateId;

        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 attachCount;
        data >> attachCount;

        switch (SenderType)
        {
            case 0: // MAIL_NORMAL
                if (!SenderGuid.Read(data))
                    return false;
                break;
            case 2: case 3: case 4: // MAIL_AUCTION, MAIL_CREATURE, MAIL_GAMEOBJECT
                if (data.size() - data.rpos() < sizeof(int32))
                    return false;
                data >> AltSenderId;
                break;
            default:
                break;
        }

        if (data.size() - data.rpos() < 3) // 8 + 13 bits
            return false;
        uint32 const subjectLength = data.ReadBits(8);
        uint32 const bodyLength = data.ReadBits(13);
        data.ResetBitPos();

        Attachments.clear();
        for (uint32 i = 0; i < attachCount; ++i)
        {
            MailAttachedItem att;
            if (!att.Read(data))
                return false;
            Attachments.push_back(att);
        }

        if (data.size() - data.rpos() < subjectLength)
            return false;
        Subject = data.ReadString(subjectLength);
        if (data.size() - data.rpos() < bodyLength)
            return false;
        Body = data.ReadString(bodyLength);
        return true;
    }
};

struct MailListResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_MAIL_LIST_RESULT;

    int32 TotalNumRecords = 0;
    std::vector<MailListEntry> Mails;

    void Write(ByteBuffer& data) const
    {
        data << uint32(Mails.size());
        data << TotalNumRecords;
        for (MailListEntry const& entry : Mails)
            entry.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) + sizeof(int32))
            return false;
        uint32 count;
        data >> count >> TotalNumRecords;

        Mails.clear();
        for (uint32 i = 0; i < count; ++i)
        {
            MailListEntry entry;
            if (!entry.Read(data))
                return false;
            Mails.push_back(entry);
        }
        return true;
    }
};

#endif
