// Round trips for the modern combat wire (target selection and melee).
#include "Packets/CombatMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("SetSelection round-trips", "[packets]")
{
    SetSelection packet;
    packet.Target = { 0x12345678, 0 };
    RoundTrip(packet, [](SetSelection const& a, SetSelection const& b)
    {
        CHECK(a.Target == b.Target);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AttackSwing round-trips", "[packets]")
{
    AttackSwing packet;
    packet.Victim = { 0x87654321, 0 };
    RoundTrip(packet, [](AttackSwing const& a, AttackSwing const& b)
    {
        CHECK(a.Victim == b.Victim);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AttackStop is an empty body", "[packets]")
{
    ByteBuffer data;
    AttackStop{}.Write(data);
    CHECK(data.empty());
    AttackStop in;
    CHECK(in.Read(data));
}

TEST_CASE("AttackSwingError round-trips", "[packets]")
{
    AttackSwingError packet;
    packet.Reason = CLASSIC_ATTACKSWINGERR_BAD_FACING;
    RoundTrip(packet, [](AttackSwingError const& a, AttackSwingError const& b)
    {
        CHECK(a.Reason == b.Reason);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AttackStart round-trips", "[packets]")
{
    AttackStart packet;
    packet.Attacker = { 11, 0 };
    packet.Victim = { 22, 0 };
    RoundTrip(packet, [](AttackStart const& a, AttackStart const& b)
    {
        CHECK(a.Attacker == b.Attacker);
        CHECK(a.Victim == b.Victim);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SAttackStop round-trips", "[packets]")
{
    SAttackStop packet;
    packet.Attacker = { 11, 0 };
    packet.Victim = { 22, 0 };
    packet.NowDead = true;
    RoundTrip(packet, [](SAttackStop const& a, SAttackStop const& b)
    {
        CHECK(a.Attacker == b.Attacker);
        CHECK(a.Victim == b.Victim);
        CHECK(a.NowDead == b.NowDead);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("StandStateChange round-trips", "[packets]")
{
    StandStateChange packet;
    packet.StandState = 1;
    RoundTrip(packet, [](StandStateChange const& a, StandStateChange const& b)
    {
        CHECK(a.StandState == b.StandState);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("StandStateChange reads the low byte of a four-byte body", "[packets]")
{
    ByteBuffer data;
    data << uint32(8);
    StandStateChange in;
    REQUIRE(in.Read(data));
    CHECK(in.StandState == 8);
}

TEST_CASE("StandStateUpdate round-trips", "[packets]")
{
    StandStateUpdate packet;
    packet.AnimKitId = 0;
    packet.StandState = 4;
    RoundTrip(packet, [](StandStateUpdate const& a, StandStateUpdate const& b)
    {
        CHECK(a.AnimKitId == b.AnimKitId);
        CHECK(a.StandState == b.StandState);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("MeleeAttackingStateUpdate round-trips with no optional hit-info fields", "[packets]")
{
    MeleeAttackingStateUpdate packet;
    packet.HitInfoFlags = 0;
    packet.Attacker = { 1, 0 };
    packet.Victim = { 2, 0 };
    packet.TotalDamage = 123;
    packet.OriginalDamage = 150;
    packet.OverDamage = -1;
    packet.SubDamage.push_back({ 1, 123, 0, 0 });
    packet.VictimState = 1;
    packet.AttackerState = 1000;
    packet.MeleeSpellId = 0;
    packet.ContentTuningType = 2;
    packet.ContentTuningTargetLevel = 30;

    RoundTrip(packet, [](MeleeAttackingStateUpdate const& a, MeleeAttackingStateUpdate const& b)
    {
        CHECK(a.HitInfoFlags == b.HitInfoFlags);
        CHECK(a.Attacker == b.Attacker);
        CHECK(a.Victim == b.Victim);
        CHECK(a.TotalDamage == b.TotalDamage);
        CHECK(a.OriginalDamage == b.OriginalDamage);
        CHECK(a.OverDamage == b.OverDamage);
        REQUIRE(a.SubDamage.size() == b.SubDamage.size());
        CHECK(a.SubDamage[0].SchoolMask == b.SubDamage[0].SchoolMask);
        CHECK(a.SubDamage[0].Damage == b.SubDamage[0].Damage);
        CHECK(a.VictimState == b.VictimState);
        CHECK(a.AttackerState == b.AttackerState);
        CHECK(a.MeleeSpellId == b.MeleeSpellId);
        CHECK(a.ContentTuningType == b.ContentTuningType);
        CHECK(a.ContentTuningTargetLevel == b.ContentTuningTargetLevel);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("MeleeAttackingStateUpdate round-trips with absorb, resist and block set", "[packets]")
{
    MeleeAttackingStateUpdate packet;
    packet.HitInfoFlags = CLASSIC_HITINFO_ABSORB | CLASSIC_HITINFO_RESIST | CLASSIC_HITINFO_BLOCK;
    packet.Attacker = { 1, 0 };
    packet.Victim = { 2, 0 };
    packet.TotalDamage = 50;
    packet.OriginalDamage = 200;
    packet.OverDamage = 30;
    packet.SubDamage.push_back({ 1, 50, 40, 60 });
    packet.SubDamage.push_back({ 4, 10, 5, 0 });
    packet.VictimState = 5;
    packet.AttackerState = 0;
    packet.MeleeSpellId = 1680;
    packet.BlockedAmount = 15;
    packet.ContentTuningType = 1;
    packet.ContentTuningTargetLevel = 60;

    RoundTrip(packet, [](MeleeAttackingStateUpdate const& a, MeleeAttackingStateUpdate const& b)
    {
        CHECK(a.HitInfoFlags == b.HitInfoFlags);
        REQUIRE(a.SubDamage.size() == b.SubDamage.size());
        for (size_t i = 0; i < a.SubDamage.size(); ++i)
        {
            CHECK(a.SubDamage[i].SchoolMask == b.SubDamage[i].SchoolMask);
            CHECK(a.SubDamage[i].Damage == b.SubDamage[i].Damage);
            CHECK(a.SubDamage[i].Absorb == b.SubDamage[i].Absorb);
            CHECK(a.SubDamage[i].Resist == b.SubDamage[i].Resist);
        }
        CHECK(a.BlockedAmount == b.BlockedAmount);
        CHECK(a.MeleeSpellId == b.MeleeSpellId);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AIReactionUpdate round-trips", "[packets]")
{
    AIReactionUpdate packet;
    packet.Unit = { 0x1122, 0 };
    packet.Reaction = 2;
    RoundTrip(packet, [](AIReactionUpdate const& a, AIReactionUpdate const& b)
    {
        CHECK(a.Unit == b.Unit);
        CHECK(a.Reaction == b.Reaction);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("CancelAutoRepeat round-trips", "[packets]")
{
    CancelAutoRepeat packet;
    packet.Target = { 0x3344, 0 };
    RoundTrip(packet, [](CancelAutoRepeat const& a, CancelAutoRepeat const& b)
    {
        CHECK(a.Target == b.Target);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SetSheathed round-trips both values of Animate", "[packets]")
{
    for (bool animate : { false, true })
    {
        SetSheathed packet;
        packet.CurrentSheathState = 2;
        packet.Animate = animate;
        RoundTrip(packet, [](SetSheathed const& a, SetSheathed const& b)
        {
            CHECK(a.CurrentSheathState == b.CurrentSheathState);
            CHECK(a.Animate == b.Animate);
        });
        RejectsEveryTruncation(packet);
    }
}

TEST_CASE("FeignDeathResisted round-trips its empty body", "[packets]")
{
    FeignDeathResisted packet;
    RoundTrip(packet, [](FeignDeathResisted const&, FeignDeathResisted const&) {});
}

TEST_CASE("PvpCredit round-trips", "[packets]")
{
    PvpCredit packet;
    packet.OriginalHonor = 42;
    packet.Honor = 42;
    packet.Victim = { 0x5566, 0 };
    packet.VictimRank = 5;
    RoundTrip(packet, [](PvpCredit const& a, PvpCredit const& b)
    {
        CHECK(a.OriginalHonor == b.OriginalHonor);
        CHECK(a.Honor == b.Honor);
        CHECK(a.Victim == b.Victim);
        CHECK(a.VictimRank == b.VictimRank);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ThreatUpdate round-trips", "[packets]")
{
    ThreatUpdate packet;
    packet.UnitGuid = { 0x1111, 0 };
    packet.ThreatList = { { { 0x2222, 0 }, 100 }, { { 0x3333, 0 }, 250 } };
    RoundTrip(packet, [](ThreatUpdate const& a, ThreatUpdate const& b)
    {
        CHECK(a.UnitGuid == b.UnitGuid);
        REQUIRE(a.ThreatList.size() == b.ThreatList.size());
        for (size_t i = 0; i < a.ThreatList.size(); ++i)
        {
            CHECK(a.ThreatList[i].UnitGuid == b.ThreatList[i].UnitGuid);
            CHECK(a.ThreatList[i].Threat == b.ThreatList[i].Threat);
        }
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ThreatUpdate rejects an oversized list", "[packets]")
{
    ByteBuffer data;
    data << ClassicGuid{ 0x1111, 0 };
    data << int32(CLASSIC_MAX_THREAT_LIST_SIZE + 1);

    ThreatUpdate in;
    CHECK_FALSE(in.Read(data));
}

TEST_CASE("HighestThreatUpdate round-trips", "[packets]")
{
    HighestThreatUpdate packet;
    packet.UnitGuid = { 0x1111, 0 };
    packet.HighestThreatGuid = { 0x2222, 0 };
    packet.ThreatList = { { { 0x2222, 0 }, 500 } };
    RoundTrip(packet, [](HighestThreatUpdate const& a, HighestThreatUpdate const& b)
    {
        CHECK(a.UnitGuid == b.UnitGuid);
        CHECK(a.HighestThreatGuid == b.HighestThreatGuid);
        REQUIRE(a.ThreatList.size() == b.ThreatList.size());
        CHECK(a.ThreatList[0].UnitGuid == b.ThreatList[0].UnitGuid);
        CHECK(a.ThreatList[0].Threat == b.ThreatList[0].Threat);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ThreatRemove round-trips", "[packets]")
{
    ThreatRemove packet;
    packet.UnitGuid = { 0x1111, 0 };
    packet.AboutGuid = { 0x2222, 0 };
    RoundTrip(packet, [](ThreatRemove const& a, ThreatRemove const& b)
    {
        CHECK(a.UnitGuid == b.UnitGuid);
        CHECK(a.AboutGuid == b.AboutGuid);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ThreatClear round-trips", "[packets]")
{
    ThreatClear packet;
    packet.UnitGuid = { 0x1111, 0 };
    RoundTrip(packet, [](ThreatClear const& a, ThreatClear const& b)
    {
        CHECK(a.UnitGuid == b.UnitGuid);
    });
    RejectsEveryTruncation(packet);
}
