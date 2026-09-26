#ifndef MANGOS_PACKETS_NPCMESSAGES_H
#define MANGOS_PACKETS_NPCMESSAGES_H

// The modern wire for talking to NPCs: the gossip window, its quest list and points of interest.
// Light like Packets/CombatMessages.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <optional>
#include <string>
#include <vector>

constexpr uint32 CLASSIC_MAX_GOSSIP_OPTIONS = 64;
constexpr uint32 CLASSIC_MAX_GOSSIP_QUESTS = 64;

// Right-clicking an NPC or game object.
struct GossipHello
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_TALK_TO_GOSSIP;

    ClassicGuid Unit;

    void Write(ByteBuffer& data) const { Unit.Write(data); }
    bool Read(ByteBuffer& data) { return Unit.Read(data); }
};

// Picking a gossip option. The option id is the index the server gave it in the menu it sent.
struct GossipSelectOption
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GOSSIP_SELECT_OPTION;

    ClassicGuid GossipUnit;
    int32 GossipId = 0;
    int32 GossipOptionId = 0;
    std::string PromotionCode;

    void Write(ByteBuffer& data) const
    {
        GossipUnit.Write(data);
        data << GossipId << GossipOptionId;
        data.WriteBits(uint32(PromotionCode.length()), 8);
        data.FlushBits();
        data.WriteString(PromotionCode);
    }

    bool Read(ByteBuffer& data)
    {
        if (!GossipUnit.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(int32) + 1)
            return false;
        data >> GossipId >> GossipOptionId;
        uint32 const length = data.ReadBits(8);
        data.ResetBitPos();
        if (data.size() - data.rpos() < length)
            return false;
        PromotionCode = data.ReadString(length);
        return true;
    }
};

// One entry of the gossip menu. Treasure lists, spells and icon overrides are never sent, and a body
// that carries one is rejected on read.
struct GossipOptionData
{
    int32 GossipOptionId = 0;
    uint8 OptionNpc = 0;   // the icon: what kind of service the option opens
    int8 OptionFlags = 0;  // bit 0: asks for a code
    int32 OptionCost = 0;
    uint32 OptionLanguage = 0;
    int32 Flags = 0;
    int32 OrderIndex = 0;
    std::string Text;
    std::string Confirm;
    uint8 Status = 0;

    void Write(ByteBuffer& data) const
    {
        data << GossipOptionId << OptionNpc << OptionFlags << OptionCost << OptionLanguage << Flags << OrderIndex;
        data.WriteBits(uint32(Text.length()), 12);
        data.WriteBits(uint32(Confirm.length()), 12);
        data.WriteBits(Status, 2);
        data.WriteBit(false);  // spell
        data.WriteBit(false);  // icon override
        data.WriteBits(1, 8);  // failure description: length 1, no bytes
        data.FlushBits();
        data << uint32(0);     // treasure items
        data.WriteString(Text);
        data.WriteString(Confirm);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 22 + 5 + sizeof(uint32))
            return false;
        data >> GossipOptionId >> OptionNpc >> OptionFlags >> OptionCost >> OptionLanguage >> Flags >> OrderIndex;
        uint32 const textLength = data.ReadBits(12);
        uint32 const confirmLength = data.ReadBits(12);
        Status = uint8(data.ReadBits(2));
        bool const hasSpell = data.ReadBit();
        bool const hasIcon = data.ReadBit();
        uint32 const failureLength = data.ReadBits(8);
        data.ResetBitPos();
        if (hasSpell || hasIcon || failureLength > 1)
            return false;
        uint32 treasure;
        data >> treasure;
        if (treasure)
            return false;
        if (data.size() - data.rpos() < textLength + confirmLength)
            return false;
        Text = data.ReadString(textLength);
        Confirm = data.ReadString(confirmLength);
        return true;
    }
};

// A quest offered in the gossip window.
struct GossipQuestEntry
{
    int32 QuestId = 0;
    int32 QuestType = 0;   // 0 available, 1 active
    int32 QuestLevel = 0;
    int32 QuestFlags[3] = {};
    bool Repeatable = false;
    bool Important = false;
    std::string Title;

    void Write(ByteBuffer& data) const
    {
        data << QuestId;
        data << int32(0);  // content tuning
        data << QuestType << QuestLevel;
        data << int32(0);  // max scaling level
        data << int32(0);  // unused
        data << QuestFlags[0] << QuestFlags[1] << QuestFlags[2];
        data.WriteBit(Repeatable);
        data.WriteBit(false);  // reset by scheduler
        data.WriteBit(Important);
        data.WriteBit(false);  // meta
        data.WriteBits(uint32(Title.length()), 9);
        data.FlushBits();
        data.WriteString(Title);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 9 * sizeof(int32) + 2)
            return false;
        int32 contentTuning, maxScaling, unused;
        data >> QuestId >> contentTuning >> QuestType >> QuestLevel >> maxScaling >> unused;
        data >> QuestFlags[0] >> QuestFlags[1] >> QuestFlags[2];
        Repeatable = data.ReadBit();
        bool const resetByScheduler = data.ReadBit();
        Important = data.ReadBit();
        bool const meta = data.ReadBit();
        uint32 const length = data.ReadBits(9);
        data.ResetBitPos();
        if (contentTuning || maxScaling || unused || resetByScheduler || meta)
            return false;
        if (data.size() - data.rpos() < length)
            return false;
        Title = data.ReadString(length);
        return true;
    }
};

// The gossip window: what the NPC says (a text id the client queries), its options and its quests.
struct GossipMessage
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GOSSIP_MESSAGE;

    ClassicGuid GossipGuid;
    int32 GossipId = 0;
    std::vector<GossipOptionData> Options;
    std::vector<GossipQuestEntry> Quests;
    std::optional<int32> TextId;

    void Write(ByteBuffer& data) const
    {
        GossipGuid.Write(data);
        data << GossipId;
        data << int32(0);  // lfg dungeons
        data << int32(0);  // friendship faction
        data << uint32(Options.size());
        data << uint32(Quests.size());
        data.WriteBit(TextId.has_value());
        data.WriteBit(false);  // broadcast text
        data.FlushBits();
        for (GossipOptionData const& option : Options)
            option.Write(data);
        if (TextId)
            data << *TextId;
        for (GossipQuestEntry const& quest : Quests)
            quest.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!GossipGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(int32) + 2 * sizeof(uint32) + 1)
            return false;
        int32 lfg, friendship;
        uint32 optionCount, questCount;
        data >> GossipId >> lfg >> friendship >> optionCount >> questCount;
        bool const hasText = data.ReadBit();
        bool const hasBroadcast = data.ReadBit();
        data.ResetBitPos();
        if (lfg || friendship || hasBroadcast || optionCount > CLASSIC_MAX_GOSSIP_OPTIONS || questCount > CLASSIC_MAX_GOSSIP_QUESTS)
            return false;

        Options.assign(optionCount, GossipOptionData());
        for (GossipOptionData& option : Options)
            if (!option.Read(data))
                return false;

        TextId.reset();
        if (hasText)
        {
            if (data.size() - data.rpos() < sizeof(int32))
                return false;
            int32 text;
            data >> text;
            TextId = text;
        }

        Quests.assign(questCount, GossipQuestEntry());
        for (GossipQuestEntry& quest : Quests)
            if (!quest.Read(data))
                return false;
        return true;
    }
};

// The gossip window closes.
struct GossipComplete
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GOSSIP_COMPLETE;

    bool SuppressSound = false;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(SuppressSound);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        SuppressSound = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// A point of interest on the map, shown when a gossip option points at one.
struct GossipPoi
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GOSSIP_POI;

    int32 Id = 0;
    uint32 Flags = 0;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    int32 Icon = 0;
    int32 Importance = 0;
    int32 WmoGroupId = 0;
    std::string Name;

    void Write(ByteBuffer& data) const
    {
        data << Id << Flags << X << Y << Z << Icon << Importance << WmoGroupId;
        data.WriteBits(uint32(Name.length()), 6);
        data.FlushBits();
        data.WriteString(Name);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 8 * sizeof(int32) + 1)
            return false;
        data >> Id >> Flags >> X >> Y >> Z >> Icon >> Importance >> WmoGroupId;
        uint32 const length = data.ReadBits(6);
        data.ResetBitPos();
        if (data.size() - data.rpos() < length)
            return false;
        Name = data.ReadString(length);
        return true;
    }
};

// One spell a trainer offers.
struct TrainerListSpell
{
    uint32 SpellId = 0;
    uint32 MoneyCost = 0;
    uint32 ReqSkillLine = 0;
    uint32 ReqSkillRank = 0;
    uint32 ReqAbility[3] = {};
    uint8 Usable = 0;   // 0 known, 1 available, 2 unavailable
    uint8 ReqLevel = 0;

    void Write(ByteBuffer& data) const
    {
        data << SpellId << MoneyCost << ReqSkillLine << ReqSkillRank;
        data << ReqAbility[0] << ReqAbility[1] << ReqAbility[2];
        data << Usable << ReqLevel;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 7 * sizeof(uint32) + 2)
            return false;
        data >> SpellId >> MoneyCost >> ReqSkillLine >> ReqSkillRank;
        data >> ReqAbility[0] >> ReqAbility[1] >> ReqAbility[2];
        data >> Usable >> ReqLevel;
        return true;
    }
};

// Opens the trainer window.
struct TrainerList
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_TRAINER_LIST;

    ClassicGuid Guid;

    void Write(ByteBuffer& data) const { Guid.Write(data); }
    bool Read(ByteBuffer& data) { return Guid.Read(data); }
};

// A trainer's spell list and greeting.
struct TrainerSpellList
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_TRAINER_LIST;

    ClassicGuid TrainerGuid;
    int32 TrainerType = 0;
    uint32 TrainerId = 0;   // HARDCODE: no id distinct from the trainer's own guid/entry
    std::vector<TrainerListSpell> Spells;
    std::string Greeting;

    void Write(ByteBuffer& data) const
    {
        TrainerGuid.Write(data);
        data << TrainerType << TrainerId << int32(Spells.size());
        for (TrainerListSpell const& spell : Spells)
            spell.Write(data);
        data.WriteBits(uint32(Greeting.length()), 11);
        data.FlushBits();
        data.WriteString(Greeting);
    }

    bool Read(ByteBuffer& data)
    {
        if (!TrainerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(int32) + sizeof(uint32))
            return false;
        int32 count;
        data >> TrainerType >> TrainerId >> count;
        if (count < 0 || uint32(count) > (data.size() - data.rpos()) / 30)
            return false;
        Spells.assign(uint32(count), TrainerListSpell());
        for (TrainerListSpell& spell : Spells)
            if (!spell.Read(data))
                return false;
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const length = data.ReadBits(11);
        data.ResetBitPos();
        if (data.size() - data.rpos() < length)
            return false;
        Greeting = data.ReadString(length);
        return true;
    }
};

// Buys a spell from a trainer.
struct TrainerBuySpell
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_TRAINER_BUY_SPELL;

    ClassicGuid Guid;
    uint32 TrainerId = 0;
    uint32 SpellId = 0;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << TrainerId << SpellId;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32))
            return false;
        data >> TrainerId >> SpellId;
        return true;
    }
};

// A trainer purchase was refused. Purchases have no success ack: the client infers it from the
// resulting spellbook and gold updates.
struct TrainerBuyFailed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_TRAINER_BUY_FAILED;

    ClassicGuid TrainerGuid;
    uint32 ServiceId = 0;
    uint32 ErrorCode = 0;

    void Write(ByteBuffer& data) const
    {
        TrainerGuid.Write(data);
        data << ServiceId << ErrorCode;
    }

    bool Read(ByteBuffer& data)
    {
        if (!TrainerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32))
            return false;
        data >> ServiceId >> ErrorCode;
        return true;
    }
};

// Casts the player's hearthstone bind at an innkeeper.
struct BinderActivate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_BINDER_ACTIVATE;

    ClassicGuid NpcGuid;

    void Write(ByteBuffer& data) const { NpcGuid.Write(data); }
    bool Read(ByteBuffer& data) { return NpcGuid.Read(data); }
};

// Opens the personal bank at a banker.
struct BankerActivate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_BANKER_ACTIVATE;

    ClassicGuid Guid;
    int32 InteractionType = 0;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << InteractionType;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> InteractionType;
        return true;
    }
};

// Opens the guild tabard designer.
struct TabardVendorActivate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_TABARD_VENDOR_ACTIVATE;

    ClassicGuid Guid;
    int32 Type = 0;   // 0 guild, 1 personal -- this engine only has guild tabards

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << Type;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> Type;
        return true;
    }
};

// Asks a spirit healer to resurrect the player where they died.
struct SpiritHealerActivate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SPIRIT_HEALER_ACTIVATE;

    ClassicGuid Guid;

    void Write(ByteBuffer& data) const { Guid.Write(data); }
    bool Read(ByteBuffer& data) { return Guid.Read(data); }
};

// The client tore down an NPC interaction window on its own (Escape, walking away), without the
// server having closed it first.
struct CloseInteraction
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CLOSE_INTERACTION;

    ClassicGuid Guid;

    void Write(ByteBuffer& data) const { Guid.Write(data); }
    bool Read(ByteBuffer& data) { return Guid.Read(data); }
};

// A subset of the client's PlayerInteractionType enum -- which NPC-interaction UI
// NpcInteractionOpenResult opens. Numeric values match the real client enum.
enum class PlayerInteractionType : int32
{
    None              = 0,
    Gossip            = 3,
    Trainer           = 7,
    Banker            = 8,
    GuildTabardVendor = 14,
    SpiritHealer      = 18,
    AreaSpiritHealer  = 19,
    Binder            = 20,
    StableMaster      = 22,
};

// Replaces the old separate SMSG_SHOW_BANK / SMSG_BINDER_CONFIRM /
// SMSG_PLAYER_TABARD_VENDOR_ACTIVATE opcodes: every NPC-interaction window now opens through this
// one packet, tagged by type.
struct NpcInteractionOpenResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_NPC_INTERACTION_OPEN_RESULT;

    ClassicGuid NpcGuid;
    PlayerInteractionType InteractionType = PlayerInteractionType::None;
    bool Success = true;

    void Write(ByteBuffer& data) const
    {
        NpcGuid.Write(data);
        data << int32(InteractionType);
        data.WriteBit(Success);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!NpcGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32) + 1)
            return false;
        int32 type;
        data >> type;
        InteractionType = PlayerInteractionType(type);
        Success = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

#endif
