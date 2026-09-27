#ifndef MANGOS_PACKETS_MISCMESSAGES_H
#define MANGOS_PACKETS_MISCMESSAGES_H

// The rest of the modern Misc.h leftovers: /who is, animation and text emotes, the tutorial
// flag family, and the account-data cache round trip. Light like Packets/DuelMessages.h, so
// RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <string>
#include <vector>

// --- Client packets ---

// The 1.14.x client sends CMSG_EMOTE with an EMPTY body at world entry (a login-time
// emote-state sync quirk); nothing to parse then, EmoteId stays at its default.
struct EmoteRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_EMOTE;

    uint32 EmoteId = 0;

    void Write(ByteBuffer& data) const { data << EmoteId; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() == 0)
            return true;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> EmoteId;
        return true;
    }
};

// Field order/count confirmed against TrinityCore's CTextEmote::Read: packed-guid128 Target, i32
// EmoteID, i32 SoundIndex, a u32 SpellVisualKitIDs count, that many i32 kit ids, then i32
// SequenceVariation. Neither the kit ids nor the sequence variation are consumed by emote logic.
struct TextEmoteRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SEND_TEXT_EMOTE;

    ClassicGuid TargetGuid;
    uint32 EmoteId = 0;
    uint32 SoundIndex = 0;

    void Write(ByteBuffer& data) const
    {
        TargetGuid.Write(data);
        data << EmoteId << SoundIndex;
        data << uint32(0); // SpellVisualKitIDs count -- never sent by this port
        data << int32(0);  // SequenceVariation
    }

    bool Read(ByteBuffer& data)
    {
        if (!TargetGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(uint32))
            return false;
        data >> EmoteId >> SoundIndex;
        uint32 kitCount;
        data >> kitCount;
        if (data.size() - data.rpos() < kitCount * sizeof(int32) + sizeof(int32))
            return false;
        for (uint32 i = 0; i < kitCount; ++i)
            data.read_skip<int32>(); // kit id, discarded
        data.read_skip<int32>();     // SequenceVariation, discarded
        return true;
    }
};

// TUTORIAL_ACTION_* -- the modern client folds the legacy set/clear/reset opcode triplet into
// this one 2-bit Action field.
struct TutorialRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_TUTORIAL;

    static constexpr uint8 ActionUpdate = 0;
    static constexpr uint8 ActionClear = 1;
    static constexpr uint8 ActionReset = 2;

    uint8 Action = ActionUpdate;
    uint32 TutorialBit = 0;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(Action, 2);
        data.FlushBits();
        if (Action == ActionUpdate)
            data << TutorialBit;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        Action = uint8(data.ReadBits(2));
        data.ResetBitPos();
        if (Action == ActionUpdate)
        {
            if (data.size() - data.rpos() < sizeof(uint32))
                return false;
            data >> TutorialBit;
        }
        return true;
    }
};

// A 6-bit length-prefixed name, per TrinityCore's WhoIsRequest::Read. Was bound to the legacy
// CMSG_WHOIS opcode with a plain C-string body.
struct WhoIsRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_WHO_IS;

    std::string CharacterName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(CharacterName.length()), 6);
        data.FlushBits();
        data.WriteString(CharacterName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const nameLength = data.ReadBits(6);
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLength)
            return false;
        CharacterName = data.ReadString(nameLength);
        return true;
    }
};

// [guid128][i64 Time][u32 DecompressedSize][4-bit DataType], per TrinityCore's
// ClientConfig::RequestAccountData::Read.
struct AccountDataRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_REQUEST_ACCOUNT_DATA;

    ClassicGuid PlayerGuid;
    uint32 DataType = 0;

    void Write(ByteBuffer& data) const
    {
        PlayerGuid.Write(data);
        data << DataType;
    }

    bool Read(ByteBuffer& data)
    {
        if (!PlayerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> DataType;
        return true;
    }
};

// [guid128][i64 Time][u32 DecompressedSize][4-bit DataType][u32 compressed size][compressed
// bytes], per TrinityCore's ClientConfig::UserClientUpdateAccountData::Read. The legacy 1.12
// layout (bare u32 type, u32 size, rest-of-packet is the payload) has no player guid, no
// timestamp, no 4-bit DataType and no real length prefix.
struct AccountDataUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_UPDATE_ACCOUNT_DATA;

    ClassicGuid PlayerGuid;
    int64 Time = 0;
    uint32 DecompressedSize = 0;
    uint32 DataType = 0;
    std::vector<uint8> CompressedData;

    void Write(ByteBuffer& data) const
    {
        PlayerGuid.Write(data);
        data << Time;
        data << DecompressedSize;
        data.WriteBits(DataType, 4);
        data.FlushBits();
        data << uint32(CompressedData.size());
        if (!CompressedData.empty())
            data.append(CompressedData.data(), CompressedData.size());
    }

    bool Read(ByteBuffer& data)
    {
        if (!PlayerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int64) + sizeof(uint32))
            return false;
        data >> Time >> DecompressedSize;
        if (data.size() - data.rpos() < 1)
            return false;
        DataType = uint32(data.ReadBits(4));
        data.ResetBitPos();
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 compressedSize;
        data >> compressedSize;
        if (data.size() - data.rpos() < compressedSize)
            return false;
        CompressedData.resize(compressedSize);
        if (compressedSize > 0)
            data.read(CompressedData.data(), compressedSize);
        return true;
    }
};

// --- Server packets ---

// Field order confirmed against TrinityCore's Chat::Emote::Write: Guid, EmoteID, a
// SpellVisualKitIDs count (always empty here), SequenceVariation.
struct EmoteNotify
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_EMOTE;

    ClassicGuid UnitGuid;
    uint32 EmoteId = 0;

    void Write(ByteBuffer& data) const
    {
        UnitGuid.Write(data);
        data << EmoteId;
        data << uint32(0); // SpellVisualKitIDs count
        data << int32(0);  // SequenceVariation
    }

    bool Read(ByteBuffer& data)
    {
        if (!UnitGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(uint32))
            return false;
        data >> EmoteId;
        uint32 kitCount;
        data >> kitCount;
        if (kitCount != 0)
            return false;
        int32 sequenceVariation;
        data >> sequenceVariation;
        return true;
    }
};

// The modern SMSG_TEXT_EMOTE carries no name string at all: the client resolves both source and
// target names from its own cache via SourceGuid/TargetGuid. Field order/set confirmed against
// TrinityCore's Chat::STextEmote::Write: SourceGUID, SourceAccountGUID, EmoteID, SoundIndex,
// TargetGUID. SourceAccountGuid is always empty (no spam-report id wired up to this send).
struct TextEmoteNotify
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_TEXT_EMOTE;

    ClassicGuid SourceGuid;
    ClassicGuid SourceAccountGuid;
    uint32 EmoteId = 0;
    uint32 SoundIndex = 0;
    ClassicGuid TargetGuid;

    void Write(ByteBuffer& data) const
    {
        SourceGuid.Write(data);
        SourceAccountGuid.Write(data);
        data << EmoteId << SoundIndex;
        TargetGuid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!SourceGuid.Read(data) || !SourceAccountGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32))
            return false;
        data >> EmoteId >> SoundIndex;
        return TargetGuid.Read(data);
    }
};

// An 11-bit length-prefixed string, per TrinityCore's WhoIsResponse::Write. Was bound to the
// legacy SMSG_WHOIS opcode with a plain C-string body.
struct WhoIsResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_WHO_IS;

    std::string Message;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(Message.length()), 11);
        data.FlushBits();
        data.WriteString(Message);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const length = data.ReadBits(11);
        data.ResetBitPos();
        if (data.size() - data.rpos() < length)
            return false;
        Message = data.ReadString(length);
        return true;
    }
};

// [i64 Timestamp][u32 DecompressedLength][guid128 PlayerGuid][i32 DataType][u32 compressed
// size][compressed bytes], per TrinityCore's ClientConfig::UpdateAccountData::Write. The legacy
// body (type/decompressedLength/raw data, no length prefix) gave the client no way to know where
// the compressed payload ended.
struct AccountDataUpdateResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_UPDATE_ACCOUNT_DATA;

    int64 Timestamp = 0;
    uint32 DecompressedLength = 0;
    ClassicGuid PlayerGuid;
    uint32 DataType = 0;
    std::vector<uint8> CompressedData;

    void Write(ByteBuffer& data) const
    {
        data << Timestamp;
        data << DecompressedLength;
        PlayerGuid.Write(data);
        data << int32(DataType);
        data << uint32(CompressedData.size());
        if (!CompressedData.empty())
            data.append(CompressedData.data(), CompressedData.size());
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int64) + sizeof(uint32))
            return false;
        data >> Timestamp >> DecompressedLength;
        if (!PlayerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32) + sizeof(uint32))
            return false;
        int32 dataType;
        data >> dataType;
        DataType = uint32(dataType);
        uint32 compressedSize;
        data >> compressedSize;
        if (data.size() - data.rpos() < compressedSize)
            return false;
        CompressedData.resize(compressedSize);
        if (compressedSize > 0)
            data.read(CompressedData.data(), compressedSize);
        return true;
    }
};

// The PvP toggle button. Replaces vanilla CMSG_TOGGLE_PVP (an optional bool, toggling when
// absent); this client always sends an explicit desired state.
struct SetPvP
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_PVP;

    bool EnablePvP = false;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(EnablePvP);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        EnablePvP = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// The "/played" command's request. TriggerScriptEvent is parsed but unused (no PLAYER_PVP_KILLS-
// style script hook exists here for it to trigger).
struct RequestPlayedTime
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_REQUEST_PLAYED_TIME;

    bool TriggerScriptEvent = false;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(TriggerScriptEvent);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        TriggerScriptEvent = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Sets or clears one action button. Same packed-uint32 (action, type) body as the vanilla
// CMSG_SET_ACTION_BUTTON, per TrinityCore's WorldPackets::Spells::SetActionButton::Read
// (Action/Index): only the opcode number changed.
struct SetActionButton
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_ACTION_BUTTON;

    uint32 Action = 0;
    uint8 Index = 0;

    void Write(ByteBuffer& data) const
    {
        data << Action;
        data << Index;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) + sizeof(uint8))
            return false;
        data >> Action;
        data >> Index;
        return true;
    }
};

// Which action bar pages are enabled (stance/form bar swaps). Same single-byte mask as vanilla
// CMSG_SET_ACTIONBAR_TOGGLES, per TrinityCore's SetActionBarToggles::Read.
struct SetActionBarToggles
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_ACTION_BAR_TOGGLES;

    uint8 Mask = 0;

    void Write(ByteBuffer& data) const { data << Mask; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint8))
            return false;
        data >> Mask;
        return true;
    }
};

// Right-clicking (using) a game object: chests, doors, levers, quest objects. Same single-guid
// body as vanilla CMSG_GAMEOBJ_USE, per TrinityCore's WorldPackets::GameObject::GameObjUse::Read.
struct GameObjectUse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GAME_OBJ_USE;

    ClassicGuid Guid;

    void Write(ByteBuffer& data) const { Guid.Write(data); }
    bool Read(ByteBuffer& data) { return Guid.Read(data); }
};

// Entering an area trigger volume: quest exploration, instance portals, taverns, battleground
// entrances. Same trigger id as vanilla CMSG_AREATRIGGER; Entered/FromClient are new fields this
// port has no use for (matched against vmangos, which only ever handled the "entered" case), per
// TrinityCore's WorldPackets::AreaTrigger::AreaTrigger::Read.
struct AreaTrigger
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AREA_TRIGGER;

    int32 TriggerId = 0;
    bool Entered = true;
    bool FromClient = true;

    void Write(ByteBuffer& data) const
    {
        data << TriggerId;
        data.WriteBit(Entered);
        data.WriteBit(FromClient);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32) + 1)
            return false;
        data >> TriggerId;
        Entered = data.ReadBit();
        FromClient = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Accepting or declining a warlock/meeting-stone-style summon. Vanilla CMSG_SUMMON_RESPONSE had
// no decline case (the client just closed the dialog and sent nothing); the modern client always
// answers, so Accept can be false. Field order/types per TrinityCore's
// WorldPackets::Movement::SummonResponse::Read.
struct SummonResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SUMMON_RESPONSE;

    ClassicGuid SummonerGuid;
    bool Accept = false;

    void Write(ByteBuffer& data) const
    {
        SummonerGuid.Write(data);
        data.WriteBit(Accept);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!SummonerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        Accept = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Taking or releasing far sight (Eagle Eye, mind vision-style spells, the camera-only case).
// Vanilla CMSG_FAR_SIGHT carried a 0/1 op byte; the modern client sends one Enable bit instead,
// per TrinityCore's WorldPackets::Misc::FarSight::Read.
struct FarSightRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_FAR_SIGHT;

    bool Enable = false;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(Enable);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        Enable = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Which faction's reputation bar is pinned to the experience bar. Same reputation list index as
// vanilla CMSG_SET_WATCHED_FACTION, per TrinityCore's SetWatchedFaction::Read.
struct SetWatchedFaction
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_WATCHED_FACTION;

    int32 FactionIndex = 0;

    void Write(ByteBuffer& data) const { data << FactionIndex; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> FactionIndex;
        return true;
    }
};

// Declares or clears "at war" on a faction from the reputation pane's context menu. Vanilla
// folded both directions into one CMSG_SET_FACTION_ATWAR with a flag byte; the modern client
// splits it into two opcodes with no body but the faction index, per TrinityCore's
// SetFactionAtWar/SetFactionNotAtWar::Read. Two thin types, same shape, since the classic
// dispatch table keys one struct type to exactly one opcode.
struct SetFactionAtWar
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_FACTION_AT_WAR;

    int32 FactionIndex = 0;

    void Write(ByteBuffer& data) const { data << FactionIndex; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> FactionIndex;
        return true;
    }
};

struct SetFactionNotAtWar
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_FACTION_NOT_AT_WAR;

    int32 FactionIndex = 0;

    void Write(ByteBuffer& data) const { data << FactionIndex; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> FactionIndex;
        return true;
    }
};

// Hides or shows an inactive (below-max) faction's reputation bar. Same (index, state) body as
// vanilla CMSG_SET_FACTION_INACTIVE with State moved from a byte to a bit, per TrinityCore's
// SetFactionInactive::Read.
struct SetFactionInactive
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_FACTION_INACTIVE;

    uint32 Index = 0;
    bool State = false;

    void Write(ByteBuffer& data) const
    {
        data << Index;
        data.WriteBit(State);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) + 1)
            return false;
        data >> Index;
        State = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

struct PlayedTimeResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PLAYED_TIME;

    int32 TotalTime = 0;
    int32 LevelTime = 0;
    bool TriggerEvent = false;

    void Write(ByteBuffer& data) const
    {
        data << TotalTime;
        data << LevelTime;
        data.WriteBit(TriggerEvent);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32) * 2 + 1)
            return false;
        data >> TotalTime >> LevelTime;
        TriggerEvent = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

#endif
