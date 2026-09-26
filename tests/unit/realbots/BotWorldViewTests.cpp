// BotWorldView reads SMSG_UPDATE_OBJECT itself, independently of the server's writer
// (bot-phase-4.md decision 5's risk note). These build blocks by hand from the wire layout
// (UpdateObjectWriter.cpp's BuildMovementUpdate), not from the server's own serializer.
#include "BotWorldView.h"
#include "Packets/Auras.h"
#include "Packets/ObjectUpdate.h"
#include "Packets/MonsterMove.h"
#include "Packets/MovementMessages.h"
#include "ByteBuffer.h"
#include <catch_amalgamated.hpp>

namespace
{
ClassicGuid TestGuid(uint64 low, uint64 high = 0x0400000000000000ULL)
{
    ClassicGuid guid;
    guid.Low = low;
    guid.High = high;
    return guid;
}

constexpr uint8 UPDATETYPE_VALUES = 0;
constexpr uint8 UPDATETYPE_CREATE_OBJECT = 2;

struct MovementFrameOptions
{
    bool MovementUpdate = true;
    bool Stationary = false;
    bool Rotation = false;
    bool ActivePlayer = false;
    // An update-flag bit the parser doesn't implement (mirrors, say, Vehicle): the parser must
    // stop on it rather than guess a payload shape.
    bool UnhandledBit = false;
    float X = 0.0f, Y = 0.0f, Z = 0.0f, O = 0.0f;
};

// Writes a movement frame matching ParseMovementFrame's (BotWorldView.cpp) expectations, itself
// matching UpdateObjectWriter.cpp's BuildMovementUpdate bit for bit.
void WriteMovementFrame(ByteBuffer& data, MovementFrameOptions const& opt)
{
    data.WriteBit(true);  // HasPositionFragment
    data.WriteBit(false); // NoBirthAnim
    data.WriteBit(false); // EnablePortals
    data.WriteBit(false); // PlayHoverAnim
    data.WriteBit(opt.MovementUpdate);
    data.WriteBit(false); // MovementTransport
    data.WriteBit(opt.Stationary);
    data.WriteBit(false); // CombatVictim
    data.WriteBit(false); // ServerTime
    data.WriteBit(opt.UnhandledBit); // Vehicle
    data.WriteBit(false); // AnimKit
    data.WriteBit(opt.Rotation);
    data.WriteBit(false); // AreaTrigger
    data.WriteBit(false); // GameObject
    data.WriteBit(false); // SmoothPhasing
    data.WriteBit(false); // ThisIsYou
    data.WriteBit(false); // SceneObject
    data.WriteBit(opt.ActivePlayer);
    data.WriteBit(false); // Conversation
    data.FlushBits();

    if (opt.MovementUpdate)
    {
        TestGuid(0x1).Write(data); // MoverGUID: the parser reads and discards it
        data << uint32(0) << uint32(0) << uint32(0) << uint32(0); // flags, extra flags x2, move time
        data << opt.X << opt.Y << opt.Z << opt.O;
        data << float(0.0f) << float(0.0f); // pitch, step-up start elevation
        data << uint32(0) << uint32(0);     // remove-forces count, move index

        data.WriteBit(false); // HasStandingOnGameObjectGUID
        data.WriteBit(false); // HasTransport
        data.WriteBit(false); // HasFall
        data.WriteBit(false); // HasSpline
        data.WriteBit(false); // HeightChangeFailed
        data.WriteBit(false); // RemoteTimeValid
        data.WriteBit(false); // HasInertia
        data.WriteBit(false); // HasAdvFlying
        data.FlushBits();

        for (int i = 0; i < 9; ++i)
            data << float(0.0f); // speeds
        data << uint32(0) << float(1.0f); // movement forces count, magnitude
        for (int i = 0; i < 17; ++i)
            data << float(0.0f); // adv-flying tuning

        data.WriteBit(false); // trailing HasSpline
        data.FlushBits();
    }

    data << uint32(0); // PauseTimes.size()

    if (opt.Stationary)
        data << opt.X << opt.Y << opt.Z << opt.O;

    if (opt.Rotation)
        data << uint64(0);

    if (opt.ActivePlayer)
    {
        data.WriteBit(false); // HasSceneInstanceIDs
        data.WriteBit(false); // HasRuneState
        data.WriteBit(false); // HasActionButtons
        data.FlushBits();
    }
}

void WriteCreateBlock(ByteBuffer& data, ClassicGuid const& guid, uint8 objectType, MovementFrameOptions const& opt)
{
    data << UPDATETYPE_CREATE_OBJECT;
    guid.Write(data);
    data << objectType;
    WriteMovementFrame(data, opt);
    data << uint32(0); // values payload size: not parsed here
}

void WriteValuesBlock(ByteBuffer& data, ClassicGuid const& guid, std::vector<uint8> const& payload = {})
{
    data << UPDATETYPE_VALUES;
    guid.Write(data);
    data << uint32(payload.size());
    if (!payload.empty())
        data.append(payload.data(), payload.size());
}

UpdateObject MakePacket(ByteBuffer const& blocks, uint32 blockCount)
{
    UpdateObject packet;
    packet.MapId = 0;
    packet.BlockCount = blockCount;
    packet.BlockData.assign(blocks.contents(), blocks.contents() + blocks.size());
    return packet;
}
}

TEST_CASE("BotWorldView: the bot's own create block gives the right guid, type and position", "[realbots]")
{
    BotWorldView view;

    ClassicGuid const self = TestGuid(0x42);
    MovementFrameOptions opt;
    opt.X = 100.0f;
    opt.Y = 200.0f;
    opt.Z = 50.0f;
    opt.O = 1.5f;
    opt.ActivePlayer = true;

    ByteBuffer blocks;
    WriteCreateBlock(blocks, self, BOT_TYPEID_ACTIVE_PLAYER, opt);

    view.OnUpdateObject(MakePacket(blocks, 1));

    std::optional<BotKnownObject> const known = view.Get(self);
    REQUIRE(known);
    CHECK(known->ObjectType == BOT_TYPEID_ACTIVE_PLAYER);
    CHECK(known->X == 100.0f);
    CHECK(known->Y == 200.0f);
    CHECK(known->Z == 50.0f);
    CHECK(known->O == 1.5f);
}

TEST_CASE("BotWorldView: a stationary object with rotation still parses, and the next block is still read", "[realbots]")
{
    BotWorldView view;

    ClassicGuid const door = TestGuid(0x10);
    MovementFrameOptions doorOpt;
    doorOpt.MovementUpdate = false; // a game object is never a unit
    doorOpt.Stationary = true;
    doorOpt.Rotation = true;
    doorOpt.X = 10.0f;
    doorOpt.Y = 20.0f;
    doorOpt.Z = 30.0f;
    doorOpt.O = 0.25f;

    ClassicGuid const creature = TestGuid(0x11);
    MovementFrameOptions creatureOpt;
    creatureOpt.X = 5.0f;
    creatureOpt.Y = 6.0f;
    creatureOpt.Z = 7.0f;

    ByteBuffer blocks;
    WriteCreateBlock(blocks, door, BOT_TYPEID_GAMEOBJECT, doorOpt);
    WriteCreateBlock(blocks, creature, BOT_TYPEID_UNIT, creatureOpt);

    view.OnUpdateObject(MakePacket(blocks, 2));

    std::optional<BotKnownObject> const doorKnown = view.Get(door);
    REQUIRE(doorKnown);
    CHECK(doorKnown->X == 10.0f);
    CHECK(doorKnown->Y == 20.0f);
    CHECK(doorKnown->Z == 30.0f);
    CHECK(doorKnown->O == 0.25f);

    std::optional<BotKnownObject> const creatureKnown = view.Get(creature);
    REQUIRE(creatureKnown);
    CHECK(creatureKnown->X == 5.0f);
    CHECK(creatureKnown->Y == 6.0f);
    CHECK(creatureKnown->Z == 7.0f);
}

TEST_CASE("BotWorldView: a bit the parser doesn't handle stops the parse and leaves the registry as it was", "[realbots]")
{
    BotWorldView view;

    ClassicGuid const known = TestGuid(0x20);
    MovementFrameOptions knownOpt;
    knownOpt.X = 1.0f;
    ByteBuffer firstBlocks;
    WriteCreateBlock(firstBlocks, known, BOT_TYPEID_UNIT, knownOpt);
    view.OnUpdateObject(MakePacket(firstBlocks, 1));
    REQUIRE(view.Get(known));

    // A second packet: a block this parser can't understand (a vehicle bit set), followed by a
    // block that would otherwise be perfectly readable.
    ClassicGuid const unparseable = TestGuid(0x21);
    MovementFrameOptions badOpt;
    badOpt.UnhandledBit = true;

    ClassicGuid const wouldFollow = TestGuid(0x22);
    MovementFrameOptions followOpt;

    ByteBuffer secondBlocks;
    WriteCreateBlock(secondBlocks, unparseable, BOT_TYPEID_UNIT, badOpt);
    WriteCreateBlock(secondBlocks, wouldFollow, BOT_TYPEID_UNIT, followOpt);
    view.OnUpdateObject(MakePacket(secondBlocks, 2));

    CHECK_FALSE(view.Get(unparseable));
    CHECK_FALSE(view.Get(wouldFollow));

    // The object from the earlier, successfully-parsed packet is untouched.
    std::optional<BotKnownObject> const stillKnown = view.Get(known);
    REQUIRE(stillKnown);
    CHECK(stillKnown->X == 1.0f);
}

TEST_CASE("BotWorldView: destroy and out-of-range lists remove objects", "[realbots]")
{
    BotWorldView view;

    ClassicGuid const destroyed = TestGuid(0x30);
    ClassicGuid const outOfRange = TestGuid(0x31);
    MovementFrameOptions opt;

    ByteBuffer blocks;
    WriteCreateBlock(blocks, destroyed, BOT_TYPEID_UNIT, opt);
    WriteCreateBlock(blocks, outOfRange, BOT_TYPEID_UNIT, opt);
    view.OnUpdateObject(MakePacket(blocks, 2));
    REQUIRE(view.Get(destroyed));
    REQUIRE(view.Get(outOfRange));

    UpdateObject removal;
    removal.MapId = 0;
    removal.BlockCount = 0;
    removal.DestroyGUIDs.push_back(destroyed);
    removal.OutOfRangeGUIDs.push_back(outOfRange);
    view.OnUpdateObject(removal);

    CHECK_FALSE(view.Get(destroyed));
    CHECK_FALSE(view.Get(outOfRange));
}

TEST_CASE("BotWorldView: a values block bumps ValuesUpdates", "[realbots]")
{
    BotWorldView view;

    ClassicGuid const target = TestGuid(0x40);
    MovementFrameOptions opt;
    ByteBuffer createBlocks;
    WriteCreateBlock(createBlocks, target, BOT_TYPEID_UNIT, opt);
    view.OnUpdateObject(MakePacket(createBlocks, 1));
    REQUIRE(view.Get(target)->ValuesUpdates == 0);

    ByteBuffer valuesBlocks;
    WriteValuesBlock(valuesBlocks, target, { 0x01, 0x02, 0x03 });
    view.OnUpdateObject(MakePacket(valuesBlocks, 1));
    CHECK(view.Get(target)->ValuesUpdates == 1);

    view.OnUpdateObject(MakePacket(valuesBlocks, 1));
    CHECK(view.Get(target)->ValuesUpdates == 2);
}

TEST_CASE("BotWorldView: MoveUpdate and MonsterMove update the position and bump their counters", "[realbots]")
{
    BotWorldView view;

    ClassicGuid const target = TestGuid(0x50);
    MovementFrameOptions opt;
    ByteBuffer createBlocks;
    WriteCreateBlock(createBlocks, target, BOT_TYPEID_UNIT, opt);
    view.OnUpdateObject(MakePacket(createBlocks, 1));

    ClassicMovementInfo status;
    status.Guid = target;
    status.X = 11.0f;
    status.Y = 22.0f;
    status.Z = 33.0f;
    status.O = 0.5f;
    view.OnMoveUpdate(status);

    std::optional<BotKnownObject> afterMove = view.Get(target);
    REQUIRE(afterMove);
    CHECK(afterMove->X == 11.0f);
    CHECK(afterMove->Y == 22.0f);
    CHECK(afterMove->Z == 33.0f);
    CHECK(afterMove->O == 0.5f);
    CHECK(afterMove->MoveUpdates == 1);

    MonsterMove move;
    move.Mover = target;
    move.Start = { 11.0f, 22.0f, 33.0f };
    move.Points = { { 40.0f, 41.0f, 42.0f }, { 99.0f, 98.0f, 97.0f } };
    view.OnMonsterMove(move);

    std::optional<BotKnownObject> afterMonsterMove = view.Get(target);
    REQUIRE(afterMonsterMove);
    CHECK(afterMonsterMove->X == 99.0f); // the path end, not the start
    CHECK(afterMonsterMove->Y == 98.0f);
    CHECK(afterMonsterMove->Z == 97.0f);
    CHECK(afterMonsterMove->MonsterMoves == 1);
}

TEST_CASE("BotWorldView: a MonsterMove with no points places the object at its start", "[realbots]")
{
    BotWorldView view;

    ClassicGuid const target = TestGuid(0x51);
    MovementFrameOptions opt;
    ByteBuffer createBlocks;
    WriteCreateBlock(createBlocks, target, BOT_TYPEID_UNIT, opt);
    view.OnUpdateObject(MakePacket(createBlocks, 1));

    MonsterMove move;
    move.Mover = target;
    move.Start = { 7.0f, 8.0f, 9.0f };
    view.OnMonsterMove(move);

    std::optional<BotKnownObject> known = view.Get(target);
    REQUIRE(known);
    CHECK(known->X == 7.0f);
    CHECK(known->Y == 8.0f);
    CHECK(known->Z == 9.0f);
    CHECK(known->MonsterMoves == 1);
}

namespace
{
AuraInfo MakeAura(uint8 slot, int32 spellId, uint8 applications = 1)
{
    AuraInfo info;
    info.Slot = slot;
    info.AuraData.emplace().SpellId = spellId;
    info.AuraData->Applications = applications;
    return info;
}
}

TEST_CASE("BotWorldView: a full aura list replaces the unit's auras and a delta changes one slot", "[realbots]")
{
    BotWorldView view;
    ClassicGuid const unit = TestGuid(0x50);
    ByteBuffer blocks;
    WriteCreateBlock(blocks, unit, BOT_TYPEID_UNIT, MovementFrameOptions{});
    view.OnUpdateObject(MakePacket(blocks, 1));

    AuraUpdate full;
    full.UpdateAll = true;
    full.UnitGuid = unit;
    full.Auras = { MakeAura(0, 6673), MakeAura(1, 21562) };
    view.OnAuraUpdate(full);
    REQUIRE(view.Get(unit)->Auras.size() == 2);

    AuraUpdate delta;
    delta.UnitGuid = unit;
    delta.Auras = { MakeAura(0, 6673, 3), AuraInfo{ 1, std::nullopt } };
    view.OnAuraUpdate(delta);

    std::vector<BotAura> const auras = view.Get(unit)->Auras;
    REQUIRE(auras.size() == 1);
    CHECK(auras[0].Slot == 0);
    CHECK(auras[0].SpellId == 6673);
    CHECK(auras[0].Applications == 3);

    full.Auras = { MakeAura(5, 133) };
    view.OnAuraUpdate(full);
    REQUIRE(view.Get(unit)->Auras.size() == 1);
    CHECK(view.Get(unit)->Auras[0].SpellId == 133);
}

TEST_CASE("BotWorldView: an aura update for an unknown unit is ignored", "[realbots]")
{
    BotWorldView view;
    AuraUpdate update;
    update.UpdateAll = true;
    update.UnitGuid = TestGuid(0x51);
    update.Auras = { MakeAura(0, 6673) };
    view.OnAuraUpdate(update);
    CHECK_FALSE(view.Get(update.UnitGuid));
}
