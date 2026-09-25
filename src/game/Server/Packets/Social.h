#ifndef MANGOS_PACKETS_SOCIAL_H
#define MANGOS_PACKETS_SOCIAL_H

// Friends, ignores and the contact list request that replaced them. Light like
// Packets/PlayerSetup.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <string>
#include <vector>

// Guid + realm address pair the client sends for every contact-targeting request. This server has
// no cross-realm concept, so the realm address is always 0/discarded on read.
struct QualifiedGuid
{
    ClassicGuid Guid;
    uint32 VirtualRealmAddress = 0;

    void Write(ByteBuffer& data) const
    {
        data << VirtualRealmAddress;
        Guid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> VirtualRealmAddress;
        return Guid.Read(data);
    }
};

// Replaces the old zero-body CMSG_FRIEND_LIST: the modern client asks for friends and ignores
// together, filtered by a SocialFlag bitmask.
struct SendContactList
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SEND_CONTACT_LIST;

    uint32 Flags = 0; // SocialFlag bitmask

    void Write(ByteBuffer& data) const { data << Flags; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Flags;
        return true;
    }
};

struct SetContactNotes
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_CONTACT_NOTES;

    QualifiedGuid Player;
    std::string Notes;

    void Write(ByteBuffer& data) const
    {
        Player.Write(data);
        data.WriteSizedString(Notes, 10);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Player.Read(data))
            return false;
        if (data.size() - data.rpos() < 2) // 10 bits, flushed to two bytes
            return false;
        uint32 notesLength = data.ReadBits(10);
        data.ResetBitPos();
        if (data.size() - data.rpos() < notesLength)
            return false;
        Notes = data.ReadString(notesLength);
        return true;
    }
};

struct AddFriend
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ADD_FRIEND;

    std::string FriendName;
    std::string Notes;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(FriendName.length()), 9);
        data.WriteBits(uint32(Notes.length()), 10);
        data.FlushBits();
        data.WriteString(FriendName);
        data.WriteString(Notes);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 3) // 9 + 10 bits, flushed to three bytes
            return false;
        uint32 nameLength = data.ReadBits(9);
        uint32 notesLength = data.ReadBits(10);
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLength + notesLength)
            return false;
        FriendName = data.ReadString(nameLength);
        Notes = data.ReadString(notesLength);
        return true;
    }
};

struct DelFriend
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_DEL_FRIEND;

    QualifiedGuid Player;

    void Write(ByteBuffer& data) const { Player.Write(data); }
    bool Read(ByteBuffer& data) { return Player.Read(data); }
};

struct AddIgnore
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ADD_IGNORE;

    std::string IgnoreName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(IgnoreName.length()), 9);
        data.FlushBits();
        ClassicGuid().Write(data); // Battle.net account guid; no account linking here
        data.WriteString(IgnoreName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2) // 9 bits, flushed to two bytes
            return false;
        uint32 nameLength = data.ReadBits(9);
        data.ResetBitPos();

        ClassicGuid accountGuid;
        if (!accountGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < nameLength)
            return false;
        IgnoreName = data.ReadString(nameLength);
        return true;
    }
};

struct DelIgnore
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_DEL_IGNORE;

    QualifiedGuid Player;

    void Write(ByteBuffer& data) const { Player.Write(data); }
    bool Read(ByteBuffer& data) { return Player.Read(data); }
};

// One entry in ContactList: a friend or an ignore (TypeFlags tells which; an entry can be both).
struct ContactInfo
{
    ClassicGuid Guid;
    uint32 TypeFlags = 0; // SocialFlag bitmask
    std::string Notes;
    uint8 Status = 0;     // FriendStatus
    uint32 Area = 0;
    uint32 Level = 0;
    uint32 ClassId = 0;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        ClassicGuid().Write(data); // WowAccountGuid; no Battle.net account linking here
        data << uint32(0); // VirtualRealmAddr
        data << uint32(0); // NativeRealmAddr
        data << TypeFlags;
        data << Status;
        data << Area;
        data << Level;
        data << ClassId;
        data.WriteSizedString(Notes, 10);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        ClassicGuid accountGuid;
        if (!accountGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) * 2 + sizeof(TypeFlags) + sizeof(Status) + sizeof(Area) + sizeof(Level) + sizeof(ClassId))
            return false;
        uint32 virtualRealmAddr, nativeRealmAddr;
        data >> virtualRealmAddr >> nativeRealmAddr >> TypeFlags >> Status >> Area >> Level >> ClassId;
        if (data.size() - data.rpos() < 2) // 10 bits, flushed to two bytes
            return false;
        uint32 notesLength = data.ReadBits(10);
        data.ResetBitPos();
        if (data.size() - data.rpos() < notesLength)
            return false;
        Notes = data.ReadString(notesLength);
        return true;
    }
};

// Replaces the old separate SMSG_FRIEND_LIST/SMSG_IGNORE_LIST -- one list, filtered by the flags
// the client requested (Flags is echoed back, not necessarily every contact's own flags).
struct ContactList
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CONTACT_LIST;

    std::vector<ContactInfo> Contacts;
    uint32 Flags = 0;

    void Write(ByteBuffer& data) const
    {
        data << Flags;
        data.WriteBits(uint32(Contacts.size()), 8);
        data.FlushBits();
        for (ContactInfo const& contact : Contacts)
            contact.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Flags;
        if (data.size() - data.rpos() < 1) // 8 bits, flushed to one byte
            return false;
        uint32 count = data.ReadBits(8);
        data.ResetBitPos();

        Contacts.clear();
        Contacts.reserve(count);
        for (uint32 i = 0; i < count; ++i)
        {
            ContactInfo contact;
            if (!contact.Read(data))
                return false;
            Contacts.push_back(std::move(contact));
        }
        return true;
    }
};

// Named Update, not Status, to avoid clashing with the FriendStatus enum in SocialMgr.h.
struct FriendStatusUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_FRIEND_STATUS;

    uint8 Result = 0; // FriendsResult
    ClassicGuid FriendGuid;
    std::string Notes;
    uint8 Status = 0; // FriendStatus
    uint32 Area = 0;
    uint32 Level = 0;
    uint32 ClassId = 0;

    void Write(ByteBuffer& data) const
    {
        data << Result;
        FriendGuid.Write(data);
        ClassicGuid().Write(data); // WowAccountGuid; no Battle.net account linking here
        data << uint32(0); // VirtualRealmAddress
        data << Status;
        data << Area;
        data << Level;
        data << ClassId;
        data.WriteBits(uint32(Notes.length()), 10);
        data.WriteBit(false); // Mobile
        data.FlushBits();
        data.WriteString(Notes);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint8))
            return false;
        data >> Result;
        if (!FriendGuid.Read(data))
            return false;
        ClassicGuid accountGuid;
        if (!accountGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + sizeof(Status) + sizeof(Area) + sizeof(Level) + sizeof(ClassId))
            return false;
        uint32 virtualRealmAddress;
        data >> virtualRealmAddress >> Status >> Area >> Level >> ClassId;
        if (data.size() - data.rpos() < 2) // 10 bits + 1 bit, flushed to two bytes
            return false;
        uint32 notesLength = data.ReadBits(10);
        data.ReadBit(); // Mobile
        data.ResetBitPos();
        if (data.size() - data.rpos() < notesLength)
            return false;
        Notes = data.ReadString(notesLength);
        return true;
    }
};

#endif
