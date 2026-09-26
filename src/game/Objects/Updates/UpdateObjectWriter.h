#ifndef UPDATES_UPDATEOBJECTWRITER_H
#define UPDATES_UPDATEOBJECTWRITER_H

#include "UpdateField.h"

class ByteBuffer;
class Object;
class Player;
class Unit;
class UpdateData;

// Serializes a create block for `object` into `data`, the way the modern client wants it:
// [update type][packed guid128][object type][movement frame][size]{[field flags][entity
// fragments][indirect-fragment byte][per-struct WriteCreate payloads]}. `target` is the receiving
// player -- when it is `object` itself, the block includes ActivePlayerData and the owner-only
// fields.
void BuildCreateUpdateBlockForPlayer(Object const& object, UpdateData& data, Player* target);

// Writes nothing when nothing visible to `target` changed.
void BuildValuesUpdateBlockForPlayer(Object const& object, UpdateData& data, Player* target);

// Sends every field visible under `flags`, ignoring the change masks.
void BuildValuesUpdateBlockForPlayerWithFlag(Object const& object, UpdateData& data, EnumFlag<UpdateFieldFlag> flags, Player* target);

// Resends only the dynamic flags, for when their per-viewer value changed.
void BuildDynamicFlagsUpdateBlockForPlayer(Object const& object, UpdateData& data, Player* target);

// Resends only the unit flags, for when their per-viewer value changed.
void BuildUnitFlagsUpdateBlockForPlayer(Unit const& unit, UpdateData& data, Player* target);

#endif
