// Round trips for the modern game object anim and fishing wire.
#include "Packets/GameObjectMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("GameObjectCustomAnim round-trips a normal anim and a despawn anim", "[packets]")
{
    for (bool playAsDespawn : { false, true })
    {
        GameObjectCustomAnim packet;
        packet.ObjectGuid = { 0x4242, 0 };
        packet.CustomAnim = 1;
        packet.PlayAsDespawn = playAsDespawn;
        RoundTrip(packet, [](GameObjectCustomAnim const& a, GameObjectCustomAnim const& b)
        {
            CHECK(a.ObjectGuid == b.ObjectGuid);
            CHECK(a.CustomAnim == b.CustomAnim);
            CHECK(a.PlayAsDespawn == b.PlayAsDespawn);
        });
        RejectsEveryTruncation(packet);
    }
}

TEST_CASE("FishNotHooked round-trips its empty body", "[packets]")
{
    FishNotHooked packet;
    RoundTrip(packet, [](FishNotHooked const&, FishNotHooked const&) {});
}

TEST_CASE("FishEscaped round-trips its empty body", "[packets]")
{
    FishEscaped packet;
    RoundTrip(packet, [](FishEscaped const&, FishEscaped const&) {});
}
