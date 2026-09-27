#ifndef MANGOS_PACKETS_CHANNELMESSAGES_H
#define MANGOS_PACKETS_CHANNELMESSAGES_H

// The modern channel wire: join/leave, the administrative commands, chat-channel notices and the
// roster snapshot/incremental updates. Light like Packets/DuelMessages.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <string>
#include <vector>

// --- Client packets ---

struct JoinChannel
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_JOIN_CHANNEL;

    int32 ChatChannelId = 0;      // ChatChannels.dbc id for a built-in channel, 0 for a custom one
    bool CreateVoiceSession = false;
    bool Internal = false;
    std::string ChannelName;
    std::string Password;

    void Write(ByteBuffer& data) const
    {
        data << ChatChannelId;
        data.WriteBit(CreateVoiceSession);
        data.WriteBit(Internal);
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.WriteBits(uint32(Password.length()), 7);
        data.FlushBits();
        data.WriteString(ChannelName);
        data.WriteString(Password);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32) + 2) // 2 + 7 + 7 bits, flushed to two bytes
            return false;
        data >> ChatChannelId;
        CreateVoiceSession = data.ReadBit();
        Internal = data.ReadBit();
        uint32 const channelLen = data.ReadBits(7);
        uint32 const passwordLen = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen + passwordLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        Password = data.ReadString(passwordLen);
        return true;
    }
};

struct LeaveChannel
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_LEAVE_CHANNEL;

    int32 ZoneChannelId = 0;
    std::string ChannelName;

    void Write(ByteBuffer& data) const
    {
        data << ZoneChannelId;
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.FlushBits();
        data.WriteString(ChannelName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32) + 1)
            return false;
        data >> ZoneChannelId;
        uint32 const channelLen = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        return true;
    }
};

// The modern "channel name only" command family -- one 7-bit-length-prefixed string, nothing
// else. TrinityCore dispatches ANNOUNCEMENTS/DECLINE_INVITE/DISPLAY_LIST/LIST/OWNER through a
// single runtime-opcode class; kept as separate structs here to match this engine's existing
// one-struct-per-opcode style.
struct ChannelList
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_LIST;
    std::string ChannelName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.FlushBits();
        data.WriteString(ChannelName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        return true;
    }
};

// DISPLAY_LIST is handled identically to LIST (both list the channel's members).
struct ChannelDisplayList
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_DISPLAY_LIST;
    std::string ChannelName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.FlushBits();
        data.WriteString(ChannelName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        return true;
    }
};

struct ChannelOwner
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_OWNER;
    std::string ChannelName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.FlushBits();
        data.WriteString(ChannelName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        return true;
    }
};

struct ChannelAnnouncements
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_ANNOUNCEMENTS;
    std::string ChannelName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.FlushBits();
        data.WriteString(ChannelName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        return true;
    }
};

// TrinityCore's own Channel::DeclineInvite() is an empty stub on a real reference server; mirrored
// here so this opcode stops falling through to "unhandled" rather than to add unimplemented
// behavior no client build actually exercises.
struct ChannelDeclineInvite
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_DECLINE_INVITE;
    std::string ChannelName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.FlushBits();
        data.WriteString(ChannelName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        return true;
    }
};

struct ChannelPassword
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_PASSWORD;
    std::string ChannelName;
    std::string Password;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.WriteBits(uint32(Password.length()), 7);
        data.FlushBits();
        data.WriteString(ChannelName);
        data.WriteString(Password);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        uint32 const passwordLen = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen + passwordLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        Password = data.ReadString(passwordLen);
        return true;
    }
};

// The modern "channel name + target player name" command family -- channel name is 7-bit-length-
// prefixed, player name is 9-bit-length-prefixed. Covers BAN, INVITE, KICK, MODERATOR, SET_OWNER,
// SILENCE_ALL, UNBAN, UNMODERATOR, UNSILENCE_ALL.
struct ChannelSetOwner
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_SET_OWNER;
    std::string ChannelName;
    std::string PlayerName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.WriteBits(uint32(PlayerName.length()), 9);
        data.FlushBits();
        data.WriteString(ChannelName);
        data.WriteString(PlayerName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        uint32 const playerLen = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen + playerLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        PlayerName = data.ReadString(playerLen);
        return true;
    }
};

struct ChannelModerator
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_MODERATOR;
    std::string ChannelName;
    std::string PlayerName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.WriteBits(uint32(PlayerName.length()), 9);
        data.FlushBits();
        data.WriteString(ChannelName);
        data.WriteString(PlayerName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        uint32 const playerLen = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen + playerLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        PlayerName = data.ReadString(playerLen);
        return true;
    }
};

struct ChannelUnmoderator
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_UNMODERATOR;
    std::string ChannelName;
    std::string PlayerName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.WriteBits(uint32(PlayerName.length()), 9);
        data.FlushBits();
        data.WriteString(ChannelName);
        data.WriteString(PlayerName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        uint32 const playerLen = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen + playerLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        PlayerName = data.ReadString(playerLen);
        return true;
    }
};

struct ChannelInvite
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_INVITE;
    std::string ChannelName;
    std::string PlayerName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.WriteBits(uint32(PlayerName.length()), 9);
        data.FlushBits();
        data.WriteString(ChannelName);
        data.WriteString(PlayerName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        uint32 const playerLen = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen + playerLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        PlayerName = data.ReadString(playerLen);
        return true;
    }
};

struct ChannelKick
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_KICK;
    std::string ChannelName;
    std::string PlayerName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.WriteBits(uint32(PlayerName.length()), 9);
        data.FlushBits();
        data.WriteString(ChannelName);
        data.WriteString(PlayerName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        uint32 const playerLen = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen + playerLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        PlayerName = data.ReadString(playerLen);
        return true;
    }
};

struct ChannelBan
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_BAN;
    std::string ChannelName;
    std::string PlayerName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.WriteBits(uint32(PlayerName.length()), 9);
        data.FlushBits();
        data.WriteString(ChannelName);
        data.WriteString(PlayerName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        uint32 const playerLen = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen + playerLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        PlayerName = data.ReadString(playerLen);
        return true;
    }
};

struct ChannelUnban
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_UNBAN;
    std::string ChannelName;
    std::string PlayerName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.WriteBits(uint32(PlayerName.length()), 9);
        data.FlushBits();
        data.WriteString(ChannelName);
        data.WriteString(PlayerName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        uint32 const playerLen = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen + playerLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        PlayerName = data.ReadString(playerLen);
        return true;
    }
};

// TrinityCore's own Channel::SilenceAll()/UnsilenceAll() are themselves empty stubs (no-op) on a
// real reference server -- mirrored here so these opcodes stop falling through to "unhandled"
// rather than to reuse the old vanilla per-player mute, which the modern command no longer drives.
struct ChannelSilenceAll
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_SILENCE_ALL;
    std::string ChannelName;
    std::string PlayerName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.WriteBits(uint32(PlayerName.length()), 9);
        data.FlushBits();
        data.WriteString(ChannelName);
        data.WriteString(PlayerName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        uint32 const playerLen = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen + playerLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        PlayerName = data.ReadString(playerLen);
        return true;
    }
};

struct ChannelUnsilenceAll
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAT_CHANNEL_UNSILENCE_ALL;
    std::string ChannelName;
    std::string PlayerName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.length()), 7);
        data.WriteBits(uint32(PlayerName.length()), 9);
        data.FlushBits();
        data.WriteString(ChannelName);
        data.WriteString(PlayerName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        uint32 const playerLen = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < channelLen + playerLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        PlayerName = data.ReadString(playerLen);
        return true;
    }
};

// --- Server packets ---

// The general notify family (join/left/error notices, moderator flag changes, ...). "You joined"
// and "you left" no longer ride this opcode for a real player -- see ChannelNotifyJoined and
// ChannelNotifyLeft below.
struct ChannelNotify
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CHANNEL_NOTIFY;

    // Mirrors ChatNotify::CHAT_MODE_CHANGE_NOTICE (Chat/Channel.h) -- the only notify type whose
    // body carries the extra before/after member-flag byte pair.
    static constexpr uint8 ModeChangeNotice = 0x0C;

    uint8 Type = 0;
    std::string ChannelName;
    std::string Sender;
    ClassicGuid SenderGuid;
    ClassicGuid SenderAccountGuid;
    uint32 SenderVirtualRealm = 0;
    ClassicGuid TargetGuid;
    uint32 TargetVirtualRealm = 0;
    int32 ChatChannelId = 0;
    uint8 OldFlags = 0;
    uint8 NewFlags = 0;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(Type, 6);
        data.WriteBits(uint32(ChannelName.size()), 7);
        data.WriteBits(uint32(Sender.size()), 6);
        data.FlushBits();

        SenderGuid.Write(data);
        SenderAccountGuid.Write(data);
        data << SenderVirtualRealm;
        TargetGuid.Write(data);
        data << TargetVirtualRealm;
        data << ChatChannelId;

        if (Type == ModeChangeNotice)
        {
            data << OldFlags;
            data << NewFlags;
        }

        data.WriteString(ChannelName);
        data.WriteString(Sender);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 3) // 6 + 7 + 6 bits, flushed to three bytes
            return false;
        Type = uint8(data.ReadBits(6));
        uint32 const channelLen = data.ReadBits(7);
        uint32 const senderLen = data.ReadBits(6);
        data.ResetBitPos();

        if (!SenderGuid.Read(data) || !SenderAccountGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> SenderVirtualRealm;
        if (!TargetGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + sizeof(int32))
            return false;
        data >> TargetVirtualRealm >> ChatChannelId;

        if (Type == ModeChangeNotice)
        {
            if (data.size() - data.rpos() < 2)
                return false;
            data >> OldFlags >> NewFlags;
        }

        if (data.size() - data.rpos() < channelLen + senderLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        Sender = data.ReadString(senderLen);
        return true;
    }
};

// The "you personally joined this channel" notice a real player gets -- distinct from
// ChannelNotify's generic JOINED_NOTICE (which announces someone else's join to the channel).
struct ChannelNotifyJoined
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CHANNEL_NOTIFY_JOINED;

    std::string ChannelName;
    std::string ChannelWelcomeMsg; // no welcome-message feature on this server; always empty
    uint32 ChannelFlags = 0;
    int32 ChatChannelId = 0;       // ChatChannels.dbc id for a built-in channel, 0 for a custom one
    uint64 InstanceId = 0;
    ClassicGuid ChannelGuid;       // see ClassicGuidMapping::MakeChannel

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.size()), 7);
        data.WriteBits(uint32(ChannelWelcomeMsg.size()), 11);
        data.FlushBits();

        data << ChannelFlags;
        data << ChatChannelId;
        data << InstanceId;
        ChannelGuid.Write(data);

        data.WriteString(ChannelName);
        data.WriteString(ChannelWelcomeMsg);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 3) // 7 + 11 bits, flushed to three bytes
            return false;
        uint32 const channelLen = data.ReadBits(7);
        uint32 const welcomeLen = data.ReadBits(11);
        data.ResetBitPos();

        if (data.size() - data.rpos() < sizeof(uint32) + sizeof(int32) + sizeof(uint64))
            return false;
        data >> ChannelFlags >> ChatChannelId >> InstanceId;
        if (!ChannelGuid.Read(data))
            return false;

        if (data.size() - data.rpos() < channelLen + welcomeLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        ChannelWelcomeMsg = data.ReadString(welcomeLen);
        return true;
    }
};

// The "you personally left this channel" notice -- distinct opcode from ChannelNotify's generic
// LEFT_NOTICE (which announces someone else's departure).
struct ChannelNotifyLeft
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CHANNEL_NOTIFY_LEFT;

    std::string ChannelName;
    int32 ChatChannelId = 0;
    bool Suspended = false; // false on a normal leave, true only on an automatic zone-change drop

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(ChannelName.size()), 7);
        data.WriteBit(Suspended);
        data.FlushBits();
        data << ChatChannelId;
        data.WriteString(ChannelName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const channelLen = data.ReadBits(7);
        Suspended = data.ReadBit();
        data.ResetBitPos();

        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> ChatChannelId;

        if (data.size() - data.rpos() < channelLen)
            return false;
        ChannelName = data.ReadString(channelLen);
        return true;
    }
};

// One roster entry in ChannelListResponse below.
struct ChannelListMember
{
    ClassicGuid Guid;
    uint32 VirtualRealmAddress = 0;
    uint8 Flags = 0; // Channel::ChannelMemberFlags

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << VirtualRealmAddress;
        data << Flags;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + 1)
            return false;
        data >> VirtualRealmAddress >> Flags;
        return true;
    }
};

struct ChannelListResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CHANNEL_LIST;

    bool Display = false; // always true on a real reference server
    std::string ChannelName;
    uint32 ChannelFlags = 0;
    std::vector<ChannelListMember> Members;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(Display);
        data.WriteBits(uint32(ChannelName.size()), 7);
        data.FlushBits();

        data << ChannelFlags;
        data << uint32(Members.size());
        data.WriteString(ChannelName);
        for (ChannelListMember const& member : Members)
            member.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        Display = data.ReadBit();
        uint32 const channelLen = data.ReadBits(7);
        data.ResetBitPos();

        if (data.size() - data.rpos() < 2 * sizeof(uint32))
            return false;
        data >> ChannelFlags;
        uint32 count;
        data >> count;

        if (data.size() - data.rpos() < channelLen)
            return false;
        ChannelName = data.ReadString(channelLen);

        Members.clear();
        for (uint32 i = 0; i < count; ++i)
        {
            ChannelListMember member;
            if (!member.Read(data))
                return false;
            Members.push_back(member);
        }
        return true;
    }
};

// Modern SMSG_USERLIST_ADD/REMOVE/UPDATE -- incremental roster updates the client uses to keep a
// joined channel's member list in sync without a full ChannelListResponse round trip.
struct UserlistAdd
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_USERLIST_ADD;

    ClassicGuid AddedUserGuid;
    uint8 UserFlags = 0;
    uint32 ChannelFlags = 0;
    uint32 ChannelId = 0;
    std::string ChannelName;

    void Write(ByteBuffer& data) const
    {
        AddedUserGuid.Write(data);
        data << UserFlags;
        data << ChannelFlags;
        data << ChannelId;
        data.WriteBits(uint32(ChannelName.size()), 7);
        data.FlushBits();
        data.WriteString(ChannelName);
    }

    bool Read(ByteBuffer& data)
    {
        if (!AddedUserGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 1 + 2 * sizeof(uint32) + 1) // +1 for the 7-bit name length that follows
            return false;
        data >> UserFlags >> ChannelFlags >> ChannelId;
        uint32 const nameLen = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLen)
            return false;
        ChannelName = data.ReadString(nameLen);
        return true;
    }
};

struct UserlistRemove
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_USERLIST_REMOVE;

    ClassicGuid RemovedUserGuid;
    uint32 ChannelFlags = 0;
    uint32 ChannelId = 0;
    std::string ChannelName;

    void Write(ByteBuffer& data) const
    {
        RemovedUserGuid.Write(data);
        data << ChannelFlags;
        data << ChannelId;
        data.WriteBits(uint32(ChannelName.size()), 7);
        data.FlushBits();
        data.WriteString(ChannelName);
    }

    bool Read(ByteBuffer& data)
    {
        if (!RemovedUserGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32) + 1) // +1 for the 7-bit name length that follows
            return false;
        data >> ChannelFlags >> ChannelId;
        uint32 const nameLen = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLen)
            return false;
        ChannelName = data.ReadString(nameLen);
        return true;
    }
};

struct UserlistUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_USERLIST_UPDATE;

    ClassicGuid UpdatedUserGuid;
    uint8 UserFlags = 0;
    uint32 ChannelFlags = 0;
    uint32 ChannelId = 0;
    std::string ChannelName;

    void Write(ByteBuffer& data) const
    {
        UpdatedUserGuid.Write(data);
        data << UserFlags;
        data << ChannelFlags;
        data << ChannelId;
        data.WriteBits(uint32(ChannelName.size()), 7);
        data.FlushBits();
        data.WriteString(ChannelName);
    }

    bool Read(ByteBuffer& data)
    {
        if (!UpdatedUserGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 1 + 2 * sizeof(uint32) + 1) // +1 for the 7-bit name length that follows
            return false;
        data >> UserFlags >> ChannelFlags >> ChannelId;
        uint32 const nameLen = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLen)
            return false;
        ChannelName = data.ReadString(nameLen);
        return true;
    }
};

#endif
