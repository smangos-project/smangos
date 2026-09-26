#include "Objects/MovementFlags.h"
#include "Objects/MovementInfo.h"
#include "Server/Packets/MovementMessages.h"
#include <catch_amalgamated.hpp>

namespace
{
struct FlagPair
{
    uint32 legacy;
    uint32 modern;
    char const* name;
};

// Mirrors Objects/MovementFlags.cpp's own table; kept separate so the test would fail if the two
// drift apart rather than silently agreeing with itself.
FlagPair const knownPairs[] =
{
    { MOVEFLAG_FORWARD,          CLASSIC_MOVEFLAG_FORWARD,          "FORWARD" },
    { MOVEFLAG_BACKWARD,         CLASSIC_MOVEFLAG_BACKWARD,         "BACKWARD" },
    { MOVEFLAG_STRAFE_LEFT,      CLASSIC_MOVEFLAG_STRAFE_LEFT,      "STRAFE_LEFT" },
    { MOVEFLAG_STRAFE_RIGHT,     CLASSIC_MOVEFLAG_STRAFE_RIGHT,     "STRAFE_RIGHT" },
    { MOVEFLAG_TURN_LEFT,        CLASSIC_MOVEFLAG_TURN_LEFT,        "TURN_LEFT" },
    { MOVEFLAG_TURN_RIGHT,       CLASSIC_MOVEFLAG_TURN_RIGHT,       "TURN_RIGHT" },
    { MOVEFLAG_PITCH_UP,         CLASSIC_MOVEFLAG_PITCH_UP,         "PITCH_UP" },
    { MOVEFLAG_PITCH_DOWN,       CLASSIC_MOVEFLAG_PITCH_DOWN,       "PITCH_DOWN" },
    { MOVEFLAG_WALK_MODE,        CLASSIC_MOVEFLAG_WALKING,          "WALK_MODE" },
    { MOVEFLAG_LEVITATING,       CLASSIC_MOVEFLAG_DISABLE_GRAVITY,  "LEVITATING" },
    { MOVEFLAG_ROOT,             CLASSIC_MOVEFLAG_ROOT,             "ROOT" },
    { MOVEFLAG_JUMPING,          CLASSIC_MOVEFLAG_FALLING,          "JUMPING" },
    { MOVEFLAG_FALLINGFAR,       CLASSIC_MOVEFLAG_FALLING_FAR,      "FALLINGFAR" },
    { MOVEFLAG_SWIMMING,         CLASSIC_MOVEFLAG_SWIMMING,         "SWIMMING" },
    { MOVEFLAG_FLYING,           CLASSIC_MOVEFLAG_FLYING,           "FLYING" },
    { MOVEFLAG_SPLINE_ELEVATION, CLASSIC_MOVEFLAG_SPLINE_ELEVATION, "SPLINE_ELEVATION" },
    { MOVEFLAG_WATERWALKING,     CLASSIC_MOVEFLAG_WATERWALKING,     "WATERWALKING" },
    { MOVEFLAG_SAFE_FALL,        CLASSIC_MOVEFLAG_FALLING_SLOW,     "SAFE_FALL" },
    { MOVEFLAG_HOVER,            CLASSIC_MOVEFLAG_HOVER,            "HOVER" },
};
}

TEST_CASE("MovementFlags: every 1.12 flag maps to its modern bit and back", "[movement]")
{
    for (FlagPair const& pair : knownPairs)
    {
        INFO(pair.name);
        CHECK(ToModernMovementFlags(pair.legacy) == pair.modern);
        CHECK(FromModernMovementFlags(pair.modern) == pair.legacy);
    }
}

TEST_CASE("MovementFlags: several flags combine without bleeding into each other", "[movement]")
{
    uint32 const legacy = MOVEFLAG_ROOT | MOVEFLAG_SWIMMING | MOVEFLAG_HOVER;
    uint32 const modern = CLASSIC_MOVEFLAG_ROOT | CLASSIC_MOVEFLAG_SWIMMING | CLASSIC_MOVEFLAG_HOVER;

    CHECK(ToModernMovementFlags(legacy) == modern);
    CHECK(FromModernMovementFlags(modern) == legacy);
}

TEST_CASE("MovementFlags: a 1.12 flag with no modern counterpart is dropped", "[movement]")
{
    // MOVEFLAG_ONTRANSPORT has no entry in the flag table: transport state rides in its own
    // wire field (ClassicMovementInfo::TransportInfo), not a movement flag bit.
    CHECK(ToModernMovementFlags(MOVEFLAG_ONTRANSPORT) == 0);
    CHECK(ToModernMovementFlags(MOVEFLAG_ROOT | MOVEFLAG_ONTRANSPORT) == CLASSIC_MOVEFLAG_ROOT);
}

TEST_CASE("MovementFlags: an unknown modern bit is dropped", "[movement]")
{
    // A bit no CLASSIC_MOVEFLAG_* constant names.
    uint32 const unknownModernBit = 0x00040000;
    CHECK(FromModernMovementFlags(unknownModernBit) == 0);
    CHECK(FromModernMovementFlags(CLASSIC_MOVEFLAG_HOVER | unknownModernBit) == MOVEFLAG_HOVER);
}
