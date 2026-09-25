#ifndef MANGOS_PACKETS_CHATMESSAGES_H
#define MANGOS_PACKETS_CHATMESSAGES_H

// The modern chat message opcodes, each with its own wire shape. Kept out of Packets/Chat.h: that
// header pulls in SharedDefines.h/Packet.h, which RealBots must not depend on.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"

// Base shape shared by SAY/PARTY/RAID/RAID_WARNING/INSTANCE_CHAT: a language, an 11-bit
// length-prefixed message, and a trailing "is secure" gate bit, never set here.
struct ChatMessageSay
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_MESSAGE_SAY;

    uint32 Language = 0;
    std::string Message;

    void Write(ByteBuffer& data) const
    {
        data << Language;
        data.WriteBits(uint32(Message.size()), 11);
        data.WriteBit(false); // IsSecure
        data.FlushBits();
        data.WriteString(Message);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Language;
        if (data.size() - data.rpos() < 2) // 11 + 1 bits, flushed to two bytes
            return false;
        uint32 len = data.ReadBits(11);
        data.ReadBit(); // IsSecure
        data.ResetBitPos();
        if (data.size() - data.rpos() < len)
            return false;
        Message = data.ReadString(len);
        return true;
    }
};

struct ChatMessageParty : ChatMessageSay { static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_MESSAGE_PARTY; };
struct ChatMessageRaid : ChatMessageSay { static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_MESSAGE_RAID; };
struct ChatMessageRaidWarning : ChatMessageSay { static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_MESSAGE_RAID_WARNING; };
struct ChatMessageInstanceChat : ChatMessageSay { static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_MESSAGE_INSTANCE_CHAT; };

// Base shape shared by YELL/GUILD/OFFICER: same as ChatMessageSay but with no secure-gate bit.
struct ChatMessageYell
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_MESSAGE_YELL;

    uint32 Language = 0;
    std::string Message;

    void Write(ByteBuffer& data) const
    {
        data << Language;
        data.WriteBits(uint32(Message.size()), 11);
        data.FlushBits();
        data.WriteString(Message);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Language;
        if (data.size() - data.rpos() < 2) // 11 bits, flushed to two bytes
            return false;
        uint32 len = data.ReadBits(11);
        data.ResetBitPos();
        if (data.size() - data.rpos() < len)
            return false;
        Message = data.ReadString(len);
        return true;
    }
};

struct ChatMessageGuild : ChatMessageYell { static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_MESSAGE_GUILD; };
struct ChatMessageOfficer : ChatMessageYell { static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_MESSAGE_OFFICER; };

// EMOTE/AFK/DND carry no language at all on the wire (assumed universal) -- just the message.
struct ChatMessageEmote
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_MESSAGE_EMOTE;

    std::string Message;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(Message.size()), 11);
        data.FlushBits();
        data.WriteString(Message);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2) // 11 bits, flushed to two bytes
            return false;
        uint32 len = data.ReadBits(11);
        data.ResetBitPos();
        if (data.size() - data.rpos() < len)
            return false;
        Message = data.ReadString(len);
        return true;
    }
};

struct ChatMessageAfk : ChatMessageEmote { static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_MESSAGE_AFK; };
struct ChatMessageDnd : ChatMessageEmote { static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_MESSAGE_DND; };

// A whisper target/message string: an 11-or-7-bit length that *includes* a trailing NUL, so an
// empty string is length 1 (NUL only), not 0.
inline void WriteNulTerminatedBits(ByteBuffer& data, std::string const& value, int32 bitCount)
{
    data.WriteBits(uint32(value.size() + 1), bitCount);
}

inline bool ReadNulTerminatedBits(ByteBuffer& data, std::string& value, uint32 len)
{
    if (len == 0)
        return true;
    if (data.size() - data.rpos() < len)
        return false;
    if (len > 1)
        value = data.ReadString(len - 1);
    data.read_skip<uint8>(); // trailing NUL
    return true;
}

// This server addresses whispers and channels by name, so TargetGUID/ChannelGUID are written
// empty and read but discarded.
struct ChatMessageWhisper
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_MESSAGE_WHISPER;

    uint32 Language = 0;
    std::string TargetName;
    std::string Message;

    void Write(ByteBuffer& data) const
    {
        data << Language;
        ClassicGuid().Write(data); // TargetGUID
        data << uint32(0); // TargetVirtualRealmAddress
        WriteNulTerminatedBits(data, TargetName, 7);
        data.FlushBits();
        data.WriteString(TargetName);
        data << uint8(0);
        WriteNulTerminatedBits(data, Message, 11);
        data.FlushBits();
        data.WriteString(Message);
        data << uint8(0);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Language;
        ClassicGuid targetGuid;
        if (!targetGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 targetVirtualRealmAddress;
        data >> targetVirtualRealmAddress;

        if (data.size() - data.rpos() < 1) // 7 bits, flushed to one byte
            return false;
        uint32 targetLen = data.ReadBits(7);
        data.ResetBitPos();
        if (!ReadNulTerminatedBits(data, TargetName, targetLen))
            return false;

        if (data.size() - data.rpos() < 2) // 11 bits, flushed to two bytes
            return false;
        uint32 messageLen = data.ReadBits(11);
        data.ResetBitPos();
        return ReadNulTerminatedBits(data, Message, messageLen);
    }
};

struct ChatMessageChannel
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_MESSAGE_CHANNEL;

    uint32 Language = 0;
    std::string ChannelName;
    std::string Message;

    void Write(ByteBuffer& data) const
    {
        data << Language;
        ClassicGuid().Write(data); // ChannelGUID
        data.WriteBits(uint32(ChannelName.size()), 9);
        data.WriteBits(uint32(Message.size()), 11);
        data.WriteBit(false); // IsSecure gate
        data.FlushBits();
        data.WriteString(ChannelName);
        data.WriteString(Message);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Language;
        ClassicGuid channelGuid;
        if (!channelGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 3) // 9 + 11 + 1(+1) bits, flushed to three bytes
            return false;
        uint32 targetLen = data.ReadBits(9);
        uint32 textLen = data.ReadBits(11);
        bool secureGate = data.ReadBit();
        if (secureGate)
            data.ReadBit(); // IsSecure
        data.ResetBitPos();
        if (data.size() - data.rpos() < targetLen + textLen)
            return false;
        ChannelName = data.ReadString(targetLen);
        Message = data.ReadString(textLen);
        return true;
    }
};

// RealBots-side reader for ChatHandler::BuildChatPacket's output; the server keeps building it
// directly into a WorldPacket, so this struct's Write is for bot-side tests only.
struct Chat
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CHAT;

    uint8 MsgType = 0;    // the modern client's ChatMsg values, not vmangos's own numbering
    uint32 Language = 0;
    ClassicGuid SenderGuid;
    ClassicGuid TargetGuid;
    uint16 ChatFlags = 0; // ChatFlags bitmask (Chat/Chat.cpp)
    std::string SenderName;
    std::string TargetName;
    std::string ChannelName;
    std::string Message;
    ClassicGuid ChannelGuid; // empty when the line isn't tied to a joined channel

    void Write(ByteBuffer& data) const
    {
        data << MsgType;
        data << Language;
        SenderGuid.Write(data);
        ClassicGuid().Write(data); // SenderGuildGUID
        ClassicGuid().Write(data); // SenderAccountGUID
        TargetGuid.Write(data);
        data << uint32(0); // TargetVirtualRealmAddress
        data << uint32(0); // SenderVirtualRealmAddress
        data << uint32(0); // AchievementID
        data << ChatFlags;
        data << float(0.0f); // DisplayTime
        data << int32(0); // SpellID

        data.WriteBits(uint32(SenderName.size()), 11);
        data.WriteBits(uint32(TargetName.size()), 11);
        data.WriteBits(uint32(0), 5); // Prefix (addon chat); not supported here
        data.WriteBits(uint32(ChannelName.size()), 7);
        data.WriteBits(uint32(Message.size()), 12);
        data.WriteBit(false); // HideChatLog
        data.WriteBit(false); // FakeSenderName
        data.WriteBit(false); // Unused_801
        data.WriteBit(!ChannelGuid.IsEmpty());
        data.FlushBits();

        data.WriteString(SenderName);
        data.WriteString(TargetName);
        data.WriteString(ChannelName);
        data.WriteString(Message);
        if (!ChannelGuid.IsEmpty())
            ChannelGuid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint8))
            return false;
        data >> MsgType;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Language;
        if (!SenderGuid.Read(data))
            return false;
        ClassicGuid senderGuildGuid, senderAccountGuid;
        if (!senderGuildGuid.Read(data) || !senderAccountGuid.Read(data))
            return false;
        if (!TargetGuid.Read(data))
            return false;

        if (data.size() - data.rpos() < sizeof(uint32) * 3 + sizeof(uint16) + sizeof(float) + sizeof(int32))
            return false;
        uint32 targetVirtualRealmAddress, senderVirtualRealmAddress, achievementId;
        float displayTime;
        int32 spellId;
        data >> targetVirtualRealmAddress >> senderVirtualRealmAddress >> achievementId >> ChatFlags >> displayTime >> spellId;

        if (data.size() - data.rpos() < 7) // 11+11+5+7+12+4 bits, flushed to seven bytes
            return false;
        uint32 senderNameLen = data.ReadBits(11);
        uint32 targetNameLen = data.ReadBits(11);
        uint32 prefixLen = data.ReadBits(5);
        uint32 channelNameLen = data.ReadBits(7);
        uint32 messageLen = data.ReadBits(12);
        data.ReadBit(); // HideChatLog
        data.ReadBit(); // FakeSenderName
        data.ReadBit(); // Unused_801
        bool hasChannelGuid = data.ReadBit();
        data.ResetBitPos();

        if (data.size() - data.rpos() < senderNameLen + targetNameLen + prefixLen + channelNameLen + messageLen)
            return false;
        SenderName = data.ReadString(senderNameLen);
        TargetName = data.ReadString(targetNameLen);
        data.ReadString(prefixLen); // addon prefix; not supported here
        ChannelName = data.ReadString(channelNameLen);
        Message = data.ReadString(messageLen);

        ChannelGuid = ClassicGuid();
        if (hasChannelGuid && !ChannelGuid.Read(data))
            return false;
        return true;
    }
};

#endif // MANGOS_PACKETS_CHATMESSAGES_H
