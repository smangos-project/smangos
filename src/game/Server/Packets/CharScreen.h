#ifndef MANGOS_PACKETS_CHARSCREEN_H
#define MANGOS_PACKETS_CHARSCREEN_H

// The modern (1.15.x classic-era) character-screen wire structs: create, delete, and player-login,
// plus the two ways a login can fail or succeed. Kept out of Packets/Character.h deliberately: that
// header pulls in ObjectGuid.h and the rest of the WorldPackets machinery, which RealBots must not
// depend on (see RealBots/README.md) -- these structs stay as light as Packets/Auth.h/Session.h.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include "Packets/Classic/ChrCustomizationChoice.h"
#include <algorithm>
#include <vector>
#include <string>

// A packed guid and nothing else.
struct CharDelete
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAR_DELETE;

    ClassicGuid Guid;

    void Write(ByteBuffer& data) const { Guid.Write(data); }
    bool Read(ByteBuffer& data) { return Guid.Read(data); }
};

// One ResponseCodes byte (CLASSIC_CHAR_DELETE_SUCCESS on success).
struct CharDeleteResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_DELETE_CHAR;

    uint8 Result = 0;

    void Write(ByteBuffer& data) const { data << Result; }
    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data >> Result;
        return true;
    }
};

// One LoginFailureReason byte (1 is NoWorld, also sent when the Instance connect fails five times).
struct CharacterLoginFailed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CHARACTER_LOGIN_FAILED;

    uint8 Reason = 0;

    void Write(ByteBuffer& data) const { data << Reason; }
    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data >> Reason;
        return true;
    }
};

// The bot sends no ChrCustomization choices (CustomizationCount = 0), which a server maps to a
// plain default appearance.
struct CharCreate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CREATE_CHARACTER;

    std::string Name;
    uint8 Race = 0;
    uint8 Class = 0;
    uint8 Gender = 0;
    std::vector<ChrCustomizationChoice> Customizations; // sorted by option on Read

    static constexpr uint32 MAX_CUSTOMIZATIONS = 250;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(Name.size()), 6);
        data.WriteBit(false); // hasTemplateSet
        data.WriteBit(false); // IsTrialBoost
        data.WriteBit(false); // UseNPE
        data.WriteBit(false); // HardcoreSelfFound
        data.FlushBits();

        data << Race;
        data << Class;
        data << Gender;
        data << uint32(Customizations.size());
        data << int32(0);  // timerunningSeasonId

        data.append(reinterpret_cast<uint8 const*>(Name.data()), Name.size());
        // No templateSetId (hasTemplateSet is false).
        for (ChrCustomizationChoice const& choice : Customizations)
            data << choice.OptionID << choice.ChoiceID;
    }

    // Reads what Write produces; a body with a template set is rejected rather than skipped, since
    // this side never produces one.
    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2) // 6 + 4 bits, flushed to two bytes
            return false;

        uint32 nameLength = data.ReadBits(6);
        bool hasTemplateSet = data.ReadBit();
        data.ReadBit(); // IsTrialBoost
        data.ReadBit(); // UseNPE
        data.ReadBit(); // HardcoreSelfFound
        data.ResetBitPos();
        if (hasTemplateSet)
            return false;

        if (data.size() - data.rpos() < sizeof(Race) + sizeof(Class) + sizeof(Gender) + sizeof(uint32) + sizeof(int32) + nameLength)
            return false;

        uint32 customizationCount = 0;
        int32 timerunningSeasonId = 0;
        data >> Race >> Class >> Gender >> customizationCount >> timerunningSeasonId;
        if (customizationCount > MAX_CUSTOMIZATIONS)
            return false;

        Name = data.ReadString(nameLength);
        if (data.size() - data.rpos() < size_t(customizationCount) * 2 * sizeof(uint32))
            return false;
        Customizations.clear();
        for (uint32 i = 0; i < customizationCount; ++i)
        {
            ChrCustomizationChoice choice;
            data >> choice.OptionID >> choice.ChoiceID;
            Customizations.push_back(choice);
        }
        std::stable_sort(Customizations.begin(), Customizations.end());
        return true;
    }
};

struct CharCreateResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CREATE_CHAR;

    uint8 Result = 0; // ResponseCodes; CLASSIC_CHAR_CREATE_SUCCESS on success
    ClassicGuid Guid; // the new character's guid on success, empty otherwise

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data >> Result;
        return Guid.Read(data);
    }

    void Write(ByteBuffer& data) const
    {
        data << Result;
        Guid.Write(data);
    }
};

struct PlayerLogin
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PLAYER_LOGIN;

    ClassicGuid Guid;
    // Terrain visibility distance the client requests; not applied to anything yet.
    float FarClip = 0.0f;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << FarClip;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data) || data.size() - data.rpos() < sizeof(float))
            return false;
        data >> FarClip;
        return true;
    }
};

// Reaching this is the InWorld marker; SMSG_UPDATE_OBJECT is received and skipped.
struct LoginVerifyWorld
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOGIN_VERIFY_WORLD;

    uint32 MapId = 0;
    float X = 0.0f, Y = 0.0f, Z = 0.0f, O = 0.0f;
    uint32 Reason = 0;

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) * 2 + sizeof(float) * 4)
            return false;
        data >> MapId >> X >> Y >> Z >> O >> Reason;
        return true;
    }

    void Write(ByteBuffer& data) const { data << MapId << X << Y << Z << O << Reason; }
};

#endif
