#ifndef MANGOS_MOVEMENTFLAGS_H
#define MANGOS_MOVEMENTFLAGS_H

#include "Common.h"

// 1.12 <-> modern movement flag translation. Header-light on purpose (no World.h), so it links
// into a unit test without pulling in the rest of the server.
uint32 ToModernMovementFlags(uint32 moveFlags);
uint32 FromModernMovementFlags(uint32 flags);

#endif
