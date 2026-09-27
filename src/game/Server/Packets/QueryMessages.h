#ifndef MANGOS_PACKETS_QUERYMESSAGES_H
#define MANGOS_PACKETS_QUERYMESSAGES_H

// The modern static-data queries: player names, creature and game object templates, NPC and page
// text, server time. Light like Packets/CombatMessages.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <array>
#include <optional>
#include <string>
#include <vector>

// The most names one request may ask for.
constexpr uint32 CLASSIC_MAX_NAME_QUERIES = 50;
constexpr uint32 CLASSIC_MAX_DECLINED_NAME_CASES = 5;
constexpr uint32 CLASSIC_MAX_NPC_TEXT_OPTIONS = 8;
constexpr uint32 CLASSIC_MAX_GAMEOBJECT_DATA = 35;

// A string whose byte length rides in a bit field: the length counts the terminator, and a
// string with no characters has no bytes at all.
inline void ClassicWriteCStringBytes(ByteBuffer& data, std::string const& value)
{
    if (!value.empty())
        data << value;
}

inline bool ClassicReadCStringBytes(ByteBuffer& data, uint32 lengthWithTerminator, std::string& out)
{
    out.clear();
    if (lengthWithTerminator <= 1)
        return true;
    if (data.size() - data.rpos() < lengthWithTerminator)
        return false;
    out = data.ReadString(lengthWithTerminator - 1);
    uint8 terminator;
    data >> terminator;
    return terminator == 0;
}

// Batched name lookups: the modern client never asks for one name alone.
struct QueryPlayerNames
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUERY_PLAYER_NAMES;

    std::vector<ClassicGuid> Players;

    void Write(ByteBuffer& data) const
    {
        data << uint32(Players.size());
        for (ClassicGuid const& guid : Players)
            guid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 count;
        data >> count;
        if (count > CLASSIC_MAX_NAME_QUERIES)
            return false;

        Players.assign(count, ClassicGuid());
        for (ClassicGuid& guid : Players)
            if (!guid.Read(data))
                return false;
        return true;
    }
};

struct PlayerNameData
{
    bool IsDeleted = false;
    std::string Name;
    std::array<std::string, CLASSIC_MAX_DECLINED_NAME_CASES> DeclinedNames;
    ClassicGuid AccountId;
    ClassicGuid BnetAccountId;
    ClassicGuid PlayerGuid;
    uint64 GuildClubMemberId = 0;
    uint32 VirtualRealmAddress = 0;
    uint8 Race = 0;
    uint8 Sex = 0;
    uint8 ClassId = 0;
    uint8 Level = 0;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(IsDeleted);
        data.WriteBits(uint32(Name.length()), 6);
        for (std::string const& declined : DeclinedNames)
            data.WriteBits(uint32(declined.length()), 7);
        data.FlushBits();
        for (std::string const& declined : DeclinedNames)
            data.WriteString(declined);

        AccountId.Write(data);
        BnetAccountId.Write(data);
        PlayerGuid.Write(data);
        data << GuildClubMemberId;
        data << VirtualRealmAddress;
        data << Race << Sex << ClassId << Level;
        data << uint8(0);  // unused
        data << int32(0);  // timerunning season
        data.WriteString(Name);
    }

    bool Read(ByteBuffer& data)
    {
        // 1 + 6 + 5 * 7 bits, flushed to six bytes
        if (data.size() - data.rpos() < 6)
            return false;
        IsDeleted = data.ReadBit();
        uint32 const nameLength = data.ReadBits(6);
        uint32 declinedLengths[CLASSIC_MAX_DECLINED_NAME_CASES];
        for (uint32& length : declinedLengths)
            length = data.ReadBits(7);
        data.ResetBitPos();

        for (uint32 i = 0; i < CLASSIC_MAX_DECLINED_NAME_CASES; ++i)
        {
            if (data.size() - data.rpos() < declinedLengths[i])
                return false;
            DeclinedNames[i] = data.ReadString(declinedLengths[i]);
        }

        if (!AccountId.Read(data) || !BnetAccountId.Read(data) || !PlayerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint64) + sizeof(uint32) + 5 + sizeof(int32))
            return false;
        uint8 unusedByte;
        int32 timerunningSeason;
        data >> GuildClubMemberId >> VirtualRealmAddress >> Race >> Sex >> ClassId >> Level;
        data >> unusedByte >> timerunningSeason;
        if (unusedByte != 0 || timerunningSeason != 0)
            return false;

        if (data.size() - data.rpos() < nameLength)
            return false;
        Name = data.ReadString(nameLength);
        return true;
    }
};

struct PlayerNameLookup
{
    // 0: the data follows; anything else: the server does not know the guid.
    uint8 Result = 0;
    ClassicGuid Player;
    std::optional<PlayerNameData> Data;

    void Write(ByteBuffer& data) const
    {
        data << Result;
        Player.Write(data);
        data.WriteBit(Data.has_value());
        data.WriteBit(false);  // unused block
        data.FlushBits();
        if (Data)
            Data->Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint8))
            return false;
        data >> Result;
        if (!Player.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasData = data.ReadBit();
        bool const hasUnused = data.ReadBit();
        data.ResetBitPos();
        if (hasUnused)
            return false;

        Data.reset();
        if (hasData)
        {
            PlayerNameData parsed;
            if (!parsed.Read(data))
                return false;
            Data = std::move(parsed);
        }
        return true;
    }
};

struct QueryPlayerNamesResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUERY_PLAYER_NAMES_RESPONSE;

    std::vector<PlayerNameLookup> Players;

    void Write(ByteBuffer& data) const
    {
        data << uint32(Players.size());
        for (PlayerNameLookup const& lookup : Players)
            lookup.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 count;
        data >> count;
        // Every entry is at least a result byte, a guid header and a flag byte.
        if (count > (data.size() - data.rpos()) / 4)
            return false;

        Players.assign(count, PlayerNameLookup());
        for (PlayerNameLookup& lookup : Players)
            if (!lookup.Read(data))
                return false;
        return true;
    }
};

struct QueryCreature
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUERY_CREATURE;

    uint32 Entry = 0;

    void Write(ByteBuffer& data) const { data << Entry; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Entry;
        return true;
    }
};

struct CreatureDisplayEntry
{
    uint32 DisplayId = 0;
    float Scale = 1.0f;
    float Probability = 1.0f;
};

// A creature template. Only the first name slot is used; a body that fills another name, the
// alternate title or the cursor name is rejected on read, as are quest items and currencies.
struct QueryCreatureResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUERY_CREATURE_RESPONSE;

    uint32 Entry = 0;
    bool Allow = false;

    std::string Name;
    std::string Title;
    bool Civilian = false;
    bool Leader = false;
    uint32 TypeFlags = 0;
    uint32 TypeFlags2 = 0;
    int32 CreatureType = 0;
    int32 Family = 0;
    int32 Classification = 0;
    uint32 PetSpellDataId = 0;
    std::array<uint32, 2> ProxyCreatureId{};
    float TotalProbability = 1.0f;
    std::vector<CreatureDisplayEntry> Displays;
    float HealthMultiplier = 1.0f;
    float EnergyMultiplier = 1.0f;
    int32 UnitClass = 0;

    void Write(ByteBuffer& data) const
    {
        data << Entry;
        data.WriteBit(Allow);
        data.FlushBits();
        if (!Allow)
            return;

        data.WriteBits(uint32(Title.length() + 1), 11);
        data.WriteBits(1, 11);  // alternate title
        data.WriteBits(1, 6);   // cursor name
        data.WriteBit(Civilian);
        data.WriteBit(Leader);
        for (uint32 i = 0; i < 4; ++i)
        {
            data.WriteBits(uint32(i == 0 ? Name.length() + 1 : 1), 11);
            data.WriteBits(1, 11);
        }
        data.FlushBits();

        ClassicWriteCStringBytes(data, Name);

        data << TypeFlags << TypeFlags2;
        data << CreatureType << Family << Classification;
        data << PetSpellDataId;
        data << ProxyCreatureId[0] << ProxyCreatureId[1];
        data << uint32(Displays.size());
        data << TotalProbability;
        for (CreatureDisplayEntry const& display : Displays)
            data << display.DisplayId << display.Scale << display.Probability;
        data << HealthMultiplier << EnergyMultiplier;
        data << uint32(0);  // quest items
        data << uint32(0);  // quest currencies
        data << int32(0);   // movement info
        data << int32(0);   // health scaling expansion
        data << int32(0);   // required expansion
        data << int32(0);   // vignette
        data << UnitClass;
        data << int32(0);   // difficulty
        data << int32(0);   // widget set
        data << int32(0);   // widget set unit condition

        ClassicWriteCStringBytes(data, Title);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) + 1)
            return false;
        data >> Entry;
        Allow = data.ReadBit();
        data.ResetBitPos();
        if (!Allow)
            return true;

        // 11 + 11 + 6 + 2 + 8 * 11 bits, flushed to 15 bytes
        if (data.size() - data.rpos() < 15)
            return false;
        uint32 const titleLength = data.ReadBits(11);
        uint32 const titleAltLength = data.ReadBits(11);
        uint32 const cursorLength = data.ReadBits(6);
        Civilian = data.ReadBit();
        Leader = data.ReadBit();
        uint32 nameLengths[4], nameAltLengths[4];
        for (uint32 i = 0; i < 4; ++i)
        {
            nameLengths[i] = data.ReadBits(11);
            nameAltLengths[i] = data.ReadBits(11);
        }
        data.ResetBitPos();
        if (titleAltLength > 1 || cursorLength > 1)
            return false;
        for (uint32 i = 0; i < 4; ++i)
            if ((i > 0 && nameLengths[i] > 1) || nameAltLengths[i] > 1)
                return false;

        if (!ClassicReadCStringBytes(data, nameLengths[0], Name))
            return false;

        if (data.size() - data.rpos() < 10 * sizeof(uint32))
            return false;
        uint32 displayCount;
        data >> TypeFlags >> TypeFlags2 >> CreatureType >> Family >> Classification >> PetSpellDataId;
        data >> ProxyCreatureId[0] >> ProxyCreatureId[1] >> displayCount >> TotalProbability;
        if (displayCount > (data.size() - data.rpos()) / (sizeof(uint32) + 2 * sizeof(float)))
            return false;
        Displays.assign(displayCount, CreatureDisplayEntry());
        for (CreatureDisplayEntry& display : Displays)
            data >> display.DisplayId >> display.Scale >> display.Probability;

        if (data.size() - data.rpos() < 2 * sizeof(float) + 10 * sizeof(uint32))
            return false;
        uint32 questItems, questCurrencies;
        int32 ignored;
        data >> HealthMultiplier >> EnergyMultiplier >> questItems >> questCurrencies;
        if (questItems || questCurrencies)
            return false;
        for (int i = 0; i < 4; ++i)
            data >> ignored;
        data >> UnitClass;
        for (int i = 0; i < 3; ++i)
            data >> ignored;

        return ClassicReadCStringBytes(data, titleLength, Title);
    }
};

struct QueryGameObject
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUERY_GAME_OBJECT;

    uint32 Entry = 0;
    ClassicGuid Guid;

    void Write(ByteBuffer& data) const
    {
        data << Entry;
        Guid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Entry;
        return Guid.Read(data);
    }
};

// A game object template. The body is a length-prefixed sub-blob; a body with quest items is
// rejected on read.
struct QueryGameObjectResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUERY_GAME_OBJECT_RESPONSE;

    uint32 Entry = 0;
    ClassicGuid Guid;
    bool Allow = false;

    int32 Type = 0;
    int32 DisplayId = 0;
    std::string Name;
    std::string IconName;
    std::array<int32, CLASSIC_MAX_GAMEOBJECT_DATA> Data{};
    float Size = 1.0f;

    void Write(ByteBuffer& data) const
    {
        data << Entry;
        Guid.Write(data);
        data.WriteBit(Allow);
        data.FlushBits();

        ByteBuffer stats;
        if (Allow)
        {
            stats << Type << DisplayId;
            stats << Name << "" << "" << "";
            stats << IconName;
            stats << "";  // cast bar caption
            stats << "";  // unused
            for (int32 value : Data)
                stats << value;
            stats << Size;
            stats << uint8(0);  // quest items
            stats << int32(0);  // content tuning
        }

        data << uint32(stats.size());
        if (!stats.empty())
            data.append(stats.contents(), stats.size());
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Entry;
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        Allow = data.ReadBit();
        data.ResetBitPos();
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 statsSize;
        data >> statsSize;
        if (data.size() - data.rpos() < statsSize)
            return false;
        if (!Allow)
            return statsSize == 0;

        ByteBuffer stats;
        stats.append(data.contents() + data.rpos(), statsSize);
        data.read_skip(statsSize);

        if (stats.size() < 2 * sizeof(int32))
            return false;
        stats >> Type >> DisplayId;

        std::string names[4], caption, unused;
        std::string* strings[] = { &names[0], &names[1], &names[2], &names[3], &IconName, &caption, &unused };
        for (std::string* target : strings)
        {
            if (stats.rpos() >= stats.size())
                return false;
            *target = stats.ReadCString();
        }
        Name = names[0];
        if (!names[1].empty() || !names[2].empty() || !names[3].empty() || !caption.empty() || !unused.empty())
            return false;

        if (stats.size() - stats.rpos() < CLASSIC_MAX_GAMEOBJECT_DATA * sizeof(int32) + sizeof(float) + 1 + sizeof(int32))
            return false;
        for (int32& value : Data)
            stats >> value;
        uint8 questItems;
        stats >> Size >> questItems;
        return questItems == 0 && stats.size() - stats.rpos() == sizeof(int32);
    }
};

struct QueryNpcText
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUERY_NPC_TEXT;

    uint32 TextId = 0;
    ClassicGuid Guid;

    void Write(ByteBuffer& data) const
    {
        data << TextId;
        Guid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> TextId;
        return Guid.Read(data);
    }
};

// The client resolves the actual text from its own broadcast text data; the server only picks
// which ones and how likely each is.
struct QueryNpcTextResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUERY_NPC_TEXT_RESPONSE;

    uint32 TextId = 0;
    bool Allow = false;
    std::array<float, CLASSIC_MAX_NPC_TEXT_OPTIONS> Probabilities{};
    std::array<uint32, CLASSIC_MAX_NPC_TEXT_OPTIONS> BroadcastTextIds{};

    void Write(ByteBuffer& data) const
    {
        data << TextId;
        data.WriteBit(Allow);
        data.FlushBits();
        data << int32(Allow ? CLASSIC_MAX_NPC_TEXT_OPTIONS * (sizeof(float) + sizeof(uint32)) : 0);
        if (!Allow)
            return;
        for (float probability : Probabilities)
            data << probability;
        for (uint32 id : BroadcastTextIds)
            data << id;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) + 1 + sizeof(int32))
            return false;
        data >> TextId;
        Allow = data.ReadBit();
        data.ResetBitPos();
        int32 size;
        data >> size;
        if (!Allow)
            return size == 0;
        if (size != int32(CLASSIC_MAX_NPC_TEXT_OPTIONS * (sizeof(float) + sizeof(uint32))))
            return false;
        if (data.size() - data.rpos() < uint32(size))
            return false;
        for (float& probability : Probabilities)
            data >> probability;
        for (uint32& id : BroadcastTextIds)
            data >> id;
        return true;
    }
};

struct QueryPageText
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUERY_PAGE_TEXT;

    uint32 PageTextId = 0;
    ClassicGuid ItemGuid;

    void Write(ByteBuffer& data) const
    {
        data << PageTextId;
        ItemGuid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> PageTextId;
        return ItemGuid.Read(data);
    }
};

struct PageTextEntry
{
    uint32 Id = 0;
    uint32 NextPageId = 0;
    std::string Text;
};

// A whole chain of pages; each links to the next by id.
struct QueryPageTextResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUERY_PAGE_TEXT_RESPONSE;

    // The text length field is 12 bits wide.
    static constexpr uint32 MaxTextLength = 0xFFF;

    uint32 PageTextId = 0;
    bool Allow = false;
    std::vector<PageTextEntry> Pages;

    void Write(ByteBuffer& data) const
    {
        data << PageTextId;
        data.WriteBit(Allow);
        data.FlushBits();
        if (!Allow)
            return;

        data << uint32(Pages.size());
        for (PageTextEntry const& page : Pages)
        {
            data << page.Id << page.NextPageId;
            data << int32(0);  // player condition
            data << uint8(0);  // flags
            data.WriteBits(uint32(page.Text.length()), 12);
            data.FlushBits();
            data.WriteString(page.Text);
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) + 1)
            return false;
        data >> PageTextId;
        Allow = data.ReadBit();
        data.ResetBitPos();
        if (!Allow)
            return true;

        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 count;
        data >> count;
        // A page is at least id, next id, condition, flags and the two length bytes.
        if (count > (data.size() - data.rpos()) / 15)
            return false;

        Pages.assign(count, PageTextEntry());
        for (PageTextEntry& page : Pages)
        {
            if (data.size() - data.rpos() < 15)
                return false;
            int32 condition;
            uint8 flags;
            data >> page.Id >> page.NextPageId >> condition >> flags;
            uint32 const length = data.ReadBits(12);
            data.ResetBitPos();
            if (condition != 0 || flags != 0)
                return false;
            if (data.size() - data.rpos() < length)
                return false;
            page.Text = data.ReadString(length);
        }
        return true;
    }
};

struct QueryTime
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUERY_TIME;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct QueryTimeResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUERY_TIME_RESPONSE;

    uint32 CurrentTime = 0;

    void Write(ByteBuffer& data) const { data << CurrentTime; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> CurrentTime;
        return true;
    }
};

// "Release corpse" screen's request for where the player's own corpse is. The legacy
// MSG_CORPSE_QUERY had no body (implicit: the requester's own corpse); the modern client always
// names a player guid, per TrinityCore's WorldPackets::Query::QueryCorpseLocationFromClient::Read.
// This port keeps vmangos's own-corpse-only behavior, so the handler ignores anything but the
// requester's own guid.
struct QueryCorpseLocationFromClient
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUERY_CORPSE_LOCATION_FROM_CLIENT;

    ClassicGuid Player;

    void Write(ByteBuffer& data) const { Player.Write(data); }
    bool Read(ByteBuffer& data) { return Player.Read(data); }
};

// Field order/set confirmed against TrinityCore's WorldPackets::Query::CorpseLocation::Write.
// Transport is always empty: vmangos never places a corpse on a transport.
struct CorpseLocation
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CORPSE_LOCATION;

    bool Valid = false;
    ClassicGuid Player;
    int32 ActualMapId = 0;
    int32 MapId = 0;
    ClassicGuid Transport;
    float PositionX = 0.0f;
    float PositionY = 0.0f;
    float PositionZ = 0.0f;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(Valid);
        data.FlushBits();
        Player.Write(data);
        data << ActualMapId;
        data << MapId;
        Transport.Write(data);
        data << PositionX << PositionY << PositionZ;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        Valid = data.ReadBit();
        data.ResetBitPos();
        if (!Player.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(int32))
            return false;
        data >> ActualMapId >> MapId;
        if (!Transport.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(float))
            return false;
        data >> PositionX >> PositionY >> PositionZ;
        return true;
    }
};

#endif
