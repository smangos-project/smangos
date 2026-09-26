#include "MovementFlags.h"
#include "MovementInfo.h"
#include "Server/Packets/MovementMessages.h"

namespace
{
struct FlagPair
{
    uint32 legacy;
    uint32 modern;
};

FlagPair const flagPairs[] =
{
    { MOVEFLAG_FORWARD,          CLASSIC_MOVEFLAG_FORWARD },
    { MOVEFLAG_BACKWARD,         CLASSIC_MOVEFLAG_BACKWARD },
    { MOVEFLAG_STRAFE_LEFT,      CLASSIC_MOVEFLAG_STRAFE_LEFT },
    { MOVEFLAG_STRAFE_RIGHT,     CLASSIC_MOVEFLAG_STRAFE_RIGHT },
    { MOVEFLAG_TURN_LEFT,        CLASSIC_MOVEFLAG_TURN_LEFT },
    { MOVEFLAG_TURN_RIGHT,       CLASSIC_MOVEFLAG_TURN_RIGHT },
    { MOVEFLAG_PITCH_UP,         CLASSIC_MOVEFLAG_PITCH_UP },
    { MOVEFLAG_PITCH_DOWN,       CLASSIC_MOVEFLAG_PITCH_DOWN },
    { MOVEFLAG_WALK_MODE,        CLASSIC_MOVEFLAG_WALKING },
    { MOVEFLAG_LEVITATING,       CLASSIC_MOVEFLAG_DISABLE_GRAVITY },
    { MOVEFLAG_ROOT,             CLASSIC_MOVEFLAG_ROOT },
    { MOVEFLAG_JUMPING,          CLASSIC_MOVEFLAG_FALLING },
    { MOVEFLAG_FALLINGFAR,       CLASSIC_MOVEFLAG_FALLING_FAR },
    { MOVEFLAG_SWIMMING,         CLASSIC_MOVEFLAG_SWIMMING },
    { MOVEFLAG_FLYING,           CLASSIC_MOVEFLAG_FLYING },
    { MOVEFLAG_SPLINE_ELEVATION, CLASSIC_MOVEFLAG_SPLINE_ELEVATION },
    { MOVEFLAG_WATERWALKING,     CLASSIC_MOVEFLAG_WATERWALKING },
    { MOVEFLAG_SAFE_FALL,        CLASSIC_MOVEFLAG_FALLING_SLOW },
    { MOVEFLAG_HOVER,            CLASSIC_MOVEFLAG_HOVER },
};
}

uint32 ToModernMovementFlags(uint32 moveFlags)
{
    uint32 flags = 0;
    for (FlagPair const& pair : flagPairs)
        if (moveFlags & pair.legacy)
            flags |= pair.modern;
    return flags;
}

uint32 FromModernMovementFlags(uint32 flags)
{
    uint32 moveFlags = 0;
    for (FlagPair const& pair : flagPairs)
        if (flags & pair.modern)
            moveFlags |= pair.legacy;
    return moveFlags;
}
