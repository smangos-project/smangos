#ifndef MANGOS_PACKETS_PLAYERSETUP_H
#define MANGOS_PACKETS_PLAYERSETUP_H

// The player's own state, sent in the login burst and when it changes: bind point, clock,
// tutorials, proficiencies, action bars, reputations, account data times. Light like
// Packets/Session.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <array>
#include <vector>

struct BindPointUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_BIND_POINT_UPDATE;

    float X = 0.0f, Y = 0.0f, Z = 0.0f;
    uint32 MapId = 0;
    uint32 AreaId = 0;

    void Write(ByteBuffer& data) const { data << X << Y << Z << MapId << AreaId; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(float) * 3 + sizeof(uint32) * 2)
            return false;
        data >> X >> Y >> Z >> MapId >> AreaId;
        return true;
    }
};

// Times are packed wow times (minute, hour, weekday, day, month, year - 2000 in one uint32).
struct LoginSetTimeSpeed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOGIN_SET_TIME_SPEED;

    uint32 ServerTime = 0;
    uint32 GameTime = 0;
    float NewSpeed = 0.0f; // game minutes per real second
    uint32 ServerTimeHolidayOffset = 0;
    uint32 GameTimeHolidayOffset = 0;

    void Write(ByteBuffer& data) const
    {
        data << ServerTime << GameTime << NewSpeed << ServerTimeHolidayOffset << GameTimeHolidayOffset;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) * 4 + sizeof(float))
            return false;
        data >> ServerTime >> GameTime >> NewSpeed >> ServerTimeHolidayOffset >> GameTimeHolidayOffset;
        return true;
    }
};

constexpr uint32 CLASSIC_TUTORIAL_FLAG_WORDS = 8;

struct TutorialFlags
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_TUTORIAL_FLAGS;

    std::array<uint32, CLASSIC_TUTORIAL_FLAG_WORDS> Flags = {};

    void Write(ByteBuffer& data) const
    {
        for (uint32 flag : Flags)
            data << flag;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) * Flags.size())
            return false;
        for (uint32& flag : Flags)
            data >> flag;
        return true;
    }
};

struct SetProficiency
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SET_PROFICIENCY;

    uint32 SubclassMask = 0;
    uint8 ItemClass = 0;

    void Write(ByteBuffer& data) const { data << SubclassMask << ItemClass; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) + sizeof(uint8))
            return false;
        data >> SubclassMask >> ItemClass;
        return true;
    }
};

constexpr uint32 CLASSIC_ACTION_BUTTON_COUNT = 180;

// Each button is the action in the low 24 bits and its type in the next 8, as the server packs it.
struct UpdateActionButtons
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_UPDATE_ACTION_BUTTONS;

    enum : uint8
    {
        REASON_INITIAL = 0, // the client doesn't check the spells are known
        REASON_UPDATE  = 1,
        REASON_CLEAR   = 2,
    };

    std::array<uint64, CLASSIC_ACTION_BUTTON_COUNT> Buttons = {};
    uint8 Reason = REASON_INITIAL;

    void Write(ByteBuffer& data) const
    {
        for (uint64 button : Buttons)
            data << button;
        data << Reason;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint64) * Buttons.size() + sizeof(uint8))
            return false;
        for (uint64& button : Buttons)
            data >> button;
        data >> Reason;
        return true;
    }
};

// One entry per faction the character has a reputation with, keyed by faction id.
struct InitializeFactions
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_INITIALIZE_FACTIONS;

    struct Faction
    {
        int32 FactionId = 0;
        uint16 Flags = 0;
        int32 Standing = 0;
    };

    struct Bonus
    {
        int32 FactionId = 0;
        bool HasBonus = false;
    };

    std::vector<Faction> Factions;
    std::vector<Bonus> Bonuses;

    void Write(ByteBuffer& data) const
    {
        data << uint32(Factions.size());
        data << uint32(Bonuses.size());
        for (Faction const& faction : Factions)
            data << faction.FactionId << faction.Flags << faction.Standing;
        for (Bonus const& bonus : Bonuses)
        {
            data << bonus.FactionId;
            data.WriteBit(bonus.HasBonus);
            data.FlushBits();
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) * 2)
            return false;
        uint32 factionCount, bonusCount;
        data >> factionCount >> bonusCount;

        size_t const factionSize = sizeof(int32) + sizeof(uint16) + sizeof(int32);
        size_t const bonusSize = sizeof(int32) + 1;
        if ((data.size() - data.rpos()) / factionSize < factionCount)
            return false;
        if ((data.size() - data.rpos() - factionCount * factionSize) / bonusSize < bonusCount)
            return false;

        Factions.resize(factionCount);
        for (Faction& faction : Factions)
            data >> faction.FactionId >> faction.Flags >> faction.Standing;
        Bonuses.resize(bonusCount);
        for (Bonus& bonus : Bonuses)
        {
            data >> bonus.FactionId;
            bonus.HasBonus = data.ReadBit();
            data.ResetBitPos();
        }
        return true;
    }
};

constexpr uint32 CLASSIC_ACCOUNT_DATA_TYPES = 17;

// Unix times of the last change to each account data slot; 0 means empty. The guid is the
// character for the per-character slots, empty before a character is chosen.
struct AccountDataTimes
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ACCOUNT_DATA_TIMES;

    ClassicGuid PlayerGuid;
    int64 ServerTime = 0;
    std::array<int64, CLASSIC_ACCOUNT_DATA_TYPES> Times = {};

    void Write(ByteBuffer& data) const
    {
        PlayerGuid.Write(data);
        data << ServerTime;
        for (int64 time : Times)
            data << time;
    }

    bool Read(ByteBuffer& data)
    {
        if (!PlayerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int64) * (Times.size() + 1))
            return false;
        data >> ServerTime;
        for (int64& time : Times)
            data >> time;
        return true;
    }
};

// Empty body: stops the client's auto-attack.
struct CancelCombat
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CANCEL_COMBAT;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

#endif
