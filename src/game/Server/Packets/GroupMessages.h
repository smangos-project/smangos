#ifndef MANGOS_PACKETS_GROUPMESSAGES_H
#define MANGOS_PACKETS_GROUPMESSAGES_H

// The modern group wire: invitations, the party roster, its leader, loot and assistant settings,
// raid markers, ready checks, and the member state a party frame shows. Every client request may
// carry a party index (0 is the player's own party, 1 the instance party); the server has one party
// per player, so it is parsed and ignored. Light like Packets/LootMessages.h, so RealBots can
// include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <optional>
#include <string>
#include <vector>

constexpr uint32 CLASSIC_MAX_PARTY_MEMBERS = 40;
constexpr uint32 CLASSIC_MAX_PARTY_AURAS = 255;
constexpr uint32 CLASSIC_MAX_PARTY_AURA_POINTS = 64;
constexpr uint32 CLASSIC_MAX_PARTY_NAME_LENGTH = 48;

// PartyUpdate::PartyFlags.
constexpr uint16 CLASSIC_PARTY_FLAG_FAKE_RAID = 0x001;
constexpr uint16 CLASSIC_PARTY_FLAG_RAID = 0x002;
constexpr uint16 CLASSIC_PARTY_FLAG_DESTROYED = 0x010;

// PartyUpdate::PartyType, and the pair a member's state carries.
constexpr uint8 CLASSIC_PARTY_TYPE_NONE = 0;
constexpr uint8 CLASSIC_PARTY_TYPE_NORMAL = 1;
constexpr uint8 CLASSIC_PARTY_TYPE_PVP = 2;

// PartyPlayerInfo::Flags.
constexpr uint8 CLASSIC_PARTY_MEMBER_FLAG_ASSISTANT = 0x01;
constexpr uint8 CLASSIC_PARTY_MEMBER_FLAG_MAIN_TANK = 0x02;
constexpr uint8 CLASSIC_PARTY_MEMBER_FLAG_MAIN_ASSIST = 0x04;

// The phase shift a member with no phases carries: everyone sees them.
constexpr uint32 CLASSIC_PARTY_PHASE_SHIFT_UNPHASED = 0x08;

// SetPartyAssignment::Assignment.
constexpr uint8 CLASSIC_PARTY_ASSIGN_MAIN_TANK = 0;
constexpr uint8 CLASSIC_PARTY_ASSIGN_MAIN_ASSIST = 1;

// The optional trailing party index every client request may carry.
inline void ClassicWritePartyIndex(ByteBuffer& data, std::optional<uint8> const& index)
{
    if (index)
        data << *index;
}

inline bool ClassicReadPartyIndex(ByteBuffer& data, bool present, std::optional<uint8>& index)
{
    index.reset();
    if (!present)
        return true;
    if (data.size() - data.rpos() < 1)
        return false;
    uint8 value;
    data >> value;
    index = value;
    return true;
}

// Reads `length` string bytes, refusing a body that ends first (ByteBuffer::ReadString clamps).
inline bool ClassicReadPartyString(ByteBuffer& data, uint32 length, std::string& out)
{
    if (data.size() - data.rpos() < length)
        return false;
    out = data.ReadString(length);
    return true;
}

// A player asks another to join their party.
struct PartyInviteClient
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PARTY_INVITE;

    std::optional<uint8> PartyIndex;
    uint32 ProposedRoles = 0;
    ClassicGuid TargetGuid;
    std::string TargetName;
    std::string TargetRealm;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.FlushBits();
        data.WriteBits(uint32(TargetName.length()), 9);
        data.WriteBits(uint32(TargetRealm.length()), 9);
        data.FlushBits();
        data << ProposedRoles;
        TargetGuid.Write(data);
        data.WriteString(TargetName);
        data.WriteString(TargetRealm);
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        // 1 bit and 2 * 9 bits, each group flushed to whole bytes, then the roles
        if (data.size() - data.rpos() < 1 + 3 + sizeof(uint32))
            return false;
        bool const hasPartyIndex = data.ReadBit();
        data.ResetBitPos();
        uint32 const nameLength = data.ReadBits(9);
        uint32 const realmLength = data.ReadBits(9);
        data.ResetBitPos();
        data >> ProposedRoles;
        if (!TargetGuid.Read(data))
            return false;
        return ClassicReadPartyString(data, nameLength, TargetName)
            && ClassicReadPartyString(data, realmLength, TargetRealm)
            && ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

// The invited player's answer.
struct PartyInviteResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PARTY_INVITE_RESPONSE;

    std::optional<uint8> PartyIndex;
    bool Accept = false;
    std::optional<uint32> RolesDesired;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.WriteBit(Accept);
        data.WriteBit(RolesDesired.has_value());
        data.FlushBits();
        ClassicWritePartyIndex(data, PartyIndex);
        if (RolesDesired)
            data << *RolesDesired;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasPartyIndex = data.ReadBit();
        Accept = data.ReadBit();
        bool const hasRoles = data.ReadBit();
        data.ResetBitPos();
        if (!ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex))
            return false;
        RolesDesired.reset();
        if (hasRoles)
        {
            if (data.size() - data.rpos() < sizeof(uint32))
                return false;
            uint32 roles;
            data >> roles;
            RolesDesired = roles;
        }
        return true;
    }
};

// The leader or an assistant removes a member (or a pending invitee).
struct PartyUninvite
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PARTY_UNINVITE;

    std::optional<uint8> PartyIndex;
    ClassicGuid TargetGuid;
    std::string Reason;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.WriteBits(uint32(Reason.length()), 8);
        data.FlushBits();
        TargetGuid.Write(data);
        ClassicWritePartyIndex(data, PartyIndex);
        data.WriteString(Reason);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        bool const hasPartyIndex = data.ReadBit();
        uint32 const reasonLength = data.ReadBits(8);
        data.ResetBitPos();
        if (!TargetGuid.Read(data) || !ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex))
            return false;
        return ClassicReadPartyString(data, reasonLength, Reason);
    }
};

// A member leaves the party.
struct LeaveGroup
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_LEAVE_GROUP;

    std::optional<uint8> PartyIndex;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.FlushBits();
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasPartyIndex = data.ReadBit();
        data.ResetBitPos();
        return ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

struct SetPartyLeader
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_PARTY_LEADER;

    std::optional<uint8> PartyIndex;
    ClassicGuid TargetGuid;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.FlushBits();
        TargetGuid.Write(data);
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasPartyIndex = data.ReadBit();
        data.ResetBitPos();
        return TargetGuid.Read(data) && ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

// The leader makes a member an assistant, or takes it back.
struct SetAssistantLeader
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_ASSISTANT_LEADER;

    std::optional<uint8> PartyIndex;
    bool Apply = false;
    ClassicGuid Target;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.WriteBit(Apply);
        data.FlushBits();
        Target.Write(data);
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasPartyIndex = data.ReadBit();
        Apply = data.ReadBit();
        data.ResetBitPos();
        return Target.Read(data) && ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

// The leader gives a member the main tank or main assist mark, or takes it back.
struct SetPartyAssignment
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_PARTY_ASSIGNMENT;

    std::optional<uint8> PartyIndex;
    bool Set = false;
    uint8 Assignment = 0;
    ClassicGuid Target;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.WriteBit(Set);
        data.FlushBits();
        data << Assignment;
        Target.Write(data);
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        bool const hasPartyIndex = data.ReadBit();
        Set = data.ReadBit();
        data.ResetBitPos();
        data >> Assignment;
        return Target.Read(data) && ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

// The leader picks how loot is shared: 0 free for all, 1 round robin, 2 master, 3 group, 4 need
// before greed.
struct SetLootMethod
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_LOOT_METHOD;

    std::optional<uint8> PartyIndex;
    uint8 Method = 0;
    ClassicGuid LootMaster;
    uint32 Threshold = 0;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.FlushBits();
        data << Method;
        LootMaster.Write(data);
        data << Threshold;
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1 + 1)
            return false;
        bool const hasPartyIndex = data.ReadBit();
        data.ResetBitPos();
        data >> Method;
        if (!LootMaster.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Threshold;
        return ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

// A ping on the minimap, shown to the party.
struct MinimapPingClient
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MINIMAP_PING;

    std::optional<uint8> PartyIndex;
    float PositionX = 0.0f;
    float PositionY = 0.0f;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.FlushBits();
        data << PositionX << PositionY;
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1 + 2 * sizeof(float))
            return false;
        bool const hasPartyIndex = data.ReadBit();
        data.ResetBitPos();
        data >> PositionX >> PositionY;
        return ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

// `/roll`.
struct RandomRollClient
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_RANDOM_ROLL;

    std::optional<uint8> PartyIndex;
    int32 Min = 0;
    int32 Max = 0;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.FlushBits();
        data << Min << Max;
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1 + 2 * sizeof(int32))
            return false;
        bool const hasPartyIndex = data.ReadBit();
        data.ResetBitPos();
        data >> Min >> Max;
        return ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

// Puts a raid marker on a target, or (symbol -1) asks for the current markers.
struct UpdateRaidTarget
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_UPDATE_RAID_TARGET;

    std::optional<uint8> PartyIndex;
    ClassicGuid Target;
    int8 Symbol = 0;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.FlushBits();
        Target.Write(data);
        data << Symbol;
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasPartyIndex = data.ReadBit();
        data.ResetBitPos();
        if (!Target.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int8))
            return false;
        data >> Symbol;
        return ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

// Turns the party into a raid, or a raid back into a party.
struct ConvertRaid
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CONVERT_RAID;

    bool Raid = false;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(Raid);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        Raid = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Asks for the party's raid markers.
struct RequestPartyJoinUpdates
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_REQUEST_PARTY_JOIN_UPDATES;

    std::optional<uint8> PartyIndex;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.FlushBits();
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasPartyIndex = data.ReadBit();
        data.ResetBitPos();
        return ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

// Moves a raid member to another subgroup.
struct ChangeSubGroup
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHANGE_SUB_GROUP;

    ClassicGuid Target;
    uint8 NewSubGroup = 0;
    std::optional<uint8> PartyIndex;

    void Write(ByteBuffer& data) const
    {
        Target.Write(data);
        data << NewSubGroup;
        data.WriteBit(PartyIndex.has_value());
        data.FlushBits();
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Target.Read(data))
            return false;
        if (data.size() - data.rpos() < 1 + 1)
            return false;
        data >> NewSubGroup;
        bool const hasPartyIndex = data.ReadBit();
        data.ResetBitPos();
        return ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

// Swaps two raid members' subgroups.
struct SwapSubGroups
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SWAP_SUB_GROUPS;

    std::optional<uint8> PartyIndex;
    ClassicGuid FirstTarget;
    ClassicGuid SecondTarget;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.FlushBits();
        FirstTarget.Write(data);
        SecondTarget.Write(data);
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasPartyIndex = data.ReadBit();
        data.ResetBitPos();
        return FirstTarget.Read(data) && SecondTarget.Read(data) && ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

// The leader or an assistant asks everyone whether they are ready.
struct DoReadyCheck
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_DO_READY_CHECK;

    std::optional<uint8> PartyIndex;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.FlushBits();
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasPartyIndex = data.ReadBit();
        data.ResetBitPos();
        return ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

// A member's answer to a ready check.
struct ReadyCheckResponseClient
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_READY_CHECK_RESPONSE;

    bool IsReady = false;
    std::optional<uint8> PartyIndex;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(IsReady);
        data.WriteBit(PartyIndex.has_value());
        data.FlushBits();
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        IsReady = data.ReadBit();
        bool const hasPartyIndex = data.ReadBit();
        data.ResetBitPos();
        return ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

// Asks for a member's full state (health, position, auras, ...).
struct RequestPartyMemberStats
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_REQUEST_PARTY_MEMBER_STATS;

    std::optional<uint8> PartyIndex;
    ClassicGuid Target;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartyIndex.has_value());
        data.FlushBits();
        Target.Write(data);
        ClassicWritePartyIndex(data, PartyIndex);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasPartyIndex = data.ReadBit();
        data.ResetBitPos();
        return Target.Read(data) && ClassicReadPartyIndex(data, hasPartyIndex, PartyIndex);
    }
};

// What a party command came to: `Command` is the operation (0 invite, 1 uninvite, 2 leave, 4 swap),
// `Result` the reason (0 is success).
struct PartyCommandResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PARTY_COMMAND_RESULT;

    std::string Name;
    uint8 Command = 0;
    uint8 Result = 0;
    uint32 ResultData = 0;
    ClassicGuid ResultGuid;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(Name.length()), 9);
        data.WriteBits(Command, 4);
        data.WriteBits(Result, 6);
        data.FlushBits();
        data << ResultData;
        ResultGuid.Write(data);
        data.WriteString(Name);
    }

    bool Read(ByteBuffer& data)
    {
        // 9 + 4 + 6 bits, flushed to three bytes
        if (data.size() - data.rpos() < 3 + sizeof(uint32))
            return false;
        uint32 const nameLength = data.ReadBits(9);
        Command = uint8(data.ReadBits(4));
        Result = uint8(data.ReadBits(6));
        data.ResetBitPos();
        data >> ResultData;
        return ResultGuid.Read(data) && ClassicReadPartyString(data, nameLength, Name);
    }
};

// The realm a player belongs to, as the invitation names it.
struct PartyInviteRealm
{
    uint32 RealmAddress = 0;
    bool IsLocal = true;
    bool IsInternalRealm = false;
    std::string RealmNameActual;
    std::string RealmNameNormalized;

    void Write(ByteBuffer& data) const
    {
        data << RealmAddress;
        data.WriteBit(IsLocal);
        data.WriteBit(IsInternalRealm);
        data.WriteBits(uint32(RealmNameActual.length()), 8);
        data.WriteBits(uint32(RealmNameNormalized.length()), 8);
        data.FlushBits();
        data.WriteString(RealmNameActual);
        data.WriteString(RealmNameNormalized);
    }

    bool Read(ByteBuffer& data)
    {
        // the address, then 2 + 8 + 8 bits flushed to three bytes
        if (data.size() - data.rpos() < sizeof(uint32) + 3)
            return false;
        data >> RealmAddress;
        IsLocal = data.ReadBit();
        IsInternalRealm = data.ReadBit();
        uint32 const actualLength = data.ReadBits(8);
        uint32 const normalizedLength = data.ReadBits(8);
        data.ResetBitPos();
        return ClassicReadPartyString(data, actualLength, RealmNameActual)
            && ClassicReadPartyString(data, normalizedLength, RealmNameNormalized);
    }
};

// Someone invited the player to a party.
struct PartyInvite
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PARTY_INVITE;

    bool CanAccept = true;
    bool MightCRZYou = false;
    bool IsXRealm = false;
    bool MustBeBNetFriend = false;
    bool AllowMultipleRoles = false;
    bool QuestSessionActive = false;
    std::string InviterName;
    PartyInviteRealm InviterRealm;
    ClassicGuid InviterGuid;
    ClassicGuid InviterBNetAccountId;
    uint16 Unk1 = 0;
    uint8 ProposedRoles = 0;
    uint32 LfgCompletedMask = 0;

    // The dungeon finder slots are never sent, and a body that names one is rejected on read.
    void Write(ByteBuffer& data) const
    {
        data.WriteBit(CanAccept);
        data.WriteBit(MightCRZYou);
        data.WriteBit(IsXRealm);
        data.WriteBit(MustBeBNetFriend);
        data.WriteBit(AllowMultipleRoles);
        data.WriteBit(QuestSessionActive);
        data.WriteBits(uint32(InviterName.length()), 6);
        data.WriteBit(false);  // unused
        data.FlushBits();
        InviterRealm.Write(data);
        InviterGuid.Write(data);
        InviterBNetAccountId.Write(data);
        data << Unk1 << ProposedRoles;
        data << uint32(0);  // dungeon finder slots
        data << LfgCompletedMask;
        data.WriteString(InviterName);
    }

    bool Read(ByteBuffer& data)
    {
        // 6 + 6 + 1 bits, flushed to two bytes
        if (data.size() - data.rpos() < 2)
            return false;
        CanAccept = data.ReadBit();
        MightCRZYou = data.ReadBit();
        IsXRealm = data.ReadBit();
        MustBeBNetFriend = data.ReadBit();
        AllowMultipleRoles = data.ReadBit();
        QuestSessionActive = data.ReadBit();
        uint32 const nameLength = data.ReadBits(6);
        bool const unused = data.ReadBit();
        data.ResetBitPos();
        if (unused)
            return false;
        if (!InviterRealm.Read(data) || !InviterGuid.Read(data) || !InviterBNetAccountId.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint16) + 1 + 2 * sizeof(uint32))
            return false;
        uint32 slots;
        data >> Unk1 >> ProposedRoles >> slots >> LfgCompletedMask;
        return slots == 0 && ClassicReadPartyString(data, nameLength, InviterName);
    }
};

// The invitee declined.
struct GroupDecline
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GROUP_DECLINE;

    std::string Name;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(Name.length()), 9);
        data.FlushBits();
        data.WriteString(Name);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const nameLength = data.ReadBits(9);
        data.ResetBitPos();
        return ClassicReadPartyString(data, nameLength, Name);
    }
};

// The player was removed from the party.
struct GroupUninvite
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GROUP_UNINVITE;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// The party was disbanded.
struct GroupDestroyed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GROUP_DESTROYED;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// The party has a new leader.
struct GroupNewLeader
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GROUP_NEW_LEADER;

    uint8 PartyIndex = 0;
    std::string Name;

    void Write(ByteBuffer& data) const
    {
        data << PartyIndex;
        data.WriteBits(uint32(Name.length()), 9);
        data.FlushBits();
        data.WriteString(Name);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1 + 2)
            return false;
        data >> PartyIndex;
        uint32 const nameLength = data.ReadBits(9);
        data.ResetBitPos();
        return ClassicReadPartyString(data, nameLength, Name);
    }
};

// One member of the roster.
struct PartyPlayerInfo
{
    ClassicGuid Guid;
    std::string Name;
    uint8 Subgroup = 0;
    uint8 Flags = 0;
    uint8 RolesAssigned = 0;
    uint8 ClassId = 0;
    uint8 FactionGroup = 0;
    bool Connected = false;

    // The voice chat id is always empty, announced as the length 1 the client expects for it.
    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(Name.length()), 6);
        data.WriteBits(1, 6);
        data.WriteBit(Connected);
        data.WriteBit(false);  // voice chat silenced
        data.WriteBit(false);  // from social queue
        data.FlushBits();
        Guid.Write(data);
        data << Subgroup << Flags << RolesAssigned << ClassId << FactionGroup;
        data.WriteString(Name);
    }

    bool Read(ByteBuffer& data)
    {
        // 6 + 6 + 3 bits, flushed to two bytes
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const nameLength = data.ReadBits(6);
        uint32 const voiceLength = data.ReadBits(6);
        Connected = data.ReadBit();
        bool const silenced = data.ReadBit();
        bool const fromSocialQueue = data.ReadBit();
        data.ResetBitPos();
        if (voiceLength != 1 || silenced || fromSocialQueue || !Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < 5)
            return false;
        data >> Subgroup >> Flags >> RolesAssigned >> ClassId >> FactionGroup;
        return ClassicReadPartyString(data, nameLength, Name);
    }
};

struct PartyLootSettings
{
    uint8 Method = 0;
    ClassicGuid LootMaster;
    uint8 Threshold = 0;

    void Write(ByteBuffer& data) const
    {
        data << Method;
        LootMaster.Write(data);
        data << Threshold;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data >> Method;
        if (!LootMaster.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        data >> Threshold;
        return true;
    }
};

struct PartyDifficultySettings
{
    uint32 DungeonDifficultyId = 0;
    uint32 RaidDifficultyId = 0;
    uint32 LegacyRaidDifficultyId = 0;

    void Write(ByteBuffer& data) const { data << DungeonDifficultyId << RaidDifficultyId << LegacyRaidDifficultyId; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 3 * sizeof(uint32))
            return false;
        data >> DungeonDifficultyId >> RaidDifficultyId >> LegacyRaidDifficultyId;
        return true;
    }
};

// The whole roster, sent to each member whenever it changes. A destroyed party is sent with the
// destroyed flag, no members and index -1, so the client drops its party frames. The sequence
// number only ever grows per player, so the client can tell a stale update from a newer one.
struct PartyUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PARTY_UPDATE;

    uint16 PartyFlags = 0;
    uint8 PartyIndex = 0;
    uint8 PartyType = CLASSIC_PARTY_TYPE_NORMAL;
    int32 MyIndex = -1;
    ClassicGuid PartyGuid;
    uint32 SequenceNum = 0;
    ClassicGuid LeaderGuid;
    uint8 LeaderFactionGroup = 0;
    int32 PingRestriction = 0;
    std::vector<PartyPlayerInfo> Players;
    std::optional<PartyLootSettings> LootSettings;
    std::optional<PartyDifficultySettings> DifficultySettings;

    // The dungeon finder block is never sent, and a body that carries one is rejected on read.
    void Write(ByteBuffer& data) const
    {
        data << PartyFlags << PartyIndex << PartyType << MyIndex;
        PartyGuid.Write(data);
        data << SequenceNum;
        LeaderGuid.Write(data);
        data << LeaderFactionGroup << PingRestriction;
        data << uint32(Players.size());
        data.WriteBit(false);  // dungeon finder block
        data.WriteBit(LootSettings.has_value());
        data.WriteBit(DifficultySettings.has_value());
        data.FlushBits();
        for (PartyPlayerInfo const& player : Players)
            player.Write(data);
        if (LootSettings)
            LootSettings->Write(data);
        if (DifficultySettings)
            DifficultySettings->Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2 + 1 + 1 + sizeof(int32))
            return false;
        data >> PartyFlags >> PartyIndex >> PartyType >> MyIndex;
        if (!PartyGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> SequenceNum;
        if (!LeaderGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 1 + sizeof(int32) + sizeof(uint32) + 1)
            return false;
        uint32 count;
        data >> LeaderFactionGroup >> PingRestriction >> count;
        bool const hasDungeonFinder = data.ReadBit();
        bool const hasLoot = data.ReadBit();
        bool const hasDifficulty = data.ReadBit();
        data.ResetBitPos();
        if (hasDungeonFinder || count > CLASSIC_MAX_PARTY_MEMBERS)
            return false;
        Players.assign(count, PartyPlayerInfo());
        for (PartyPlayerInfo& player : Players)
            if (!player.Read(data))
                return false;
        LootSettings.reset();
        if (hasLoot)
        {
            LootSettings.emplace();
            if (!LootSettings->Read(data))
                return false;
        }
        DifficultySettings.reset();
        if (hasDifficulty)
        {
            DifficultySettings.emplace();
            if (!DifficultySettings->Read(data))
                return false;
        }
        return true;
    }
};

// One aura on a party member. Points is filled like AuraDataInfo::Points.
struct PartyMemberAuraStates
{
    int32 SpellId = 0;
    uint16 Flags = 0;
    uint32 ActiveFlags = 0;
    std::vector<float> Points;

    void Write(ByteBuffer& data) const
    {
        data << SpellId << Flags << ActiveFlags;
        data << uint32(Points.size());
        for (float points : Points)
            data << points;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32) + sizeof(uint16) + 2 * sizeof(uint32))
            return false;
        uint32 count;
        data >> SpellId >> Flags >> ActiveFlags >> count;
        if (count > CLASSIC_MAX_PARTY_AURA_POINTS || data.size() - data.rpos() < count * sizeof(float))
            return false;
        Points.resize(count);
        for (float& points : Points)
            data >> points;
        return true;
    }
};

inline void ClassicWritePartyAuras(ByteBuffer& data, std::vector<PartyMemberAuraStates> const& auras)
{
    for (PartyMemberAuraStates const& aura : auras)
        aura.Write(data);
}

inline bool ClassicReadPartyAuras(ByteBuffer& data, uint32 count, std::vector<PartyMemberAuraStates>& auras)
{
    // Every aura is at least 14 bytes.
    if (count > CLASSIC_MAX_PARTY_AURAS || count > (data.size() - data.rpos()) / 14)
        return false;
    auras.assign(count, PartyMemberAuraStates());
    for (PartyMemberAuraStates& aura : auras)
        if (!aura.Read(data))
            return false;
    return true;
}

struct PartyMemberPetStats
{
    ClassicGuid Guid;
    int32 ModelId = 0;
    int32 CurrentHealth = 0;
    int32 MaxHealth = 0;
    std::vector<PartyMemberAuraStates> Auras;
    std::string Name;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << ModelId << CurrentHealth << MaxHealth;
        data << uint32(Auras.size());
        ClassicWritePartyAuras(data, Auras);
        data.WriteBits(uint32(Name.length()), 8);
        data.FlushBits();
        data.WriteString(Name);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < 4 * sizeof(uint32))
            return false;
        uint32 auraCount;
        data >> ModelId >> CurrentHealth >> MaxHealth >> auraCount;
        if (!ClassicReadPartyAuras(data, auraCount, Auras))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const nameLength = data.ReadBits(8);
        data.ResetBitPos();
        return ClassicReadPartyString(data, nameLength, Name);
    }
};

// A member's state as a party frame shows it, sent to a member that cannot see them in the world
// (whose own client would otherwise have the values from the member's unit fields). The phase and
// the tuning options are always the defaults.
struct PartyMemberFullState
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PARTY_MEMBER_FULL_STATE;

    bool ForEnemy = false;
    uint8 PartyType[2] = { CLASSIC_PARTY_TYPE_NORMAL, CLASSIC_PARTY_TYPE_NONE };
    uint16 Status = 0;
    uint8 PowerType = 0;
    uint16 PowerDisplayId = 0;
    int32 CurrentHealth = 0;
    int32 MaxHealth = 0;
    uint16 CurrentPower = 0;
    uint16 MaxPower = 0;
    uint16 Level = 0;
    uint16 SpecId = 0;
    uint16 ZoneId = 0;
    uint16 WmoGroupId = 0;
    uint32 WmoDoodadPlacementId = 0;
    int16 PositionX = 0;
    int16 PositionY = 0;
    int16 PositionZ = 0;
    int32 VehicleSeat = 0;
    std::vector<PartyMemberAuraStates> Auras;
    std::optional<PartyMemberPetStats> Pet;
    ClassicGuid MemberGuid;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(ForEnemy);
        data.FlushBits();
        data << PartyType[0] << PartyType[1];
        data << Status << PowerType << PowerDisplayId;
        data << CurrentHealth << MaxHealth << CurrentPower << MaxPower;
        data << Level << SpecId << ZoneId << WmoGroupId << WmoDoodadPlacementId;
        data << PositionX << PositionY << PositionZ << VehicleSeat;
        data << uint32(Auras.size());
        // phase shift: flags, no phases, no personal guid
        data << CLASSIC_PARTY_PHASE_SHIFT_UNPHASED << uint32(0);
        ClassicGuid().Write(data);
        // tuning options: condition mask, unused, expansion mask
        data << uint32(0) << int32(0) << uint32(0);
        ClassicWritePartyAuras(data, Auras);
        data.WriteBit(Pet.has_value());
        data.FlushBits();
        if (Pet)
            Pet->Write(data);
        MemberGuid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1 + 2 + 1 + 2 + 2 + 2 * sizeof(int32) + 2 * sizeof(uint16))
            return false;
        ForEnemy = data.ReadBit();
        data.ResetBitPos();
        data >> PartyType[0] >> PartyType[1];
        data >> Status >> PowerType >> PowerDisplayId;
        data >> CurrentHealth >> MaxHealth >> CurrentPower >> MaxPower;
        if (data.size() - data.rpos() < 4 * sizeof(uint16) + sizeof(uint32) + 3 * sizeof(int16) + sizeof(int32) + sizeof(uint32) + 2 * sizeof(uint32))
            return false;
        data >> Level >> SpecId >> ZoneId >> WmoGroupId >> WmoDoodadPlacementId;
        data >> PositionX >> PositionY >> PositionZ >> VehicleSeat;
        uint32 auraCount, phaseFlags, phaseCount;
        data >> auraCount >> phaseFlags >> phaseCount;
        ClassicGuid personalGuid;
        if (phaseFlags != CLASSIC_PARTY_PHASE_SHIFT_UNPHASED || phaseCount != 0 || !personalGuid.Read(data) || !personalGuid.IsEmpty())
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(uint32))
            return false;
        uint32 tuningMask, expansionMask;
        int32 tuningUnused;
        data >> tuningMask >> tuningUnused >> expansionMask;
        if (tuningMask || tuningUnused || expansionMask)
            return false;
        if (!ClassicReadPartyAuras(data, auraCount, Auras))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasPet = data.ReadBit();
        data.ResetBitPos();
        Pet.reset();
        if (hasPet)
        {
            Pet.emplace();
            if (!Pet->Read(data))
                return false;
        }
        return MemberGuid.Read(data);
    }
};

// A ready check started; `Duration` is how long the members have to answer, in milliseconds.
struct ReadyCheckStarted
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_READY_CHECK_STARTED;

    uint8 PartyIndex = 0;
    ClassicGuid PartyGuid;
    ClassicGuid InitiatorGuid;
    uint64 Duration = 0;

    void Write(ByteBuffer& data) const
    {
        data << PartyIndex;
        PartyGuid.Write(data);
        InitiatorGuid.Write(data);
        data << Duration;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data >> PartyIndex;
        if (!PartyGuid.Read(data) || !InitiatorGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint64))
            return false;
        data >> Duration;
        return true;
    }
};

// A member answered the ready check.
struct ReadyCheckResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_READY_CHECK_RESPONSE;

    ClassicGuid PartyGuid;
    ClassicGuid Player;
    bool IsReady = false;

    void Write(ByteBuffer& data) const
    {
        PartyGuid.Write(data);
        Player.Write(data);
        data.WriteBit(IsReady);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!PartyGuid.Read(data) || !Player.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        IsReady = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// The ready check is over.
struct ReadyCheckCompleted
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_READY_CHECK_COMPLETED;

    uint8 PartyIndex = 0;
    ClassicGuid PartyGuid;

    void Write(ByteBuffer& data) const
    {
        data << PartyIndex;
        PartyGuid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data >> PartyIndex;
        return PartyGuid.Read(data);
    }
};

// One raid marker changed; an empty target clears it.
struct RaidTargetUpdateSingle
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SEND_RAID_TARGET_UPDATE_SINGLE;

    int8 PartyIndex = 0;
    int8 Symbol = 0;
    ClassicGuid Target;
    ClassicGuid ChangedBy;

    void Write(ByteBuffer& data) const
    {
        data << PartyIndex << Symbol;
        Target.Write(data);
        ChangedBy.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        data >> PartyIndex >> Symbol;
        return Target.Read(data) && ChangedBy.Read(data);
    }
};

// Every raid marker in use.
struct RaidTargetUpdateAll
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SEND_RAID_TARGET_UPDATE_ALL;

    struct Entry
    {
        ClassicGuid Target;
        uint8 Symbol = 0;
    };

    uint8 PartyIndex = 0;
    std::vector<Entry> Targets;

    void Write(ByteBuffer& data) const
    {
        data << PartyIndex << uint32(Targets.size());
        for (Entry const& entry : Targets)
        {
            entry.Target.Write(data);
            data << entry.Symbol;
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1 + sizeof(uint32))
            return false;
        uint32 count;
        data >> PartyIndex >> count;
        // one marker per symbol
        if (count > 8)
            return false;
        Targets.assign(count, Entry());
        for (Entry& entry : Targets)
        {
            if (!entry.Target.Read(data))
                return false;
            if (data.size() - data.rpos() < 1)
                return false;
            data >> entry.Symbol;
        }
        return true;
    }
};

// A party member pinged the minimap.
struct MinimapPing
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_MINIMAP_PING;

    ClassicGuid Sender;
    float PositionX = 0.0f;
    float PositionY = 0.0f;

    void Write(ByteBuffer& data) const
    {
        Sender.Write(data);
        data << PositionX << PositionY;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Sender.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(float))
            return false;
        data >> PositionX >> PositionY;
        return true;
    }
};

// The result of a `/roll`.
struct RandomRoll
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_RANDOM_ROLL;

    ClassicGuid Roller;
    ClassicGuid RollerWowAccount;
    int32 Min = 0;
    int32 Max = 0;
    int32 Result = 0;

    void Write(ByteBuffer& data) const
    {
        Roller.Write(data);
        RollerWowAccount.Write(data);
        data << Min << Max << Result;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Roller.Read(data) || !RollerWowAccount.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(int32))
            return false;
        data >> Min >> Max >> Result;
        return true;
    }
};

// The players a master looter may give an item of the open loot window to.
struct MasterLootCandidateList
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_MASTER_LOOT_CANDIDATE_LIST;

    ClassicGuid LootObj;
    std::vector<ClassicGuid> Players;

    void Write(ByteBuffer& data) const
    {
        LootObj.Write(data);
        data << uint32(Players.size());
        for (ClassicGuid const& player : Players)
            player.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!LootObj.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 count;
        data >> count;
        if (count > CLASSIC_MAX_PARTY_MEMBERS)
            return false;
        Players.assign(count, ClassicGuid());
        for (ClassicGuid& player : Players)
            if (!player.Read(data))
                return false;
        return true;
    }
};

#endif
