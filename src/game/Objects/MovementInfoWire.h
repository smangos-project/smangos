#ifndef MANGOS_MOVEMENTINFOWIRE_H
#define MANGOS_MOVEMENTINFOWIRE_H

#include "MovementInfo.h"
#include "MovementFlags.h"
#include "Server/Packets/MovementMessages.h"

// The server keeps movement in the 1.12 MovementInfo (flags, jump data, transport offset); these
// convert to and from the modern wire block.

// False if a guid in the block has no server counterpart.
bool MovementInfoFromWire(ClassicMovementInfo const& wire, MovementInfo& info);
ClassicMovementInfo MovementInfoToWire(MovementInfo const& info, ObjectGuid const& mover);

#endif
