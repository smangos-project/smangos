#ifndef MANGOS_PACKETS_PETITIONMESSAGES_H
#define MANGOS_PACKETS_PETITIONMESSAGES_H

// The modern guild-charter wire: buying one from a tabard designer, showing and offering its
// signatures, signing, renaming, querying and turning it in. Light like Packets/DuelMessages.h,
// so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <string>
#include <vector>

// --- Client packets ---

// Opens a tabard designer's charter-purchase window.
struct PetitionShow
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PETITION_SHOW_LIST;

    ClassicGuid PetitionerGuid;

    void Write(ByteBuffer& data) const { PetitionerGuid.Write(data); }
    bool Read(ByteBuffer& data) { return PetitionerGuid.Read(data); }
};

struct PetitionBuy
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PETITION_BUY;

    ClassicGuid NpcGuid;
    std::string Name;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(Name.length()), 7);
        NpcGuid.Write(data);
        data << uint32(0); // unused
        data.FlushBits();
        data.WriteString(Name);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const nameLength = data.ReadBits(7);
        if (!NpcGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data.read_skip<uint32>(); // unused
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLength)
            return false;
        Name = data.ReadString(nameLength);
        return true;
    }
};

struct PetitionShowSignatures
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PETITION_SHOW_SIGNATURES;

    ClassicGuid ItemGuid;

    void Write(ByteBuffer& data) const { ItemGuid.Write(data); }
    bool Read(ByteBuffer& data) { return ItemGuid.Read(data); }
};

struct QueryPetition
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUERY_PETITION;

    uint32 PetitionId = 0;
    ClassicGuid ItemGuid;

    void Write(ByteBuffer& data) const
    {
        data << PetitionId;
        ItemGuid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> PetitionId;
        return ItemGuid.Read(data);
    }
};

// The argument of `/run SignPetition(123)` is never used in the official interface, so only the
// charter guid is read.
struct SignPetition
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SIGN_PETITION;

    ClassicGuid ItemGuid;

    void Write(ByteBuffer& data) const
    {
        ItemGuid.Write(data);
        data << uint8(0);
    }

    bool Read(ByteBuffer& data)
    {
        if (!ItemGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        data.read_skip<uint8>();
        return true;
    }
};

struct DeclinePetition
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_DECLINE_PETITION;

    ClassicGuid ItemGuid;

    void Write(ByteBuffer& data) const { ItemGuid.Write(data); }
    bool Read(ByteBuffer& data) { return ItemGuid.Read(data); }
};

// The wire carries a leading unused id ahead of the two guids; parsed and kept unused, matching
// how the rest of this port treats a client-carried field nothing here acts on.
struct OfferPetition
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_OFFER_PETITION;

    uint32 UnusedId = 0;
    ClassicGuid ItemGuid;
    ClassicGuid PlayerGuid;

    void Write(ByteBuffer& data) const
    {
        data << UnusedId;
        ItemGuid.Write(data);
        PlayerGuid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> UnusedId;
        return ItemGuid.Read(data) && PlayerGuid.Read(data);
    }
};

// The five tabard-customization fields are optional on the wire (an older client sends only the
// guid); parsed when present and left unused (this engine's guild creation never reads them).
struct TurnInPetition
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_TURN_IN_PETITION;

    ClassicGuid ItemGuid;
    uint32 BackgroundColor = 0;
    uint32 EmblemStyle = 0;
    uint32 EmblemColor = 0;
    uint32 BorderStyle = 0;
    uint32 BorderColor = 0;

    void Write(ByteBuffer& data) const
    {
        ItemGuid.Write(data);
        data << BackgroundColor << EmblemStyle << EmblemColor << BorderStyle << BorderColor;
    }

    bool Read(ByteBuffer& data)
    {
        if (!ItemGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 5 * sizeof(uint32))
            return true; // an older client sends only the guid
        data >> BackgroundColor >> EmblemStyle >> EmblemColor >> BorderStyle >> BorderColor;
        return true;
    }
};

struct RenamePetition
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PETITION_RENAME_GUILD;

    ClassicGuid ItemGuid;
    std::string NewName;

    void Write(ByteBuffer& data) const
    {
        ItemGuid.Write(data);
        data.WriteBits(uint32(NewName.length()), 7);
        data.FlushBits();
        data.WriteString(NewName);
    }

    bool Read(ByteBuffer& data)
    {
        if (!ItemGuid.Read(data))
            return false;
        data.ResetBitPos();
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const nameLength = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLength)
            return false;
        NewName = data.ReadString(nameLength);
        return true;
    }
};

// --- Server packets ---

struct PetitionShowList
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PETITION_SHOW_LIST;

    ClassicGuid NpcGuid;
    uint32 Price = 0;

    void Write(ByteBuffer& data) const
    {
        NpcGuid.Write(data);
        data << Price;
    }

    bool Read(ByteBuffer& data)
    {
        if (!NpcGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Price;
        return true;
    }
};

// One signature on a petition, as shown to either the owner or an offered signer. Named apart
// from GuildMgr.h's own PetitionSignature (the server's bookkeeping entry), which this wire
// struct is built from at the send site.
struct PetitionSignatureEntry
{
    ClassicGuid SignerGuid;
    int32 Choice = 0;

    void Write(ByteBuffer& data) const
    {
        SignerGuid.Write(data);
        data << Choice;
    }

    bool Read(ByteBuffer& data)
    {
        if (!SignerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> Choice;
        return true;
    }
};

struct PetitionSignatures
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PETITION_SHOW_SIGNATURES;

    ClassicGuid ItemGuid;
    ClassicGuid OwnerGuid;
    ClassicGuid OwnerAccountGuid;
    int32 PetitionId = 0;
    std::vector<PetitionSignatureEntry> Signatures;

    void Write(ByteBuffer& data) const
    {
        ItemGuid.Write(data);
        OwnerGuid.Write(data);
        OwnerAccountGuid.Write(data);
        data << PetitionId;
        data << uint32(Signatures.size());
        for (PetitionSignatureEntry const& signature : Signatures)
            signature.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!ItemGuid.Read(data) || !OwnerGuid.Read(data) || !OwnerAccountGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32) + sizeof(uint32))
            return false;
        data >> PetitionId;
        uint32 count;
        data >> count;
        Signatures.clear();
        for (uint32 i = 0; i < count; ++i)
        {
            PetitionSignatureEntry signature;
            if (!signature.Read(data))
                return false;
            Signatures.push_back(signature);
        }
        return true;
    }
};

struct PetitionSignResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PETITION_SIGN_RESULTS;

    // The 4-bit result field's values.
    static constexpr uint8 Ok = 0;
    static constexpr uint8 AlreadySigned = 1;
    static constexpr uint8 AlreadyInGuild = 2;
    static constexpr uint8 CantSignOwn = 3;
    static constexpr uint8 NotOnServer = 5;
    static constexpr uint8 Full = 8;
    static constexpr uint8 AlreadySignedOther = 10;
    static constexpr uint8 RestrictedAccountTrial = 11;
    static constexpr uint8 HasRestriction = 13;

    ClassicGuid ItemGuid;
    ClassicGuid PlayerGuid;
    uint8 Result = Ok;

    void Write(ByteBuffer& data) const
    {
        ItemGuid.Write(data);
        PlayerGuid.Write(data);
        data.WriteBits(Result, 4);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!ItemGuid.Read(data) || !PlayerGuid.Read(data))
            return false;
        data.ResetBitPos();
        if (data.size() - data.rpos() < 1)
            return false;
        Result = uint8(data.ReadBits(4));
        data.ResetBitPos();
        return true;
    }
};

struct TurnInPetitionResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_TURN_IN_PETITION_RESULT;

    // The 4-bit result field's values -- a separate value space from PetitionSignResult above,
    // even though the two overlap at 0/2/4.
    static constexpr uint8 Ok = 0;
    static constexpr uint8 AlreadyInGuild = 2;
    static constexpr uint8 NeedMoreSignatures = 4;
    static constexpr uint8 GuildPermissions = 11;
    static constexpr uint8 GuildNameInvalid = 12;
    static constexpr uint8 HasRestriction = 13;

    uint8 Result = Ok;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(Result, 4);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        data.ResetBitPos();
        if (data.size() - data.rpos() < 1)
            return false;
        Result = uint8(data.ReadBits(4));
        data.ResetBitPos();
        return true;
    }
};

// The tail's ten choice-text slots are always sent, even empty: the client reads a fixed count
// regardless of how many this server actually offers (a guild charter never offers any), and
// writing fewer desyncs the bit stream that follows (the name and body text read from the wrong
// offset).
struct QueryPetitionResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUERY_PETITION_RESPONSE;

    static constexpr uint32 ChoiceSlots = 10;

    uint32 PetitionId = 0;
    bool Allow = true;
    ClassicGuid OwnerGuid;
    std::string Name;
    std::string BodyText;
    uint32 MinSignatures = 0;
    uint32 MaxSignatures = 0;
    uint32 DeadlineTimestamp = 0;
    uint32 CreationTimestamp = 0;
    uint32 AllowedGuildId = 0;
    uint32 AllowedClasses = 0;
    uint32 AllowedRaces = 0;
    uint16 AllowedGender = 0;
    uint32 AllowedMinLevel = 0;
    uint32 AllowedMaxLevel = 0;
    uint32 StaticType = 0;

    void Write(ByteBuffer& data) const
    {
        data << PetitionId;
        data.WriteBit(Allow);
        data.FlushBits();
        if (!Allow)
            return;

        data << PetitionId;
        OwnerGuid.Write(data);
        data << MinSignatures << MaxSignatures << DeadlineTimestamp << CreationTimestamp;
        data << AllowedGuildId << AllowedClasses << AllowedRaces;
        data << AllowedGender;
        data << AllowedMinLevel << AllowedMaxLevel;
        data << uint32(0); // choice count -- this server never offers any
        data << StaticType;
        data << uint32(0); // default choice

        data.WriteBits(uint32(Name.length()), 7);
        data.WriteBits(uint32(BodyText.length()), 12);
        for (uint32 i = 0; i < ChoiceSlots; ++i)
            data.WriteBits(0, 6);
        data.FlushBits();

        for (uint32 i = 0; i < ChoiceSlots; ++i)
            data.WriteString(std::string());
        data.WriteString(Name);
        data.WriteString(BodyText);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> PetitionId;
        if (data.size() - data.rpos() < 1)
            return false;
        Allow = data.ReadBit();
        data.ResetBitPos();
        if (!Allow)
            return true;

        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> PetitionId;
        if (!OwnerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 9 * sizeof(uint32) + sizeof(uint16))
            return false;
        data >> MinSignatures >> MaxSignatures >> DeadlineTimestamp >> CreationTimestamp;
        data >> AllowedGuildId >> AllowedClasses >> AllowedRaces;
        data >> AllowedGender;
        data >> AllowedMinLevel >> AllowedMaxLevel;

        if (data.size() - data.rpos() < 3 * sizeof(uint32))
            return false;
        uint32 choiceCount, defaultChoice;
        data >> choiceCount >> StaticType >> defaultChoice;
        if (choiceCount != 0)
            return false;

        if (data.size() - data.rpos() < 10)
            return false; // 7 + 12 + 10*6 bits, flushed to ten bytes
        uint32 const nameLength = data.ReadBits(7);
        uint32 const bodyLength = data.ReadBits(12);
        for (uint32 i = 0; i < ChoiceSlots; ++i)
            if (data.ReadBits(6) != 0)
                return false; // this server never offers a choice
        data.ResetBitPos();

        if (data.size() - data.rpos() < nameLength + bodyLength)
            return false;
        Name = data.ReadString(nameLength);
        BodyText = data.ReadString(bodyLength);
        return true;
    }
};

struct RenamePetitionResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PETITION_RENAME_GUILD_RESPONSE;

    ClassicGuid ItemGuid;
    std::string NewName;

    void Write(ByteBuffer& data) const
    {
        ItemGuid.Write(data);
        data.WriteBits(uint32(NewName.length()), 7);
        data.FlushBits();
        data.WriteString(NewName);
    }

    bool Read(ByteBuffer& data)
    {
        if (!ItemGuid.Read(data))
            return false;
        data.ResetBitPos();
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const nameLength = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLength)
            return false;
        NewName = data.ReadString(nameLength);
        return true;
    }
};

#endif
